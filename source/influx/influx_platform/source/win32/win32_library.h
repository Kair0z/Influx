#pragma once
#include "influx_platform/library.h"

namespace influx::platform
{
	class win32_library final : public library
	{
		void* m_instance = nullptr;
		void* m_module = nullptr;
	public:
		win32_library(const string& path);
		~win32_library();

		virtual void* get_func_address(const string& func_name) override;
		virtual void call(const string& func_name) override;
	};
}