#pragma once

#include "rgcommon.h"

namespace influx::rendergraph
{
	class rendergraph;
	class rgpass;

	class rgpass_builder final
	{
	public:
		bool is_texture_declared(const rgname& name) const;
		bool is_buffer_declared(const rgname& name) const;
		void declare_texture(const rgname& name, const texture_desc& desc);
		void declare_buffer(const rgname& name, const buffer_desc& desc);

		void dummy_write_texture(const rgname& name);
		void dummy_read_texture(const rgname& name);
		void dummy_write_buffer(const rgname& name);
		void dummy_read_buffer(const rgname& name);

		rgtex_copysrc_id read_copysrc_texture(const rgname& name);
		rgtex_copydst_id write_copydst_texture(const rgname& name);
		rgbuf_copysrc_id read_copysrc_buffer(const rgname& name);
		rgbuf_copydst_id write_copydst_buffer(const rgname& name);

		rgbuf_indargs_id	read_indirect_args_buffer(const rgname& name);
		rgbuf_index_id		read_index_buffer(const rgname&);

		rgtexture_readonly_id read_texture(const rgname& name, rgread_access read_acc = rgread_access::all_shader,
			uint32 first_mip = 0u, uint32 num_mips = -1, uint32 first_slice = 0u, uint32 slice_count = -1);

		rgtexture_readwrite_id write_texture(const rgname& name,
			uint32 first_mip = 0u, uint32 num_mips = -1, uint32 first_slice = 0u, uint32 slice_count = -1);

		rgrendertarget_id write_rendertarget(const rgname& name, rgaccess load_store_op,
			uint32 first_mip = 0u, uint32 num_mips = -1, uint32 first_slice = 0u, uint32 slice_count = -1);

		rgdepthtarget_id write_depthtarget(const rgname& name, rgaccess load_store_op,
			uint32 first_mip = 0u, uint32 num_mips = -1, uint32 first_slice = 0u, uint32 slice_count = -1);

		rgdepthtarget_id read_depthtarget(const rgname& name, rgaccess load_store_op,
			uint32 first_mip = 0u, uint32 num_mips = -1, uint32 first_slice = 0u, uint32 slice_count = -1);

		rgbuffer_readonly_id read_buffer(const rgname& name, rgread_access read_acc = rgread_access::all_shader,
			uint32 offset = 0u, uint32 size = -1);

		rgbuffer_readwrite_id write_buffer(const rgname& name, uint32 offset = 0u, uint32 size = -1);
		rgbuffer_readwrite_id write_buffer(const rgname& name, const rgname& counter_name, uint32 offset = 0, uint32 size = -1);

		void set_viewport(uint32 width, uint32 height);

		texture_desc get_texture_desc(const rgname& name) const;
		buffer_desc get_buffer_desc(const rgname& name) const;

	private:
		friend class rendergraph;
		rendergraph& m_graph;
		rgpass& m_pass;

		rgpass_builder(rendergraph& graph, rgpass& pass);

		rgtexture_readonly_id	read_texture_impl(const rgname& name, rgread_access read_access, const texture_desc& desc);
		rgtexture_readwrite_id	write_texture_impl(const rgname& name, const texture_desc& desc);
		rgrendertarget_id		write_rendertarget_impl(const rgname& name, rgaccess load_store_op, const texture_desc& desc);
		rgdepthtarget_id		write_depthtarget_impl(const rgname& name, rgaccess load_store_op, rgaccess stencil_load_store_op, const texture_desc& desc);
		rgdepthtarget_id		read_depthtarget_impl(const rgname& name, rgaccess load_store_op, rgaccess stencil_load_store_op, const texture_desc& desc);

		rgbuffer_readonly_id	read_buffer_impl(const rgname& name, rgread_access read_access, const buffer_desc& desc);
		rgbuffer_readwrite_id	write_buffer_impl(const rgname& name, const buffer_desc& desc);
		rgbuffer_readwrite_id	write_buffer_impl(const rgname& name, const rgname& counter_name, const buffer_desc& desc);
	};
}