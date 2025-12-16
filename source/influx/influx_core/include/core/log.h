#pragma once
// influx::core
#include "core/basetypes.h"
#include "core/string.h"

// STL
#include <iostream>
#include <format>

namespace influx
{
	enum class e_log_category : uint8
	{
		normal,
		warning,
		error
	};

	template <typename ..._args>
	inline string log(e_log_category category, const string& format, const _args&... args)
	{
		string prefix = "";
		switch (category)
		{
		case e_log_category::normal: 
			prefix = "[log] ";
			break;

		case e_log_category::warning:
			prefix = "[warn] ";
			break;

		case e_log_category::error:
			prefix = "[err] ";
			break;
		}
		
		const string log_string = prefix + std::vformat(format.get_std(), std::make_format_args(args...));
		wprintf( string(log_string + "\n").c_wstr() );
		return log_string;
	}

	template <typename ..._args>
	inline void logn(const string& format, const _args&... args)
	{
		log(e_log_category::normal, format, args...);
	}

	template <typename ..._args>
	inline void logerr(const string& format, const _args&... args)
	{
		log(e_log_category::error, format, args...);
	}

	template <typename ..._args>
	inline void logwar(const string& format, const _args&... args)
	{
		log(e_log_category::warning, format, args...);
	}

#define logonce(cat, format, ...) { static bool once = true; if (once == true) { once = false; log(cat, format, __VA_ARGS__); } }
}