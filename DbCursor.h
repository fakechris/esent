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
#include "DbTable.h"
#include "DbSession.h"
#include "DbBookmark.h"
#include "DbTableSchema.h"

//
// DbCursor
// Wraps a JET table cursor object of type JET_TABLEID. When this class is destructed, it automatically closes the
// table cusror object that it is managing. You can take possession of the underlying handle using release().
// Various methods for operating on the cursor are provided, include methods for updating and navigating the table
//
class DbCursor
{
public:
	DbCursor(DbSession *session, bool nullStringOnNull=true, bool zeroBytesOnNull=true)
		: _session(session), _table(NULL), _tableid(JET_tableidNil), _updateInProgress(false)
		, _nullStringMode(nullStringOnNull), _zeroBytesOnNullMode(zeroBytesOnNull)
	{
		;
	}

	DbCursor(DbCursor const& copy)
		: _session(copy._session)
		, _tableid(copy._tableid)
		, _table(copy._table)
		, _updateInProgress(copy._updateInProgress)
		, _nullStringMode(copy._nullStringMode)
		, _zeroBytesOnNullMode(copy._zeroBytesOnNullMode)
	{
		_tableid = JET_tableidNil;
	}
	
	virtual ~DbCursor(void)
	{
		JET_ERR rc;

		if (_updateInProgress)
		{
			ERR_LOG( cancel_update() )
		}

		if (_tableid != JET_tableidNil)
		{
			ERR_LOG( JetCloseTable(_session->get_sesid(), _tableid) )
		}
	}

	// Assign a new table id to the cursor. Destroy earlier cursor
	JET_ERR reset(JET_TABLEID tableid, DbTable *table)
	{
		JET_ERR rc;

		if (_updateInProgress)
		{
			ERR_RET( cancel_update() )
		}

		if (_tableid != JET_tableidNil)
		{
			ERR_RET( JetCloseTable(_session->get_sesid(), _tableid) )
		}

		_tableid = tableid; // cursor

		_table = table; // table description

		return JET_errSuccess;
	}

	// Release the encapsulated tableid handle from the DbCursor object - equivalent to auto_ptr<>.release()
	JET_TABLEID release(void)
	{
		JET_TABLEID tmp = _tableid;
		_tableid = JET_tableidNil;
		return tmp;
	}

	// Get the encapsulated tableid handle 
	JET_TABLEID get_tableid(void)
	{
		return _tableid;
	}

	// Get a reference to the encapsulated tableid handle e.g. to pass to JetOpenTable
	// WARNING: this function should only be used in special circumstances
	JET_TABLEID *get_tableid_ref(void)
	{
		return &_tableid;
	}

	DbTable *get_table(void) { return _table; }

	DbSession *get_session(void) { return _session; }

	JET_ERR set_position(DbBookmark *bookmark)
	{
		JET_ERR rc;

		ERR_RET( JetGotoBookmark(_session->get_sesid(), _tableid, bookmark->get_bookmark_ref(), bookmark->get_size()) )

		return JET_errSuccess;
	}

	// Open a tableid cursor handle on a specified table
	JET_ERR open(DbTable *table, JET_GRBIT grbit)
	{
		JET_ERR rc;

		ERR_RET( reset(JET_tableidNil, NULL) )

		ERR_RET( JetOpenTable(_session->get_sesid(), _session->get_dbid(), table->get_name(), NULL, 0, grbit, &_tableid) )

		_table = table;

		return JET_errSuccess;
	}

	// Enter the "update preparation" state, so it is possible to start assigning values to the current table's
	// columns. These values will be written to the table on update()
	JET_ERR prepare_update(unsigned long prep)
	{
		JET_ERR rc;

		ERR_RET( JetPrepareUpdate(_session->get_sesid(), _tableid, prep) )
		_updateInProgress = true;
		
		return JET_errSuccess;
	}

	// Make the cursor leave the "update preparation" state. This enables the preparation of a new update, for
	// example, if the current update cannot continue
	JET_ERR cancel_update(void)
	{
		JET_ERR rc;

		ERR_RET( JetPrepareUpdate(_session->get_sesid(), _tableid, JET_prepCancel) )
		_updateInProgress = false;
		
		return JET_errSuccess;
	}

	// Returns whether the cursor is in the "update preparation" state
	bool get_update_in_progress()
	{
		return _updateInProgress;
	}

	// When updated column values have been written into the copy buffer, using set_column(), this method sends those
	// values into the actual database table (whether this will become visible immediately will depend on whether this
	// occurs within a transaction or not). A bookmark to the newly stored record is returned.
	JET_ERR write_update(DbBookmark *bookmark)
	{
		JET_ERR rc;

		if (!bookmark) {
			ERR_RET( JetUpdate(_session->get_sesid(), _tableid, NULL, 0, NULL) )
		} else {
			ERR_RET( JetUpdate(_session->get_sesid(), _tableid, bookmark->get_bookmark_ref(), bookmark->get_buffer_size(), bookmark->get_size_ref()) )
		}
		_updateInProgress = false;

		return JET_errSuccess;
	}

	// If set to true, if a string column is null, a null pointer is returned instead of the string and JET_errSuccess is the
	// return code. This is does not apply to tag columns
	bool set_null_strings_allowed(bool yes)
	{
		bool tmp = _nullStringMode;
		_nullStringMode = yes;
		return tmp;
	}

	bool get_null_strings_allowed()
	{
		return _nullStringMode;
	}

	// If set to true, when a column is null, the supplied buffer is set to zero and JET_errSuccess is the return code
	bool set_zero_bytes_on_null(bool yes)
	{
		bool tmp = _zeroBytesOnNullMode;
		_zeroBytesOnNullMode = yes;
		return tmp;
	}

	bool get_zero_bytes_on_null()
	{
		return _zeroBytesOnNullMode;
	}

	JET_ERR get_column_size(const char *colName, unsigned long *size, bool *isNull, JET_GRBIT grbit=JET_bitNil, JET_RETINFO *pretinfo=NULL)
	{
		JET_ERR rc;

		DbColumn *column = _table->get_schema()->get_col(colName);
		JET_COLUMNID colid = column->get_colid();

		rc = JetRetrieveColumn(_session->get_sesid(), _tableid, colid, NULL, 0, size, grbit, pretinfo);
		if (rc == JET_wrnBufferTruncated)
		{
			if (isNull)
			{
				*isNull = false;
			}
		}
		else if (rc == JET_wrnColumnNull)
		{
			*size = 0;
			if (isNull)
			{
				*isNull = true;
			}
		}
		else {
			ERR_RET( rc )
		}

		return JET_errSuccess;
	}

	JET_ERR get_column_tag_size(const char *colName, unsigned long *size, bool *isNull, unsigned long tagSeqNo, JET_GRBIT grbit=JET_bitNil)
	{
		JET_ERR rc;

		JET_RETINFO pretinfo;
		pretinfo.cbStruct = sizeof(pretinfo);
		pretinfo.ibLongValue = 0;
		pretinfo.itagSequence = tagSeqNo;
		pretinfo.columnidNextTagged = 0;

		ERR_RET( get_column_size(colName, size, isNull, grbit, &pretinfo) );

		return JET_errSuccess;
	}

	JET_ERR set_columns(JET_SETCOLUMN* cols, int cb_cols)
	{
		JET_ERR rc;

		ERR_RET( JetSetColumns(_session->get_sesid(), _tableid, cols, cb_cols) )

		return JET_errSuccess;
	}

	// Assign a new column value to the copy buffer. The cursor must be in the "update preparation" state.
	JET_ERR set_column(const char *colName, const void* pvData, unsigned long cbData, JET_GRBIT grbit=JET_bitNil, JET_SETINFO *info=NULL)
	{
		JET_ERR rc;

		JET_COLUMNID colid = _table->get_schema()->get_col(colName)->get_colid();

		ERR_RET( JetSetColumn(_session->get_sesid(), _tableid, colid, pvData, cbData, grbit, info) )

		return JET_errSuccess;
	}
	JET_ERR set_column_str(const char *colName, const wchar_t* str, JET_GRBIT grbit=JET_bitNil, JET_SETINFO *info=NULL)
	{
		return set_column(colName, str, StringUtil::get_byte_size(str), grbit, info);
	}
	// Assign a compressed version of a new column value to the copy buffer. The cursor must be in the "update preparation" state.
	JET_ERR set_column_zlib(const char *colName, const void* pvData, unsigned long cbData, JET_GRBIT grbit=JET_bitNil, JET_SETINFO *info=NULL);
	JET_ERR set_column_zlib_str(const char *colName, const wchar_t* str, JET_GRBIT grbit=JET_bitNil, JET_SETINFO *info=NULL)
	{
		return set_column_zlib(colName, str, StringUtil::get_byte_size(str), grbit, info);
	}
	// Set a specific value in a multivalued or "tagged" column. To overwrite the first value, use a tagSeqNo of 1. To overwrite 
	// the second value, use a tagSeqNo of 2 etc. If the tagSeqNo is 0, then a new value is appended to the tagged sequence.
	JET_ERR set_column_tag(const char *colName, const void* pvData, unsigned long cbData, unsigned long tagSeqNo, JET_GRBIT grbit=JET_bitNil)
	{
		JET_SETINFO info;
		info.cbStruct = sizeof(info);
		info.ibLongValue = 0;
		info.itagSequence = tagSeqNo;

		return set_column(colName, pvData, cbData, grbit, &info);
	}
	JET_ERR set_column_tag_str(const char *colName, const wchar_t* str, unsigned long tagSeqNo, JET_GRBIT grbit=JET_bitNil)
	{
		return set_column_tag(colName, str, StringUtil::get_byte_size(str), tagSeqNo, grbit);
	}
	// Delete a value from a tagged column. Tag index ("sequence") starts at 1
	JET_ERR del_column_tag(const char *colName, unsigned long tagSeqNo)
	{
		return set_column_tag(colName, NULL, 0, tagSeqNo);
	}
	// Cause a tagged, sparce or multi-valued column to revert to its default value
	JET_ERR del_column_tags(const char *colName)
	{
		JET_ERR rc;

		JET_COLUMNID colid = _table->get_schema()->get_col(colName)->get_colid();

		// first we have to find out how many tags there are...
		JET_RETINFO retinfo;
		retinfo.cbStruct = sizeof(retinfo);
		retinfo.ibLongValue = 0;
		retinfo.itagSequence = 1;
		retinfo.columnidNextTagged = 0;
		unsigned long size;
		while (true)
		{
			// TO DO: check we can really use JET_bitRetrieveCopy to deal with case where method is called on newly 
			// created record i.e. one which does not yet exist within the database
			rc = JetRetrieveColumn(_session->get_sesid(), _tableid, colid, NULL, 0, &size, JET_bitRetrieveCopy, &retinfo);
			if (rc == JET_wrnColumnNull)
			{
				break;
			}
			else if (rc != JET_wrnBufferTruncated)
			{
				ERR_RET( rc );
			}
		
			retinfo.itagSequence++;
		}

		// then we have to manually set each tag to zero
		JET_SETINFO setinfo;
		setinfo.cbStruct = sizeof(setinfo);
		setinfo.ibLongValue = 0;
		setinfo.itagSequence = 1;
		for (unsigned long i = 0; i < retinfo.itagSequence; i++)
		{
			ERR_RET( JetSetColumn(_session->get_sesid(), _tableid, colid, NULL, 0, JET_bitNil, &setinfo) );
		}

		return JET_errSuccess;
	}

	// Set a compressed version of a value in a multivalued or "tagged" column. To overwrite the first value, use a tagSeqNo of 1. To overwrite 
	// the second value, use a tagSeqNo of 2 etc. If the tagSeqNo is 0, then a new value is appended to the tagged sequence.
	JET_ERR set_column_zlib_tag(const char *colName, const void* pvData, unsigned long cbData, unsigned long tagSeqNo, JET_GRBIT grbit=JET_bitNil)
	{
		JET_ERR rc;

		JET_SETINFO info;
		info.cbStruct = sizeof(info);
		info.ibLongValue = 0;
		info.itagSequence = tagSeqNo;

		ERR_RET( set_column_zlib(colName, pvData, cbData, grbit, &info) )

		return JET_errSuccess;
	}
	JET_ERR set_column_zlib_tag_str(const char *colName, const wchar_t* str, unsigned long tagSeqNo, JET_GRBIT grbit=JET_bitNil)
	{
		return set_column_zlib_tag(colName, str, StringUtil::get_byte_size(str), tagSeqNo, grbit);
	}
	JET_ERR set_column_norm_ulong(const char *colName, unsigned long ul);
	JET_ERR set_column_norm_ulonglong(const char *colName, unsigned long long ull);
	JET_ERR set_column_norm_filetime(const char *colName, FILETIME ft, JET_GRBIT grbit=JET_bitNil, JET_SETINFO *pretinfo=NULL);
	JET_ERR set_column_norm_filetime_tag(const char *colName, FILETIME ft, unsigned long tagSeqNo, JET_GRBIT grbit=JET_bitNil);
	// Update an escrow value
	JET_ERR set_column_escrow(const char *colName, long addend, long *pCurrValue=NULL, JET_GRBIT grbit=JET_bitNil)
	{
		JET_ERR rc;

		JET_COLUMNID colid = _table->get_schema()->get_col(colName)->get_colid();

		long currValue;
		unsigned long currValueBytes;
		ERR_RET( JetEscrowUpdate(_session->get_sesid(), _tableid, colid,
			&addend, sizeof(addend), &currValue, sizeof(currValue), &currValueBytes, grbit) )

		if (pCurrValue)
			*pCurrValue = currValue;

		return JET_errSuccess;
	}


	// Set the length of a long value column
	JET_ERR set_column_length(const char *colName, unsigned long length, JET_SETINFO *pSetInfo=NULL)
	{
		JET_ERR rc;

		JET_COLUMNID colid = _table->get_schema()->get_col(colName)->get_colid();

		ERR_RET( JetSetColumn(_session->get_sesid(), _tableid, colid, &length, sizeof(length), JET_bitSetSizeLV, pSetInfo) );

		return JET_errSuccess;
	}
	
	// Get a column value from the current record into the provided buffer
	JET_ERR get_column(const char *colName, void *pData, unsigned long cbData, unsigned long* pcbActual=NULL, JET_GRBIT grbit=JET_bitNil, JET_RETINFO *pretinfo=NULL)
	{
		JET_ERR rc;

		JET_COLUMNID colid = _table->get_schema()->get_col(colName)->get_colid();

		rc = JetRetrieveColumn(_session->get_sesid(), _tableid, colid, pData, cbData, pcbActual, grbit, pretinfo);
		if (rc == JET_wrnColumnNull)
		{
			if (_zeroBytesOnNullMode && cbData > 0)
			{
				ZeroMemory(pData, cbData);
				return JET_errSuccess;
			}
			return rc;
		}
		else
		{
			ERR_RET( rc );
		}

		return JET_errSuccess;
	}

	// Get a tagged column value, which is returned in the provided buffer
	JET_ERR get_column_tag(const char *colName, void* pData, unsigned long cbData, unsigned long tagSeqNo, unsigned long* pcbActual=NULL, JET_GRBIT grbit=JET_bitNil)
	{
		JET_ERR rc;

		JET_RETINFO pretinfo;
		pretinfo.cbStruct = sizeof(pretinfo);
		pretinfo.ibLongValue = 0;
		pretinfo.itagSequence = tagSeqNo;
		pretinfo.columnidNextTagged = 0;

		unsigned long cbActual;
		rc = get_column(colName, pData, cbData, &cbActual, grbit, &pretinfo);
		if (rc == JET_wrnColumnNull)
		{
			return rc; // do not trigger handle tracking code, as this condition is expected
		}
		else
		{
			ERR_RET( rc )
		}

		if (pcbActual)
		{
			*pcbActual = cbActual;
		}

		return JET_errSuccess;
	}	
	// Get a column value, which is returned in a newly allocated buffer for which the caller becomes responsible	
	JET_ERR get_column(const char *colName, BYTE** ppData, unsigned long* pcbActual, JET_GRBIT grbit=JET_bitNil, JET_RETINFO *pretinfo=NULL)
	{
		JET_ERR rc;
		*ppData = NULL;

		JET_COLUMNID colid = _table->get_schema()->get_col(colName)->get_colid();

		unsigned long size;
		rc = JetRetrieveColumn(_session->get_sesid(), _tableid, colid, NULL, 0, &size, grbit, pretinfo);
		if (rc == JET_wrnBufferTruncated)
		{
			; // do nothing, we retrieved the size ok!
		}
		else if (rc == JET_wrnColumnNull)
		{
			return rc; // do not trigger error tracking code, as this condition is used
		}
		else {
			ERR_RET( rc ) // a real problem occurred!
		}

		auto_ptr<BYTE> buf(new BYTE[size]);
		ERR_RET( JetRetrieveColumn(_session->get_sesid(), _tableid, colid, buf.get(), size, pcbActual, grbit, pretinfo) )

		// return string
		*ppData = buf.release();

		return JET_errSuccess;
	}
	// Get a tagged column value, which is returned in a newly allocated buffer for which the caller becomes responsible
	JET_ERR get_column_tag(const char *colName, BYTE** ppData, unsigned long* pcbActual, unsigned long tagSeqNo, JET_GRBIT grbit=JET_bitNil)
	{
		JET_ERR rc;

		JET_RETINFO pretinfo;
		pretinfo.cbStruct = sizeof(pretinfo);
		pretinfo.ibLongValue = 0;
		pretinfo.itagSequence = tagSeqNo;
		pretinfo.columnidNextTagged = 0;

		rc = get_column(colName, ppData, pcbActual, grbit, &pretinfo);
		if (rc == JET_wrnColumnNull)
		{
			return rc; // do not trigger handle tracking code, as this condition is expected
		}
		else
		{
			ERR_RET( rc )
		}

		return JET_errSuccess;
	}
	// Get all string tag values for specified column on current record
	JET_ERR get_column_tag_array(const char *colName, BYTE*** ppData, vector<unsigned long> *lengths, unsigned long capacity=64, unsigned long reqTagLength=0);
	// Get a column string value from the current record as a newly allocated buffer
	JET_ERR get_column_str(const char *colName, WTDB_STR* pString, JET_GRBIT grbit=JET_bitNil, JET_RETINFO *pretinfo=NULL)
	{
		JET_ERR rc;
		*pString = NULL;

		DbColumn *column = _table->get_schema()->get_col(colName);
		JET_COLUMNID colid = column->get_colid();

		// get size of string bytes stored. We assume, *without* terminating null
		unsigned long size;
		rc = JetRetrieveColumn(_session->get_sesid(), _tableid, colid, NULL, 0, &size, grbit, pretinfo);
		if (rc == JET_wrnBufferTruncated)
		{
			; // do nothing, we retrieved the size ok!
		}
		else if (rc == JET_wrnColumnNull)
		{
			if (_nullStringMode && !pretinfo)
			{
				return JET_errSuccess;
			}
			return rc; // do not trigger error tracking code, as this condition is used
		}
		else {
			ERR_RET( rc ) // a real problem occurred!
		}

		// adjust size to allow for null terminator in memory representation
		unsigned long buf_size = size;
		if (column->get_type() == JET_coltypText || column->get_type() == JET_coltypLongText)
		{
			if (column->get_code_page() == 1252) // ASCII
			{
				buf_size += 1;
			}
			else if (column->get_code_page() == 1200) // Unicode
			{
				buf_size += 2;
			}
			else
			{ 
				ERR_RET( JET_errInternalError )
			}
		}
		else
		{
			// we are loading a string from a binary field of some kind... a binary field might be used
			// to store strings that should be indexed without regard to text rules such as case 
			// insensitivity etc. Leave space for 2 byte null that will make either ASCI or unicode safe (at the 
			// possible expense of 1 wasted byte if this is ASCI)
			buf_size += 2;
		}

		// allocate the buffer
		auto_ptr<unsigned char> buf(new unsigned char[buf_size]);

		// retrieve the value
		unsigned long read_size;
		ERR_RET( JetRetrieveColumn(_session->get_sesid(), _tableid, colid, buf.get(), buf_size, &read_size, grbit, pretinfo) )

		// TO DO: probably not necessary to compare size==read_size because JET_wrnBufferTruncated is an error condition anyway

		// assign null terminator
		if (column->get_code_page() == 1252) // ASCII
		{
			buf.get()[size] = 0;
		}
		else // (column->get_code_page() == 1200) // Unicode (and unknown)
		{
			buf.get()[size] = 0;
			buf.get()[size+1] = 0;
		}

		// return string
		*pString = (WTDB_STR)buf.release();

		return JET_errSuccess;
	}

	// Get a column string value from a compressed field from the current record as a newly allocated buffer
	JET_ERR get_column_zlib_str(const char *colName, WTDB_STR* pString, JET_GRBIT grbit=JET_bitNil, JET_RETINFO *pretinfo=NULL);
	// Get a tagged column string value from the current record as a newly allocated buffer
	JET_ERR get_column_tag_str(const char *colName, WTDB_STR* pString, unsigned long tagSeqNo, JET_GRBIT grbit=JET_bitNil)
	{
		JET_ERR rc;

		JET_RETINFO pretinfo;
		pretinfo.cbStruct = sizeof(pretinfo);
		pretinfo.ibLongValue = 0;
		pretinfo.itagSequence = tagSeqNo;
		pretinfo.columnidNextTagged = 0;

		rc = get_column_str(colName, pString, grbit, &pretinfo);
		if (rc == JET_wrnColumnNull)
		{
			return rc; // do not trigger handle tracking code, as this condition is expected
		}
		else
		{
			ERR_RET( rc )
		}

		return JET_errSuccess;
	}

	// Get a tagged column string value from a compressed field from the current record as a newly allocated buffer
	JET_ERR get_column_zlib_tag_str(const char *colName, WTDB_STR* pString, unsigned long tagSeqNo, JET_GRBIT grbit=JET_bitNil)
	{
		JET_ERR rc;

		JET_RETINFO pretinfo;
		pretinfo.cbStruct = sizeof(pretinfo);
		pretinfo.ibLongValue = 0;
		pretinfo.itagSequence = tagSeqNo;
		pretinfo.columnidNextTagged = 0;

		rc = get_column_zlib_str(colName, pString, grbit, &pretinfo);
		if (rc == JET_wrnColumnNull)
		{
			return rc; // do not trigger handle tracking code, as this condition is expected
		}
		else
		{
			ERR_RET( rc )
		}

		return JET_errSuccess;
	}

	// Get all string tag values for specified column on current record
	JET_ERR get_column_tag_str_array(const char *colName, WTDB_STR** pvString, unsigned long capacity);
	// Get all string tag values for specified column from a compressed field on current record
	JET_ERR get_column_zlib_tag_str_array(const char *colName, WTDB_STR** pvString, unsigned long capacity);
	// Load all tag data directly into a typed array. This is more efficient that loading tag data into an array of byte pointers,
	// since only the array itself must be loaded on the heap. 
	template <class T>
	WTDB_ERR get_column_tags_value_array(const char *colName, T **tagArray, unsigned long *count, JET_GRBIT grbit=JET_bitNil)
	{
		JET_ERR rc;

		unsigned long _count = 0;
		unsigned long tagSeqNo = 1;
		while (1)
		{
			bool isNull;
			unsigned long size;
			ERR_RET( get_column_tag_size(colName, &size, &isNull, tagSeqNo, grbit) );

			if (isNull)
				break;

			if (size != sizeof(T))
			{
				return JET_wrnBufferTruncated;
				// buffer could be too big, but never mind. The bottom line is size of requested type does not match stored data
			}
			
			_count++;
			tagSeqNo++;
		}

		*count = _count;

		if (_count == 0)
		{
			*tagArray = NULL;
		}
		else
		{
			array_ptr<T> _typedTagArray(new T[_count], _count);

			for (unsigned long i = 0; i < _count; i++)
			{
				get_column_tag(colName, &_typedTagArray.get()[i], sizeof(T), i+1, NULL, grbit);
			}

			*tagArray = _typedTagArray.release();
		}

		return JET_errSuccess;
	}
	// Get a FILETIME that has been stored in normalized form
	JET_ERR get_column_norm_ulong(const char *colName, unsigned long *ul, JET_GRBIT grbit=JET_bitNil);
	JET_ERR get_column_norm_ulonglong(const char *colName, unsigned long long *ull, JET_GRBIT grbit=JET_bitNil);
	JET_ERR get_column_norm_filetime(const char *colName, FILETIME *ft, JET_GRBIT grbit=JET_bitNil, JET_RETINFO *pretinfo=NULL);
	JET_ERR get_column_norm_filetime_tag(const char *colName, FILETIME *ft, unsigned long tagSeqNo, JET_GRBIT grbit=JET_bitNil);
	// Delete the current row
	JET_ERR delete_row()
	{
		return JetDelete(_session->get_sesid(), _tableid);
	}
	// Set current index that cursor will enumerate or search against
	JET_ERR set_index(const char *indexName)
	{
		return JetSetCurrentIndex(_session->get_sesid(), _tableid, indexName);
	}
	// Set a range within the current index that the cursor may enumerate over
	JET_ERR set_index_range(JET_GRBIT grbit)
	{
		return JetSetIndexRange(_session->get_sesid(), _tableid, grbit);
	}
	// Make the current key that can then be used in seek operations. Multiple calls used to specify value for successive fields
	JET_ERR make_key(const void* pvData, unsigned long cbData, JET_GRBIT grbit)
	{
		return JetMakeKey(_session->get_sesid(), _tableid, pvData, cbData, grbit);
	}
	// Seek to a particular record within current index as specified by current key value (must have been set before calling this method)
	JET_ERR seek(JET_GRBIT grbit)
	{
		return JetSeek(_session->get_sesid(), _tableid, grbit);
	}
	// Move cursor to first record in current index
	JET_ERR move_first()
	{
		return JetMove(_session->get_sesid(), _tableid, JET_MoveFirst, JET_bitMoveKeyNE);
	}
	// Move cursor to last record in current index
	JET_ERR move_last()
	{
		return JetMove(_session->get_sesid(), _tableid, JET_MoveLast, JET_bitMoveKeyNE);
	}
	// Move cursor to next record in current index
	JET_ERR move_next(bool notEqual=false)
	{
		return JetMove(_session->get_sesid(), _tableid, JET_MoveNext, notEqual ? JET_bitMoveKeyNE : JET_bitNil);
	}
	// Move cursor to previous record in current index
	JET_ERR move_previous(bool notEqual=false)
	{
		return JetMove(_session->get_sesid(), _tableid, JET_MovePrevious, notEqual ? JET_bitMoveKeyNE : JET_bitNil);
	}
	// Move cursor to by specified offset within current index
	JET_ERR move_offset(long cRow, bool notEqual=false, JET_GRBIT grbit=JET_bitNil)
	{
		return JetMove(_session->get_sesid(), _tableid, cRow, notEqual ? JET_bitMoveKeyNE | grbit : grbit);
	}
	// Add a column to the table
	JET_ERR add_column(const char *colName, const JET_COLUMNDEF *colDef, const void* pvDefault, unsigned long cbDefault)
	{
		JET_COLUMNID cid;
		return JetAddColumn(_session->get_sesid(), _tableid, colName, colDef, pvDefault, cbDefault, &cid);
	}


private:
	DbSession *_session;  // this is the session the cursor is valid inside
	JET_TABLEID _tableid; // this is the cursor on a table
	DbTable *_table;	  // this is the description of the table 
	bool _updateInProgress; // this records whether the cursor is in PrepareUpdate mode
	bool _nullStringMode;
	bool _zeroBytesOnNullMode;

public:
	// For the duration of the existence of this object, place the cursor into a mode where
	// it has a specific behaviour on encountering null string fields - either it returns an error,
	// or it returns a null string
	// NOTE: tag fields cannot be null, so this does not apply to loading tag fields
	class NullStringOnNull
	{
	public:
		NullStringOnNull(DbCursor *cursor, bool on=true)
		{
			_cursor = cursor;
			_originalValue = _cursor->get_null_strings_allowed();
			_cursor->set_null_strings_allowed(on);
		}

		~NullStringOnNull(void)
		{
			_cursor->set_null_strings_allowed(_originalValue);
		}

		DbCursor *_cursor;
		bool _originalValue;
	};

	// For the duration of the existence of this object, place the cursor into a mode where 
	// it has a specific behaviour on encountering null byte fields - either it returns an error,
	// or it zeros the buffer supplied
	// NOTE: tag fields cannot be null, so this does not apply to loading tag fields
	class ZeroedBytesOnNull
	{
	public:
		ZeroedBytesOnNull(DbCursor *cursor, bool on=true)
		{
			_cursor = cursor;
			_originalValue = cursor->get_zero_bytes_on_null();
			_cursor->set_zero_bytes_on_null(on);
		}

		~ZeroedBytesOnNull(void)
		{
			_cursor->set_zero_bytes_on_null(_originalValue);
		}

		DbCursor *_cursor;
		bool _originalValue;
	};
};
