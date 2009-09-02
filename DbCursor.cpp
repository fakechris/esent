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
#include "DbCursor.h"
#include <Utility/Basic_Buffer.h>
#include <Utility/Basic_Output_Buffer.h>
#include <Utility/Basic_Output_Deflate.h>
#include <Utility/Basic_Input_Buffer.h>
#include <Utility/Basic_Input_Inflate.h>

JET_ERR
DbCursor::get_column_tag_str_array(const char *colName, WTDB_STR** pvString, unsigned long capacity)
{
	JET_ERR rc;

	*pvString = NULL;

	Objects_Holder<WTDB_CONST_CHAR> ltags;
	ltags.reserve(capacity);
	unsigned long jetIdx = 1;
	while (true)
	{
		WTDB_STR strp;
		rc = get_column_tag_str(colName, &strp, jetIdx);
		auto_ptr<WTDB_CONST_CHAR> s(strp);

		if (rc == JET_errSuccess)
		{
			ltags.push_back(s.get());
			s.release();
		}
		else if (rc == JET_wrnColumnNull)
		{
			break;
		}
		else
		{
			ERR_RET( rc )
		}
		jetIdx++;
	}

	*pvString = ltags.release();

	return JET_errSuccess;
}

JET_ERR
DbCursor::get_column_tag_array(const char *colName, BYTE*** ppData, vector<unsigned long> *lengths, unsigned long capacity, unsigned long reqTagLength)
{
	JET_ERR rc;

	*ppData = NULL;

	lengths->reserve(capacity);

	Objects_Holder<BYTE> tags;
	tags.reserve(capacity);
	unsigned long jetIdx = 1;
	while (true)
	{
		BYTE *tag;
		unsigned long tag_size;
		rc = get_column_tag(colName, &tag, &tag_size, jetIdx);
		auto_ptr<BYTE> t(tag);

		if (rc == JET_errSuccess)
		{
			if (reqTagLength && tag_size != reqTagLength)
			{
				throw wyki_schema_error(wstring(L"Tag value stored for field is invalid length"));
			}
			lengths->push_back(tag_size);
			tags.push_back(t.get());
			t.release();
		}
		else if (rc == JET_wrnColumnNull)
		{
			break;
		}
		else
		{
			ERR_RET( rc )
		}
		jetIdx++;
	}

	*ppData = tags.release();

	return JET_errSuccess;
}


JET_ERR
DbCursor::get_column_zlib_tag_str_array(const char *colName, WTDB_STR** pvString, unsigned long capacity)
{
	JET_ERR rc;

	*pvString = NULL;

	Objects_Holder<WTDB_CONST_CHAR> ltags;
	ltags.reserve(capacity);
	unsigned long jetIdx = 1;
	while (true)
	{
		WTDB_STR strp;
		rc = get_column_zlib_tag_str(colName, &strp, jetIdx);
		auto_ptr<WTDB_CONST_CHAR> s(strp);

		if (rc == JET_errSuccess)
		{
			ltags.push_back(s.get());
			s.release();
		}
		else if (rc == JET_wrnColumnNull)
		{
			break;
		}
		else
		{
			ERR_RET( rc )
		}
		jetIdx++;
	}

	*pvString = ltags.release();

	return JET_errSuccess;
}

JET_ERR
DbCursor::set_column_zlib(const char *colName, const void* pvData, unsigned long cbData, JET_GRBIT grbit, JET_SETINFO *info)
{
	JET_ERR rc;

	Basic_Buffer buf;
	Basic_Output_Buffer out(&buf);
	{
		Basic_Output_Deflate deflate(&out, false);
		assert(deflate.begin_compressing() != -1);
		unsigned long written;
		assert(deflate.write_n(pvData, cbData, &written) != -1);
		assert(deflate.flush() != -1);
		assert(deflate.finish_compressing() != -1);
	}

	JET_COLUMNID colid = _table->get_schema()->get_col(colName)->get_colid();

	ERR_RET( JetSetColumn(_session->get_sesid(), _tableid, colid, buf.get_buffer(), buf.value_size(), grbit, info) )

	return JET_errSuccess;
}

JET_ERR
DbCursor::get_column_zlib_str(const char *colName, WTDB_STR* pString, JET_GRBIT grbit, JET_RETINFO *pretinfo)
{
	JET_ERR rc;
	*pString = NULL;

	BYTE *zValue;
	unsigned long size;
	rc = get_column(colName, &zValue, &size, grbit, pretinfo);
	if (rc == JET_wrnColumnNull)
	{
		if (_nullStringMode && !pretinfo)
		{
			*pString = NULL;
			return JET_errSuccess;
		}
		return rc; // do not trigger handle tracking code, as this condition is expected
	}
	else
	{
		ERR_RET( rc )
	}

	Basic_Buffer zBuf(zValue, size);
	Basic_Input_Buffer in(&zBuf);
	Basic_Buffer str;
	{
		// TO DO: Something strange, unless we scope inflate thus, if we do not call the inflate.finish_decompression()
		// method, which is not strictly necessary here and is rather designed for emptying network streams,
		// then we quickly get failure of inflate.try_read in release mode (not in debug though). It seems that somehow
		// the zlib library relies upon some of the buffers passed from the previous items, or something, and we
		// have not got destructor orders right.
		Basic_Input_Inflate inflate(&in, false);

		assert(inflate.begin_decompression() != -1);
		unsigned long read;
		do
		{
			if (str.get_spare_capacity_size() <= 0) // only increase buffer size when we have to!
			{
				assert(str.set_spare_capacity_size(4096) != -1);
			}
			assert(inflate.try_read(str.get_spare_capacity(), str.get_spare_capacity_size(), &read) != -1);
			str.append_spare_capacity_to_data(read);
		}
		while (!inflate.is_finished() && read != 0);
		assert(inflate.finish_decompression() != -1);
	}

	assert(str.zero_terminate(sizeof(WTDB_CHAR)) != -1);
	assert(str.minimize_capacity() != -1);
	
	*pString = (WTDB_STR)str.release();

	return JET_errSuccess;
}

JET_ERR
DbCursor::set_column_norm_ulong(const char *colName, unsigned long ul)
{
	unsigned long normUl = MemoryUtility::key_normalize(ul);
	return set_column(colName, &normUl, sizeof(normUl));
}

JET_ERR
DbCursor::set_column_norm_ulonglong(const char *colName, unsigned long long ull)
{
	unsigned long long normUll = MemoryUtility::key_normalize(ull);
	return set_column(colName, &normUll, sizeof(normUll));
}

JET_ERR
DbCursor::set_column_norm_filetime(const char *colName, FILETIME ft, JET_GRBIT grbit, JET_SETINFO *pretinfo)
{
	BYTE normFILETIME[8];
	WykiDateTime::get_key_normalized_filetime(normFILETIME, ft);
	return set_column(colName, normFILETIME, sizeof(normFILETIME), grbit, pretinfo);
}

JET_ERR
DbCursor::set_column_norm_filetime_tag(const char *colName, FILETIME ft, unsigned long tagSeqNo, JET_GRBIT grbit)
{
	JET_SETINFO info;
	info.cbStruct = sizeof(info);
	info.ibLongValue = 0;
	info.itagSequence = tagSeqNo;

	return set_column_norm_filetime(colName, ft, grbit, &info);
}

JET_ERR
DbCursor::get_column_norm_ulong(const char *colName, unsigned long *ul, JET_GRBIT grbit)
{
	JET_ERR rc;
	unsigned long normUl;
	
	rc = get_column(colName, &normUl, sizeof(normUl), NULL, grbit);
	if (rc == JET_wrnColumnNull) return rc;
	else ERR_RET( rc );

	*ul = MemoryUtility::key_denormalize(normUl);
	return JET_errSuccess;
}

JET_ERR
DbCursor::get_column_norm_ulonglong(const char *colName, unsigned long long *ull, JET_GRBIT grbit)
{
	JET_ERR rc;
	unsigned long long normUll;
	
	rc = get_column(colName, &normUll, sizeof(normUll), NULL, grbit);
	if (rc == JET_wrnColumnNull) return rc;
	else ERR_RET( rc );

	*ull = MemoryUtility::key_denormalize(normUll);
	return JET_errSuccess;
}

JET_ERR
DbCursor::get_column_norm_filetime(const char *colName, FILETIME *ft, JET_GRBIT grbit, JET_RETINFO *pretinfo)
{
	JET_ERR rc;
	BYTE normFILETIME[8];
	
	rc = get_column(colName, normFILETIME, sizeof(normFILETIME), NULL, grbit, pretinfo);
	if (rc == JET_wrnColumnNull) return rc;
	else ERR_RET( rc );

	WykiDateTime::get_filetime_from_normalized_key(ft, normFILETIME);
	return JET_errSuccess;
}

JET_ERR
DbCursor::get_column_norm_filetime_tag(const char *colName, FILETIME *ft, unsigned long tagSeqNo, JET_GRBIT grbit)
{
		JET_ERR rc;

		JET_RETINFO pretinfo;
		pretinfo.cbStruct = sizeof(pretinfo);
		pretinfo.ibLongValue = 0;
		pretinfo.itagSequence = tagSeqNo;
		pretinfo.columnidNextTagged = 0;

		rc = get_column_norm_filetime(colName, ft, grbit, &pretinfo);
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