#pragma once

#include <string>
#include <algorithm>
namespace Influx
{
	// TODO: Make own string once that's necessary enough ;)
	using String = std::string;
	using WString = std::wstring;

#pragma warning (push)
#pragma warning (disable : 4244)
	inline String ToString(const WString& wstring)
	{
		std::string res(wstring.length(), ' ');
		std::copy(wstring.cbegin(), wstring.cend(), res.begin());
		return res;
	}

	inline WString ToWString(const String& string)
	{
		std::wstring res(string.length(), L' ');
		std::copy(string.cbegin(), string.cend(), res.begin());
		return res;
	}
#pragma warning (pop)
}


