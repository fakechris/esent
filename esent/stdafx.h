// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <Windows.h>

//#define  JET_UNICODE
typedef __nullterminated char *  JET_PSTR;    /* ASCII string (char *) null terminated */
typedef __nullterminated const char *  JET_PCSTR;   /* const ASCII string (char *) null terminated */
typedef __nullterminated WCHAR * JET_PWSTR;   /* Unicode string (char *) null terminated */
typedef __nullterminated const WCHAR * JET_PCWSTR;  /* const Unicode string (char *) null terminated */
#include "esent.h"

#include <exception>
#include <map>
#include <string>
#include <vector>
using namespace std;

#include <assert.h>

typedef wchar_t WTDB_CHAR;
typedef const wchar_t* WTDB_STR ;
typedef ULONG WTDB_ERR;
typedef const wchar_t WTDB_CONST_CHAR;

#include "StringUtil.h"
// TODO: reference additional headers your program requires here
