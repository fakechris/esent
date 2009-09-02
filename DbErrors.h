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

class DbErrors
{
public:
	DbErrors(void);
	virtual ~DbErrors(void);

	WTDB_STR get_error(WTDB_ERR errcode)
	{
		if (emap.find(errcode) == emap.end())
		{
			return L"Internal error (cannot find appropriate error code)";
		}
		return emap[errcode];
	}
	
private:
	typedef map<WTDB_ERR, WTDB_STR> ERRORMAP;
	ERRORMAP emap;
};

extern DbErrors *wtdbErrors;

// ERR_RET() macro
// The ERR_RET macro makes it easy/elegant in the code to test for a JET protocol function call being unsuccessful,
// and then to return that error code. Furthermore, in the special debug version of the macro, a simple exception
// is thrown and immediately caught when it triggers. This makes it possible to catch in the debugger exactly the
// point where an error is being returned through functional programming return statements. Make sure that you
// check VS->Debug->Exceptions...->C++ Exceptions Thrown[x] if you want to catch where the ERR_RET statements trigger
#ifdef _DEBUG
class ERR_RET_Is_Returning_An_Error { public: ERR_RET_Is_Returning_An_Error(){} };
#define ERR_RET(x) if ((rc = x) != JET_errSuccess) { try { throw ERR_RET_Is_Returning_An_Error(); } catch (...) { return rc; } }
#else
#define ERR_RET(x) if ((rc = x) != JET_errSuccess) return rc;
#endif

// ERR_LOG() macro
// The ERR_LOG macro is equivalent to ERR_RET except the error is logged rather than returned. This is useful in 
// places where the error cannot be returned. For example within destructors or error handling cleanup code within a
// catch statement.
#ifdef _DEBUG
class ERR_LOG_Is_Returning_An_Error { public: ERR_LOG_Is_Returning_An_Error(){} };
#define ERR_LOG(x) if ((rc = x) != JET_errSuccess) { try { throw ERR_LOG_Is_Returning_An_Error(); } catch (...) {} }
#else
#define ERR_LOG(x) if ((rc = x) != JET_errSuccess) {}
#endif

#define API_TRY_START \
	JET_ERR rc; \
	try {

#define API_CATCH_EXCEPTION \
		} \
	catch (wyki_lost_update_error &) \
	{ \
		return WTDB_errError_Lost_Update; \
	} \
	catch (wyki_schema_error &) \
	{ \
		return WTDB_errSchema_Error; \
	} \
	catch (wyki_runtime_error &) \
	{ \
		return WTDB_errWyki_Driver_Error; \
	} \
	catch (runtime_error &) \
	{ \
		return WTDB_errRuntime_Lib_Error; \
	} \
\
	return JET_errSuccess;
