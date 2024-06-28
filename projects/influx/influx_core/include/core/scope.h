#pragma once

#include <string>

namespace influx
{
	inline void begin_event(const std::string& str)
	{

	}

	inline void end_event()
	{

	}

	class scoped_event final
	{
	public:
		inline scoped_event(const std::string& str)
		{
			begin_event(str);
		}

		inline ~scoped_event()
		{
			end_event();
		}
	};
}

#define influx_scope_function() influx::scoped_event scoped_ev_{__FUNCTION__};
#define influx_scope(string) influx::scoped_event scoped_ev_{string};