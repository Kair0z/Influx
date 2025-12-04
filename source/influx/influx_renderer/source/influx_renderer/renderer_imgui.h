#pragma once

// imgui
struct ImDrawData;

// influx::graphics
#include "influx_graphics/pipeline.h"
namespace influx::graphics
{
	class device;
	class descriptor_heap;
	class rootsignature;
	class resource;
	class commandlist;
}

// influx::shader
#include "influx_shader.h"

// influx::renderer
#include "influx_renderer/common.h"
#include "influx_renderer/texture.h"

// influx::rendergraph
namespace influx::rendergraph
{
	class rgpass_builder;
}

namespace influx::renderer
{
	class imgui_manager final
	{
	public:
		imgui_manager(graphics::device* device);

		void build_rendergraph(rendergraph::rgpass_builder& builder, const target& target, const ImDrawData& drawdata);

		void render(graphics::commandlist* commandlist, const ImDrawData& draw, const target& target);
		void render(graphics::commandlist* commandlist, const vector<ImDrawData const*>& draws, const vector<target const*>& targets);

		/* fetches all textures imgui wants readable for rendering */
		static vector<imgui_texid_provider*> get_texture_dependencies(const vector<ImDrawData const*>& draws);
		/* fetches all textures imgui wants readable for rendering */
		static vector<imgui_texid_provider*> get_texture_dependencies(ImDrawData const* draw);

	private:
		void create_fonts_texture(graphics::device* device);
		result<> create_shaders();
		result<> create_pipeline(graphics::device* device);
		void update_buffers(const vector<ImDrawData const*>& draws);

		graphics::device* mp_device = nullptr;
		graphics::graphics_pipeline* mp_pipeline = nullptr;
		graphics::rootsignature* mp_rootsig = nullptr;

		graphics::resource* mp_indexbuffer[k_max_in_flight];
		graphics::resource* mp_vertexbuffer[k_max_in_flight];
		texture2D* mp_fonts_texture = nullptr;

		shader::compile_output m_vertex_shader;
		shader::compile_output m_pixel_shader;
		vector<uint32> m_per_draw_vertex_offsets{};
		vector<uint32> m_per_draw_index_offsets{};
	};
}