#pragma once

// influx::graphics
namespace influx::graphics
{
	class device;
	class command_list;
	class descriptor_heap;
}

namespace influx::renderer
{
	class renderer_backend;
	class pipeline;
}

namespace influx::renderer
{
	// [LAYOUT]
	struct gpu_instance_data final
	{
		math::matrix4x4f	m_transform;
		math::vectorf4		m_colour;
	};

	struct gpu_vs_constants final
	{
		math::matrix4x4f m_mvp;
	};

	struct gpu_ps_constants final
	{
		uint32 m_albedo_slotidx;
		uint32 m_normals_slotidx;
		uint32 m_other_slotidx;
	};

	class scene_renderer final
	{
	public:
		scene_renderer(
			renderer_backend* backend,
			graphics::device* device,
			pipeline* pipeline);

		void render(
			graphics::command_list* commandlist, 
			const scene& scene,
			const target& target);

	private:
		renderer_backend* mp_backend;
		pipeline* mp_pipeline;
		graphics::device* mp_device;
		graphics::descriptor_heap* mp_srv_heap_gpu;
		graphics::descriptor_range m_srv_gpu_range;
		gpu_vs_constants m_vs_constants;
		gpu_ps_constants m_ps_constants;
	};
}