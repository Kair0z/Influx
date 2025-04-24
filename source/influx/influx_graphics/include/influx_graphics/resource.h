#pragma once
#include "influx_graphics/base.h"
#include "influx_graphics/common.h"

// core dependencies
#include "core/basetypes.h"
#include "core/math/vector.h"
#include "core/function.h"
#include "core/range.h"
#include "core/enum.h"

namespace influx::graphics
{
	class commandlist;

	enum class e_heap_type : uint8
	{
		shared,		// this heap is shared between cpu and gpu (dx12: upload heap)
		gpu,		// gpu dedicated heap, cpu has no access (dx12: default heap)
		readback,	// heap accessible for reads from cpu
		count
	};

	struct heap_desc final
	{
		inline static heap_desc shared_heap()
		{
			heap_desc result{};
			result.m_type = e_heap_type::shared;
			return result;
		}

		e_heap_type m_type = e_heap_type::gpu;
	};

	enum class e_resource_flags : uint8
	{
		none,
		depth_stencil,
		render_target,
		count
	};

	struct buffer_desc final
	{
		uint64 m_bytesize;
		uint64 m_bytestride;
		e_bind_flags m_bindflags = e_bind_flags::none;
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
		e_bind_flags m_bindflags = e_bind_flags::none;
		e_resource_state m_init_state = e_resource_state::common;
		bool m_allow_uav = false;
	};

	struct tex3D_desc final
	{
		e_format m_format = e_format::rgba8;
		math::vectoru3 m_dimensions = { 64u, 64u, 64u };
		uint16 m_arraysize = 1u;
		uint16 m_num_mips = 1u;
		uint32 m_sample_count = 1u;
		e_bind_flags m_bindflags = e_bind_flags::none;
		e_resource_state m_init_state = e_resource_state::common;
		bool m_allow_uav = false;
	};

	struct cubemap_desc final
	{
		e_format m_format = e_format::rgba8;
		math::vectoru3 m_dimensions = { 64u, 64u, 64u };
		uint16 m_arraysize = 1u;
		uint16 m_num_mips = 1u;
		uint32 m_sample_count = 1u;
		e_bind_flags m_bindflags = e_bind_flags::none;
		e_resource_state m_init_state = e_resource_state::common;
		bool m_allow_uav = false;
	};

	struct acc_str_desc final
	{
		// e_acc_str_type m_type = e_acc_str_type::bottom;
		e_bind_flags m_bindflags = e_bind_flags::uav;
		uint64 m_bytesize = 0u;
		e_resource_state m_init_state = e_resource_state::common;
		uint64* m_update_scratch_size = nullptr;
	};

	struct map_args final
	{
		uint32 m_subres = 0u;
		uint64 m_begin = 0u;
		uint64 m_end = (uint64)-1;
	};

	class resource : public base
	{
	public:
		struct footprint final
		{
			uint32 m_subresource_index = 0u;
			uint32 m_num_rows = 0u;
			uint64 m_row_bytesize = 0u; // aka pitch
			uint64 m_bytesize = 0u;

			uint64 m_offset = 0u;
			graphics::e_format m_format;
			uint32 m_width;
			uint32 m_height;
			uint32 m_depth;
		};

		enum class e_type : uint8
		{
			tex2D,
			tex3D,
			cubemap,
			buffer,
			acc_struct,
			count
		};

	public:
		INFLUX_GFX_API virtual void* map(const map_args& args) = 0;
		INFLUX_GFX_API virtual void unmap(const map_args& args) = 0;
		INFLUX_GFX_API virtual bool allows_uav() const = 0;
		INFLUX_GFX_API virtual vector<footprint> get_footprints() const = 0;

		void map(const function<void(void*)> map_func, const map_args& args = {})
		{
			void* target = map(args);
			map_func(target);
			unmap(args);
		}

		INFLUX_GFX_API e_format get_format() const;

		INFLUX_GFX_API uint32 get_width() const;

		INFLUX_GFX_API uint32 get_height() const;

		INFLUX_GFX_API uint32 get_depth() const;

		INFLUX_GFX_API uint32 get_arraysize() const;

		INFLUX_GFX_API uint32 get_num_subresources() const;

		INFLUX_GFX_API uint64 get_bytesize() const;

		INFLUX_GFX_API uint64 get_bytestride() const;

		INFLUX_GFX_API uint32 get_num_elements() const;

		INFLUX_GFX_API e_resource_state get_state() const;

		INFLUX_GFX_API e_resource_state get_previous_state() const;

		INFLUX_GFX_API range<uint64> get_full_range() const;

		INFLUX_GFX_API result<> transition(commandlist* cmdlist, e_resource_state new_state);
		
		INFLUX_GFX_API result<> revert_transition(commandlist* cmdlist);

		INFLUX_GFX_API e_type get_type() const;

	protected:
		resource(const tex2D_desc& desc);
		resource(const buffer_desc& desc);
		resource(const tex3D_desc& desc);
		resource(const cubemap_desc& desc);
		resource(const acc_str_desc& desc);
		virtual ~resource() = default;
		
	private:
		e_type m_type{};
		tex2D_desc m_tex2D_desc{};
		buffer_desc m_buffer_desc{};
		tex3D_desc m_tex3D_desc{};
		acc_str_desc m_as_desc{};
		cubemap_desc m_cube_desc{};
		e_resource_state m_previous_state = e_resource_state::common;
		e_resource_state m_state = e_resource_state::common;
		uint64 m_bytesize{};
		uint64 m_bytestride{};
		e_format m_format{};
		vector<footprint> m_footprints{};
	};
}