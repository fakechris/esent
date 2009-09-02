#pragma once
#include "WykiDatabaseDriver.h"
#include "DbSession.h"
#include "DbErrors.h"

class DbTableSchema;
class DbTable;

///DbColumn
// Describes a table column
//
class DbColumn
{
	friend class DbTableSchema;

public:
	DbColumn(void) {}
	virtual ~DbColumn(void) {}

	const char *get_name() { return _colName; }
	JET_COLUMNID get_colid() { return _colId; }
	long get_type() { return _colType; }
	long get_cb_max() { return _colCbMax; }
	short get_code_page() { return _cp; }
	short get_collation() { return _colCollate; }
	JET_GRBIT get_grbit() { return _colGrbit; }
	
private:
	char _colName[JET_cbNameMost+1];//+1 for null
	JET_COLUMNID _colId;
	unsigned long _colType;
	unsigned long _colCbMax;
	unsigned short _cp;
	unsigned short _colCollate;
	JET_GRBIT _colGrbit;
};

///DbTableSchema
// Describing the columns of a particular table
//
class DbTableSchema
{
public:
	DbTableSchema(void);
	virtual ~DbTableSchema(void);

	JET_ERR load_schema_for_table(DbSession *dbs, DbTable *table, JET_TABLEID tableid);

	DbColumn *get_col(const char *name)
	{
		TABLE_COLS::iterator i = _tbl_cols.find(string(name));
		if (i != _tbl_cols.end())
		{
			return (*i).second;
		}

		throw wyki_schema_error(wstring(L"Invalid column access - column name not present in table"));
	}

private:
	typedef map<string, DbColumn *> TABLE_COLS;
	TABLE_COLS _tbl_cols;
	JET_TABLEID _tmp;
};
