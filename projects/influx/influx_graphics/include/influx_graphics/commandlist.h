#pragma once

// influx::graphics
#include "influx_graphics/base.h"
#include "influx_graphics/resource.h"
#include "influx_graphics/descriptors.h"
#include "influx_graphics/renderpass.h"
#include "influx_graphics/queue.h"

// influx::core
#include "core/math/vector.h"
#include "core/range.h"
#include "core/pointer.h"

namespace influx::graphics
{
	class command_allocator;
	class pipeline;
	class render_target_view;
	class depth_stencil_view;
	class rootsignature;
	class descriptor_heap;
	class shader_resource_view;
	class device;

	struct draw_instanced_args final
	{
		uint32 m_num_vertices_per_instance = 1u;
		uint32 m_num_instances = 1u;
		uint32 m_start_vertex = 0;
		uint32 m_start_instance = 0;
	};

	// draw_indexed_instanced!
	struct draw_indexed_args final
	{
		uint32 m_num_indexes_per_instance;
		uint32 m_num_instances;
		uint32 m_start_index;
		int m_start_vertex;
		uint32 m_start_instance;
	};

	struct copy_texture_args final
	{
		struct
		{
			uint32 m_subresource = 0u;
			range<size_t> m_range{};
		} m_src;
		struct
		{
			uint32 m_subresource = 0u;
			range<size_t> m_range{};
		} m_dest;
	};

	struct copy_buffer_args final
	{
		uint32 m_dest_offset = 0u;
		uint32 m_src_offset = 0u;
	};

	class commandlist : public base
	{
	public:
		enum class e_state : uint8
		{
			created,
			recording,
			submitted,
			completed,
			count
		};
		
		INFLUX_GFX_API
		void start(device* device, pipeline* init_state = nullptr);

		INFLUX_GFX_API
		void submit(queue*);

		INFLUX_GFX_API
		void wait_for_completion();

		INFLUX_GFX_API
		bool is_completed();

		INFLUX_GFX_API
		e_state get_state();

		INFLUX_GFX_API
		void set_name(const debug_name& name);

		INFLUX_GFX_API
		const debug_name& get_name() const;

		INFLUX_GFX_API
		// starts a commandlist, using the device to allocate the memory internally
		virtual void renderpass_begin(const renderpass_args& args) = 0;
	
		INFLUX_GFX_API
		virtual void renderpass_end() = 0;

		INFLUX_GFX_API
		virtual void draw_instanced(const draw_instanced_args& args) = 0;
		
		INFLUX_GFX_API
		virtual void draw_indexed(const draw_indexed_args& args) = 0;

		INFLUX_GFX_API
		virtual void set_constants(uint32 param_index, uint32 num_dwords, void* source_data) = 0;

		INFLUX_GFX_API
		virtual void set_indexbuffer(resource* index_buffer) = 0;
		
		INFLUX_GFX_API
		virtual void set_vertexbuffer(resource* vertex_buffer) = 0;

		INFLUX_GFX_API
		virtual void clear_rtv(render_target_view* view, const math::vectorf4& clear_value) = 0;

		INFLUX_GFX_API
		virtual void clear_dsv(depth_stencil_view* view, float clear_depth, uint32 clear_stencil) = 0;

		INFLUX_GFX_API
		virtual void transition_resource(resource* resource, e_resource_state before, e_resource_state after) = 0;

		INFLUX_GFX_API
		virtual void buffer_barrier(resource* resource, e_resource_state before, e_resource_state after) = 0;

		INFLUX_GFX_API
		virtual void texture_barrier(resource* resource, e_resource_state before, e_resource_state after) = 0;

		INFLUX_GFX_API 
		virtual void global_barrier(e_resource_state before, e_resource_state after) = 0;

		INFLUX_GFX_API
		virtual void copy_resource(resource* source, resource* dest) = 0;

		INFLUX_GFX_API
		virtual void copy_texture(resource* src, resource* dest, const copy_texture_args& = {}) = 0;

		INFLUX_GFX_API
		virtual void copy_buffer(resource* src, resource* dest, uint32 bytesize, const copy_buffer_args & = {}) = 0;

		INFLUX_GFX_API
		virtual void set(descriptor_heap* heap) = 0;

		INFLUX_GFX_API
		virtual void set(render_target_view* rtv, depth_stencil_view* dsv) = 0;

		INFLUX_GFX_API
		virtual void set(shader_resource_view* srv, uint32 param_idx) = 0;

		INFLUX_GFX_API
		virtual void set(const descriptor_range& gpu_range, uint32 param_idx) = 0;

		INFLUX_GFX_API
		virtual void set(rootsignature* rootsig) = 0;
		
		INFLUX_GFX_API
		virtual void set(pipeline* pipeline) = 0;

		INFLUX_GFX_API
		virtual void set(const viewport& viewport) = 0;

		INFLUX_GFX_API
		virtual void set(const rect& rect) = 0;

		INFLUX_GFX_API
		virtual void set(e_primitive_topology topo) = 0;

		INFLUX_GFX_API
		virtual void end() = 0;

	private:
		virtual void start_impl(device* device, pipeline* init_state = nullptr) = 0;

	private:
		e_state m_state = e_state::created;
		fence* m_fence = nullptr;

		// the queue will inform this commandlist its been submitted
		friend void queue::post_submit(const vector<commandlist*>& commandlists);
		void post_submit(queue*);

		// first submit has complete value 1
		uint32 m_complete_value = 1u;
		
		debug_name m_name;
	};
}