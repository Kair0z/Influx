#pragma once
#include "basetypes.h"
#include "string.h"

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
	inline void log(e_log_category category, const string& format, const _args&... args)
	{
		switch (category)
		{
		case e_log_category::normal: 
			std::cout << "[log] ";
			break;

		case e_log_category::warning:
			std::cout << "[warn] ";
			break;

		case e_log_category::error:
			std::cout << "[err] ";
			break;
		}
		
		std::cout << std::vformat(format, std::make_format_args(args...));
		std::cout << "\n";
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