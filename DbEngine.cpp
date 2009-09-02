// Version 01.03.03.09
//
// This source file has been released by Centre Technologies Ltd. under The BSD License. 
//
// The BSD License 
// Copyright (c) 2008-2009, Centre Technologies Ltd.
// All rights reserved. 
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met: 
//
// •Redistributions of source code must retain the above copyright notice, this list of conditions
//  and the following disclaimer.
//
// •Redistributions in binary form must reproduce the above copyright notice, this list of conditions
//  and the following disclaimer in the documentation and/or other materials provided with the distribution.
//
// •Neither the name of Centre Technologies Ltd, nor the names of its contributors may be used to
//  endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include "StdAfx.h"
#include "DbEngine.h"
#include "DbErrors.h"
#include "DbCursor.h"
/*
#include "TableRootPosts.h"
#include "TableReplyPosts.h"
#include "TableContext.h"
#include "TableAuthorInfo.h"
#include "TableCxtGroupAuthorInfo.h"
#include "TableOperationLog.h"
#include "TableAuthorConfigs.h"
#include "TableEventCounters.h"
*/
#include "cachetable.h"

DbEngine::DbEngine(WTDB_ENGINE_PARAMS *pParams)
	: _params(*pParams)
	, _defaultDatabase(_params.szDatabaseFilename)
	, _sessionMngr(this, _params.maxSessions)
{
	_defaultDatabase.add_table(new MyCacheTable(MyCacheTable::default_name()));
	/*
	// Root posts table (e.g. first thread post, or blog post)
	_defaultDatabase.add_table(new TableRootPosts(TableRootPosts::default_name()));

	// Reply posts table (e.g. reply on forum thread, or comment below blog post)
	_defaultDatabase.add_table(new TableReplyPosts(TableReplyPosts::default_name()));

	// Context info table e.g. is context forum or blog? Context title and description, etc
	_defaultDatabase.add_table(new TableContext(TableContext::default_name()));

	// Author information applying at global level (profile, stats etc)
	_defaultDatabase.add_table(new TableAuthorInfo(TableAuthorInfo::default_name()));

	// Author information applying at the context group level (profile, stats etc)
	_defaultDatabase.add_table(new TableCxtGroupAuthorInfo(TableCxtGroupAuthorInfo::default_name()));

	// Log of operations waiting to be processed by notification generator
	_defaultDatabase.add_table(new TableOperationLog(TableOperationLog::default_name()));

	// Rules used by notification generator to create notifications for authors based upon logged operation entries
	_defaultDatabase.add_table(new TableAuthorConfigs(TableAuthorConfigs::default_name()));

	// Notifications to be sent to authors
	_defaultDatabase.add_table(new TableEventCounters(TableEventCounters::default_name()));
	*/
}

DbEngine::~DbEngine(void)
{
	try_free();
}

JET_ERR DbEngine::try_free()
{	
	JET_ERR rc;

	// shutdown engine instance
	if (_instance != JET_instanceNil)
	{
		ERR_RET( JetTerm(_instance) )
		_instance = JET_instanceNil;
	}

	return JET_errSuccess;
}


JET_ERR DbEngine::try_init()
{
	JET_ERR rc;

	// create database engine 
	ERR_RET( JetCreateInstance(&_instance, "WykiThreadingDatabase") )

	// configure engine global scope options
	ERR_RET( JetSetSystemParameter(&_instance, JET_sesidNil, JET_paramMaxSessions, _params.maxSessions, NULL) )
	ERR_RET( JetSetSystemParameter(&_instance, JET_sesidNil, JET_paramLogFilePath, 0, _params.szEngineLogPath) )
	ERR_RET( JetSetSystemParameter(&_instance, JET_sesidNil, JET_paramSystemPath, 0, _params.szEngineLogPath) )

	// Initialize database engine instance
	ERR_RET( JetInit(&_instance) )

	// Start session to perform further operations
	DbSession session(this);
	ERR_RET( JetBeginSession(_instance, session.get_sesid_ref(), NULL, NULL) )

	// Try to create a new database instance...
	rc = JetCreateDatabase(session.get_sesid(), _params.szDatabaseFilename, NULL, session.get_dbid_ref(), JET_bitNil);
	
	if (rc == JET_errDatabaseDuplicate)
	{
		// Database already exists, so attach and open it
		ERR_RET( JetAttachDatabase(session.get_sesid(), _params.szDatabaseFilename, JET_bitNil) )
		ERR_RET( JetOpenDatabase(session.get_sesid(), _params.szDatabaseFilename, NULL, session.get_dbid_ref(), JET_bitNil) )

		// Try to open the posts table, to establish whether the database has been configured yet
		DbCursor cursor(&session);
		int rc = JetOpenTable(session.get_sesid(), session.get_dbid(), MyCacheTable::default_name(), NULL, 0, JET_bitTableReadOnly, cursor.get_tableid_ref());

		if (rc == JET_errSuccess)
		{
			// Yup, database looks like it is configured. Now initialize the table schemas for later use
			ERR_RET( _defaultDatabase.initialize_table_schemas(&session) )

			// Ready to go!
			return JET_errSuccess;
		}
		else if (rc == JET_errObjectNotFound)
		{
			// Ooops. the posts table even doesn't exist yet. This means that either (1) the
			// specified database files don't contain our tables, or the process of building our tables 
			// previously failed and the transaction was rolled back. Either way, fall through to the
			// next case and try to build the tables inside the database.
			// TO DO: decide how to log and draw notice to this condition properly!
			ERR_LOG( JET_errObjectNotFound )
			rc = JET_errSuccess;
		}
		else {
			// U oh panic, we don't know what is happening here. corruption?
			ERR_RET( rc )
		}
	}

	if (rc == JET_errSuccess)
	{
		// The database was newly created => we now need to build the database tables for first time.
		// Perform the build inside a transaction so all or nothing
		ERR_RET( JetBeginTransaction2(session.get_sesid(), JET_bitNil) )
		
		rc = _defaultDatabase.build_all_tables(&session);

		if (rc != JET_errSuccess)
		{
			JetRollback(session.get_sesid(), JET_bitRollbackAll);
			ERR_RET( rc )
		}

		ERR_RET( JetCommitTransaction(session.get_sesid(), JET_bitNil) )

		// Load table schemas for later use
		ERR_RET( _defaultDatabase.initialize_table_schemas(&session) )
	}
	else
	{
		ERR_RET( rc )
	}

	// Done, and ready to go!
	return JET_errSuccess;
}

JET_ERR
DbEngine::create_session(DbSession **session)
{
	JET_ERR rc;

	auto_ptr<DbSession> tmp(new DbSession(this));

	ERR_RET( tmp->try_init(_instance, _params.szDatabaseFilename) )

	// TO DO: at the moment if we fail above the session will be destroyed, and its database objects freed,
	// when this function returns. We should investigate whether it might ever be necessary to repeatedly
	// attempt to free the database objects inside the sessio using session->try_free() as this function
	// returns as soon as it hits an error, and it is only called once within the destructor of the session

	*session = tmp.release();

	return JET_errSuccess;
}

JET_ERR
DbEngine::get_system_parameter(DbSession *session, unsigned long paramid, JET_API_PTR* plParam, JET_PSTR szParam, unsigned long cbMax)
{
	return JetGetSystemParameter(_instance, session->get_sesid(), paramid, plParam, szParam, cbMax); 
}

