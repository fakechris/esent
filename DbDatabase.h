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
#include "DbTable.h"

class DbDatabase
{
public:
	DbDatabase(const char *databasePath);
	virtual ~DbDatabase(void);

	// Add a table to the database description
	void add_table(DbTable *table)
	{
		_tables[string(table->get_name())] = table;
	}

	// Get a handle to object wrapping table in the database
	DbTable *get_table(const char *name)
	{
		return (*_tables.find(string(name))).second;
	}

	// Build all tables in the database for the first time (assumes don't exist yet)
	JET_ERR build_all_tables(DbSession *session)
	{
		JET_ERR rc;
		for (TABLES::iterator i = _tables.begin(); i != _tables.end(); ++i)
		{
			ERR_RET( ((*i).second)->build_table(session) )
		}
		return JET_errSuccess;
	}

	// Load the schemas of the tables that already exist in the database
	JET_ERR initialize_table_schemas(DbSession *session)
	{
		JET_ERR rc;
		for (TABLES::iterator i = _tables.begin(); i != _tables.end(); ++i)
		{
			ERR_RET( ((*i).second)->load_schema(session) )
		}
		return JET_errSuccess;
	} 

private:
	typedef map<string, DbTable *> TABLES;
	TABLES _tables;
	string _databasePath;
};
