#pragma once

// imgui
struct ImDrawData;

// influx::graphics
namespace influx::graphics
{
	class device;
	class descriptor_heap;
	class rootsignature;
	class pipeline;
	class resource;
	class command_list;
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
		void render(graphics::command_list* commandlist, ImDrawData* draw_data, const target& target);

	private:
		void create_fonts_texture(graphics::device* device);
		void create_shaders();
		void create_pipeline(graphics::device* device);
		void update_buffers(ImDrawData* draw_data);

		graphics::device* mp_device;
		graphics::pipeline* mp_pipeline;
		graphics::rootsignature* mp_rootsig;
		graphics::resource* mp_indexbuffer;
		graphics::resource* mp_vertexbuffer;
		texture* mp_fonts_texture;

		shader::compile_output m_vertex_shader;
		shader::compile_output m_pixel_shader;
	};
}