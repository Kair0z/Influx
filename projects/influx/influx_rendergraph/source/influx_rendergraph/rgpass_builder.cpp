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
		texture_view_desc view_desc
		{
			.m_first_slice = first_slice,
			.m_num_slices = slice_count,
			.m_first_mip = first_mip,
			.m_num_mips = num_mips
		};
		return read_texture_impl(name, read_acc, view_desc);
	}
	rgtexture_readwrite_id rgpass_builder::write_texture(const rgname& name, uint32 first_mip, uint32 num_mips, uint32 first_slice, uint32 slice_count)
	{
		texture_view_desc view_desc
		{
			.m_first_slice = first_slice,
			.m_num_slices = slice_count,
			.m_first_mip = first_mip,
			.m_num_mips = num_mips
		};
		return write_texture_impl(name, view_desc);
	}
	rgrendertarget_id rgpass_builder::write_rendertarget(const rgname& name, rgaccess load_store_op, uint32 first_mip, uint32 num_mips, uint32 first_slice, uint32 slice_count)
	{
		texture_view_desc view_desc
		{
			.m_first_slice = first_slice,
			.m_num_slices = slice_count,
			.m_first_mip = first_mip,
			.m_num_mips = num_mips
		};
		return write_rendertarget_impl(name, load_store_op, view_desc);
	}
	rgdepthtarget_id rgpass_builder::write_depthtarget(const rgname& name, rgaccess load_store_op, uint32 first_mip, uint32 num_mips, uint32 first_slice, uint32 slice_count)
	{
		texture_view_desc view_desc
		{
			.m_first_slice = first_slice,
			.m_num_slices = slice_count,
			.m_first_mip = first_mip,
			.m_num_mips = num_mips
		};
		rgaccess stencil_access
		{
			.m_load = e_rg_load::no_access,
			.m_store = e_rg_store::no_access
		};
		return write_depthtarget_impl(name, load_store_op, stencil_access, view_desc);
	}
	rgdepthtarget_id rgpass_builder::read_depthtarget(const rgname& name, rgaccess load_store_op, uint32 first_mip, uint32 num_mips, uint32 first_slice, uint32 slice_count)
	{
		texture_view_desc view_desc
		{
			.m_first_slice = first_slice,
			.m_num_slices = slice_count,
			.m_first_mip = first_mip,
			.m_num_mips = num_mips
		};
		rgaccess stencil_access
		{
			.m_load = e_rg_load::no_access,
			.m_store = e_rg_store::no_access
		};
		return read_depthtarget_impl(name, load_store_op, stencil_access, view_desc);
	}
	rgbuffer_readonly_id rgpass_builder::read_buffer(const rgname& name, rgread_access read_acc, uint32 offset, uint32 size)
	{
		buffer_view_desc view_desc
		{
			.m_offset = offset,
			.m_size = size
		};
		return read_buffer_impl(name, read_acc, view_desc);
	}
	rgbuffer_readwrite_id rgpass_builder::write_buffer(const rgname& name, uint32 offset, uint32 size)
	{
		buffer_view_desc view_desc
		{
			.m_offset = offset,
			.m_size = size
		};
		return write_buffer_impl(name, view_desc);
	}
	rgbuffer_readwrite_id rgpass_builder::write_buffer(const rgname& name, const rgname& counter_name, uint32 offset, uint32 size)
	{
		buffer_view_desc view_desc
		{
			.m_offset = offset,
			.m_size = size
		};
		return write_buffer_impl(name, counter_name, view_desc);
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
	rgtexture_readonly_id rgpass_builder::read_texture_impl(const rgname& name, rgread_access read_access, const texture_view_desc& view_desc)
	{
		rgtexture_readonly_id read_id = m_graph.read_texture(name, view_desc);
		rgtexture_id res_id = read_id.get_resource_id();

		switch (m_pass.m_type)
		{
		case e_rgpass_type::graphics:
			switch (read_access)
			{
			case rgread_access::ps: m_pass.m_texture_state_map[res_id] = graphics::e_resource_state::ps_srv; break;
			case rgread_access::non_ps: m_pass.m_texture_state_map[res_id] = graphics::e_resource_state::cs_srv; break;
			case rgread_access::all_shader: m_pass.m_texture_state_map[res_id] = graphics::e_resource_state::all_srv; break;
			}
			break;

		case e_rgpass_type::compute:
		case e_rgpass_type::async_compute:
			m_pass.m_texture_state_map[res_id] = graphics::e_resource_state::cs_srv;
			break;
		}

		m_pass.m_texture_reads.insert(res_id);
		return read_id;
	}
	rgtexture_readwrite_id rgpass_builder::write_texture_impl(const rgname& name, const texture_view_desc& view_desc)
	{
		rgtexture_readwrite_id rw_id = m_graph.write_texture(name, view_desc);
		rgtexture_id res_id = rw_id.get_resource_id();

		m_pass.m_texture_state_map[res_id] = graphics::e_resource_state::cs_uav;
		
		if (!m_pass.m_texture_creates.contains(res_id))
		{
			dummy_read_texture(name);
		}
		
		m_pass.m_texture_writes.insert(res_id);
		rgtexture* texture = m_graph.get_texture(res_id);
		if (texture->is_imported()) m_pass.m_flags |= e_rgpass_flags::force_no_cull;

		return rw_id;
	}
	rgrendertarget_id rgpass_builder::write_rendertarget_impl(const rgname& name, rgaccess load_store_op, const texture_view_desc& view_desc)
	{
		rgrendertarget_id rt_id = m_graph.rendertarget(name, view_desc);
		rgtexture_id res_id = rt_id.get_resource_id();

		m_pass.m_texture_state_map[res_id] = graphics::e_resource_state::render_target;
		m_pass.m_rtvs.push_back(rgpass::render_target{ .m_texture_id = res_id, .m_access = load_store_op };);

		if (!m_pass.m_texture_creates.contains(res_id))
		{
			dummy_read_texture(name);
		}

		m_pass.m_texture_writes.insert(res_id);
		rgtexture* texture = m_graph.get_texture(res_id);
		if (texture->is_imported()) m_pass.m_flags |= e_rgpass_flags::force_no_cull;
		return rt_id;
	}
	rgdepthtarget_id rgpass_builder::write_depthtarget_impl(const rgname& name, rgaccess load_store_op, rgaccess stencil_load_store_op, const texture_view_desc& view_desc)
	{
		rgdepthtarget_id dt_id = m_graph.depthtarget(name, view_desc);
		rgtexture_id res_id = dt_id.get_resource_id();

		m_pass.m_texture_state_map[res_id] = graphics::e_resource_state::depth_target;
		m_pass.m_dsv = rgpass::depth_stencil{ .m_texture_id = res_id, .m_depth_access = load_store_op, 
			.m_stencil_access = stencil_load_store_op, .m_depth_read_only = false };

		if (!m_pass.m_texture_creates.contains(res_id))
		{
			dummy_read_texture(name);
		}

		m_pass.m_texture_writes.insert(res_id);
		rgtexture* texture = m_graph.get_texture(res_id);
		if (texture->is_imported()) m_pass.m_flags |= e_rgpass_flags::force_no_cull;
		return dt_id;
	}
	rgdepthtarget_id rgpass_builder::read_depthtarget_impl(const rgname& name, rgaccess load_store_op, rgaccess stencil_load_store_op, const texture_view_desc& view_desc)
	{
		rgdepthtarget_id dt_id = m_graph.depthtarget(name, view_desc);
		rgtexture_id res_id = dt_id.get_resource_id();

		m_pass.m_texture_state_map[res_id] = graphics::e_resource_state::depth_readonly;
		m_pass.m_dsv = rgpass::depth_stencil{ .m_texture_id = res_id, .m_depth_access = load_store_op,
			.m_stencil_access = stencil_load_store_op, .m_depth_read_only = true };

		m_pass.m_texture_reads.insert(res_id);
		rgtexture* texture = m_graph.get_texture(res_id);
		if (texture->is_imported()) m_pass.m_flags |= e_rgpass_flags::force_no_cull;
		return dt_id;
	}
	rgbuffer_readonly_id rgpass_builder::read_buffer_impl(const rgname& name, rgread_access read_access, const buffer_view_desc& view_desc)
	{
		rgbuffer_readonly_id read_id = m_graph.read_buffer(name, view_desc);
		rgbuffer_id res_id = read_id.get_resource_id();

		if (m_pass.is_compute_any())
		{
			read_access = rgread_access::non_ps;

			m_pass.m_buffer_state_map[res_id] = graphics::e_resource_state::cs_srv;
		}
		else
		{
			switch (read_access)
			{
			case rgread_access::ps: m_pass.m_buffer_state_map[res_id] = graphics::e_resource_state::ps_srv; break;
			case rgread_access::non_ps: m_pass.m_buffer_state_map[res_id] = graphics::e_resource_state::cs_srv; break;
			case rgread_access::all_shader: m_pass.m_buffer_state_map[res_id] = graphics::e_resource_state::all_srv; break;
			}
		}

		m_pass.m_buffer_reads.insert(res_id);
		return read_id;
	}
	rgbuffer_readwrite_id rgpass_builder::write_buffer_impl(const rgname& name, const buffer_view_desc& view_desc)
	{
		rgbuffer_readwrite_id rw_id = m_graph.write_buffer(name, view_desc);
		rgbuffer_id res_id = rw_id.get_resource_id();

		m_pass.m_buffer_state_map[res_id] = graphics::e_resource_state::cs_uav;

		if (!m_pass.m_buffer_creates.contains(res_id))
		{
			dummy_read_buffer(name);
		}

		m_pass.m_buffer_writes.insert(res_id);
		rgbuffer* buffer = m_graph.get_buffer(res_id);
		if (buffer->is_imported()) m_pass.m_flags |= e_rgpass_flags::force_no_cull;
		return rw_id;
	}
	rgbuffer_readwrite_id rgpass_builder::write_buffer_impl(const rgname& name, const rgname& counter_name, const buffer_view_desc& view_desc)
	{
		rgbuffer_readwrite_id rw_id = m_graph.write_buffer(name, counter_name, view_desc);

		rgbuffer_id counter_id = m_graph.get_buffer_id(counter_name);
		rgbuffer_id res_id = rw_id.get_resource_id();

		m_pass.m_buffer_state_map[res_id] = graphics::e_resource_state::cs_uav;
		m_pass.m_buffer_state_map[counter_id] = graphics::e_resource_state::cs_uav;

		dummy_write_buffer(counter_name);

		if (!m_pass.m_buffer_creates.contains(res_id))
		{
			dummy_read_buffer(name);
			dummy_read_buffer(counter_name);
		}

		m_pass.m_buffer_writes.insert(res_id);
		rgbuffer* buffer = m_graph.get_buffer(res_id);
		if (buffer->is_imported()) m_pass.m_flags |= e_rgpass_flags::force_no_cull;
		return rw_id;
	}
#pragma endregion
}