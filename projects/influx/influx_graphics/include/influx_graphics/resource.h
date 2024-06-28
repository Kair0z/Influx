#pragma once
#include "influx_graphics/base.h"
#include "influx_graphics/common.h"

// core dependencies
#include "core/basetypes.h"
#include "core/math/vector.h"

namespace influx::graphics
{
	class command_list;

	enum class e_resource_flags : uint8
	{
		none,
		max
	};

	enum class e_resource_state : uint8
	{
		none,
		render_target,
		copy_source,
		copy_dest,
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
		e_format get_format() const;

		uint32 get_width() const;

		uint32 get_height() const;

		e_resource_state get_state() const;

		e_resource_state get_previous_state() const;

		void transition(command_list* cmdlist, e_resource_state new_state);
		void revert_transition(command_list* cmdlist);

		virtual ~resource() = default;

	protected:
		resource() = default;
		resource(const tex2D_desc& desc);
		resource(const buffer_desc& desc);

	private:
		tex2D_desc m_tex2D_desc{};
		buffer_desc m_buffer_desc{};
		e_resource_state m_previous_state = e_resource_state::none;
		e_resource_state m_state = e_resource_state::none;
	};
}