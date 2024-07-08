#pragma once
#include "influx_graphics/base.h"
#include "influx_graphics/common.h"

// core dependencies
#include "core/basetypes.h"
#include "core/math/vector.h"
#include "core/function.h"

namespace influx::graphics
{
	class command_list;

	enum class e_heap_type : uint8
	{
		shared,		// this heap is shared between cpu and gpu (dx12: upload heap)
		gpu,		// gpu dedicated heap, cpu has no access (dx12: default heap)
		readback,	// heap accessible for reads from cpu
		count
	};

	struct heap_desc final
	{
		e_heap_type m_type = e_heap_type::gpu;
	};

	enum class e_resource_flags : uint8
	{
		none,
		max
	};

	enum class e_resource_state : uint8
	{
		common,
		render_target,
		copy_source,
		copy_dest,
		present,
		read,
		count
	};

	struct buffer_desc final
	{
		size_t m_bytesize;
		size_t m_bytestride;
		e_resource_flags m_flags;
		e_resource_state m_init_state;
		e_format m_format;
	};

	struct tex2D_desc final
	{
		e_format m_format = e_format::rgba8;
		math::vectoru2 m_dimensions = { 64u, 64u };
		uint16 m_arraysize = 1u;
		uint16 m_num_mips = 1u;
		uint32 m_sample_count = 1u;
		e_resource_flags m_flags = e_resource_flags::none;
		e_resource_state m_init_state = e_resource_state::common;
	};

	struct map_args final
	{
		uint32 m_subres = 0u;
		size_t m_begin = 0u;
		size_t m_end = (size_t)-1;
	};

	class resource : public base
	{
		virtual void* map(const map_args& args) = 0;
		virtual void unmap(const map_args& args) = 0;

	public:
		void map(const function<void(void*)> map_func, const map_args& args = {})
		{
			void* target = map(args);
			map_func(target);
			unmap(args);
		}

		e_format get_format() const;

		uint32 get_width() const;

		uint32 get_height() const;

		size_t get_bytesize() const;

		size_t get_bytestride() const;

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
		e_resource_state m_previous_state = e_resource_state::common;
		e_resource_state m_state = e_resource_state::common;
		size_t m_bytesize{};
		size_t m_bytestride{};
		e_format m_format{};
	};
}