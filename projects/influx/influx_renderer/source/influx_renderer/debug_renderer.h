#pragma once 

namespace influx::renderer
{
	// renders instanced debug lines
	class debug_renderer final
	{
		struct gpu_instance_data;
		struct gpu_perview;

	public:
		debug_renderer(
			renderer_backend* backend,
			graphics::device* device);

		~debug_renderer();

		void render(
			graphics::commandlist* commandlist,
			const scene_debug& scene,
			const target& target);

	private:
		renderer_backend* mp_backend;
		graphics::device* mp_device;

		constexpr static uint32 k_max_instances = 4096u;
		
		vector<gpu_instance_data> m_instance_data;
		gpu_perview* m_gpu_perview;

		graphics::resource* mp_instancebuffer;
		graphics::resource* mp_vertexbuffer;
		graphics::descriptor_handle m_instance_buffer_srv;

		void update_instance_buffer(const scene_debug& scene);
	};
}