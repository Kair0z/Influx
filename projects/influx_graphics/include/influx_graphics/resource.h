#pragma once
#include "influx_graphics.h"
#include "influx_graphics/base.h"

#include "core/math/vector.h"

namespace influx::graphics
{
	enum class e_resource_flags : uint8
	{
		none,
		max
	};

	struct buffer_desc final
	{

	};

	struct tex2D_desc final
	{
		e_format m_format = e_format::rgba8;
		math::vectoru2 m_dimensions = { 64u, 64u };
		uint16_t m_arraysize = 1u;
		uint16_t m_num_mips = 1u;
		uint32_t m_sample_count = 1u;
		e_resource_flags m_flags = e_resource_flags::none;
	};

	class resource : public base
	{
	public:

	protected:
		inline resource(const tex2D_desc& desc)
		{

		}

		inline resource(const buffer_desc& desc)
		{

		}
	};
}