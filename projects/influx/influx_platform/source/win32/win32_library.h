#pragma once
#include "influx_platform/library.h"

namespace influx::platform
{
	class win32_library final : public library
	{
	public:
		win32_library(const string& path);
		~win32_library();

		virtual void call(const string& func_name) override;
	};
}