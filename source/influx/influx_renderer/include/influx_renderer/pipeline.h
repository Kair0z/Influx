#pragma once

#include "influx_renderer/common.h"

namespace influx::renderer
{
	using pipeline_id = uint32;

	class pipeline final
	{
	public:
		INFLUX_RENDER_API static result<pipeline_id> parse(const string& filepath);

	private:

	};
}