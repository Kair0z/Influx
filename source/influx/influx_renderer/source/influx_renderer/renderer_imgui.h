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
#include "influx_renderer/types.h"
#include "influx_renderer/texture.h"

namespace influx::renderer
{
	struct drawdata_dependencies final
	{
		vector<graphics::descriptor_handle> m_texture_cpu_handles;
		vector<graphics::resource*> m_textures;
	};

	class imgui_manager final
	{
	public:
		imgui_manager(graphics::device* device);
		void render(graphics::commandlist* commandlist, const ImDrawData& draw, const target& target);
		void render(graphics::commandlist* commandlist, const vector<ImDrawData const*>& draws, const vector<target const*>& targets);

		/* fetches all textures imgui wants readable */
		static vector<drawdata_dependencies> get_dependencies(const vector<ImDrawData const*>& draws);

	private:
		void create_fonts_texture(graphics::device* device);
		result<> create_shaders();
		result<> create_pipeline(graphics::device* device);
		void update_buffers(const vector<ImDrawData const*>& draws);
		void setup_state(graphics::commandlist*, const vector<ImDrawData const*>& draws);

		graphics::device* mp_device = nullptr;
		graphics::graphics_pipeline* mp_pipeline = nullptr;
		graphics::rootsignature* mp_rootsig = nullptr;
		graphics::resource* mp_indexbuffer = nullptr;
		graphics::resource* mp_vertexbuffer = nullptr;
		texture2D* mp_fonts_texture = nullptr;

		shader::compile_output m_vertex_shader;
		shader::compile_output m_pixel_shader;

		vector<uint32> m_per_draw_vertex_offsets{};
		vector<uint32> m_per_draw_index_offsets{};
	};
}