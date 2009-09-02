#pragma once

#include "DbTable.h"

class MyCacheTable : public DbTable
{
public:
	static const char* default_name()
	{
		return "cache_table";
	}

	MyCacheTable() : DbTable(MyCacheTable::default_name())
	{
	}

	MyCacheTable(const char *tableName) : DbTable(tableName) 
	{
	}

	JET_ERR do_build_columns(DbCursor *cursor) 
	{
		JET_ERR rc;

		JET_COLUMNDEF columndef = {0};
		JET_COLUMNID columnid;

		columndef.cbStruct = sizeof(columndef);
		columndef.coltyp = JET_coltypText;
		columndef.cp = 1252;  // INFOHASH(20bytes) + PIECEID(4bytes) + SUBPIECEID (1bytes)

		ERR_RET( JetAddColumn(cursor->get_session()->get_sesid(),
			cursor->get_tableid(),
			"infohash_pieceid_subid",
			&columndef,
			NULL,
			0,
			&columnid
			) );

		JET_COLUMNDEF columndef1 = {0};
		JET_COLUMNID columnid1;

		columndef1.cbStruct = sizeof(columndef1);
		columndef1.coltyp = JET_coltypDateTime;
		columndef1.cp = 1252;  

		ERR_RET( JetAddColumn(cursor->get_session()->get_sesid(),
			cursor->get_tableid(),
			"cache_time",
			&columndef1,
			NULL,
			0,
			&columnid1
			) );

		JET_COLUMNDEF columndef2 = {0};
		JET_COLUMNID columnid2;

		columndef2.cbStruct = sizeof(columndef2);
		columndef2.coltyp = JET_coltypLongBinary;
		columndef2.cp = 1252;  

		ERR_RET( JetAddColumn(cursor->get_session()->get_sesid(),
			cursor->get_tableid(),
			"data",
			&columndef2,
			NULL,
			0,
			&columnid2
			) );

		return JET_errSuccess;
	}

	JET_ERR do_build_indexes(DbCursor *cursor)
	{
		JET_ERR rc;

		ERR_RET(JetCreateIndex(cursor->get_session()->get_sesid(),
			cursor->get_tableid(),
			"idx_infohash",
			JET_bitIndexPrimary,
			"+infohash_pieceid_subid\0",
			strlen("+infohash_pieceid_subid\0")+2,
			100
			) );

		ERR_RET(JetCreateIndex(cursor->get_session()->get_sesid(),
			cursor->get_tableid(),
			"idx_date",
			0,
			"+cache_time\0",
			strlen("+cache_time\0")+2,
			100
			) );

		return JET_errSuccess;
	}

};