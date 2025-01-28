#pragma once

#pragma region forward_decl
// influx::core
namespace influx
{
	class material;
}

// influx::graphics
namespace influx::graphics
{
	class device;
	class commandlist;
	class descriptor_heap;
	class resource;
	class shader_resource_view;
}

// influx::renderer
namespace influx::renderer
{
	class renderer_backend;
	class pipeline;
}
#pragma endregion

namespace influx::renderer
{
	constexpr static uint32 k_max_num_instances = 4096u;
	constexpr static uint32 k_max_num_vertices = 24u * 1024u;

	// [LAYOUT]
	struct gpu_perscene final
	{
		math::vectorf4 m_time = {};
		math::vectorf4 m_light_direction = { -0.5f, -0.5f, -0.5f };
		math::vectorf4 m_light_colour = { 1.0f, 1.0f, 1.0f, 1.0f };
	};

	struct gpu_perview final
	{
		math::matrix4x4f m_vp;
	};

	struct gpu_permaterial final
	{
		math::colour_rgba m_colour;
	};

	struct gpu_perdraw final
	{
		uint32 m_start_instance = 0u;
	};

	struct gpu_instance_data final
	{
		math::matrix4x4f	m_transform;
		math::vectorf4		m_colour;
		uint32 m_albedo_index;
		uint32 m_normal_index;
	};

	struct gpu_vertex_data final
	{
		math::float3 m_position;
		math::float4 m_colour;
		math::float3 m_normal;
		math::float2 m_texcoord;
	};

	class batch;

	class scene_renderer final
	{
	public:
		scene_renderer(
			renderer_backend* backend,
			graphics::device* device,
			pipeline* pipeline);

		~scene_renderer();

		void render(
			graphics::commandlist* commandlist, 
			const scene& scene,
			const target& target);

	private:
		vector<batch> create_batches(const scene& scene, graphics::commandlist* commandlist);

		void gather_textures(const scene& scene);
		void update_instance_buffer(const vector<batch>& batches);

		void render_basepass(graphics::commandlist* commandlist, 
			const scene& scene, const vector<batch>& batches, const target& target);

		void apply_pipeline_settings();

	private:
		renderer_backend* mp_backend;
		pipeline* mp_pipeline;

		graphics::device* mp_device;

		graphics::resource* mp_instancebuffer;
		graphics::descriptor_handle m_instance_buffer_srv;
		graphics::descriptor_handle m_sampler_view;

		// gpu data
		gpu_perscene m_gpu_perscene;
		gpu_perview m_gpu_perview;
		gpu_permaterial m_gpu_permaterial;
		gpu_perdraw m_gpu_perdraw;
		gpu_instance_data* m_instance_data;
	};
}