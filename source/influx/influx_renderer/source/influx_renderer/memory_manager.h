#pragma once

namespace influx::renderer
{
	class memory_manager final
	{
	public:
		memory_manager() = default;

		result<> allocate_resource();
	};
}