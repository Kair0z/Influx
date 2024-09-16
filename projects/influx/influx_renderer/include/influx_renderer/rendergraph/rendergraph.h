#pragma once

#include "core/string.h"
#include "core/container/vector.h"
#include "core/container/map.h"

#include "rgcommon.h"
#include "rgpass.h"

namespace influx::graphics
{
	class device;
	class commandlist;
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

	class rendergraph final
	{
	public:
		rendergraph(graphics::device* device);

		void build();

		// single threaded, single command list...
		void execute(graphics::commandlist* commandlist);

		// adding nodes & resources
		template <typename _passdata, typename... _args> requires std::is_constructible_v<rgpass<_passdata>, _args...>
		rgpass<_passdata>* add_pass(_args&&... args)
		{
			m_passes.emplace_back(new rgpass<_passdata>(std::forward<_args>(args)...));

			// setup the base
			rgpass_base* pass_base = m_passes.back();
			rgpass_id new_id = m_passes.size() - 1u;
			pass_base->set_id(new_id);
			pass_base->setup();

			// register in map
			m_id_to_pass_map[new_id] = pass_base;

			// return the spec
			return dynamic_cast<rgpass<_passdata>*>(pass_base);
		}

		rgtexture_id add_texture(const rgname& name, const texture_desc& desc);
		rgbuffer_id add_buffer(const rgname& name, const buffer_desc& desc);

		// adding externals
		rgtexture_id import_texture(const rgname& name, texture* texture);
		rgtexture_id import_buffer(const rgname& name, buffer* buffer);

		bool is_texture_declared(rgtexture_id id) const;
		bool is_buffer_declared(rgbuffer_id id) const;
		bool is_pass_declared(rgpass_id id) const;

		rgtexture* get_texture(rgtexture_id id);
		rgbuffer* get_buffer(rgbuffer_id id);
		rgpass_base* get_pass(rgpass_id id);

		graphics::descriptor_handle* get_rtv(rgtexture_id id);
		graphics::descriptor_handle* get_dsv(rgtexture_id id);
		graphics::descriptor_handle* get_readonly(rgtexture_id id);
		graphics::descriptor_handle* get_readwrite(rgtexture_id id);

		graphics::descriptor_handle* get_readonly(rgbuffer_id id);
		graphics::descriptor_handle* get_readwrite(rgbuffer_id id);

	private:
		vector<rgpass_base*> m_passes{};
		vector<rgbuffer*> m_buffers{};
		vector<rgtexture*> m_textures{};
		vector<class rglayer*> m_layers{};

		rgpool* m_pool = nullptr;
		class gpu_view_manager* m_view_manager = nullptr;

		vector<vector<uint64>> m_adjacency_lists{};
		graphics::device* m_device;

		umap<rgtexture_id, rgtexture*> m_id_to_texture_map;
		umap<rgbuffer_id, rgbuffer*> m_id_to_buffer_map;
		umap<rgpass_id, rgpass_base*> m_id_to_pass_map;
		umap<texture*, rgtexture*> m_imported_texture_map;
		umap<buffer*, rgbuffer*> m_imported_buffer_map;

		enum class e_descriptor_type : uint8
		{
			readwrite,
			readonly,
			rendertarget,
			depthstencil,
			count
		};
		static constexpr uint8_t k_num_descriptor_types = static_cast<uint8_t>(e_descriptor_type::count);

		umap<rgtexture_id, graphics::descriptor_handle*[k_num_descriptor_types]> m_texture_to_descriptors_map;
		umap<rgtexture_id, graphics::descriptor_handle*[k_num_descriptor_types]> m_buffer_to_descriptors_map;

	private:
		void build_adjacency();
		void build_layers();
	};
}