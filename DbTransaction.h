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
class DbOperation;

class DbTransaction
{
public:
	DbTransaction(
		DbSession *session,
		bool start=false,
		JET_GRBIT beginGrbit=JET_bitTransactionReadOnly,
		JET_GRBIT rollbackGrbit=JET_bitNil,
		JET_GRBIT commitGrbit=JET_bitNil)
		: _session(session)
		, _finalized(false)
		, _beginGrbit(beginGrbit)
		, _rollbackGrbit(rollbackGrbit)
		, _commitGrbit(commitGrbit)
		, _lastRegisteredListener(NULL)
	{
		if (start)
		{
			JET_ERR rc = this->start();
			if (rc != JET_errSuccess)
			{
				ERR_LOG( rc )
				throw wyki_database_error(L"Failed to start new database transaction", rc);
			}
		}
	}

	DbTransaction(DbTransaction& copy);
	
	virtual ~DbTransaction(void);

	JET_ERR start(void)
	{
		JET_ERR rc;
		ERR_RET( JetBeginTransaction2(_session->get_sesid(), _beginGrbit) )
		return JET_errSuccess;
	}

	JET_ERR start(JET_GRBIT grbit)
	{
		JET_ERR rc;
		ERR_RET( JetBeginTransaction2(_session->get_sesid(), grbit) )
		return JET_errSuccess;
	}

	JET_ERR commit(void)
	{
		return commit(_commitGrbit);
	}

	JET_ERR commit(JET_GRBIT grbit)
	{
		JET_ERR rc;
		
		ERR_RET( JetCommitTransaction(_session->get_sesid(), grbit) )
		_finalized = true;

		ERR_RET( notify_commit_listeners() );

		return JET_errSuccess;
	}

	JET_ERR rollback(void)
	{
		return rollback(_rollbackGrbit);
	}

	JET_ERR rollback(JET_GRBIT grbit)
	{
		JET_ERR rc;
		ERR_RET( JetRollback(_session->get_sesid(), grbit) )
		_finalized = true;
		return JET_errSuccess;
	}

	void register_commit_listener(DbOperation *op)
	{
		_onCommitListeners.push_back(op);
		_lastRegisteredListener = op;
	}

	DbOperation *get_last_registered_listener()
	{
		return _lastRegisteredListener;
	}

	DbSession *get_session()
	{
		return _session;
	}

private:
	JET_ERR notify_commit_listeners();

	DbSession *_session;
	JET_GRBIT _beginGrbit;
	JET_GRBIT _commitGrbit;
	JET_GRBIT _rollbackGrbit;
	bool _finalized;
	vector<DbOperation *> _onCommitListeners;
	DbOperation *_lastRegisteredListener;
};
