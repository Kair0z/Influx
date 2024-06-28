#pragma once

namespace influx::renderer
{
	class rendersystem
	{
		friend class renderer_backend;

		virtual void initialize() = 0;

		virtual void tick() = 0;

	protected:
		rendersystem() = default;
	};
}