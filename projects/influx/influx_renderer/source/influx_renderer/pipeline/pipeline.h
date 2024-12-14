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
	class commandlist;
	struct descriptor_range;
}
#pragma endregion

namespace influx::renderer
{
	class pipeline final
	{
	public:
		pipeline(
			graphics::device* device,
			renderer::shader_data const* vertex_shader,
			renderer::shader_data const* pixel_shader);

		static pipeline* load_from_file(const string& path);

		void set_state(graphics::commandlist* cmdlist);

		template <typename _constants>
		void set_constants(graphics::commandlist* cmdlist, const string& name, _constants& constants)
		{
			set_constants(cmdlist, name, sizeof(_constants) / sizeof(uint32), &constants);
		}

		void set_constants(graphics::commandlist* cmdlist, const string& name, uint32 num_dwords, void* data);

		void set_resource_table(graphics::commandlist* cmdlist, const string& name, const graphics::descriptor_range& gpu_range);

		uint32 get_shader_register(const string& resource_name);
		uint32 get_param_index(const string& resource_name);

#if INFLUX_DEBUG
		void set_name(const string& name);
		const string& get_name() const;
#endif

		void save_to_file(const string& path) const;
		

	private:
		graphics::rootsignature* mp_rootsig = nullptr;
		graphics::pipeline* mp_pipeline = nullptr;
		umap<string, uint32> m_name_to_register;
		umap<string, uint32> m_name_to_param_idx;
		graphics::pipeline_desc m_create_desc{};
		graphics::rootsignature_desc m_rootsig_desc{};

#if INFLUX_DEBUG
		string m_debug_name;
#endif
	};
}