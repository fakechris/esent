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
#include "DbTable.h"
#include "DbCursor.h"
#include "DbTransaction.h"
#include "WykiIdentityServer.h"

DbTable::DbTable(const char *tableName)

: _schema(NULL), _tableName(tableName), _idServer(NULL)
{
}

DbTable::~DbTable(void)
{
	delete _schema;
}

JET_ERR
DbTable::open_cursor(DbCursor *cursor, JET_GRBIT grbit)
{
	JET_ERR rc;

	ERR_RET( cursor->open(this, grbit) )

	return JET_errSuccess;
}

JET_ERR
DbTable::load_schema(DbSession *session)
{
	JET_ERR rc;

	DbCursor cursor(session);
	ERR_RET( cursor.open(this, JET_bitTableReadOnly) )

	auto_ptr<DbTableSchema> jts(new DbTableSchema());

	ERR_RET( jts->load_schema_for_table(session, this, cursor.get_tableid()) )

	_schema = jts.release();

	return JET_errSuccess;
}


WykiIdentityServer *
DbTable::get_id_server() // ? what's this chris
{
	if (_idServer)
	{
		return _idServer;
	}

	//LockSection lock(this);

	if (_idServer)
	{
		return _idServer;
	}

	// Dominic Williams: unless you have good reason, don't mess with the following parameter values
	// because they have been extensively tested. Thanks. 450, 3. Tell me if you want to change it!
	_idServer = new WykiIdentityServer(450, 3);

	return _idServer;
}

JET_ERR
DbTable::build_table(DbSession *session)
{
	JET_ERR rc;

	DbTransaction transaction(session, true, JET_bitNil);

	DbCursor cursor(session);
	ERR_RET( JetCreateTable(session->get_sesid(), session->get_dbid(), _tableName.c_str(), get_initial_pages(), get_table_density(), cursor.get_tableid_ref()) )

	ERR_RET( do_build_columns(&cursor) )

	ERR_RET( do_build_indexes(&cursor) )

	ERR_RET( transaction.commit() )

	return JET_errSuccess;
}
