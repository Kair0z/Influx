#pragma once

#include "rgcommon.h"
#include "rendergraph.h"

namespace influx::rendergraph
{
	class rendergraph;
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
		INFLUX_RG_API void set_viewport(uint32 width, uint32 height);

		INFLUX_RG_API result<bool> is_texture_declared(const rgname& name) const;
		INFLUX_RG_API result<bool> is_buffer_declared(const rgname& name) const;
		INFLUX_RG_API result<> declare_texture(const rgname& name, const texture_desc& desc);
		INFLUX_RG_API result<> declare_buffer(const rgname& name, const buffer_desc& desc);

		INFLUX_RG_API result<> dummy_write_texture(const rgname& name);
		INFLUX_RG_API result<> dummy_read_texture(const rgname& name);
		INFLUX_RG_API result<> dummy_write_buffer(const rgname& name);
		INFLUX_RG_API result<> dummy_read_buffer(const rgname& name);

		INFLUX_RG_API result<rgtex_copysrc_id> read_copysrc_texture(const rgname& name);
		INFLUX_RG_API result<rgtex_copydst_id> write_copydst_texture(const rgname& name);
		INFLUX_RG_API result<rgbuf_copysrc_id> read_copysrc_buffer(const rgname& name);
		INFLUX_RG_API result<rgbuf_copydst_id> write_copydst_buffer(const rgname& name);

		INFLUX_RG_API result<rgbuf_indargs_id> read_indirect_args_buffer(const rgname& name);
		INFLUX_RG_API result<rgbuf_index_id> read_index_buffer(const rgname&);

		INFLUX_RG_API
		result<rgtexture_readonly_id> read_texture(
			const rgname& name, 
			rgread_access read_acc = rgread_access::all_shader, 
			const texture_desc_options& = {});

		INFLUX_RG_API
		result<rgtexture_readwrite_id> write_texture(
			const rgname& name, 
			const texture_desc_options& = {});

		INFLUX_RG_API
		result<rgrendertarget_id> write_rendertarget(
			const rgname& name, 
			rgaccess load_store_op,
			const texture_desc_options& = {});

		INFLUX_RG_API
		result<rgdepthtarget_id> write_depthtarget(
			const rgname& name, 
			rgaccess load_store_op,
			const texture_desc_options& = {});

		INFLUX_RG_API
		result<rgdepthtarget_id> read_depthtarget(
			const rgname& name, 
			rgaccess load_store_op,
			const texture_desc_options& = {});

		INFLUX_RG_API
		result<rgbuffer_readonly_id> read_buffer(
			const rgname& name, 
			rgread_access read_acc = rgread_access::all_shader,
			uint32 offset = 0u, uint32 size = -1);

		INFLUX_RG_API
		result<rgbuffer_readwrite_id> write_buffer(
			const rgname& name, 
			uint32 offset = 0u, uint32 size = -1);
		
		INFLUX_RG_API
		result<rgbuffer_readwrite_id> write_buffer(
			const rgname& name, 
			const rgname& counter_name, 
			uint32 offset = 0, uint32 size = -1);

		INFLUX_RG_API result<texture_desc> get_texture_desc(const rgname& name) const;
		INFLUX_RG_API result<buffer_desc> get_buffer_desc(const rgname& name) const;
		INFLUX_RG_API result<rgtexture_id> get_texture_id(const rgname& name) const;
		INFLUX_RG_API result<rgbuffer_id> get_buffer_id(const rgname& name) const;

	private:
		friend class rendergraph;
		rendergraph& m_graph;
		rgpass& m_pass;

		rgpass_builder(rendergraph& graph, rgpass& pass);

		result<rgtexture_readonly_id>	read_texture_impl(const rgname& name, rgread_access read_access, const texture_view_desc& desc);
		result<rgtexture_readwrite_id>	write_texture_impl(const rgname& name, const texture_view_desc& desc);
		result<rgrendertarget_id>		write_rendertarget_impl(const rgname& name, rgaccess load_store_op, const texture_view_desc& desc);
		result<rgdepthtarget_id>		write_depthtarget_impl(const rgname& name, rgaccess load_store_op, rgaccess stencil_load_store_op, const texture_view_desc& desc);
		result<rgdepthtarget_id>		read_depthtarget_impl(const rgname& name, rgaccess load_store_op, rgaccess stencil_load_store_op, const texture_view_desc& desc);

		result<rgbuffer_readonly_id>	read_buffer_impl(const rgname& name, rgread_access read_access, const buffer_view_desc& desc);
		result<rgbuffer_readwrite_id>	write_buffer_impl(const rgname& name, const buffer_view_desc& desc);
		result<rgbuffer_readwrite_id>	write_buffer_impl(const rgname& name, const rgname& counter_name, const buffer_view_desc& desc);
	};
}