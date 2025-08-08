#pragma once
#include "influx_platform/platform.h"

// influx::core
#include "core/string.h"

namespace influx::platform
{
	class library
	{
	public:
		INFLUX_PLATFORM_API 
		static library* load(const string& path);

		INFLUX_PLATFORM_API 
		static void free(library*);

		const vector<string>& get_functions() const
		{
			return m_functions;
		}

		virtual void* get_func_address(const string& func_name) = 0;

		virtual void call(const string& func_name) = 0;

		virtual ~library() = default;

	protected:
		vector<string> m_functions;
		string m_name;
		string m_path;
	};
}