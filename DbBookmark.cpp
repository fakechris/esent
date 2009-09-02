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
#include "DbBookmark.h"
#include "DbEngine.h"
#include "DbCursor.h"

#ifndef JET_paramKeyMost
#define JET_paramKeyMost						134
#endif

DbBookmark::DbBookmark(DbSession *session)
: _session(session), _bookmark(NULL), _size(0)
{
	_cbMax = JET_cbBookmarkMost;

	// In Vista/2008 max bookmark size can grow beyond JET_cbBookmarkMost. Adjust if necessary!
	// See JetGetBookmark() for more info
	int paramKeyMost;
	JET_ERR rc = session->get_engine()->get_system_parameter(session, JET_paramKeyMost, (JET_API_PTR *)&paramKeyMost, NULL, 0);
	if (rc == JET_errSuccess)
	{
		_cbMax = (paramKeyMost+1) > (int)_cbMax ? (paramKeyMost+1) : _cbMax;
	}
	else if (rc != JET_errInvalidParameter) // i.e. not XP/2003
	{
		ERR_LOG( rc )
	}

	_bookmark = new unsigned char[_cbMax];
}

DbBookmark::~DbBookmark(void)
{
	delete []_bookmark;
}

JET_ERR
DbBookmark::set_position(DbCursor *cursor)
{
	JET_ERR rc;

	ERR_RET( JetGotoBookmark(_session->get_sesid(), cursor->get_tableid(), _bookmark, _size) )

	return JET_errSuccess;
}
