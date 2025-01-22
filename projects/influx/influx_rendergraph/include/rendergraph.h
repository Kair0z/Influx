#pragma once

#if _DLL
#define INFLUX_RG_API __declspec(dllexport)
#else
#define INFLUX_RG_API __declspec(dllimport)
#endif

// influx::core
#include "core/string.h"
#include "core/container/vector.h"
#include "core/container/map.h"

// influx::graphics
#include "influx_graphics/descriptors.h"

namespace influx::graphics
{
	class device;
	class commandlist;
}

// influx::rendergraph
#include "rgcommon.h"
#include "rgpass.h"

namespace influx::rendergraph
{
	class rgbuffer;
	class rgtexture;
	class rgpool;
	class rglayer;
	class view_manager;

	class rendergraph final
	{
		friend class rgpass_builder;

	public:
		INFLUX_RG_API rendergraph(graphics::device* device);

		INFLUX_RG_API void build();

		// single threaded, single command list...
		INFLUX_RG_API void execute(graphics::commandlist* commandlist);

		// adds a node outputting to root
		INFLUX_RG_API rgpass* add_pass(
			const rgpass_builder_clb& builder_clb,
			const rgpass_process_clb& context_clb);

		// in/out resources
		INFLUX_RG_API void import_texture(const rgname& name, graphics::resource* resource);
		INFLUX_RG_API void import_buffer(const rgname& name, graphics::resource* resource);
		INFLUX_RG_API void export_texture(const rgname& name, graphics::resource* resource);
		INFLUX_RG_API void export_buffer(const rgname& name, graphics::resource* resource);

	private:
		vector<rgpass*> m_passes{};
		vector<rgbuffer*> m_buffers{};
		vector<rgtexture*> m_textures{};
		vector<rglayer*> m_layers{};

		rgpool* m_pool = nullptr;
		view_manager* m_view_manager = nullptr;

		vector<vector<uint64>> m_adjacency_lists{};
		graphics::device* m_device;

		umap<rgtexture_id, rgtexture*> m_id_to_texture_map;
		umap<rgbuffer_id, rgbuffer*> m_id_to_buffer_map;
		umap<rgpass_id, rgpass*> m_id_to_pass_map;

		umap<rgname, rgtexture_id> m_texture_name_to_id_map;
		umap<rgname, rgbuffer_id> m_buffer_name_to_id_map;
		umap<rgbuffer_readwrite_id, rgbuffer_id> m_buffer_uav_counter_map;

		static constexpr uint8 k_num_descriptor_types = static_cast<uint8>(rgdescriptor_type::count);
		umap<rgtexture_id, texture_view_desc[k_num_descriptor_types]> m_texid_to_viewdesc_map;
		umap<rgtexture_id, graphics::descriptor_handle[k_num_descriptor_types]> m_texid_to_descriptors_map;
		umap<rgbuffer_id, buffer_view_desc[k_num_descriptor_types]> m_bufid_to_viewdesc_map;
		umap<rgbuffer_id, graphics::descriptor_handle[k_num_descriptor_types]> m_bufid_to_descriptors_map;

		void build_adjacency();
		void sort_topological();
		void build_layers();
		void cull_passes();
		void calc_resource_lifetimes();
		void depth_search(uint64 parent_idx, vector<bool>& visited_list, vector<uint64>& topo_sorted_passes);
		void create_texture_views(rgtexture_id);
		void create_buffer_views(rgbuffer_id);
		vector<uint64> m_topo_sorted_passes;

		// misc
		graphics::descriptor_handle get_rtv(rgtexture_id id);
		graphics::descriptor_handle get_dsv(rgtexture_id id);
		graphics::descriptor_handle get_readonly(rgtexture_id id);
		graphics::descriptor_handle get_readwrite(rgtexture_id id);

		// rgpass_builder uses these
		rgtexture_id declare_texture(const rgname& name, const texture_desc& desc);
		rgbuffer_id declare_buffer(const rgname& name, const buffer_desc& desc);
		bool is_texture_declared(rgtexture_id id) const;
		bool is_buffer_declared(rgbuffer_id id) const;
		bool is_texture_declared(const rgname& name) const;
		bool is_buffer_declared(const rgname& name) const;
		
		rgtex_copysrc_id read_copysrc_texture(const rgname& name);
		rgtex_copydst_id write_copydst_texture(const rgname& name);
		rgbuf_copysrc_id read_copysrc_buffer(const rgname& name);
		rgbuf_copydst_id write_copydst_buffer(const rgname& name);
		rgbuf_indargs_id read_indirect_args_buffer(const rgname& name);
		rgbuf_vertex_id read_vertex_buffer(const rgname& name);
		rgbuf_index_id read_index_buffer(const rgname& name);
		rgbuf_const_id read_constant_buffer(const rgname& name);

		rgrendertarget_id rendertarget(const rgname& name, const texture_view_desc& view_desc);
		rgdepthtarget_id depthtarget(const rgname& name, const texture_view_desc& view_desc);
		rgtexture_readonly_id read_texture(const rgname& name, const texture_view_desc& view_desc);
		rgtexture_readwrite_id write_texture(const rgname& name, const texture_view_desc& view_desc);
		rgbuffer_readonly_id read_buffer(const rgname& name, const buffer_view_desc& view_desc);
		rgbuffer_readwrite_id write_buffer(const rgname& name, const buffer_view_desc& view_desc);
		rgbuffer_readwrite_id write_buffer(const rgname& name, const rgname& counter_name, const buffer_view_desc& view_desc);

		// lookups
		rgtexture* get_texture(rgtexture_id id);
		rgbuffer* get_buffer(rgbuffer_id id);
		rgtexture_id get_texture_id(const rgname& name) const;
		rgbuffer_id get_buffer_id(const rgname& name) const;
		texture_desc get_texture_desc(const rgname& name) const;
		buffer_desc get_buffer_desc(const rgname& name) const;
	};
}