#pragma once

#ifndef __CORE_STRING_H_
#define __CORE_STRING_H_

#include <string>
#include <algorithm>
#include "Core/basetypes.h"

namespace influx
{
	using string = std::string;
	using wstring = std::wstring;

#pragma warning (push)
#pragma warning (disable : 4244)
	inline string to_string(const wstring& wstring)
	{
		std::string res(wstring.length(), ' ');
		std::copy(wstring.cbegin(), wstring.cend(), res.begin());
		return res;
	}

	inline wstring to_wstring(const string& string)
	{
		std::wstring res(string.length(), L' ');
		std::copy(string.cbegin(), string.cend(), res.begin());
		return res;
	}

	inline string to_string(int i)
	{
		return std::to_string(i);
	}

	inline string to_string(uint32 i)
	{
		return std::to_string(i);
	}

	inline string to_string(uint64 i)
	{
		return std::to_string(i);
	}

#pragma warning (pop)
}

#endif
