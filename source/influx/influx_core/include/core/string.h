#pragma once

#ifndef __CORE_STRING_H_
#define __CORE_STRING_H_

#include <string>
#include <algorithm>
#include <sstream>
#include <cstdio> // snprintf
#include <cinttypes> // PRIu64

#include "core/container/vector.h"
#include "core/basetypes.h"

namespace influx
{
	using string = std::string;
	using wstring = std::wstring;
	
	namespace str
	{
		inline static vector<string> split(const string& str, const string& delimiter) 
		{
			vector<string> tokens;
			size_t start = 0, end;

			while ((end = str.find(delimiter, start)) != string::npos) 
			{
				tokens.push_back(str.substr(start, end - start));
				start = end + delimiter.length();
			}

			tokens.push_back(str.substr(start));
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

	inline wstring to_wstring(uint32 i)
	{
		return std::to_wstring(i);
	}

	inline string to_string(uint64 i)
	{
		return std::to_string(i);
	}

	inline string to_string(float value)
	{
		return std::to_string(value);
	}
#pragma warning (pop)

	inline static bool str_to_float(const string& value, float& out_result)
	{
		out_result = std::stof(value);
		return true;
	}
	inline static bool str_to_int(const string& value, int& out_result)
	{
		out_result = std::stoi(value);
		return true;
	}

	template <typename _t>
	inline static bool from_string(const string& value, _t& out_result)
	{
		if constexpr (std::is_same<_t, float>())
			return str_to_float(value, out_result);
		else if constexpr (std::is_same<_t, int>())
			return str_to_int(value, out_result);
		else
		{
			out_result = value;
		}
		return false;
	}

	// string that is only represented as string in debug
	class debug_name final
	{
	private:
		static constexpr uint64 k_invalid = (uint64)-1;
		static constexpr uint64 k_num_digits_per_uint64 = 20; 
		static constexpr uint64 k_hashcstr_buffersize = k_num_digits_per_uint64 + 1u;

#if INFLUX_DEBUG
		string m_string = "";
#endif
		uint64 m_hash = k_invalid;
		char m_hash_cstr[k_num_digits_per_uint64]{};

		inline void format_hash()
		{
			int written = std::snprintf(m_hash_cstr, sizeof(m_hash_cstr), "%" PRIu64, m_hash);
			if (written < 0 || written >= sizeof(m_hash_cstr))
			{
				// Handle error (buffer too small or formatting failed)
			}
		}

	public:
		debug_name() = default;
		debug_name(const char* cstr)
		{
			set(cstr);
		}
		debug_name(const string& str)
		{
			set(str);
		}

		// set the hash manually
		inline void set(uint64 hash)
		{
			m_hash = hash;
#if INFLUX_DEBUG
			m_string = to_string(hash);
#endif
			format_hash();
		}

		inline void set(const char* cstr)
		{
			set(string(cstr));
		}

		// set the string manually
		inline void set(const string& str)
		{
			m_hash = std::hash<string>()(str);
#if INFLUX_DEBUG
			m_string = str;
#endif
			format_hash();
		}

		inline bool is_empty() const
		{
#if INFLUX_DEBUG
			return m_string.empty();
#endif
			return m_hash == 0u;
		}
		inline uint64 get_hash() const
		{
			return m_hash;
		}
		inline string get_string() const
		{
			return get_cstr();
		}
		inline const char* get_cstr() const
		{
#if INFLUX_DEBUG
			return m_string.c_str();
#else
			return m_hash_cstr;
#endif
		}

		inline bool is_valid() const
		{
			return m_hash != k_invalid;
		}

		inline operator char const* () const
		{ 
			return get_cstr();
		}

		debug_name(const debug_name&) = default;
		debug_name(debug_name&&) = default;
		debug_name& operator=(const debug_name&) = default;
		debug_name& operator=(debug_name&&) = default;
		~debug_name() = default;
	};
	inline bool operator==(const debug_name& name1, const debug_name& name2)
	{
		return name1.get_hash() == name2.get_hash();
	}

}
template <> struct std::hash<influx::debug_name>
{
	inline influx::uint64 operator()(const influx::debug_name& res_name) const
	{
		return std::hash<decltype(res_name.get_hash())>()(res_name.get_hash());
	}
};

#endif
