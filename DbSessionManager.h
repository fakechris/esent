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

#include "DbSession.h"
class DbEngine;

// An ESE instance will only allow a specific number of sessions to be created concurrently. The
// default at the time of writing is 16, but it can be set to a number of values. This class 
// ensures that concurrent threads do not attempt to create more than the maximum number of database
// sessions, causing them to wait until a session is available.
//
class DbSessionManager
{
public:
	DbSessionManager(DbEngine *engine, long maxSessions)
		: _engine(engine)
	{
		_sema = CreateSemaphore(NULL, maxSessions, maxSessions, NULL);
		if (_sema == NULL)
		{
			throw wyki_win32_error(L"Session manager object could not create synchronization semaphore");
		}
	}

	virtual ~DbSessionManager(void)
	{
		CloseHandle(_sema);
	}

	WTDB_ERR acquire_session(DbSession **session, bool pooled=true, unsigned long maxWait=INFINITE);
	// session
	// The acquired session
	//
	// nonPooled
	// Create a session outside of the counted pool. For example when a cache manager requires its own session (and thus transaction scope),
	// but when called might be blocking other sessions, needs a non-pooled session, or deadlock could result.
	//
	// maxWait
	// The maximum time to wait before aquiring a session
	//
	// TO DO: check carefully the implications in relation to JET's max sessions system parameter

	WTDB_ERR release_session(DbSession *session)
	{
		JET_ERR rc;

        ReleaseSemaphore(_sema, 1, NULL);

		ERR_RET( session->try_free() )

		return JET_errSuccess;
	}

private:
	DbEngine *_engine;
	HANDLE _sema;
};

// Acquire a session object, and release it automatically on destruction
//
class AutoSession
{
public:
	AutoSession(DbSessionManager *sessMgr, bool pooled=true, unsigned long maxWait=INFINITE) : _sessMgr(sessMgr)
	{
		JET_ERR rc;
		rc = _sessMgr->acquire_session(&_session, pooled, maxWait);
		if (rc != JET_errSuccess)
		{
			ERR_LOG( rc )
			throw wyki_database_error(L"Failed to start new database session", rc);
		}
	}
	
	~AutoSession(void)
	{
retry_release:
		JET_ERR rc;
		int try_count = 0;
		rc = _sessMgr->release_session(_session);
		if (rc != JET_errSuccess)
		{
			ERR_LOG( rc )
			if (try_count++ > 500)
			{
				Sleep(5); // we'll loop for a max of 2.5 seconds
				terminate();
			}
			else
			{
				goto retry_release;
			}
		}
	}

	DbSession *get_session()
	{
		return _session;
	}

private:
	DbSessionManager *_sessMgr;
	DbSession *_session;
};