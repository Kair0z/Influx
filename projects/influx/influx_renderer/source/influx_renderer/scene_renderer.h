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
		uint32 m_start_vertex = 0u;
	};

	struct gpu_instance_data final
	{
		math::matrix4x4f	m_transform;
		math::vectorf4		m_colour;
		uint32				m_base_vertex;
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
		vector<batch> create_batches(const scene& scene);
		void update_buffers(const vector<batch>& batches);
		void render_basepass(graphics::commandlist* commandlist, 
			const scene& scene, const vector<batch>& batches, const target& target);

		void apply_pipeline_settings();

	private:
		struct mega_vertexbuffer final
		{
			graphics::resource* m_resource = nulllptr;
			graphics::descriptor_handle m_vertex_buffer_srv;
			uset<string> m_meshnames{};
			umap<string, uint64> m_meshname_to_offset{};

			void register_mesh(const string& meshname)
			{
				m_meshnames.insert(meshname);
			}

			void reset()
			{
				m_meshname_to_offset.clear();
			}

			void update_buffer()
			{
				if (m_resource == nullptr || m_meshnames.empty())
				{
					return;
				}

				reset();

				uint64 total_bytesize = 0u;
				const renderer_backend& backend = renderer_backend::get_instance();
				vector<gpu_vertex_data> gpu_data{};

				// gather a mega gpu_vertexdata vector
				for (const string& name : m_meshnames)
				{
					const vector<vertex_data> vertexbuffer_content = backend.get_vertexbuffer_content<vertex_data>(name);
					if (vertexbuffer_content.size() > 0u)
					{
						const uint64 old_size = gpu_data.size();
						const uint64 num_vertices = vertexbuffer_content.size();
						const uint64 bytesize = num_vertices * sizeof(vertex_data);
						gpu_data.resize(old_size + num_vertices);

						// copy the individual vertexbuffer content into our gpu data mega-vector
						memcpy(&gpu_data[old_size], vertexbuffer_content.data(), bytesize);

						// keep the base offset
						m_meshname_to_offset[name] = old_size;

						total_bytesize += bytesize;
					}
				}

				// map onto the resource
				m_resource->map([total_bytesize, &gpu_data](void* dest)
				{
					gpu_vertex_data* data = reinterpret_cast<gpu_vertex_data*>(dest);
					memcpy(data, gpu_data.data(), total_bytesize);
				});
			}

		} m_vertexbuffer;

		renderer_backend* mp_backend;
		pipeline* mp_pipeline;
		pipeline* mp_shadowspipeline;
		target* mp_shadowstarget;

		graphics::device* mp_device;

		graphics::resource* mp_instancebuffer;
		graphics::descriptor_handle m_instance_buffer_srv;
		
		// gpu data
		gpu_perscene m_gpu_perscene;
		gpu_perview m_gpu_perview;
		gpu_permaterial m_gpu_permaterial;
		gpu_perdraw m_gpu_perdraw;
		gpu_instance_data* m_instance_data;
	};
}