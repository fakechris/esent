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
#include "DbSession.h"
#include "DbErrors.h"
#include "DbTableSchema.h"
#include <Utility/CriticalSection.h>
class DbCursor;
class WykiIdentityServer;

//
// DbTable
// Wraps a table inside a JET database, providing a description of the underlying table's schema, and also
// allowing a cursor to be opened on the table
//
class DbTable : private CriticalSection
{
public:
	DbTable(const char *tableName);
	virtual ~DbTable(void);

	// Build the relevant table inside the database 
	// NOTES RE: TRANSACTIONS
	// It is expected that this method will be called within a transaction that creates the database.
	// The nature of JET transactions is that if they contain certain operations, like adding an index, they
	// cannot contain any other operations also. Therefore, implementations of this method must wrap such
	// operations inside individual transactions such that the higher level transaction that creates the
	// database can succeed.
	JET_ERR build_table(DbSession *session);

	// Load the schema description of the encapsulated table
	virtual JET_ERR load_schema(DbSession *session);

	// Open a cursor on the table within the specified session
	JET_ERR open_cursor(DbCursor *cursor, JET_GRBIT grbit);

	// Get the table name
	const char *get_name() { return _tableName.c_str(); }

	// Get the table schema
	DbTableSchema *get_schema() { return _schema; }

	// Get handle to server that creates monotonic Wyki identities for table
	WykiIdentityServer *get_id_server();

	virtual unsigned long get_initial_pages() { return 1000; }

	virtual unsigned long get_table_density() { return 0; } //default

protected:
	// abstract method called by build_table()
	virtual JET_ERR do_build_columns(DbCursor *cursor) = 0;

	// abstract method called by build_table()
	virtual JET_ERR do_build_indexes(DbCursor *cursor) = 0;

	string _tableName;
	DbTableSchema *_schema;
	WykiIdentityServer *_idServer;
};
