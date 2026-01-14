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

	class draw_batch;

	class world_renderer final
	{
	public:
		world_renderer();
		~world_renderer();

		// add passes to rendergraph
		void build(rendergraph::rendergraph& graph, const worldview& wv, const target& target);

	private:
		vector<draw_batch> create_batches(const worldview& wv, graphics::commandlist* commandlist);
		void update_instance_buffer(const vector<draw_batch>& batches);
		void update_line_instance_buffer(const worldview& wv);
		void update_lightbuffers(const worldview& wv);

		void build_basepass(rendergraph::rgpass_builder&, const target& target);
		void build_resolvepass(rendergraph::rgpass_builder&, const target& target);
		void execute_basepass(rendergraph::rgpass_context&, const target& target, const worldview& wv);
		void execute_resolvepass(rendergraph::rgpass_context&, const target& target, const worldview& wv);

	private:
		// this is the resources that could change each frame.
		// they need to be buffered to avoid the cpu writing where the GPU is reading
		struct buffered final
		{
			graphics::resource*				m_instancebuffer;
			graphics::resource*				m_lightbuffers[k_num_light_types];
			graphics::descriptor_handle		m_instance_buffer_srv;
			graphics::descriptor_handle		m_lightbuffer_srvs[k_num_light_types];
			graphics::resource*				m_line_instance_buffer;
			graphics::descriptor_handle		m_line_instance_buffer_srv;
		};
		buffered m_buffered[k_max_in_flight];
		buffered& get_buffered_current();

		// these are the resources that are persistent and unchanging
		graphics::resource* mp_skybox;
		graphics::descriptor_handle m_skybox_srv;
		graphics::descriptor_handle m_sampler_view;
		graphics::descriptor_handle m_skybox_sampler;
		graphics::resource* m_line_vertex_buffer;

		// this is the cached CPU-side data
		vector<frontend::per_line_instance> m_line_instance_data;
		frontend::per_scene m_gpu_perscene;
		frontend::per_view m_gpu_perview;
		frontend::per_material m_gpu_permaterial;
		frontend::per_draw m_gpu_perdraw;
		frontend::per_instance* m_instance_data;
	};
}