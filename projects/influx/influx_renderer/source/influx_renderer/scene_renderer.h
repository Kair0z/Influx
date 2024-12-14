#pragma once

// influx::graphics
namespace influx::graphics
{
	class device;
	class commandlist;
	class descriptor_heap;
	class resource;
	class shader_resource_view;
}

namespace influx::renderer
{
	class renderer_backend;
	class pipeline;
	struct material;
}

namespace influx::renderer
{
	constexpr static uint32 k_max_num_instances = 4096u;

	// [LAYOUT]
	struct gpu_perscene final
	{
		float m_seconds{};
		float m_delta_seconds{};
		math::vectorf3 m_light_direction = { -0.5f, -0.5f, -0.5f };
		math::colour_rgba m_light_colour = { 1.0f, 1.0f, 1.0f, 1.0f };
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
		bool m_invert_normals;
	};
	
	// a batch of instances grouped per material
	class batch final
	{
	public:
		batch(
			const string& mesh_name, 
			const string& material_name, 
			const vector<gpu_instance_data>& instances,
			uint32 instance_base);

		graphics::resource* get_vertex_buffer() const;
		graphics::resource* get_index_buffer() const;
		material* get_material() const;

		const vector<gpu_instance_data>& get_instances() const;
		const uint32 get_instance_base() const;

	private:
		material* m_material;
		graphics::resource* m_vertex_buffer;
		graphics::resource* m_index_buffer;
		vector<gpu_instance_data> m_instances{};
		uint32 m_base_instance;
	};

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
		vector<batch> create_batches(const scene& scene);
		void update_instance_buffer(const vector<batch>& batches);
		void render_batches(graphics::commandlist* commandlist, const vector<batch>& batches);

		void render_shadows(graphics::commandlist* commandlist, 
			const scene& scene, const vector<batch>& batches);

		void render_basepass(graphics::commandlist* commandlist, 
			const scene& scene, const vector<batch>& batches, const target& target);

	private:
		renderer_backend* mp_backend;
		pipeline* mp_pipeline;
		pipeline* mp_shadowspipeline;
		target* mp_shadowstarget;

		graphics::device* mp_device;
		graphics::resource* mp_instancebuffer;
		graphics::shader_resource_view* mp_instance_buffer_srv;

		// gpu data
		gpu_perscene m_gpu_perscene;
		gpu_perview m_gpu_perview;
		gpu_permaterial m_gpu_permaterial;
		gpu_perdraw m_gpu_perdraw;
		gpu_instance_data* m_instance_data;
	};
}