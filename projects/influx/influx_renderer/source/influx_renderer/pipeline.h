#pragma once

#include "influx_renderer.h"

#include "core/string.h"
#include "core/container/map.h"

// influx::graphics
#pragma region graphics declarations
namespace influx::graphics
{
	class device;
	class pipeline;
	class rootsignature;
	class command_list;
}
#pragma endregion

namespace influx::renderer
{
	class pipeline final
	{
	public:
		pipeline(
			graphics::device* device,
			renderer::shader_data vertex_shader,
			renderer::shader_data pixel_shader);

		void set_state(graphics::command_list* cmdlist);

		void set_constants(graphics::command_list* cmdlist, const string& name, uint32 num_dwords, void* data);

		uint32 get_shader_register(const string& resource_name);

#if _DEBUG
		void set_name(const string& name);
		const string& get_name() const;
#endif

	private:
		graphics::rootsignature* mp_rootsig = nullptr;
		graphics::pipeline* mp_pipeline = nullptr;
		umap<string, uint32> m_name_to_register;

#if _DEBUG
		string m_debug_name;
#endif
	};
}