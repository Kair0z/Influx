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

// influx string
namespace influx
{
	using std_str	= std::string;
	using std_wstr	= std::wstring;
	using cstr		= const char*;
	using wcstr		= const wchar_t*;
	using chr		= char;
	using wchr		= wchar_t;

#pragma region string operations
#pragma warning (push)
#pragma warning (disable : 4244)
	inline std_str to_string(const std_wstr& wstring)
	{
		std_str res(wstring.length(), ' ');
		std::copy(wstring.cbegin(), wstring.cend(), res.begin());
		return res;
	}

	inline std_wstr to_wstring(const std_str& string)
	{
		std_wstr res(string.length(), L' ');
		std::copy(string.cbegin(), string.cend(), res.begin());
		return res;
	}

	inline std_str to_string(int i)
	{
		return std::to_string(i);
	}

	inline std_str to_string(uint32 i)
	{
		return std::to_string(i);
	}

	inline std_wstr to_wstring(uint32 i)
	{
		return std::to_wstring(i);
	}

	inline std_str to_string(uint64 i)
	{
		return std::to_string(i);
	}

	inline std_str to_string(float value)
	{
		return std::to_string(value);
	}
#pragma warning (pop)
	inline static bool str_to_float(const std_str& value, float& out_result)
	{
		out_result = std::stof(value);
		return true;
	}
	inline static bool str_to_int(const std_str& value, int& out_result)
	{
		out_result = std::stoi(value);
		return true;
	}
	template <typename _t>
	inline static bool from_string(const std_str& value, _t& out_result)
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
#pragma endregion

	class string;

	template <typename _t>
	static string operator+(const string& str, const _t& element);

	class string final
	{
		std_wstr m_wstr{};
		using citerator = std_wstr::const_iterator;
		using iterator = std_wstr::iterator;

	public:
		static constexpr uint64 k_not_found = (uint64)-1;
		static constexpr uint64 k_max_length = (uint64)-1;

		string() = default;
		string(const std_str& std_str)
		{
			m_wstr = to_wstring(std_str);
		}
		string(const std_wstr& std_wstr)
		{
			m_wstr = std_wstr;
		}
		string(const cstr& cstr)
		{
			m_wstr = to_wstring(std_str(cstr));
		}
		string(const wcstr& wcstr)
		{
			m_wstr = wcstr;
		}
		string(std_wstr&& std_wstr) { m_wstr = std_wstr; }
		virtual ~string() = default;

		// implicit conversion back to wstring type
		operator const std_wstr&() const { return m_wstr; }
		operator std_wstr() const { return m_wstr; }
		operator std_str() const { return to_string(m_wstr); }
#if 0
		explicit operator cstr() const { return get_std().c_str(); }
		explicit operator wcstr() const { return m_wstr.c_str(); }
#endif

		citerator cbegin() const 
		{ return m_wstr.cbegin(); }
		citerator cend() const
		{ return m_wstr.cend(); }
		iterator begin()
		{ return m_wstr.begin(); }
		iterator end()
		{ return m_wstr.end(); }

		wchr& operator[](uint64 i)
		{ return m_wstr[i]; }

		uint64 size() const
		{
			return m_wstr.size();
		}

		string get_lowercase() const
		{
			return make_lowercase<string, wchr>(*this);
		}

		template <typename _t>
		uint64 find(const _t& value, uint64 range_begin = 0u) const
		{
			return k_not_found;
		}

		template <typename _t>
		uint64 find_last_of(const _t& value, uint64 range_begin = 0u) const
		{
			return k_not_found;
		}

		string substr(const uint64 range_begin, const uint64 range_length = k_max_length) const
		{
			const string& source = *this;
			string result{};
			// memcpy(&result[0], &source)
			return result;
		}

		template <typename _t>
		bool contains(const _t& element, bool case_sensitive) const
		{
			if (case_sensitive)
				return find(element) != std::string::npos;
			else
			{
				string low_a = this->get_lowercase();
				string low_b = element.get_lowercase();
				return low_a.find(low_b) != std::string::npos;
			}
		}

		bool empty() const
		{ return size() == 0u; }

		template <typename _t>
		static constexpr bool is_single_character()
		{ return std::is_same<_t, char>() || std::is_same<_t, wchar_t>(); }

		template <typename _t>
		static constexpr bool is_cstring()
		{ return std::is_same<_t, cstr>() || std::is_same<_t, wcstr>(); }

		template <typename _t, typename _func>
		static const void foreach(const _t& values, _func&& func)
		{
			if constexpr (is_single_character<_t>()) {
				func(values);
			}
			else if constexpr (is_cstring<_t>()) {
				
			}
		}

		template <typename _c, uint64 _n, typename F>
		constexpr void for_each_char(const _c(&arr)[N], F&& fn) {
			for (std::size_t i = 0; i < N - 1; ++i) { // skip null terminator
				fn(arr[i]);
			}
		}


		template <typename _t, typename _el>
		static string make_lowercase(const _t& str)
		{
			string result{};
			iterate<_t>([&result](const _el& ele) {
					result.add(ele);
				});

			string lower_str = m_wstr;
			for (uint32 i = 0u; i < lower_str.size(); ++i)
			{
				lower_str[i] = std::tolower(lower_str[i]);
			}
			return lower_str;
		}

		template <typename _t>
		vector<string> split(_t& delim) const
		{
			vector<string> tokens;

			if constexpr (is_single_character<_t>()) {
				// todo...
			}
			else if constexpr (is_cstring<_t>()) {
				// todo...
			}
			else {
				size_t start = 0, end;
				while ((end = find(delim, start)) != k_not_found)
				{
					tokens.push_back(substr(start, end - start));
					start = end + delim.size();
				}
				tokens.push_back(substr(start));
			}
			return tokens;
		}

		template <typename _t>
		string insert(const uint64 target_index, const _t& value)
		{
			string result{};
			return result;
		}

		template <typename _t>
		void add(const _t& element)
		{
			if constexpr (std::is_same<_t, char>() || std::is_same<_t, wchar_t>()) {
				const uint64 new_size = m_wstr.size() + 1u;
				m_wstr.resize(new_size);
				m_wstr[new_size - 1u] = element;
			}
			else if constexpr (std::is_same<_t, cstr>() || std::is_same<_t, wcstr>()) {
				uint64 cstr_size = 0u;
				while (element[cstr_size] != '\0') cstr_size++;
				const uint64 old_size = m_wstr.size();
				const uint64 new_size = old_size + cstr_size;
				m_wstr.resize(new_size);
				memcpy(&m_wstr[old_size], &element, cstr_size);
			}
			else if constexpr (std::is_same<_t, std_str>() || std::is_same<_t, std_wstr>()) {
				const uint64 old_size = m_wstr.size();
				const uint64 new_size = old_size + element.size();
				m_wstr.resize(new_size);
				memcpy(&m_wstr[old_size], &element, sizeof(element));
			}
			else {
				static_assert("unsupported!");
			}
		}

		std_wstr& get_std_w()
		{ return m_wstr; }

		const std_wstr& get_std_w() const
		{ return m_wstr; }

		std_str get_std() const
		{ return to_string(m_wstr); }

		cstr c_str() const
		{ return to_string(m_wstr).c_str(); }

		wcstr c_wstr() const
		{ return m_wstr.c_str(); }

		template <typename _t>
		string& operator+=(const _t& element) { this->add<_t>(element); return *this; }
	};
	using wstring = string;
	
	template <typename _t>
	inline string operator+(const string& str, const _t& element)
	{
		string combined = str;
		combined.add<_t>(element);
		return combined;
	}
}
template <> struct std::hash<influx::string>
{
	inline influx::uint64 operator()(const influx::string& str) const
	{
		return std::hash<influx::std_wstr>()(str.get_std_w());
	}
};


// DEBUG NAME
// string that is only represented as string in debug
namespace influx 
{	
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
		debug_name(const std_str& str)
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
