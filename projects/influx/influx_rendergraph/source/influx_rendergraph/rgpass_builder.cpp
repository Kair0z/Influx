#include "rendergraph_pch.h"
#include "rgpass_builder.h"

#include "rgresources.h"
#include "rendergraph.h"

namespace influx::rendergraph
{
	rgpass_builder::rgpass_builder(rendergraph& graph, rgpass& pass)
		: m_graph{graph}
		, m_pass{pass}
	{

	}

	bool rgpass_builder::is_texture_declared(const rgname& name) const
	{
		return m_graph.is_texture_declared(name);
	}
	bool rgpass_builder::is_buffer_declared(const rgname& name) const
	{
		return m_graph.is_buffer_declared(name);
	}
	void rgpass_builder::declare_texture(const rgname& name, const texture_desc& desc)
	{
		m_pass.m_texture_creates.insert(m_graph.declare_texture(name, desc));
	}
	void rgpass_builder::declare_buffer(const rgname& name, const buffer_desc& desc)
	{
		m_pass.m_buffer_creates.insert(m_graph.declare_buffer(name, desc));
	}
	void rgpass_builder::dummy_write_texture(const rgname& name)
	{
		m_pass.m_texture_writes.insert(m_graph.get_texture_id(name));
	}
	void rgpass_builder::dummy_read_texture(const rgname& name)
	{
		m_pass.m_texture_reads.insert(m_graph.get_texture_id(name));
	}
	void rgpass_builder::dummy_write_buffer(const rgname& name)
	{
		m_pass.m_buffer_writes.insert(m_graph.get_buffer_id(name));
	}
	void rgpass_builder::dummy_read_buffer(const rgname& name)
	{
		m_pass.m_buffer_reads.insert(m_graph.get_buffer_id(name));
	}

	rgtex_copysrc_id rgpass_builder::read_copysrc_texture(const rgname& name)
	{
		rgtex_copysrc_id copy_src_id = m_graph.read_copysrc_texture(name);
		
		// get the texture id from the copysrc id
		rgtexture_id res_id(copy_src_id);
		m_pass.m_texture_state_map[res_id] = graphics::e_resource_state::copy_src;
		m_pass.m_texture_reads.insert(res_id);

		return copy_src_id;
	}
	rgtex_copydst_id rgpass_builder::write_copydst_texture(const rgname& name)
	{
		rgtex_copydst_id copy_dest_id = m_graph.write_copydst_texture(name);

		rgtexture_id res_id(copy_dest_id);
		m_pass.m_texture_state_map[res_id] = graphics::e_resource_state::copy_dst;
		if (!m_pass.m_texture_creates.contains(res_id))
		{
			dummy_read_texture(name);
		}

		m_pass.m_texture_writes.insert(res_id);
		rgtexture* texture = m_graph.get_texture(res_id);
		if (texture->is_imported()) m_pass.m_flags |= e_rgpass_flags::force_no_cull;

		return copy_dest_id;
	}
	rgbuf_copysrc_id rgpass_builder::read_copysrc_buffer(const rgname& name)
	{
		rgbuf_copysrc_id copy_src_id = m_graph.read_copysrc_buffer(name);
		rgbuffer_id res_id(copy_src_id);
		m_pass.m_buffer_state_map[res_id] = graphics::e_resource_state::copy_src;
		m_pass.m_buffer_reads.insert(res_id);
		return copy_src_id;
	}
	rgbuf_copydst_id rgpass_builder::write_copydst_buffer(const rgname& name)
	{
		rgbuf_copydst_id copy_dst_id = m_graph.write_copydst_buffer(name);
		rgbuffer_id res_id(copy_dst_id);
		m_pass.m_buffer_state_map[res_id] = graphics::e_resource_state::copy_dst;
		if (!m_pass.m_buffer_creates.contains(res_id))
		{
			dummy_read_buffer(name);
		}
		m_pass.m_buffer_writes.insert(res_id);
		rgbuffer* buffer = m_graph.get_buffer(res_id);
		if (buffer->is_imported()) m_pass.m_flags |= e_rgpass_flags::force_no_cull;
		return copy_dst_id;
	}
	rgbuf_indargs_id rgpass_builder::read_indirect_args_buffer(const rgname& name)
	{
		rgbuf_indargs_id ind_args_id = m_graph.read_indirect_args_buffer(name);
		rgbuffer_id res_id(ind_args_id);
		m_pass.m_buffer_state_map[res_id] = graphics::e_resource_state::indirect_args;
		m_pass.m_buffer_reads.insert(res_id);
		return ind_args_id;
	}
	rgbuf_index_id rgpass_builder::read_index_buffer(const rgname& name)
	{
		rgbuf_index_id index_id = m_graph.read_index_buffer(name);
		rgbuffer_id res_id(index_id);
		m_pass.m_buffer_state_map[res_id] = graphics::e_resource_state::indexbuffer;
		m_pass.m_buffer_reads.insert(res_id);
		return index_id;
	}

	rgtexture_readonly_id rgpass_builder::read_texture(const rgname& name, rgread_access read_acc, uint32 first_mip, uint32 num_mips, uint32 first_slice, uint32 slice_count)
	{
		texture_desc desc{};
		return read_texture_impl(name, read_acc, desc);
	}
	rgtexture_readwrite_id rgpass_builder::write_texture(const rgname& name, uint32 first_mip, uint32 num_mips, uint32 first_slice, uint32 slice_count)
	{
		texture_desc desc{};
		return write_texture_impl(name, desc);
	}
	rgrendertarget_id rgpass_builder::write_rendertarget(const rgname& name, rgaccess load_store_op, uint32 first_mip, uint32 num_mips, uint32 first_slice, uint32 slice_count)
	{
		texture_desc desc{};
		return write_rendertarget_impl(name, load_store_op, desc);
	}
	rgdepthtarget_id rgpass_builder::write_depthtarget(const rgname& name, rgaccess load_store_op, uint32 first_mip, uint32 num_mips, uint32 first_slice, uint32 slice_count)
	{
		texture_desc desc{};
		rgaccess stencil_access{};
		stencil_access.m_load = e_rg_load::no_access;
		stencil_access.m_store = e_rg_store::no_access;
		return write_depthtarget_impl(name, load_store_op, stencil_access, desc);
	}
	rgdepthtarget_id rgpass_builder::read_depthtarget(const rgname& name, rgaccess load_store_op, uint32 first_mip, uint32 num_mips, uint32 first_slice, uint32 slice_count)
	{
		texture_desc desc{};
		rgaccess stencil_access{};
		stencil_access.m_load = e_rg_load::no_access;
		stencil_access.m_store = e_rg_store::no_access;
		return read_depthtarget_impl(name, load_store_op, stencil_access, desc);
	}
	rgbuffer_readonly_id rgpass_builder::read_buffer(const rgname& name, rgread_access read_acc, uint32 offset, uint32 size)
	{
		buffer_desc desc{};
		desc.m_bytestride = offset;
		desc.m_bytesize = size;
		return read_buffer_impl(name, read_acc, desc);
	}
	rgbuffer_readwrite_id rgpass_builder::write_buffer(const rgname& name, uint32 offset, uint32 size)
	{
		buffer_desc desc{};
		desc.m_bytestride = offset;
		desc.m_bytesize = size;
		return write_buffer_impl(name, desc);
	}
	rgbuffer_readwrite_id rgpass_builder::write_buffer(const rgname& name, const rgname& counter_name, uint32 offset, uint32 size)
	{
		buffer_desc desc{};
		desc.m_bytestride = offset;
		desc.m_bytesize = size;
		return write_buffer_impl(name, counter_name, desc);
	}

	void rgpass_builder::set_viewport(uint32 width, uint32 height)
	{
		m_pass.m_width = width;
		m_pass.m_height = height;
	}

	texture_desc rgpass_builder::get_texture_desc(const rgname& name) const
	{
		return m_graph.get_texture_desc(name);
	}

	buffer_desc rgpass_builder::get_buffer_desc(const rgname& name) const
	{
		return m_graph.get_buffer_desc(name);
	}

#pragma region impl
	rgtexture_readonly_id rgpass_builder::read_texture_impl(const rgname& name, rgread_access read_access, const texture_desc& desc)
	{
		return rgtexture_readonly_id();
	}
	rgtexture_readwrite_id rgpass_builder::write_texture_impl(const rgname& name, const texture_desc& desc)
	{
		return rgtexture_readwrite_id();
	}
	rgrendertarget_id rgpass_builder::write_rendertarget_impl(const rgname& name, rgaccess load_store_op, const texture_desc& desc)
	{
		return rgrendertarget_id();
	}
	rgdepthtarget_id rgpass_builder::write_depthtarget_impl(const rgname& name, rgaccess load_store_op, rgaccess stencil_load_store_op, const texture_desc& desc)
	{
		return rgdepthtarget_id();
	}
	rgdepthtarget_id rgpass_builder::read_depthtarget_impl(const rgname& name, rgaccess load_store_op, rgaccess stencil_load_store_op, const texture_desc& desc)
	{
		return rgdepthtarget_id();
	}
	rgbuffer_readonly_id rgpass_builder::read_buffer_impl(const rgname& name, rgread_access read_access, const buffer_desc& desc)
	{
		return rgbuffer_readonly_id();
	}
	rgbuffer_readwrite_id rgpass_builder::write_buffer_impl(const rgname& name, const buffer_desc& desc)
	{
		return rgbuffer_readwrite_id();
	}
	rgbuffer_readwrite_id rgpass_builder::write_buffer_impl(const rgname& name, const rgname& counter_name, const buffer_desc& desc)
	{
		return rgbuffer_readwrite_id();
	}
#pragma endregion
}