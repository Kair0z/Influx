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

namespace influx::renderer
{
	class texture;

	class imgui_manager final
	{
	public:
		imgui_manager(graphics::device* device);
		void render(graphics::commandlist* commandlist, const ImDrawData& draw, const target& target);
		void render(graphics::commandlist* commandlist, const vector<ImDrawData const*>& draws, const vector<target const*>& targets);
		
	private:
		void create_fonts_texture(graphics::device* device);
		void create_shaders();
		void create_pipeline(graphics::device* device);
		void update_buffers(const vector<ImDrawData const*>& draws);
		void setup_state(graphics::commandlist*, const vector<ImDrawData const*>& draws);

		graphics::device* mp_device = nullptr;
		graphics::graphics_pipeline* mp_pipeline = nullptr;
		graphics::rootsignature* mp_rootsig = nullptr;
		graphics::resource* mp_indexbuffer = nullptr;
		graphics::resource* mp_vertexbuffer = nullptr;
		texture* mp_fonts_texture = nullptr;

		shader::compile_output m_vertex_shader;
		shader::compile_output m_pixel_shader;

		vector<uint32> m_per_draw_vertex_offsets{};
		vector<uint32> m_per_draw_index_offsets{};
	};
}