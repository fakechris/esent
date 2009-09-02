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

#pragma once
#include "WykiDatabaseDriver.h"
#include "DbErrors.h"

class DbEngine;

class DbSession
{
public:
	DbSession(DbEngine *engine, JET_SESID sesid, JET_DBID dbid) : _engine(engine), _sesid(sesid), _dbid(dbid)
	{
		;
	}

	DbSession(DbEngine *engine) : _engine(engine), _sesid(JET_sesidNil), _dbid(JET_dbidNil)
	{
		;
	}

	virtual ~DbSession(void)
	{
		JET_ERR rc;
		ERR_LOG( try_free() )
	}

	JET_SESID *get_sesid_ref(void)
	{
		return &_sesid;
	}
	
	JET_DBID *get_dbid_ref(void)
	{
		return &_dbid;
	}

	void init(JET_SESID sesid, JET_DBID dbid)
	{
		_sesid = sesid;
		_dbid = dbid;
	}

	WTDB_ERR try_init(JET_INSTANCE instance, const char *databasePath)
	{
		_databasePath = databasePath;

		JET_ERR rc;

		if (_sesid == JET_sesidNil)
		{
			ERR_RET( JetBeginSession(instance, &_sesid, NULL, NULL) )
		}

		if (_dbid == JET_dbidNil)
		{
			ERR_RET( JetOpenDatabase(_sesid, _databasePath.c_str(), NULL, &_dbid, JET_bitNil) )
		}

		return JET_errSuccess;
	}

	WTDB_ERR try_free()
	{
		JET_ERR rc;

		if (_dbid != JET_dbidNil)
		{
			ERR_RET( JetCloseDatabase(_sesid, _dbid, JET_bitNil) )
			_dbid = JET_dbidNil;
		}

		if (_sesid != JET_sesidNil)
		{
			ERR_RET( JetEndSession(_sesid, JET_bitNil) )
			_sesid = JET_sesidNil;
		}

		return JET_errSuccess;
	}

	JET_SESID get_sesid() { return _sesid; }
	JET_DBID get_dbid() { return _dbid; }

	DbEngine *get_engine() { return _engine; }
	
private:
	DbEngine *_engine;
	string _databasePath;
	JET_SESID _sesid;
	JET_DBID _dbid;
};