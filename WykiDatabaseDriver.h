#pragma once

#define WTDB_errWyki_Driver_Error -5000
#define WTDB_errError_Lost_Update -5001
#define WTDB_errPost_Has_Children -5002
#define WTDB_errNot_Implemented -5003
#define WTDB_errRuntime_Lib_Error -5004
#define WTDB_wrnPost_Created_Not_Set -5005
#define WTDB_errSchema_Error -5006

class wyki_schema_error : public exception
{
private:
	wstring w;
public:
	wyki_schema_error(wstring& w) : w(w)
	{
	}

	virtual const char* what() const throw()
	{
		return StringUtil::to_string(w).c_str();
	}
};

class wyki_win32_error : public exception
{
private:
	wstring w;
public:
	wyki_win32_error(wstring w) : w(w)
	{
	}

	virtual const char* what() const throw()
	{
		return StringUtil::to_string(w).c_str();
	}
};

class wyki_resource_timeout_error : public exception
{
private:
	wstring w;
public:
	wyki_resource_timeout_error(wstring w) : w(w)
	{
	}

	virtual const char* what() const throw()
	{
		return StringUtil::to_string(w).c_str();
	}
};

class wyki_database_error : public exception
{
private:
	wstring w;
	//JET_ERR err;
public:
	wyki_database_error(wstring w, JET_ERR err) : w(w) //, err(err)
	{
		wchar_t buf[16] = {0};
		_itow_s(err, buf, 10);
		w.append(L" ").append(buf);
	}

	virtual const char* what() const throw()
	{
		return StringUtil::to_string(w).c_str();
	}
};

namespace WykiDateTime
{
	inline void get_key_normalized_filetime(BYTE* b, FILETIME ft)
	{
	}

	inline void get_filetime_from_normalized_key(FILETIME* ft, BYTE* b)
	{
	}
}

struct WTDB_ENGINE_PARAMS
{
	const char * szEngineLogPath;
	const char * szDatabaseFilename;
	long maxSessions;
};
