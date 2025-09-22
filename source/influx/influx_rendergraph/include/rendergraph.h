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
#include "core/pointer.h"

// influx::rendergraph
#include "rgcommon.h"
#include "rgresources.h"
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
		rhi_commandlist& get_commandlist()
		{ return m_commandlist; }

		struct resource_and_view final
		{
			rhi_resource* m_resource = nullptr;
			rhi_descriptor m_descriptor = {};
		};

		/* get resources & descriptors by ID */
		INFLUX_RG_API result<resource_and_view> get_copysrc(rgtex_copysrc_id);
		INFLUX_RG_API result<resource_and_view> get_copysrc(rgbuf_copysrc_id);
		INFLUX_RG_API result<resource_and_view> get_copydst(rgtex_copydst_id);
		INFLUX_RG_API result<resource_and_view> get_copydst(rgbuf_copydst_id);
		INFLUX_RG_API result<resource_and_view> get_vertexbuffer(rgbuf_vertex_id);
		INFLUX_RG_API result<resource_and_view> get_indexbuffer(rgbuf_index_id);
		INFLUX_RG_API result<resource_and_view> get_constbuffer(rgbuf_const_id);
		INFLUX_RG_API result<resource_and_view> get_indirect_args_resource(rgbuf_indargs_id);
		INFLUX_RG_API result<resource_and_view> get_rtv(rgid_rtv id);
		INFLUX_RG_API result<resource_and_view> get_dsv(rgid_rtv id);
		INFLUX_RG_API result<resource_and_view> get_read_texture(rgid_srv_tex id);
		INFLUX_RG_API result<resource_and_view> get_write_texture(rgid_uav_tex id);
		INFLUX_RG_API result<resource_and_view> get_read_buffer(rgid_srv_buff id);
		INFLUX_RG_API result<resource_and_view> get_write_buffer(rgid_uav_buff id);

		/* get resources & descriptors by rgname */
		INFLUX_RG_API result<resource_and_view> get_copysrc_texture(const rgname&);
		INFLUX_RG_API result<resource_and_view> get_copysrc_buffer(const rgname&);
		INFLUX_RG_API result<resource_and_view> get_copydst_texture(const rgname&);
		INFLUX_RG_API result<resource_and_view> get_copydst_buffer(const rgname&);
		INFLUX_RG_API result<resource_and_view> get_constbuffer(const rgname&);
		INFLUX_RG_API result<resource_and_view> get_read_texture(const rgname&);
		INFLUX_RG_API result<resource_and_view> get_write_texture(const rgname&);
		INFLUX_RG_API result<resource_and_view> get_read_buffer(const rgname&);
		INFLUX_RG_API result<resource_and_view> get_write_buffer(const rgname&);

		/* get resources & descriptors by written order by rgpass_builder */
		INFLUX_RG_API result<resource_and_view> get_rtv(uint32 at_index = 0u);
		INFLUX_RG_API result<resource_and_view> get_dsv();
		INFLUX_RG_API result<resource_and_view> get_read_texture(uint32 index);
		INFLUX_RG_API result<resource_and_view> get_write_texture(uint32 index);
		INFLUX_RG_API result<resource_and_view> get_read_buffer(uint32 index);
		INFLUX_RG_API result<resource_and_view> get_write_buffer(uint32 index);

		/* get GPU descriptor heap */
		INFLUX_RG_API rhi_descheap& get_descheap_gpu(e_gpu_descheap slot);

	private:
		rgpass_context(rendergraph& rg, rhi_commandlist& cmdlist, const rgpass& pass) : m_graph{ rg }, m_commandlist{ cmdlist }, m_pass{ pass }{}
		rhi_commandlist& m_commandlist;
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
		INFLUX_RG_API rendergraph(const global_config& config, rhi_device& device);

		INFLUX_RG_API void cleanup(rhi_device& device);

		INFLUX_RG_API ~rendergraph();

		INFLUX_RG_API void build();

		// execute the graph onto a single command list
		INFLUX_RG_API result<> execute(
			rhi_commandlist& commandlist,
			rhi_device& device);

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
		INFLUX_RG_API rgpass* add_copypass(rhi_resource* source, rhi_resource* dest, bool keep_source);

		// [utility] pre-made pass that clears dest resource
		INFLUX_RG_API rgpass* add_clear_pass(rhi_resource* dest, const clear_args& args = {});

		// importing resources allows rendergraph to operate on the given resource.
		// it will not (de)allocate these, that's the job of the external owner of the resource
		INFLUX_RG_API result<> import_texture(rhi_resource* resource);
		INFLUX_RG_API result<> import_buffer(rhi_resource* resource);
		INFLUX_RG_API result<> import_texture(const rgname& name, rhi_resource* resource);
		INFLUX_RG_API result<> import_buffer(const rgname& name, rhi_resource* resource);
		
#if 0 // todo >>
		INFLUX_RG_API result<> export_texture(const rgname& name, rhi_resource* resource);
		INFLUX_RG_API result<> export_buffer(const rgname& name, rhi_resource* resource);
#endif

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
		
		INFLUX_RG_API vector<rgtexture_info> get_textures() const;
		INFLUX_RG_API vector<rgbuffer_info> get_buffers() const;

		/* external descriptor heaps */
		INFLUX_RG_API result<> bind_ext_descheap(e_ext_descheap_slot slot, rhi_descheap& heap, bool allow_override = false);
		INFLUX_RG_API result<> unbind_ext_descheap(e_ext_descheap_slot slot);
		INFLUX_RG_API bool is_ext_descheap_bound(e_ext_descheap_slot slot) const;

		INFLUX_RG_API static texture_desc translate_texture_desc(const rhi_resource& desc);
		INFLUX_RG_API static buffer_desc translate_buffer_desc(const rhi_resource& desc);

	private:
		global_config m_config{};
		rgpool* m_pool{};

		vector<rgpass> m_passes{};
		vector<rglayer> m_layers{};
		vector<rgbuffer*> m_buffers{};
		vector<rgtexture*> m_textures{};

		vector<uint64> m_topo_sorted_passes;
		vector<vector<uint64>> m_adjacency_lists{};
		umap<rgpass_id, rgpass*> m_id_to_pass_map;
		umap<rgid_uav_buff, rgbuffer_id> m_buffer_uav_counter_map;

		umap<rgtexture_id, rgtexture*> m_id_to_texture_map;
		umap<rgbuffer_id, rgbuffer*> m_id_to_buffer_map;
		umap<rgname, rgtexture_id> m_texture_name_to_id_map;
		umap<rgname, rgbuffer_id> m_buffer_name_to_id_map;
		umap<rgid_rtv, math::colour_rgba> m_rtid_to_clear_map;

		static constexpr uint8 k_num_descriptor_types = static_cast<uint8>(rgdescriptor_type::count);
		umap<rgtexture_id, texture_view_desc[k_num_descriptor_types]> m_texid_to_viewdesc_map;
		umap<rgtexture_id, rhi_descriptor[k_num_descriptor_types]> m_texid_to_descriptors_map;
		//umap<rgtexture_id, graphics::base*[k_num_descriptor_types]> m_texid_to_deviceobjects_map;
		umap<rgbuffer_id, buffer_view_desc[k_num_descriptor_types]> m_bufid_to_viewdesc_map;
		umap<rgbuffer_id, rhi_descriptor[k_num_descriptor_types]> m_bufid_to_descriptors_map;
		//umap<rgbuffer_id, graphics::base* [k_num_descriptor_types]> m_bufid_to_deviceobjects_map;

		/* building the render graph */
		void build_adjacency();
		void sort_topological();
		void build_layers();
		void cull_passes();
		void calc_resource_lifetimes();
		void depth_search(uint64 parent_idx, vector<bool>& visited_list, vector<uint64>& topo_sorted_passes);

		/* creates views (rtv/dsv/srv/samp) based on how the resource will be used in our rendergraph */
		result<> create_texture_views(rhi_device&, rgtexture_id);
		result<> create_buffer_views(rhi_device&, rgbuffer_id);

		// misc
		rhi_descriptor get_rtv(rgtexture_id id);
		rhi_descriptor get_dsv(rgtexture_id id);
		rhi_descriptor get_readonly(rgtexture_id id);
		rhi_descriptor get_readwrite(rgtexture_id id);

		// rgpass_builder uses these
		rgtexture_id declare_texture(const rgname& name, const texture_desc& desc);
		rgbuffer_id declare_buffer(const rgname& name, const buffer_desc& desc);
		bool is_texture_declared(rgtexture_id id) const;
		bool is_buffer_declared(rgbuffer_id id) const;
		bool is_texture_declared(const rgname& name) const;
		bool is_buffer_declared(const rgname& name) const;
		
		result<rgtex_copysrc_id> read_copysrc_texture(const rgname& name);
		result<rgtex_copydst_id> write_copydst_texture(const rgname& name);
		result<rgbuf_copysrc_id> read_copysrc_buffer(const rgname& name);
		result<rgbuf_copydst_id> write_copydst_buffer(const rgname& name);
		result<rgbuf_indargs_id> read_indirect_args_buffer(const rgname& name);
		result<rgbuf_vertex_id> read_vertex_buffer(const rgname& name);
		result<rgbuf_index_id> read_index_buffer(const rgname& name);
		result<rgbuf_const_id> read_constant_buffer(const rgname& name);

		result<rgid_rtv> rendertarget(const rgname& name, const texture_view_desc& view_desc);
		result<rgid_dsv> depthtarget(const rgname& name, const texture_view_desc& view_desc);
		result<rgid_srv_tex> read_texture(const rgname& name, const texture_view_desc& view_desc);
		result<rgid_uav_tex> write_texture(const rgname& name, const texture_view_desc& view_desc);
		result<rgid_srv_buff> read_buffer(const rgname& name, const buffer_view_desc& view_desc);
		result<rgid_uav_buff> write_buffer(const rgname& name, const buffer_view_desc& view_desc);
		result<rgid_uav_buff> write_buffer(const rgname& name, const rgname& counter_name, const buffer_view_desc& view_desc);

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

#include "rgpass_builder.h"