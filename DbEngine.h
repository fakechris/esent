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
#include "DbDatabase.h"
#include "DbSessionManager.h"
#include "DbCache.h"

class DbEngine
{
public:
	DbEngine(WTDB_ENGINE_PARAMS *_pParams);
	virtual ~DbEngine(void);

	// Try to initialize the database engine. This includes, if necessary, building the database
	// tables for the first time. If this function fails, transactions ensure that the databsae
	// remains in a consistent state i.e. there will be no half-built tables, or missing tables
	JET_ERR try_init();

	// Try to free the database engine. 
	JET_ERR try_free();

	// Create a new session for accessing the default database
	JET_ERR create_session(DbSession **session);

	// = JetGetSystemParameter
	JET_ERR get_system_parameter(DbSession *session, unsigned long paramid, JET_API_PTR* plParam, JET_PSTR szParam, unsigned long cbMax);

	DbDatabase *get_default_database(void) { return &_defaultDatabase; }

	DbSessionManager *get_session_manager(void) { return &_sessionMngr; }

	DbCache *get_cache(void) { return &_cache; }

private:
	WTDB_ENGINE_PARAMS _params;
	JET_INSTANCE _instance;
	DbDatabase _defaultDatabase;
	DbSessionManager _sessionMngr;
	DbCache _cache;
};
