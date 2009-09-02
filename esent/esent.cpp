// esent.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "DbCursor.h"
#include "DbSession.h"
#include "DbEngine.h"

#include "cachetable.h"

// One possible error-handling strategy is to jump to an error-handling
// label whenever an ESENT call fails.
#define Call(func) { \
err = (func); \
if(err < JET_errSuccess) { \
goto HandleError; \
} \
} 

int test()
{
	JET_ERR err;
	JET_INSTANCE instance;
	JET_SESID sesid;
	JET_DBID dbid;
	JET_TABLEID tableid;

	JET_COLUMNDEF columndef = {0};
	JET_COLUMNID columnid;

	// Initialize ESENT. Setting JET_paramCircularLog to 1 means ESENT will automatically
	// delete unneeded logfiles. JetInit will inspect the logfiles to see if the last
	// shutdown was clean. If it wasn't (e.g. the application crashed) recovery will be
	// run automatically bringing the database to a consistent state.
	Call(JetCreateInstance(&instance, "instance"));
	Call(JetSetSystemParameter(&instance, JET_sesidNil, JET_paramCircularLog, 1, NULL));
	Call(JetInit(&instance));
	Call(JetBeginSession(instance, &sesid, 0, 0));

	// Create the database. To open an existing database use the JetAttachDatabase and 
	// JetOpenDatabase APIs.
	Call(JetCreateDatabase(sesid, "edbtest.db", 0, &dbid, JET_bitDbOverwriteExisting));

	// Create the table. Meta-data operations are transacted and can be performed concurrently.
	// For example, one session can add a column to a table while another session is reading
	// or updating records in the same table.
	// This table has no indexes defined, so it will use the default sequential index. Indexes
	// can be defined with the JetCreateIndex API.
	Call(JetBeginTransaction(sesid));
	Call(JetCreateTable(sesid, dbid, "table", 0, 100, &tableid));
	columndef.cbStruct = sizeof(columndef);
	columndef.coltyp = JET_coltypLongText;
	columndef.cp = 1252;
	Call(JetAddColumn(sesid, tableid, "column1", &columndef, NULL, 0, &columnid));
	Call(JetCommitTransaction(sesid, JET_bitCommitLazyFlush));

	// Insert a record. This table only has one column but a table can have slightly over 64,000
	// columns defined. Unless a column is declared as fixed or variable it won't take any space
	// in the record unless set. An individual record can have several hundred columns set at one
	// time, the exact number depends on the database page size and the contents of the columns.
	Call(JetBeginTransaction(sesid));
	Call(JetPrepareUpdate(sesid, tableid, JET_prepInsert));
	char * message = "Hello world";
	Call(JetSetColumn(sesid, tableid, columnid, message, strlen(message)+1, 0, NULL));
	Call(JetUpdate(sesid, tableid, NULL, 0, NULL));
	Call(JetCommitTransaction(sesid, 0));    // Use JetRollback() to abort the transaction

	// Retrieve a column from the record. Here we move to the first record with JetMove. By using
	// JetMoveNext it is possible to iterate through all records in a table. Use JetMakeKey and
	// JetSeek to move to a particular record.
	Call(JetMove(sesid, tableid, JET_MoveFirst, 0));
	char buffer[1024];
	Call(JetRetrieveColumn(sesid, tableid, columnid, buffer, sizeof(buffer), NULL, 0, NULL));
	printf("%s", buffer);

	// Terminate ESENT. This performs a clean shutdown.
	JetCloseTable(sesid, tableid);
	JetEndSession(sesid, 0);
	JetTerm(instance);
	return 0;
HandleError:
	printf("ESENT error %d\n", err);
	return 1; 
}


int _tmain(int argc, _TCHAR* argv[])
{
	//test();
	JET_ERR rc;
	if(1)
	{	
		WTDB_ENGINE_PARAMS para;
		para.maxSessions = 5;
		para.szDatabaseFilename = "c:\\temp\\ese_test.db";
		para.szEngineLogPath = "c:\\temp";
		DbEngine dbengine(&para);		
		ERR_RET( dbengine.try_init() )

		DbSession * session;
		ERR_RET( dbengine.create_session(&session) )

		DbCursor cursor(session);
		ERR_RET( JetBeginTransaction2(session->get_sesid(), JET_bitNil) )
		MyCacheTable table;
		table.open_cursor(&cursor, JET_bitTableUpdatable);
		ERR_RET( cursor.prepare_update(JET_prepInsert) )

		JET_SETCOLUMN cols[3] = {};
		JET_COLUMNDEF coldef;

		ERR_RET(JetGetTableColumnInfo(session->get_sesid(),
			cursor.get_tableid(),
			"infohash_pieceid_subid",
			(void*)&coldef,
			sizeof(JET_SETCOLUMN),
			JET_ColInfo
			) )
		cols[0].columnid = coldef.columnid;
		cols[0].ibLongValue = 0;
		cols[0].itagSequence = 0;
		cols[0].grbit = 0;
		cols[0].err = 0;
		cols[0].pvData = "test123321";
		cols[0].cbData = strlen("test123321");
		ERR_RET(JetGetTableColumnInfo(session->get_sesid(),
			cursor.get_tableid(),
			"cache_time",
			(void*)&coldef,
			sizeof(JET_SETCOLUMN),
			JET_ColInfo
			) )
		double l = 12.0;
		cols[1].columnid = coldef.columnid;
		cols[1].ibLongValue = 0;
		cols[1].itagSequence = 0;
		cols[1].grbit = 0;
		cols[1].err = 0;
		cols[1].pvData = &l;
		cols[1].cbData = sizeof(double);
		ERR_RET(JetGetTableColumnInfo(session->get_sesid(),
			cursor.get_tableid(),
			"data",
			(void*)&coldef,
			sizeof(JET_SETCOLUMN),
			JET_ColInfo
			) )
		char * cbuf = new char[10*1024];
		cols[2].columnid = coldef.columnid;
		cols[2].ibLongValue = 0;
		cols[2].itagSequence = 0;
		cols[2].grbit = 0;
		cols[2].err = 0;
		cols[2].pvData = cbuf;
		cols[2].cbData = 10*1024;

		ERR_RET( cursor.set_columns(cols, 3) )

		ERR_RET( cursor.write_update(NULL) ) 
		ERR_RET( JetCommitTransaction(session->get_sesid(), JET_bitNil) )
	}
	if (0)
		test();
	return 0;
}

