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
#include "core/result.h"

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
	class rgpool;
	class rgbuffer;
	class rgtexture;
	class rglayer;
	class rendergraph;

	class rgpass_context final
	{
		friend class rendergraph;

	public:
		graphics::commandlist& get_commandlist()
		{
			return m_commandlist;
		}

		struct resource_and_view final
		{
			graphics::resource* m_resource = nullptr;
			graphics::descriptor_handle m_descriptor = nullptr;
		};

		INFLUX_RG_API result<resource_and_view> get_copysrc(rgtex_copysrc_id);
		INFLUX_RG_API result<resource_and_view> get_copysrc(rgbuf_copysrc_id);
		INFLUX_RG_API result<resource_and_view> get_copydst(rgtex_copydst_id);
		INFLUX_RG_API result<resource_and_view> get_copydst(rgbuf_copydst_id);
		INFLUX_RG_API result<resource_and_view> get_vertexbuffer(rgbuf_vertex_id);
		INFLUX_RG_API result<resource_and_view> get_indexbuffer(rgbuf_index_id);
		INFLUX_RG_API result<resource_and_view> get_constbuffer(rgbuf_const_id);
		INFLUX_RG_API result<resource_and_view> get_indirect_args_resource(rgbuf_indargs_id);

		INFLUX_RG_API result<resource_and_view> get_copysrc_texture(const rgname&);
		INFLUX_RG_API result<resource_and_view> get_copysrc_buffer(const rgname&);
		INFLUX_RG_API result<resource_and_view> get_copydst_texture(const rgname&);
		INFLUX_RG_API result<resource_and_view> get_copydst_buffer(const rgname&);

		INFLUX_RG_API result<resource_and_view> get_rtv(uint32 at_index = 0u);
		INFLUX_RG_API result<resource_and_view> get_dsv();
		INFLUX_RG_API result<resource_and_view> get_rtv(rgrendertarget_id id);
		INFLUX_RG_API result<resource_and_view> get_dsv(rgrendertarget_id id);

		INFLUX_RG_API result<resource_and_view> get_read_texture(rgtexture_readonly_id id);
		INFLUX_RG_API result<resource_and_view> get_write_texture(rgtexture_readwrite_id id);
		INFLUX_RG_API result<resource_and_view> get_read_buffer(rgbuffer_readonly_id id);
		INFLUX_RG_API result<resource_and_view> get_write_buffer(rgbuffer_readwrite_id id);
		
		INFLUX_RG_API result<resource_and_view> get_read_texture(const rgname&);
		INFLUX_RG_API result<resource_and_view> get_write_texture(const rgname&);
		INFLUX_RG_API result<resource_and_view> get_read_buffer(const rgname&);
		INFLUX_RG_API result<resource_and_view> get_write_buffer(const rgname&);

		INFLUX_RG_API result<resource_and_view> get_read_texture(uint32 index);
		INFLUX_RG_API result<resource_and_view> get_write_texture(uint32 index);
		INFLUX_RG_API result<resource_and_view> get_read_buffer(uint32 index);
		INFLUX_RG_API result<resource_and_view> get_write_buffer(uint32 index);

	private:
		rgpass_context(rendergraph& rg, graphics::commandlist& cmdlist, const rgpass& pass) : m_graph{ rg }, m_commandlist{ cmdlist }, m_pass{ pass }{}
		graphics::commandlist& m_commandlist;
		rendergraph& m_graph;
		const rgpass& m_pass;
	};

	struct clear_args final
	{
		math::colour_rgba m_colour;
	};

	class rendergraph final
	{
		friend class rgpass_builder;
		friend class rgpass_context;

	public:
		INFLUX_RG_API rendergraph(const global_config& config, graphics::device& device);
		INFLUX_RG_API void cleanup(graphics::device& device);
		INFLUX_RG_API ~rendergraph();

		INFLUX_RG_API void build();

		// single threaded, single command list...
		INFLUX_RG_API void execute(
			graphics::commandlist& commandlist,
			graphics::device& device);

		// adds a node outputting to root
		INFLUX_RG_API rgpass* add_pass(e_rgpass_type type,
			const rgpass_builder_clb& builder_clb,
			const rgpass_process_clb& context_clb = nullptr);

		inline rgpass* add_graphics_pass(
			const rgpass_builder_clb& builder_clb,
			const rgpass_process_clb& context_clb = nullptr)
		{
			return add_pass(e_rgpass_type::graphics, builder_clb, context_clb);
		}

		inline rgpass* add_compute_pass(
			const rgpass_builder_clb& builder_clb,
			const rgpass_process_clb& context_clb = nullptr)
		{
			return add_pass(e_rgpass_type::compute, builder_clb, context_clb);
		}

		// [utility] pre-made pass that resolves source into dest
		INFLUX_RG_API rgpass* add_copypass(graphics::resource* source, graphics::resource* dest, bool keep_source);

		// [utility] pre-made pass that clears dest resource
		INFLUX_RG_API rgpass* add_clear_pass(graphics::resource* dest, const clear_args& args = {});

		// importing resources allows rendergraph to operate on the given resource.
		// it will not (de)allocate
		INFLUX_RG_API result<> import_texture(const rgname& name, graphics::resource* resource);
		INFLUX_RG_API result<> import_buffer(const rgname& name, graphics::resource* resource);
		
		// todo >>
		INFLUX_RG_API result<> export_texture(const rgname& name, graphics::resource* resource);
		INFLUX_RG_API result<> export_buffer(const rgname& name, graphics::resource* resource);

		/* removes imported resources from book-keeping ! does not de-allocate their resources !*/
		INFLUX_RG_API result<> remove_imported_texture(const rgname& name);
		INFLUX_RG_API result<> remove_imported_buffer(const rgname& name);

		/* resets all resources (also resets graph) */
		INFLUX_RG_API void reset_resources();

		/* resets passes & connections, but keeps listed resources around (imported) */
		INFLUX_RG_API void reset_graph();

		/* debug info dumps */
		INFLUX_RG_API string make_dump();
		INFLUX_RG_API string make_resources_dump();
		
	private:
		global_config m_config{};
		rgpool* m_pool{};

		vector<rgpass> m_passes{};
		vector<rglayer> m_layers{};
		vector<rgbuffer*> m_buffers{};
		vector<rgtexture*> m_textures{};

		vector<vector<uint64>> m_adjacency_lists{};

		umap<rgtexture_id, rgtexture*> m_id_to_texture_map;
		umap<rgbuffer_id, rgbuffer*> m_id_to_buffer_map;
		umap<rgpass_id, rgpass*> m_id_to_pass_map;

		umap<rgname, rgtexture_id> m_texture_name_to_id_map;
		umap<rgname, rgbuffer_id> m_buffer_name_to_id_map;
		umap<rgbuffer_readwrite_id, rgbuffer_id> m_buffer_uav_counter_map;

		static constexpr uint8 k_num_descriptor_types = static_cast<uint8>(rgdescriptor_type::count);
		umap<rgtexture_id, texture_view_desc[k_num_descriptor_types]> m_texid_to_viewdesc_map;
		umap<rgtexture_id, graphics::descriptor_handle[k_num_descriptor_types]> m_texid_to_descriptors_map;
		umap<rgtexture_id, graphics::base*[k_num_descriptor_types]> m_texid_to_deviceobjects_map;

		umap<rgrendertarget_id, math::colour_rgba> m_rtid_to_clear_map;

		umap<rgbuffer_id, buffer_view_desc[k_num_descriptor_types]> m_bufid_to_viewdesc_map;
		umap<rgbuffer_id, graphics::descriptor_handle[k_num_descriptor_types]> m_bufid_to_descriptors_map;
		umap<rgbuffer_id, graphics::base* [k_num_descriptor_types]> m_bufid_to_deviceobjects_map;

		/* building the render graph */
		void build_adjacency();
		void sort_topological();
		void build_layers();
		void cull_passes();
		void calc_resource_lifetimes();
		void depth_search(uint64 parent_idx, vector<bool>& visited_list, vector<uint64>& topo_sorted_passes);

		/* creates views (rtv/dsv/srv/samp) based on how the resource will be used in our rendergraph */
		result<> create_texture_views(graphics::device&, rgtexture_id);
		result<> create_buffer_views(graphics::device&, rgbuffer_id);

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

		bool execute_validation_checks() const;
	};
}