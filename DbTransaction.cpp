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
#include "DbTransaction.h"
#include "DbOperation.h"

DbTransaction::DbTransaction(DbTransaction& copy)
{
	_session = copy._session;
	_beginGrbit = copy._beginGrbit;
	_rollbackGrbit = copy._rollbackGrbit;
	_commitGrbit = copy._commitGrbit;
	_lastRegisteredListener = copy._lastRegisteredListener;

	_onCommitListeners = copy._onCommitListeners;
	copy._onCommitListeners.clear();

	_finalized = copy._finalized;
	copy._finalized = true;
}

DbTransaction::~DbTransaction(void)
{
	JET_ERR rc;

	// Delete operations listening for commit, which we have taken responsibility for
	for (vector<DbOperation *>::iterator i = _onCommitListeners.begin(); i != _onCommitListeners.end(); ++i)
	{
		delete (*i);
	}

	// Rollback transaction if not finalized
	if (!_finalized)
	{
		ERR_LOG( rollback() );
	}
}

JET_ERR
DbTransaction::notify_commit_listeners()
{
	JET_ERR rc;

	// Notify all listening operations that the data they read or wrote has been committed permanently. This gives
	// them a chance to perform such actions as updating caches
	for (vector<DbOperation *>::iterator i = _onCommitListeners.begin(); i != _onCommitListeners.end(); ++i)
	{
		ERR_LOG( (*i)->on_commit(this) );
	}

	return JET_errSuccess;
}
