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
		void render(graphics::commandlist* commandlist, ImDrawData* draw_data, const target& target);

	private:
		void create_fonts_texture(graphics::device* device);
		void create_shaders();
		void create_pipeline(graphics::device* device);
		void update_buffers(ImDrawData* draw_data);

		graphics::device* mp_device = nullptr;
		graphics::graphics_pipeline* mp_pipeline = nullptr;
		graphics::rootsignature* mp_rootsig = nullptr;
		graphics::resource* mp_indexbuffer = nullptr;
		graphics::resource* mp_vertexbuffer = nullptr;
		texture* mp_fonts_texture = nullptr;

		shader::compile_output m_vertex_shader;
		shader::compile_output m_pixel_shader;
	};
}