#pragma once
#include "influx_graphics/base.h"
#include "influx_graphics/common.h"

// core dependencies
#include "core/basetypes.h"
#include "core/math/vector.h"

namespace influx::graphics
{
	enum class e_resource_flags : uint8
	{
		none,
		max
	};

	enum class e_resource_state : uint8
	{
		none,
		render_target,
		present,
		count
	};

	struct buffer_desc final
	{

	};

	struct tex2D_desc final
	{
		e_format m_format = e_format::rgba8;
		math::vectoru2 m_dimensions = { 64u, 64u };
		uint16 m_arraysize = 1u;
		uint16 m_num_mips = 1u;
		uint32 m_sample_count = 1u;
		e_resource_flags m_flags = e_resource_flags::none;
	};

	class resource : public base
	{
	public:
		inline e_format get_format() const
		{
			return m_tex2D_desc.m_format;
		}

		inline uint32 get_width() const
		{
			return m_tex2D_desc.m_dimensions.x;
		}

		inline uint32 get_height() const
		{
			return m_tex2D_desc.m_dimensions.y;
		}

		inline e_resource_state get_state() const
		{
			return m_state;
		}

	protected:
		inline resource(const tex2D_desc& desc)
			: m_tex2D_desc{desc}
			, m_buffer_desc{}
		{

		}

		inline resource(const buffer_desc& desc)
			: m_buffer_desc{desc}
			, m_tex2D_desc{}
		{

		}

	private:
		tex2D_desc m_tex2D_desc{};
		buffer_desc m_buffer_desc{};
		e_resource_state m_state = e_resource_state::none;
	};
}