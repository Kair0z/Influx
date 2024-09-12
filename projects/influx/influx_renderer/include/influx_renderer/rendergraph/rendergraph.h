#pragma once

#include "core/string.h"
#include "core/container/vector.h"

#include "rgcommon.h"
#include "rgpass.h"

namespace influx::graphics
{
	class device;
	class command_list;
}

namespace influx::renderer
{
	class texture;
	class buffer;
	struct texture_desc;
	struct buffer_desc;
}

namespace influx::renderer
{
	class rendergraph;
	class rgbuffer;
	class rgtexture;
	class rgpool;
	class rglayer;

	class rendergraph final
	{
	public:
		rendergraph(graphics::device* device);

		void build();

		// single threaded, single command list...
		void execute(graphics::command_list* commandlist);

		template <typename _passdata, typename... _args> requires std::is_constructible_v<rgpass<_passdata>, _args...>
		rgpass<_passdata>* add_pass(_args&&... args)
		{
			m_passes.emplace_back(new rgpass<_passdata>(std::forward<_args>(args)...));

			// setup the base
			rgpass_base* pass_base = m_passes.back();
			pass_base->set_id(m_passes.size() - 1u);
			pass_base->setup();

			// return the spec
			return dynamic_cast<rgpass<_passdata>*>(pass_base);
		}

		// new rgresource
		rgtexture_id declare_texture(const rgname& name, const texture_desc& desc);
		rgbuffer_id declare_buffer(const rgname& name, const buffer_desc& desc);

		bool is_texture_declared(rgtexture_id id);
		bool is_buffer_declared(rgbuffer_id id);

		rgtexture* get_texture(rgtexture_id id);
		rgbuffer* get_buffer(rgbuffer_id id);

	private:
		vector<rgpass_base*> m_passes{};
		vector<rgbuffer*> m_buffers{};
		vector<rgtexture*> m_textures{};
		vector<rglayer*> m_layers{};
		rgpool* m_pool = nullptr;

		vector<vector<uint64>> m_adjacency_lists{};
		graphics::device* m_device;

	private:
		void build_adjacency();
		void build_layers();
	};
}