#pragma once

template<typename T>
class Objects_Holder
{
public:
	void push_back(T* t) {}
	T** release() {return NULL;}
	void reserve(int s) {}
};

namespace StringUtil
{
	inline std::string to_string(std::wstring t) {
		std::string s;
		int len;

		// Get the converted length
		len = WideCharToMultiByte(CP_UTF8, 0, t.c_str(), t.length(), NULL, 0, NULL, NULL);
		s.resize(len);
		// Convert string
		WideCharToMultiByte(CP_UTF8, 0, t.c_str(), t.length(), (char *)s.c_str(), s.length(), NULL, NULL);

		return s;
	};

	inline std::wstring to_wide_string(std::string t) {
		std::wstring s;
		int len;

		// Get the converted length
		len = MultiByteToWideChar(CP_UTF8, 0, t.c_str(), t.length(), NULL, 0);
		s.resize(len);
		// Convert string
		MultiByteToWideChar(CP_UTF8, 0, t.c_str(), t.length(), (wchar_t *)s.c_str(), s.length());

		return s;
	};

	inline size_t get_byte_size(const wchar_t* buf)
	{
		int len;

		// Get the converted length
		len = WideCharToMultiByte(CP_UTF8, 0, buf, -1, NULL, 0, NULL, NULL);
		return len;
	}
}

namespace MemoryUtility
{
	inline unsigned long long key_normalize(unsigned long long l)
	{
		return l;
	}

	inline unsigned long long key_denormalize(unsigned long long l)
	{
		return l;
	}	
}