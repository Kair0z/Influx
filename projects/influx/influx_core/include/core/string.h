#pragma once

#ifndef __CORE_STRING_H_
#define __CORE_STRING_H_

#include <string>
#include <algorithm>
#include <sstream>

#include "core/container/vector.h"
#include "core/basetypes.h"

namespace influx
{
	using string = std::string;
	using wstring = std::wstring;

	namespace str
	{
		inline static vector<string> split(const string& str, char delim)
		{
			std::istringstream stream(str);
			vector<string> tokens;
			string token;

			while (std::getline(stream, token, delim)) 
			{
				tokens.push_back(token);
			}

			return tokens;
		}

		inline static string to_lower(const string& str) 
		{
			string lower_str = str;
			std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(),
				[](unsigned char c) { return std::tolower(c); });
			return lower_str;
		}

		inline static bool contains(const string& a, const string& b, bool case_sensitive = true)
		{
			if (case_sensitive) 
			{
				return a.find(b) != std::string::npos;
			}
			else 
			{
				string low_a = to_lower(a);
				string low_b = to_lower(b);
				return low_a.find(low_b) != std::string::npos;
			}
		}
	}

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

	// string that is only represented as string in debug
	class debug_name final
	{
	public:
#if INFLUX_DEBUG
		using name = string;
#else
		using name = uint8;
#endif

		debug_name() = default;
		debug_name(const debug_name&) = default;
		debug_name(debug_name&&) = default;
		debug_name& operator=(const debug_name&) = default;
		debug_name& operator=(debug_name&&) = default;
		~debug_name() = default;

#if INFLUX_DEBUG
		debug_name(const string& name) { set(name); }
		debug_name(const char* name) { set(name); }
#else
		debug_name(const string& name) { set(""); }
		debug_name(const char* name) { set(""); }
#endif

#if INFLUX_DEBUG
		inline void set(const name& name)
		{
			m_name = name;
		}

		inline const name& get() const
		{
			return m_name;
		}

		// treating this class as a string in non-debug config will result in a no-op
#else
		inline void set(const string& str)
		{

		}

		inline string get() const
		{
			return "";
		}
#endif

	private:
		name m_name;
	};
}

#endif
