#include "StdAfx.h"
#include "DbTable.h"
#include "DbCursor.h"
#include "DbTableSchema.h"

DbTableSchema::DbTableSchema()
{
	_tmp = JET_tableidNil;
}

DbTableSchema::~DbTableSchema(void)
{
	for (TABLE_COLS::iterator i = _tbl_cols.begin(); i != _tbl_cols.end(); ++i)
	{
		delete (*i).second;
	}
}

JET_ERR
DbTableSchema::load_schema_for_table(DbSession *dbs, DbTable *table, JET_TABLEID tableid)
{
	JET_ERR rc;
	JET_COLUMNLIST col_list;
	col_list.cbStruct = sizeof(col_list);

	// Request the table schema (or rather, a list of colum descriptions). The JET_COLUMNLIST struct will be
	// filled for us. The tableid field will provide a handle to a temporary table, in which there will be a row
	// describing every column. The other fields of the structure will specify the ids of the temporary table
	// columns pertaining to the various the column attributes e.g. data type
	ERR_RET( JetGetTableColumnInfo(dbs->get_sesid(), tableid, NULL, &col_list, sizeof(col_list), JET_ColInfoListSortColumnid) )

	// Grab handle to temporary table holding schema information, which we must close later
	DbCursor cursor(dbs);
	cursor.reset(col_list.tableid, table);

	// Move to beginning of temporary table
	ERR_RET( JetMove(dbs->get_sesid(), cursor.get_tableid(), JET_MoveFirst, JET_bitNil) )

	// Scan table to build map of column descriptions 
	do
	{
		auto_ptr<DbColumn> jcd(new DbColumn());
		
		// Retrieve the column name
		unsigned long sz;
		ERR_RET( JetRetrieveColumn(dbs->get_sesid(), cursor.get_tableid(), col_list.columnidcolumnname, jcd->_colName, sizeof(jcd->_colName), &sz, JET_bitNil, NULL) )
		jcd->_colName[sz] = 0;

		// Retrieve the column id
		ERR_RET( JetRetrieveColumn(dbs->get_sesid(), cursor.get_tableid(), col_list.columnidcolumnid, &jcd->_colId, sizeof(jcd->_colId), &sz, JET_bitNil, NULL) )

		// Retrieve the column type
		ERR_RET( JetRetrieveColumn(dbs->get_sesid(), cursor.get_tableid(), col_list.columnidcoltyp, &jcd->_colType, sizeof(jcd->_colType), &sz, JET_bitNil, NULL) )

		// Retrieve the field's max length 
		ERR_RET( JetRetrieveColumn(dbs->get_sesid(), cursor.get_tableid(), col_list.columnidcbMax, &jcd->_colCbMax, sizeof(jcd->_colCbMax), &sz, JET_bitNil, NULL) )

		// Retrieve the code page
		ERR_RET( JetRetrieveColumn(dbs->get_sesid(), cursor.get_tableid(), col_list.columnidCp, &jcd->_cp, sizeof(jcd->_cp), &sz, JET_bitNil, NULL) )

		// Retrieve the collation
		ERR_RET( JetRetrieveColumn(dbs->get_sesid(), cursor.get_tableid(), col_list.columnidCollate, &jcd->_colCollate, sizeof(jcd->_colCollate), &sz, JET_bitNil, NULL) )

		// Retrieve the grbit
		ERR_RET( JetRetrieveColumn(dbs->get_sesid(), cursor.get_tableid(), col_list.columnidgrbit, &jcd->_colGrbit, sizeof(jcd->_colGrbit), &sz, JET_bitNil, NULL) )

		// Add the column description to the column name->description map
		_tbl_cols[string(jcd->_colName)] = jcd.get();

		jcd.release();
	}
	while ( (rc = JetMove(dbs->get_sesid(), cursor.get_tableid(), JET_MoveNext, JET_bitNil)) == JET_errSuccess);

	// Check we finished scan correctly!
	if (rc != JET_errNoCurrentRecord)
	{
		ERR_RET(rc);
	}

	return JET_errSuccess;
}