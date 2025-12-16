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
#include "core/debug.h"

// influx string
namespace influx
{
	using std_str	= std::string;
	using std_wstr	= std::wstring;
	using cstr		= const char*;
	using wcstr		= const wchar_t*;
	using chr		= char;
	using wchr		= wchar_t;

#pragma region string converters
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

	inline std_wstr to_wstring(const std_wstr& wstring)
	{
		return wstring;
	}

	inline std_wstr to_wstring(const char& chr)
	{
		return to_wstring(std_str(1, chr));
	}

	inline std_wstr to_wstring(const wchr& wchr)
	{
		return std_wstr(1, wchr);
	}

	inline std_str to_string(int i)
	{
		return std::to_string(i);
	}

	inline std_str to_string(uint32 i)
	{
		return std::to_string(i);
	}

	inline std_wstr to_wstring(int i) {
		return std::to_wstring(i);
	}

	inline std_wstr to_wstring(uint32 i) {
		return std::to_wstring(i);
	}

	inline std_wstr to_wstring(uint64 i) {
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
	inline static bool str_to_uint(const std_str& value, uint32& out_result)
	{
		out_result = std::stoul(value);
		return true;
	}
	
	template <typename _t>
	inline static bool from_string(const std_str& value, _t& out_result)
	{
		if constexpr (std::is_same<_t, float>())
			return str_to_float(value, out_result);
		else if constexpr (std::is_same<_t, int>())
			return str_to_int(value, out_result);
		else if constexpr (std::is_same<_t, uint32>())
			return str_to_uint(value, out_result);
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

	// influx string wrapper
	// it's a wstring, since that encapsulates UTF8 strings.
	// as a consequence though, getting a UTF8 representation can come with some overhead.
	// to combat this, we carry an additional UTF8 cache that updates when the string gets changed.
	class string final
	{
		std_wstr m_wstr{};
		std_str m_utf8_cache{};
		bool m_utf8_cache_dirty = true;

		void on_content_change()
		{
			m_utf8_cache_dirty = true;
		}

	public:
		static constexpr uint64 k_not_found = (uint64)-1;
		static constexpr uint64 k_max_length = (uint64)-1;
		static constexpr bool k_default_case_sensitive = true;
		static constexpr uint64 k_npos = std::wstring::npos;

		// construction (conversion from other types)
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
		string(const chr& chr) 
		{ 
			m_wstr = to_wstring(chr);
		}
		string(const wchr& wchr)
		{
			m_wstr = to_wstring(wchr);
		}
		string(std_wstr&& std_wstr) 
		{ 
			m_wstr = std_wstr; 
		}
		string(const int value) {
			m_wstr = to_wstring(value);
		}
		string(const uint32 value) {
			m_wstr = to_wstring(value);
		}
		string(const uint64 value) {
			m_wstr = to_wstring(value);
		}
		virtual ~string() = default;

		// access
		wchr& operator[](uint64 i)
		{ 
			return m_wstr[i];
		}
		const wchr& operator[](uint64 i) const
		{ 
			return m_wstr[i]; 
		}

		// implicit conversions
		operator const std_wstr&() const { return m_wstr; }
		operator std_wstr() const { return m_wstr; }
		explicit operator std_str() const { return to_string(m_wstr); }

		std_wstr& get_wstd()
		{ return m_wstr; }
		const std_wstr& get_wstd() const
		{ return m_wstr; }
		std_str get_std() const
		{ return to_string(m_wstr); }
		wcstr c_wstr() const
		{ return m_wstr.c_str(); }
		cstr c_str() const
		{
			// before you say anything...
			// this is the only time I will EVER const_cast a thing.
			// it's either this, or update our UTF8 cache every time we modify our string
			// I'd rather do it when it's used.
			string& non_const_this = *const_cast<string*>(this);
			if (m_utf8_cache_dirty)
				non_const_this.m_utf8_cache = to_string(m_wstr);

			non_const_this.m_utf8_cache_dirty = false;
			return m_utf8_cache.c_str();
		}

		// iterators
		std_wstr::const_iterator cbegin() const
		{ return m_wstr.cbegin(); }
		std_wstr::const_iterator cend() const
		{ return m_wstr.cend(); }
		std_wstr::iterator begin()
		{ return m_wstr.begin(); }
		std_wstr::iterator end()
		{ return m_wstr.end(); }

		uint64 size() const
		{ return m_wstr.size(); }

		bool empty() const
		{ return size() == 0u; }

		template <typename _c, uint64 _n, typename _f>
		static void foreach_char(const _c(&arr)[_n], _f&& fn) {
			for (uint64 i = 0; i < _n - 1; ++i) { // skip null terminator
				fn(arr[i]);
			}
		}

		template <typename _c, typename _f>
		static void foreach_char(const _c& arr, _f&& fn) {
			for (uint64 i = 0u; i < arr.size(); ++i) {
				fn(arr[i]);
			}
		}

		template <typename _t>
		static constexpr bool is_single_character()
		{
			return std::is_same<_t, char>() || std::is_same<_t, wchar_t>();
		}

		template <typename _t>
		static consteval bool is_cstring()
		{
			using U = std::remove_cvref_t<_t>;
			// case 1: char[N] or wchar_t[N]
			if constexpr (std::is_array_v<U>) {
				using E = std::remove_cv_t<std::remove_extent_t<U>>;
				return std::is_same_v<E, char> || std::is_same_v<E, wchar_t>;
			}
			// case 2: char* or wchar_t*
			else if constexpr (std::is_pointer_v<U>) {
				using E = std::remove_cv_t<std::remove_pointer_t<U>>;
				return std::is_same_v<E, char> || std::is_same_v<E, wchar_t>;
			}
			else {
				return false;
			}
		}

		static constexpr uint64 get_size(cstr s) {
			return s ? std::strlen(s) : 0u;
		}
		static constexpr uint64 get_size(wcstr s) {
			return s ? std::wcslen(s) : 0u;
		}
		static constexpr uint64 get_size(const std_str& str) {
			return str.size();
		}
		static constexpr uint64 get_size(const std_wstr& str) {
			return str.size();
		}
		static constexpr uint64 get_size(const chr chr) {
			return 1u;
		}
		static constexpr uint64 get_size(const wchr chr) {
			return 1u;
		}

		template <typename _t>
		uint64 find(const _t& value, bool case_sensitive = k_default_case_sensitive, uint64 range_begin = 0u) const
		{
			if constexpr (is_single_character<_t>())
			{
				return m_wstr.find((wchr)value, range_begin);
			}
			else
			{
				return m_wstr.find(to_wstring(value), range_begin);
			}
		}

		template <typename _t>
		uint64 find_last_of(const _t& value, uint64 range_begin = 0u) const
		{
			return m_wstr.find_last_of(value, range_begin);
		}

		string substr(const uint64 range_begin, const uint64 range_length = k_max_length) const
		{
			return m_wstr.substr(range_begin, range_length);
		}

		template <typename _t>
		bool contains(const _t& element, bool case_sensitive = k_default_case_sensitive) const
		{
			return find<_t>(element, case_sensitive) != std::string::npos;
		}

		static string make_lowercase(const string& str)
		{
			string result{};
			foreach_char(str, [&result](const wchr& ele) {
				result.append( (wchr)std::tolower(ele));
				});
			return result;
		}

		// < 0: a is less than b
		// = 0: a is equal to b
		// > 0: a is greater than b
		static int compare(const string& a, const string& b, const bool case_sensitive = k_default_case_sensitive)
		{
			if (case_sensitive == false)
			{
				string acpy = make_lowercase(a);
				string bcpy = make_lowercase(b);
				return std::wcscmp(acpy.c_wstr(), bcpy.c_wstr());
			}
			return std::wcscmp(a.c_wstr(), b.c_wstr());
		}

		static bool is_equal(const string& a, const string& b, const bool case_sensitive = k_default_case_sensitive)
		{
			if (get_size(a) != get_size(b))
				return false;

			return compare(a, b, case_sensitive) == 0;
		}

		int compare(const string& other, const bool case_sensitive = k_default_case_sensitive) const
		{
			return compare(*this, other, case_sensitive);
		}

		bool is_equal(const string& other, const bool case_sensitive = k_default_case_sensitive) const
		{
			return is_equal(*this, other, case_sensitive);
		}

		string get_lowercase() const
		{
			return make_lowercase(*this);
		}

		template <typename _t>
		vector<string> split(const _t& delim) const
		{
			vector<string> tokens;

			if constexpr (is_single_character<_t>()) {
				string delim_str{ delim };
				return split(delim_str);
			}
			else if constexpr (is_cstring<_t>()) {
				string delim_str = delim;
				return split(delim_str);
			}
			else { 
				// guessing here we're dealing with some string container that has a function .size()
				size_t start = 0, end;
				while ((end = find(delim, true, start)) != k_not_found)
				{
					tokens.push_back(substr(start, end - start));
					start = end + delim.size();
				}
				tokens.push_back(substr(start));
			}
			return tokens;
		}

		template <typename _t>
		string& insert(const uint64 target_index, const _t& value)
		{
			const uint64 size_before = this->size();
			influx_assert(target_index < size_before);

			const string prefix = substr(0u, target_index);
			const string postfix = substr(target_index, size_before - target_index);
			*this = prefix;
			this->append(value);
			this->append(postfix);

			on_content_change();
			return *this;
		}

		template <typename _t>
		void append(const _t& element)
		{
			on_content_change();
			m_wstr.append(string(element));
		}

		void clear()
		{
			on_content_change();
			m_wstr = {};
		}

		template <typename _t>
		string& operator+=(const _t& element) { this->append<_t>(element); return *this; }
	};
	static bool operator==(const string& a, const string& b)
	{
		return string::is_equal(a, b, string::k_default_case_sensitive);
	}
	static bool operator!=(const string& a, const string& b)
	{
		return !(a == b);
	}
	
	template <typename _t>
	inline string operator+(const string& str, const _t& element)
	{
		string combined = str;
		combined.append<_t>(element);
		return combined;
	}
}
template <> struct std::hash<influx::string>
{
	inline influx::uint64 operator()(const influx::string& str) const
	{
		return std::hash<influx::std_wstr>()(str.get_wstd());
	}
};


// DEBUG NAME
// string that is only represented as string in debug
namespace influx 
{
#if 0
	inline void format_hash(wcstr& cstr)
	{
		static constexpr uint64 k_num_digits_per_uint64 = 20;
		static constexpr uint64 k_hashcstr_buffersize = k_num_digits_per_uint64 + 1u;

		int written = std::swprintf(cstr, sizeof(m_hash_cstr) / sizeof(wchar_t),
			L"%llu", static_cast<unsigned long long>(m_hash));
		if (written < 0 || written >= sizeof(m_hash_cstr))
		{
			// todo: Handle error (buffer too small or formatting failed)
		}
	}
#endif

#define OMIT_STRINGS 1
	class debug_name final
	{
	private:
		static constexpr uint64 k_invalid = (uint64)-1;
		uint64 m_hash = k_invalid;
#if !OMIT_STRINGS
		string m_string = "";
#endif

	public:
		debug_name() = default;
		debug_name(cstr cstr)
		{
			set(cstr);
		}
		debug_name(wcstr wcstr)
		{
			set(wcstr);
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
#if !OMIT_STRINGS
			m_string = to_string(hash);
#endif
		}

		inline void set(cstr cstr)
		{
			set(string(cstr));
		}

		// set the string manually
		inline void set(const string& str)
		{
			m_hash = std::hash<string>()(str);
#if !OMIT_STRINGS
			m_string = str;
#endif
		}

		inline bool is_empty() const
		{
			return m_hash == 0u;
		}
		
		inline uint64 get_hash() const
		{
			return m_hash;
		}

		inline string get_string() const
		{
#if !OMIT_STRINGS
			return m_string;
#else
			return "";
#endif		
		}

		inline wcstr get_wcstr() const
		{
#if !OMIT_STRINGS
			return m_string.c_wstr();
#else
			return L"";
#endif
		}

		inline bool is_valid() const
		{
			return m_hash != k_invalid;
		}

		inline operator wchar_t const* () const
		{ 
			return get_wcstr();
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
