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
#include "DbOperationEnvironment.h"
#include "DbOperation.h"

DbOperationEnvironment::~DbOperationEnvironment()
{
	if (_deleteCurrOperation)
	{
		delete _currOperation;
	}
}

void
DbOperationEnvironment::set_current_operation(DbOperation *op)
{
	if (_deleteCurrOperation)
	{
		delete _currOperation;
		_currOperation = NULL;
	}

	op->set_environment(this);
	_currOperation = op;
}

// Get a pointer the current operation, for example to query its properties e.g. the id assigned to a newly added post
DbOperation *
DbOperationEnvironment::get_current_operation()
{
	return _currOperation;
}

// Executes the current operation
JET_ERR
DbOperationEnvironment::execute_current_operation()
{
	JET_ERR rc = _currOperation->execute();

	// If during execution the current operation registered for a callback on the commit of the top-level transaction,
	// then we should not delete it when this environment is freed. That is because responsibility for freeing the operation
	// now belongs to the transaction (operations might register a callback on top-level transaction commit, for example
	// to update a cache with a copy of data they now know to be inside the database)
	if (get_top_transaction()->get_last_registered_listener() == _currOperation)
	{
		_deleteCurrOperation = false;
	}

	return rc;
}

DbTransactedChildEnvironment
DbOperationEnvironment::create_transacted_child_environment(const char *tblName, JET_GRBIT cursorGrbit, JET_GRBIT transBeginGrbit, JET_GRBIT transRollbackGrbit, JET_GRBIT transCommitGrbit)
{
	// ASSUME Named Value Optimization i.e. /O2 option
	// http://msdn.microsoft.com/en-us/library/ms364057(VS.80).aspx
	return DbTransactedChildEnvironment(this, tblName, cursorGrbit, transBeginGrbit, transRollbackGrbit, transCommitGrbit);
}

DbChildEnvironment
DbOperationEnvironment::create_child_environment(const char *tblName, JET_GRBIT cursorGrbit)
{
	// ASSUME Named Value Optimization i.e. /O2 option
	// http://msdn.microsoft.com/en-us/library/ms364057(VS.80).aspx
	return DbChildEnvironment(this, tblName, cursorGrbit);
}

DbSharedEnvironment
DbOperationEnvironment::create_shared_environment(void)
{
	// ASSUME Named Value Optimization i.e. /O2 option
	// http://msdn.microsoft.com/en-us/library/ms364057(VS.80).aspx
	return DbSharedEnvironment(this);
}
