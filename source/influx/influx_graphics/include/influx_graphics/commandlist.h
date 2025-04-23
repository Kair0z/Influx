#pragma once

// influx::graphics
#include "influx_graphics/common.h"
#include "influx_graphics/base.h"
#include "influx_graphics/resource.h"
#include "influx_graphics/descriptors.h"
#include "influx_graphics/renderpass.h"
#include "influx_graphics/queue.h"
#include "influx_graphics/pipeline.h"

// influx::core
#include "core/math/vector.h"
#include "core/range.h"
#include "core/pointer.h"
#include "core/result.h"

namespace influx::graphics
{
#pragma region declarations
	namespace detail
	{
		class base_pipeline;
	}
	class command_allocator;
	class render_target_view;
	class depth_stencil_view;
	class rootsignature;
	class descriptor_heap;
	class shader_resource_view;
	class device;
#pragma endregion

#pragma region helper structs
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

	struct dispatch_args final
	{
		math::vectoru3 m_threadgroup_count{};
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

	struct build_acc_str_args final
	{
		e_acc_str_type m_type;
	};
#pragma endregion

	class commandlist : public base
	{
	public:
		enum class e_state : uint8
		{
			created,		// the commandlist is recently created, but not in any active state
			recording,		// the commandlist is recording commands on the CPU
			submitted,		// the commandlist is in-flight and being processed by the GPU
			completed,
			count
		};
		
		INFLUX_GFX_API result<> start(device* device, detail::base_pipeline* init_state = nullptr);

		INFLUX_GFX_API result<> submit(queue*);

		INFLUX_GFX_API void wait_for_completion();

		INFLUX_GFX_API bool is_completed();

		INFLUX_GFX_API e_state get_state();

		INFLUX_GFX_API void set_name(const debug_name& name);

		INFLUX_GFX_API const debug_name& get_name() const;

		/* commands */
		INFLUX_GFX_API virtual result<> renderpass_begin(const renderpass_args& args) = 0;
		
		INFLUX_GFX_API virtual result<> renderpass_end() = 0;

		INFLUX_GFX_API virtual result<> draw_instanced(const draw_instanced_args& args) = 0;
		
		INFLUX_GFX_API virtual result<> draw_indexed(const draw_indexed_args& args) = 0;

		INFLUX_GFX_API virtual result<> dispatch(const dispatch_args& args) = 0;

		INFLUX_GFX_API virtual result<> set_constants(uint32 param_index, uint32 num_dwords, void* source_data, graphics::e_pipeline_type type = e_pipeline_type::graphics) = 0;

		INFLUX_GFX_API virtual result<> set_indexbuffer(resource* index_buffer) = 0;
		
		INFLUX_GFX_API virtual result<> set_vertexbuffer(resource* vertex_buffer) = 0;

		INFLUX_GFX_API virtual result<> clear_rtv(descriptor_handle rtv_cpu, const math::vectorf4& clear_value) = 0;

		INFLUX_GFX_API virtual result<> clear_dsv(descriptor_handle dsv_cpu, float clear_depth, uint32 clear_stencil) = 0;

		INFLUX_GFX_API virtual result<> set_rtv(descriptor_handle rtv_cpu, descriptor_handle dsv_cpu) = 0;

		INFLUX_GFX_API virtual result<> set_srv(descriptor_handle srv_gpu, uint32 param_idx) = 0;

		INFLUX_GFX_API virtual result<> build_acceleration_struct(resource* dest_resource, resource* scratch_resource, const build_acc_str_args& args) = 0;

		INFLUX_GFX_API virtual result<> transition_resource(resource* resource, e_resource_state before, e_resource_state after) = 0;

		INFLUX_GFX_API virtual result<> buffer_barrier(resource* resource, e_resource_state before, e_resource_state after) = 0;

		INFLUX_GFX_API virtual result<> texture_barrier(resource* resource, e_resource_state before, e_resource_state after) = 0;

		INFLUX_GFX_API virtual result<> global_barrier(e_resource_state before, e_resource_state after) = 0;

		INFLUX_GFX_API virtual result<> flush_barriers() = 0;

		INFLUX_GFX_API virtual result<> copy_resource(resource* source, resource* dest) = 0;

		INFLUX_GFX_API virtual result<> copy_texture(resource* src, resource* dest, const copy_texture_args& = {}) = 0;

		INFLUX_GFX_API virtual result<> copy_buffer(resource* src, resource* dest, uint32 bytesize, const copy_buffer_args & = {}) = 0;

		INFLUX_GFX_API virtual result<> set(descriptor_heap* heap) = 0;

		INFLUX_GFX_API virtual result<> set(const vector<descriptor_heap*>& heap) = 0;

		INFLUX_GFX_API virtual result<> set(const descriptor_range& gpu_range, uint32 param_idx) = 0;

		INFLUX_GFX_API virtual result<> set(rootsignature* rootsig, const e_pipeline_type type = e_pipeline_type::graphics) = 0;
		
		INFLUX_GFX_API virtual result<> set(detail::base_pipeline* pipeline) = 0;

		INFLUX_GFX_API virtual result<> set_vp_and_rect(const math::float2& min, const math::float2& max);

		INFLUX_GFX_API virtual result<> set(const viewport& viewport);

		INFLUX_GFX_API virtual result<> set(const rect& rect);

		INFLUX_GFX_API virtual result<> set(e_primitive_topology topo) = 0;

		/* mesh shaders */
		INFLUX_GFX_API virtual result<> dispatch_mesh(uint32 groupcount_x, uint32 groupcount_y, uint32 groupcount_z) = 0;

		/* raytracing */
		INFLUX_GFX_API virtual result<> dispatch_rays(raytracing_pipeline* pipeline, uint32 width, uint32 height, uint32 depth = 1) = 0;

		INFLUX_GFX_API virtual result<> end() = 0;

	private:
		virtual void start_impl(device* device, detail::base_pipeline* init_state = nullptr) = 0;

	private:
		e_state m_state = e_state::created;
		fence* m_fence = nullptr;

		// the queue will inform this commandlist its been submitted
		friend result<> queue::post_submit(const vector<commandlist*>& commandlists);
		result<> post_submit(queue*);

		// first submit has complete value 1
		uint32 m_complete_value = 1u;
		
		debug_name m_name;

	protected:
		void pre_draw();

		viewport m_viewport;
		rect m_scissor_rect;
	};
}

