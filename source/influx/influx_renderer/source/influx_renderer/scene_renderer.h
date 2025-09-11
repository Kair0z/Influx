#pragma once

// engine shader frontend
namespace influx::renderer::frontend
{
	#include "frontend.h"
}

// influx::graphics
namespace influx::graphics
{
	class commandlist;
	class resource;
}

// influx::rendergraph
#include "rendergraph.h"
namespace influx::rendergraph
{
	class rendergraph;
	class rgpass_builder;
	class rgpass_context;
}

namespace influx::renderer
{
	// todo: some of these can be made flexible
	constexpr static uint32 k_max_num_instances = 4096u;
	constexpr static uint32 k_max_num_lights = 512u;
	constexpr static uint32 k_max_num_vertices = 24u * 1024u;
	constexpr static uint32 k_num_light_types = 3u;
	constexpr static uint32 k_max_lines = 4096u;

	class batch;

	class scene_renderer final
	{
	public:
		scene_renderer();
		~scene_renderer();

		/* compile & load internal shaders */
		void load_shaders();

		/* add passes to render graph */
		void build(rendergraph::rendergraph& graph, const scene& scene, const target& target);

	private:
		vector<batch> create_batches(const scene& scene, graphics::commandlist* commandlist);
		void update_instance_buffer(const vector<batch>& batches);
		void update_line_instance_buffer(const scene& scene);
		void update_lightbuffers(const scene& scene);
		void apply_pipeline_settings(const target& target);

		void build_basepass(rendergraph::rgpass_builder&, const target& target);
		void build_resolvepass(rendergraph::rgpass_builder&, const target& target, const scene& scene);
		void execute_basepass(rendergraph::rgpass_context&, const target& target, const scene& scene);
		void execute_resolvepass(rendergraph::rgpass_context&, const target& target, const scene& scene);

	private:
		rendergraph::rgname m_uav_proxy_name = { "uav_proxy" };

		graphics::resource* mp_instancebuffer;
		graphics::resource* mp_lightbuffers[k_num_light_types];
		graphics::descriptor_handle m_instance_buffer_srv;
		graphics::descriptor_handle m_lightbuffer_srvs[k_num_light_types];
		graphics::descriptor_handle m_sampler_view;
		graphics::descriptor_handle m_skybox_sampler;

		graphics::resource* mp_skybox;
		graphics::descriptor_handle m_skybox_srv;
		
		graphics::resource* m_line_instance_buffer;
		graphics::resource* m_line_vertex_buffer;
		graphics::descriptor_handle m_line_instance_buffer_srv;

		vector<frontend::line_gpu_instance_data> m_line_instance_data;
		frontend::per_scene m_gpu_perscene;
		frontend::per_view m_gpu_perview;
		frontend::per_material m_gpu_permaterial;
		frontend::per_draw m_gpu_perdraw;
		frontend::per_instance* m_instance_data;
	};
}