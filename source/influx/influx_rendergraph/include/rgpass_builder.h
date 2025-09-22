#pragma once

#include "rgcommon.h"
#include "rendergraph.h"

namespace influx::rendergraph
{
	class rgpass;

	// optional specification
	struct texture_desc_options final
	{
		uint32 m_first_mip = 0u;
		uint32 m_num_mips = (uint32)-1;
		uint32 m_first_slice = 0u;
		uint32 m_slice_count = (uint32)-1;
	};

	class rgpass_builder final
	{
	public:
		// [declaring resources]
		// resources that are not declared cannot be used in the rendergraph
		// they will be logged as warnings, but will be handled by dummy-resources
		// some of the below methods auto-declare their resources
		INFLUX_RG_API bool is_texture_declared(const rgname& name) const;
		INFLUX_RG_API bool is_buffer_declared(const rgname& name) const;
		INFLUX_RG_API result<> declare_texture(const rgname& name, const texture_desc& desc);
		INFLUX_RG_API result<> declare_buffer(const rgname& name, const buffer_desc& desc);

		// USE declarations: listing the resources (and their views) in this pass.
		INFLUX_RG_API result<rgid_srv_buff> read_buffer(const rgname& name, rgread_access read_acc = rgread_access::all_shader, uint32 offset = 0u, uint32 size = -1);
		INFLUX_RG_API result<rgid_uav_buff> write_buffer(const rgname& name, uint32 offset = 0u, uint32 size = -1);
		INFLUX_RG_API result<rgid_uav_buff> write_buffer(const rgname& name, const rgname& counter_name, uint32 offset = 0, uint32 size = -1);
		INFLUX_RG_API result<rgid_srv_tex> read_texture(const rgname& name, rgread_access read_acc = rgread_access::all_shader, const texture_desc_options & = {});
		INFLUX_RG_API result<rgid_uav_tex> write_texture(const rgname& name, const texture_desc_options & = {});

		INFLUX_RG_API result<rgtex_copysrc_id>	read_copysrc_texture(const rgname& name);
		INFLUX_RG_API result<rgtex_copydst_id>	write_copydst_texture(const rgname& name);
		INFLUX_RG_API result<rgbuf_copysrc_id>	read_copysrc_buffer(const rgname& name);
		INFLUX_RG_API result<rgbuf_copydst_id>	write_copydst_buffer(const rgname& name);
		INFLUX_RG_API result<rgbuf_indargs_id>	read_indirect_args_buffer(const rgname& name);
		INFLUX_RG_API result<rgbuf_vertex_id>	read_vertex_buffer(const rgname&);
		INFLUX_RG_API result<rgbuf_index_id>	read_index_buffer(const rgname&);
		INFLUX_RG_API result<rgbuf_const_id>	read_constbuffer(const rgname&);
 
		INFLUX_RG_API result<rgid_rtv> write_rendertarget(const rgname& name, rgaccess load_store_op, const texture_desc_options& = {});
		INFLUX_RG_API result<rgid_dsv> write_depthtarget(const rgname& name, rgaccess load_store_op, const texture_desc_options& = {});
		INFLUX_RG_API result<rgid_dsv> read_depthtarget(const rgname& name, rgaccess load_store_op, const texture_desc_options& = {});

		// [auto declare versions]
		inline result<rgid_srv_buff> read_buffer(const rgname& name, const buffer_desc& resource_desc,
			rgread_access read_acc = rgread_access::all_shader, uint32 offset = 0u, uint32 size = -1)
		{
			using result_type = result<rgid_srv_buff>;
			if (!is_buffer_declared(name))
			{
				auto res = declare_buffer(name, resource_desc);
				if (!res) return result_type::make_error("failed implicitly declaring new buffer resource!");
			}
			return read_buffer(name, read_acc, offset, size);
		}
		inline result<rgid_uav_buff> write_buffer(const rgname& name, const buffer_desc& resource_desc,
			const rgname& counter_name, uint32 offset = 0, uint32 size = -1)
		{
			using result_type = result<rgid_uav_buff>;
			if (!is_buffer_declared(name))
			{
				auto res = declare_buffer(name, resource_desc);
				if (!res) return result_type::make_error("failed implicitly declaring new buffer resource!");
			}
			return write_buffer(name, counter_name, offset, size);
		}
		inline result<rgbuf_const_id> read_constbuffer(const rgname& name, const buffer_desc& desc)
		{
			using result_type = result<rgbuf_const_id>;
			if (!is_buffer_declared(name))
			{
				auto res = declare_buffer(name, desc);
				if (!res) return result_type::make_error("failed implicitly declaring new buffer resource!");
			}
			return read_constbuffer(name);
		}
		inline result<rgid_rtv> write_rendertarget(const rgname& name, const texture_desc& desc,
			rgaccess load_store_op, const texture_desc_options& options = {})
		{
			using result_type = result<rgid_rtv>;
			if (!is_texture_declared(name))
			{
				auto declare_res = declare_texture(name, desc);
			}
			return write_rendertarget(name, load_store_op, options);
		}
		inline result<rgid_dsv> write_depthtarget(const rgname& name, const texture_desc& desc,
			rgaccess load_store_op, const texture_desc_options& options = {})
		{
			using result_type = result<rgid_dsv>;
			if (!is_texture_declared(name))
			{
				auto declare_res = declare_texture(name, desc);
			}
			return write_depthtarget(name, load_store_op, options);
		}
		inline result<rgid_srv_tex> read_texture(const rgname& name, const texture_desc& desc,
			rgread_access read_acc = rgread_access::all_shader, const texture_desc_options& options = {})
		{
			using result_type = result<rgid_srv_tex>;
			if (!is_texture_declared(name))
			{
				auto declare_res = declare_texture(name, desc);
			}
			return read_texture(name, read_acc, options);
		}

		// [auto import versions]
		inline result<rgid_srv_buff> read_buffer(
			rhi_resource* resource,
			rgread_access read_acc = rgread_access::all_shader,
			uint32 offset = 0u,
			uint32 size = -1)
		{
			using result_type = result<rgid_srv_buff>;
			if (resource->get_name().is_empty())
				return result_type::make_error("cannot import resource with no name!");

			const rgname name = resource->get_name();
			if (!m_graph.m_buffer_name_to_id_map.contains(name))
			{
				auto imp_res = m_graph.import_buffer(resource);
				if (!imp_res) return result_type::make_error("failed importing buffer!");
			}
			return read_buffer(name, rendergraph::translate_buffer_desc(*resource), read_acc, offset, size);
		}
		
		inline result<rgid_rtv> write_rendertarget(rhi_resource* resource, rgaccess load_store_op, const texture_desc_options& ops = {})
		{
			using result_type = result<rgid_rtv>;
			if (resource->get_name().is_empty())
				return result_type::make_error("cannot import resource with no name!");

			const rgname name = resource->get_name();
			if (!m_graph.m_texture_name_to_id_map.contains(name))
			{
				auto imp_res = m_graph.import_texture(resource);
				if (!imp_res) return result_type::make_error("failed importing texture!");
			}
			if (!is_texture_declared(name))
			{
				auto decl_res = declare_texture(name, rendergraph::translate_texture_desc(*resource));
				if (!decl_res) return result_type::make_error("failed declaring texture!");
			}
			return write_rendertarget(name, load_store_op, ops);
		}
		inline result<rgid_dsv> write_depthtarget(rhi_resource* resource, rgaccess load_store_op, const texture_desc_options& ops = {})
		{
			using result_type = result<rgid_dsv>;
			if (resource->get_name().is_empty())
				return result_type::make_error("cannot import resource with no name!");

			const rgname name = resource->get_name();
			if (!m_graph.m_texture_name_to_id_map.contains(name))
			{
				auto imp_res = m_graph.import_texture(resource);
				if (!imp_res) return result_type::make_error("failed importing texture!");
			}
			if (!is_texture_declared(name))
			{
				auto decl_res = declare_texture(name, rendergraph::translate_texture_desc(*resource));
				if (!decl_res) return result_type::make_error("failed declaring texture!");
			}
			return write_depthtarget(name, load_store_op, ops);
		}
		inline result<rgid_uav_tex> write_texture(rhi_resource* resource, const texture_desc_options& options = {})
		{
			using result_type = result<rgid_uav_tex>;
			if (resource->get_name().is_empty())
				return result_type::make_error("cannot import resource with no name!");

			const rgname name = resource->get_name();
			if (!m_graph.m_texture_name_to_id_map.contains(name))
			{
				auto imp_res = m_graph.import_texture(resource);
				if (!imp_res) return result_type::make_error("failed importing texture!");
			}
			if (!is_texture_declared(name))
			{
				auto decl_res = declare_texture(name, rendergraph::translate_texture_desc(*resource));
				if (!decl_res) return result_type::make_error("failed declaring texture!");
			}
			return write_texture(name, options);
		}
		inline result<rgid_srv_tex> read_texture(rhi_resource* resource, rgread_access read_acc = rgread_access::all_shader, const texture_desc_options& options = {})
		{
			using result_type = result<rgid_srv_tex>;
			if (resource->get_name().is_empty())
				return result_type::make_error("cannot import resource with no name!");

			const rgname name = resource->get_name();
			if (!m_graph.m_texture_name_to_id_map.contains(name))
			{
				auto imp_res = m_graph.import_texture(resource);
				if (!imp_res) return result_type::make_error("failed importing texture!");
			}
			return read_texture(name, rendergraph::translate_texture_desc(*resource), read_acc, options);
		}
		inline result<rgbuf_vertex_id> read_vertexbuffer(rhi_resource* resource) 
		{
			using result_type = result<rgbuf_vertex_id>;
			if (resource->get_name().is_empty())
				return result_type::make_error("cannot import resource with no name!");

			const rgname name = resource->get_name();
			if (!m_graph.m_buffer_name_to_id_map.contains(name))
			{
				auto imp_res = m_graph.import_buffer(resource);
				if (!imp_res) return result_type::make_error("failed importing buffer!");
			}
			if (!is_buffer_declared(name))
			{
				auto decl_res = declare_buffer(name, rendergraph::translate_buffer_desc(*resource));
				if (!decl_res) return result_type::make_error("failed declaring buffer!");
			}
			return read_vertex_buffer(name);
		}
		inline result<rgbuf_index_id> read_indexbuffer(rhi_resource* resource)
		{
			using result_type = result<rgbuf_index_id>;
			if (resource->get_name().is_empty())
				return result_type::make_error("cannot import resource with no name!");

			const rgname name = resource->get_name();
			if (!m_graph.m_buffer_name_to_id_map.contains(name))
			{
				auto imp_res = m_graph.import_buffer(resource);
				if (!imp_res) return result_type::make_error("failed importing buffer!");
			}
			if (!is_buffer_declared(name))
			{
				auto decl_res = declare_buffer(name, rendergraph::translate_buffer_desc(*resource));
				if (!decl_res) return result_type::make_error("failed declaring buffer!");
			}
			return read_index_buffer(name);
		}

		INFLUX_RG_API result<> dummy_write_texture(const rgname& name);
		INFLUX_RG_API result<> dummy_read_texture(const rgname& name);
		INFLUX_RG_API result<> dummy_write_buffer(const rgname& name);
		INFLUX_RG_API result<> dummy_read_buffer(const rgname& name);

		// [helpers]
		INFLUX_RG_API result<texture_desc>	get_texture_desc(const rgname& name) const;
		INFLUX_RG_API result<buffer_desc>	get_buffer_desc(const rgname& name) const;
		INFLUX_RG_API result<rgtexture_id>	get_texture_id(const rgname& name) const;
		INFLUX_RG_API result<rgbuffer_id>	get_buffer_id(const rgname& name) const;
		
		// signal this pass as erroneous.
		// usually not necessary, but if the pass errors, the rendergraph will not be executed!
		INFLUX_RG_API void register_error_to_current_pass();
		INFLUX_RG_API void set_viewport(uint32 width, uint32 height);
	
	private:
		friend class rendergraph;
		rendergraph& m_graph;
		rgpass& m_pass;

		rgpass_builder(rendergraph& graph, rgpass& pass);

		result<rgid_srv_tex>	read_texture_impl(const rgname& name, rgread_access read_access, const texture_view_desc& desc);
		result<rgid_uav_tex>	write_texture_impl(const rgname& name, const texture_view_desc& desc);
		result<rgid_rtv>		write_rendertarget_impl(const rgname& name, rgaccess load_store_op, const texture_view_desc& desc);
		result<rgid_dsv>		write_depthtarget_impl(const rgname& name, rgaccess load_store_op, rgaccess stencil_load_store_op, const texture_view_desc& desc);
		result<rgid_dsv>		read_depthtarget_impl(const rgname& name, rgaccess load_store_op, rgaccess stencil_load_store_op, const texture_view_desc& desc);

		result<rgid_srv_buff>	read_buffer_impl(const rgname& name, rgread_access read_access, const buffer_view_desc& desc);
		result<rgid_uav_buff>	write_buffer_impl(const rgname& name, const buffer_view_desc& desc);
		result<rgid_uav_buff>	write_buffer_impl(const rgname& name, const rgname& counter_name, const buffer_view_desc& desc);
	};
}