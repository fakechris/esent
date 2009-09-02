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
#include "DbEngine.h"
#include "DbSession.h"
#include "DbCursor.h"
#include "DbTransaction.h"
#include "DbTable.h"
#include "StringUtil.h"
class DbOperation;
class DbSharedEnvironment;
class DbChildEnvironment;
class DbTransactedChildEnvironment;

// In order to work, a database operation needs an environment. This includes a session with a particular database
// engine instance, a top-level transaction (even read ops occur within transactions so that consistent views are
// provided) and a cursor on the first table the operation will operate against. These elements are encapsulated
// within an operation environment object, which is passed to the operation.


//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//DbOperationEnvironment
// The interface for all operation environments. An operation environment consists of a session on a specific
// database engine, a transaction within which operations are made and a cursor against a table
class DbOperationEnvironment
{
public:
	DbOperationEnvironment()
		: _deleteCurrOperation(true)
		, _currOperation(NULL)
	{}
	
	virtual ~DbOperationEnvironment();

	virtual DbSharedEnvironment create_shared_environment(void);

	virtual DbChildEnvironment create_child_environment(const char *tblName, JET_GRBIT cursorGrbit=JET_bitTableReadOnly);

	virtual DbTransactedChildEnvironment create_transacted_child_environment(
		const char *tblName, JET_GRBIT cursorGrbit=JET_bitTableReadOnly, JET_GRBIT transBeginGrbit=JET_bitTransactionReadOnly,
		JET_GRBIT transRollbackGrbit=JET_bitNil, JET_GRBIT transCommitGrbit=JET_bitNil);

	// Set the current operation. Note that the environment must take responsibility for deleting the operation. Note that if the
	// operation registers a callback for execution on completion of the commit of the top-level transaction, then it will not be
	// deleted until that point
	virtual void set_current_operation(DbOperation *op);
	
	// Get a pointer the current operation, for example to query its properties e.g. the id assigned to a newly added post
	virtual DbOperation *get_current_operation();

	// Executes the current operation
	virtual JET_ERR execute_current_operation();

	// Get the database session provided by the environment
	virtual DbSession *get_session() = 0;

	// Get the database transaction scoping operations within this environment 
	virtual DbTransaction *get_transaction() = 0;

	virtual DbTransaction *get_top_transaction() = 0;
	
	// Get the database cursor that an operation inside this environment must use
	virtual DbCursor *get_cursor() = 0;

	// Get the database table the above cursor is against
	virtual DbTable *get_table() = 0;

	// Get the database engine that contains the above database table
	virtual DbEngine *get_engine() { return get_session()->get_engine(); }

protected:
	DbOperation *_currOperation;
	bool _deleteCurrOperation;
};

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//DbChildEnvironment
// An operation environment that is forked off beneath an existing parent operation environment. This is
// typically used by operations wishing to execute their own child operations
class DbChildEnvironment : public DbOperationEnvironment
{
public:
	DbChildEnvironment(
		DbOperationEnvironment *parentCxt,			// parent context this child context will exist within
		const char *tblName,						// name of table to open cursor on
		JET_GRBIT cursorGrbit=JET_bitTableReadOnly	// as documented in JetOpenTable (=open cursor)
		)
		: _parentCxt(parentCxt)
		, _cursor(get_session())
	{
		DbTable *table = get_session()->get_engine()->get_default_database()->get_table(tblName);
		JET_ERR rc = table->open_cursor(&_cursor, cursorGrbit);	
		if (rc != JET_errSuccess)
		{
			throw wyki_database_error(wstring(L"Could not open cursor on ") + StringUtil::to_wide_string(tblName) + L" table", rc);
		}
	}

	virtual DbSession *get_session()
	{
		return _parentCxt->get_session();
	}

	virtual DbTransaction *get_transaction()
	{
		return _parentCxt->get_transaction();
	}

	virtual DbTransaction *get_top_transaction()
	{
		return _parentCxt->get_top_transaction();
	}

	virtual DbCursor *get_cursor()
	{
		return &_cursor;
	}

	virtual DbTable *get_table()
	{
		return _cursor.get_table();
	}

private:
	DbOperationEnvironment *_parentCxt;
	DbCursor _cursor;
};

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//DbSharedEnvironment
// An "shared" operation environment that is forked off beneath an existing parent operation environment. This is
// typically used by operations wishing to launch new operations, where the new operations will share the same
// cursor as the parent operation (as well as the same database engine session)
class DbSharedEnvironment : public DbOperationEnvironment
{
public:
	DbSharedEnvironment(
		DbOperationEnvironment *parentCxt			// parent context this child context will exist within
		)
		: _parentCxt(parentCxt)
	{
	}

	DbSharedEnvironment(DbSharedEnvironment const& copy)
		: _parentCxt(copy._parentCxt)
	{
	}

	virtual DbSession *get_session()
	{
		return _parentCxt->get_session();
	}

	virtual DbTransaction *get_transaction()
	{
		return _parentCxt->get_transaction();
	}

	virtual DbTransaction *get_top_transaction()
	{
		return _parentCxt->get_top_transaction();
	}

	virtual DbCursor *get_cursor()
	{
		return _parentCxt->get_cursor();
	}

	virtual DbTable *get_table()
	{
		return get_cursor()->get_table();
	}

private:
	DbOperationEnvironment *_parentCxt;
};

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//DbTransactedChildEnvironment
// An operation environment that is forked off beneath an existing parent operation environment. The session is shared, however
// a new cursor is opened, and a new transaction for child operations begun
class DbTransactedChildEnvironment : public DbOperationEnvironment
{
public:
	DbTransactedChildEnvironment(
		DbOperationEnvironment *parentCxt,			// parent context this child context will exist within
		const char *tblName,						// name of table to open cursor on
		JET_GRBIT cursorGrbit=JET_bitTableReadOnly,	// as documented in JetOpenTable (=open cursor)
		JET_GRBIT transBeginGrbit=JET_bitTransactionReadOnly,	// as documented in JetBeginTransaction
		JET_GRBIT transRollbackGrbit=JET_bitNil,				// as documented in JetRollback
		JET_GRBIT transCommitGrbit=JET_bitNil					// as documented in JetCommit
		)
		: _parentCxt(parentCxt)
		, _transaction(get_session(), false, transBeginGrbit, transRollbackGrbit, transCommitGrbit)
		, _cursor(get_session())
	{
		DbTable *table = get_session()->get_engine()->get_default_database()->get_table(tblName);
		JET_ERR rc = table->open_cursor(&_cursor, cursorGrbit);	
		if (rc != JET_errSuccess)
		{
			throw wyki_database_error(wstring(L"Could not open cursor on ") + StringUtil::to_wide_string(tblName) + L" table", rc);
		}
	}

	DbTransactedChildEnvironment(DbTransactedChildEnvironment& copy)
		: _transaction(copy._transaction)
		, _parentCxt(copy._parentCxt)
		, _cursor(copy._cursor)
	{
	}

	virtual DbSession *get_session()
	{
		return _parentCxt->get_session();
	}

	virtual DbTransaction *get_transaction()
	{
		return &_transaction;
	}

	virtual DbTransaction *get_top_transaction()
	{
		return _parentCxt->get_top_transaction();
	}

	virtual DbCursor *get_cursor()
	{
		return &_cursor;
	}

	virtual DbTable *get_table()
	{
		return _cursor.get_table();
	}

private:
	DbOperationEnvironment *_parentCxt;
	DbTransaction _transaction;
	DbCursor _cursor;
};


//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//DbRootEnvironment
// A root or "master" operation environment, which is typically created upon a database API entry pointing 
// being called. This is passed to top-level operations, which will in turn pass new child environments to
// operations they themselves execute
class DbRootEnvironment : public DbOperationEnvironment
{
	friend class DbChildEnvironment;
public:
	DbRootEnvironment(
		DbEngine *engine,										// database engine instance operating against
		const char *tblName,									// name of table to open cursor on
		JET_GRBIT cursorGrbit=JET_bitTableReadOnly,				// as documented in JetOpenTable (=open cursor)
		JET_GRBIT transBeginGrbit=JET_bitTransactionReadOnly,	// as documented in JetBeginTransaction
		JET_GRBIT transRollbackGrbit=JET_bitNil,				// as documented in JetRollback
		JET_GRBIT transCommitGrbit=JET_bitNil,					// as documented in JetCommit
		bool sessMgrPooled=true,								// acquire session from fixed-size pool
		unsigned long sessMgrMaxWait=INFINITE					// max wait to acquire session from pool
		)
		: //_workScope( (engine ? engine : dbEngine)->get_session_manager(), sessMgrPooled, sessMgrMaxWait )
		_workScope( engine->get_session_manager(), sessMgrPooled, sessMgrMaxWait )
		, _transaction(get_session(), false, transBeginGrbit, transRollbackGrbit, transCommitGrbit)
		, _cursor(get_session())
	{
		JET_ERR rc;

		// begin our top-level transaction
		rc = _transaction.start();
		if (rc != JET_errSuccess)
		{
			throw wyki_database_error(L"Could not start transaction", rc);
		}

		// open a cursor on the target table
		_table = get_session()->get_engine()->get_default_database()->get_table(tblName);
		rc = _table->open_cursor(&_cursor, cursorGrbit);
		if (rc != JET_errSuccess)
		{
			throw wyki_database_error(wstring(L"Could not open cursor on ") + StringUtil::to_wide_string(tblName) + L" table", rc);
		}
	}

	virtual DbSession *get_session()
	{
		return _workScope.get_session();
	}

	virtual DbTransaction *get_transaction()
	{
		return &_transaction;
	}

	virtual DbTransaction *get_top_transaction()
	{
		return get_transaction();
	}

	virtual DbCursor *get_cursor()
	{
		return &_cursor;
	}

	virtual DbTable *get_table()
	{
		return _table;
	}

protected:
	AutoSession _workScope;
	DbTransaction _transaction;
	DbCursor _cursor;
	DbTable *_table;
};