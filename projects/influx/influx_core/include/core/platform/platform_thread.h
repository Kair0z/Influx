#pragma once

#include "core/string.h"

namespace influx::platform
{
	void set_current_thread_name(const string& name);

	string get_current_thread_name();
}