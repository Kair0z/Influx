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

	result<bool> rgpass_builder::is_texture_declared(const rgname& name) const
	{
		return m_graph.is_texture_declared(name);
	}
	result<bool> rgpass_builder::is_buffer_declared(const rgname& name) const
	{
		return m_graph.is_buffer_declared(name);
	}
	result<> rgpass_builder::declare_texture(const rgname& name, const texture_desc& desc)
	{
		m_pass.m_texture_creates.push_back(m_graph.declare_texture(name, desc));
		return {};
	}
	result<> rgpass_builder::declare_buffer(const rgname& name, const buffer_desc& desc)
	{
		m_pass.m_buffer_creates.push_back(m_graph.declare_buffer(name, desc));
		return {};
	}
	result<> rgpass_builder::dummy_write_texture(const rgname& name)
	{
		m_pass.m_texture_writes.push_back(m_graph.get_texture_id(name));
		return {};
	}
	result<> rgpass_builder::dummy_read_texture(const rgname& name)
	{
		m_pass.m_texture_reads.push_back(m_graph.get_texture_id(name));
		return {};
	}
	result<> rgpass_builder::dummy_write_buffer(const rgname& name)
	{
		m_pass.m_buffer_writes.push_back(m_graph.get_buffer_id(name));
		return {};
	}
	result<> rgpass_builder::dummy_read_buffer(const rgname& name)
	{
		m_pass.m_buffer_reads.push_back(m_graph.get_buffer_id(name));
		return {};
	}

	result<rgtex_copysrc_id> rgpass_builder::read_copysrc_texture(const rgname& name)
	{
		auto copy_src_id = m_graph.read_copysrc_texture(name);
		if (copy_src_id.is_unex())
		{
			register_error_to_current_pass();
			return result<rgtex_copysrc_id>::make_error("read_copysrc_texture failed!");
		}

		rgtexture_id res_id(copy_src_id.get());

		// register a state transition
		m_pass.m_texture_state_map[res_id] = rhi_resource_state::copy_src;

		// register a tex-read
		m_pass.m_texture_reads.push_back(res_id);

		return copy_src_id;
	}
	result<rgtex_copydst_id> rgpass_builder::write_copydst_texture(const rgname& name)
	{
		auto copy_dest_id = m_graph.write_copydst_texture(name);
		if (copy_dest_id.is_unex())
		{
			register_error_to_current_pass();
			return result<rgtex_copydst_id>::make_error("write_copydst_texture failed!");
		}

		rgtexture_id res_id(copy_dest_id.get());
		rgtexture* texture = m_graph.get_texture(res_id);

		// register a state transition
		m_pass.m_texture_state_map[res_id] = rhi_resource_state::copy_dst;
		
		// register a tex-write
		m_pass.m_texture_writes.push_back(res_id);
		
		// if the pass is not creating this texture... ? I dont remember why...
		if (!m_pass.has_create(res_id))
		{
			dummy_read_texture(name);
		}

		// writing to imported textures should never be culled
		if (texture->is_imported())
		{
			m_pass.m_flags |= e_rgpass_flags::force_no_cull;
		}

		return copy_dest_id.get();
	}
	result<rgbuf_copysrc_id> rgpass_builder::read_copysrc_buffer(const rgname& name)
	{
		auto copy_src_id = m_graph.read_copysrc_buffer(name);
		if (copy_src_id.is_unex())
		{
			register_error_to_current_pass();
			return result<rgbuf_copysrc_id>::make_error("read_copysrc_buffer failed!");
		}

		rgbuffer_id res_id(copy_src_id.get());

		// register a state transition
		m_pass.m_buffer_state_map[res_id] = rhi_resource_state::copy_src;
		
		// register a buff-read
		m_pass.m_buffer_reads.push_back(res_id);
		
		return copy_src_id.get();
	}
	result<rgbuf_copydst_id> rgpass_builder::write_copydst_buffer(const rgname& name)
	{
		auto copy_dst_id = m_graph.write_copydst_buffer(name);
		if (copy_dst_id.is_unex())
		{
			register_error_to_current_pass();
			return result<rgbuf_copydst_id>::make_error("write_copydst_buffer failed!");
		}

		rgbuffer_id res_id(copy_dst_id.get());
		rgbuffer* buffer = m_graph.get_buffer(res_id);

		// register a state transition
		m_pass.m_buffer_state_map[res_id] = rhi_resource_state::copy_dst;

		// register a buff-write
		m_pass.m_buffer_writes.push_back(res_id);

		// ...
		if (!m_pass.has_create(res_id))
		{
			dummy_read_buffer(name);
		}
		
		if (buffer->is_imported()) 
			m_pass.m_flags |= e_rgpass_flags::force_no_cull;
		
		return copy_dst_id.get();
	}
	result<rgbuf_indargs_id> rgpass_builder::read_indirect_args_buffer(const rgname& name)
	{
		auto ind_args_id = m_graph.read_indirect_args_buffer(name);
		if (ind_args_id.is_unex())
		{
			register_error_to_current_pass();
			return result<rgbuf_indargs_id>::make_error("read_indirect_args_buffer failed!");
		}

		rgbuffer_id res_id(ind_args_id.get());

		// register a state transition
		m_pass.m_buffer_state_map[res_id] = rhi_resource_state::indirect_args;
		
		// register a buff-read
		m_pass.m_buffer_reads.push_back(res_id);
		
		return ind_args_id.get();
	}
	result<rgbuf_index_id> rgpass_builder::read_index_buffer(const rgname& name)
	{
		auto index_id = m_graph.read_index_buffer(name);
		if (index_id.is_unex())
		{
			register_error_to_current_pass();
			return result<rgbuf_index_id>::make_error("read_index_buffer failed!");
		}

		rgbuffer_id res_id(index_id.get());

		// register a state transition
		m_pass.m_buffer_state_map[res_id] = rhi_resource_state::indexbuffer;
		
		// register a buff-read
		m_pass.m_buffer_reads.push_back(res_id);
		
		return index_id.get();
	}

	result<rgbuf_const_id> rgpass_builder::read_constbuffer(const rgname& name)
	{
		using result_type = result<rgbuf_const_id>;
		auto index_id = m_graph.read_constant_buffer(name);
		if (index_id.is_unex())
		{
			register_error_to_current_pass();
			return result_type::make_error("read_constbuffer failed!");
		}

		rgbuffer_id res_id(index_id.get());
		// register a state transition
		m_pass.m_buffer_state_map[res_id] = rhi_resource_state::constbuffer;
		// register a buff-read
		m_pass.m_buffer_reads.push_back(res_id);
		return index_id.get();
	}

	result<rgtexture_readonly_id> rgpass_builder::read_texture(const rgname& name, rgread_access read_acc, const texture_desc_options& options)
	{
		texture_view_desc view_desc
		{
			.m_first_slice = options.m_first_slice,
			.m_num_slices = options.m_slice_count,
			.m_first_mip = options.m_first_mip,
			.m_num_mips = options.m_num_mips
		};
		return read_texture_impl(name, read_acc, view_desc);
	}
	result<rgtexture_readwrite_id> rgpass_builder::write_texture(const rgname& name, const texture_desc_options& options)
	{
		texture_view_desc view_desc
		{
			.m_first_slice = options.m_first_slice,
			.m_num_slices = options.m_slice_count,
			.m_first_mip = options.m_first_mip,
			.m_num_mips = options.m_num_mips
		};
		return write_texture_impl(name, view_desc);
	}
	result<rgrendertarget_id> rgpass_builder::write_rendertarget(const rgname& name, rgaccess load_store_op, const texture_desc_options& options)
	{
		texture_view_desc view_desc
		{
			.m_first_slice = options.m_first_slice,
			.m_num_slices = options.m_slice_count,
			.m_first_mip = options.m_first_mip,
			.m_num_mips = options.m_num_mips
		};
		return write_rendertarget_impl(name, load_store_op, view_desc);
	}
	result<rgdepthtarget_id> rgpass_builder::write_depthtarget(const rgname& name, rgaccess load_store_op, const texture_desc_options& options)
	{
		texture_view_desc view_desc
		{
			.m_first_slice = options.m_first_slice,
			.m_num_slices = options.m_slice_count,
			.m_first_mip = options.m_first_mip,
			.m_num_mips = options.m_num_mips
		};
		rgaccess stencil_access
		{
			.m_load = e_rg_load::no_access,
			.m_store = e_rg_store::no_access
		};
		return write_depthtarget_impl(name, load_store_op, stencil_access, view_desc);
	}
	result<rgdepthtarget_id> rgpass_builder::read_depthtarget(const rgname& name, rgaccess load_store_op, const texture_desc_options& options)
	{
		texture_view_desc view_desc
		{
			.m_first_slice = options.m_first_slice,
			.m_num_slices = options.m_slice_count,
			.m_first_mip = options.m_first_mip,
			.m_num_mips = options.m_num_mips
		};
		rgaccess stencil_access
		{
			.m_load = e_rg_load::no_access,
			.m_store = e_rg_store::no_access
		};
		return read_depthtarget_impl(name, load_store_op, stencil_access, view_desc);
	}
	result<rgbuffer_readonly_id> rgpass_builder::read_buffer(const rgname& name, rgread_access read_acc, uint32 offset, uint32 size)
	{
		buffer_view_desc view_desc
		{
			.m_offset = offset,
			.m_size = size
		};
		return read_buffer_impl(name, read_acc, view_desc);
	}
	result<rgbuffer_readwrite_id> rgpass_builder::write_buffer(const rgname& name, uint32 offset, uint32 size)
	{
		buffer_view_desc view_desc
		{
			.m_offset = offset,
			.m_size = size
		};
		return write_buffer_impl(name, view_desc);
	}
	result<rgbuffer_readwrite_id> rgpass_builder::write_buffer(const rgname& name, const rgname& counter_name, uint32 offset, uint32 size)
	{
		buffer_view_desc view_desc
		{
			.m_offset = offset,
			.m_size = size
		};
		return write_buffer_impl(name, counter_name, view_desc);
	}

	void rgpass_builder::register_error_to_current_pass()
	{
		m_pass.m_num_errors += 1u;
	}

	void rgpass_builder::set_viewport(uint32 width, uint32 height)
	{
		m_pass.m_width = width;
		m_pass.m_height = height;
	}

	result<texture_desc> rgpass_builder::get_texture_desc(const rgname& name) const
	{
		return m_graph.get_texture_desc(name);
	}
	result<buffer_desc> rgpass_builder::get_buffer_desc(const rgname& name) const
	{
		return m_graph.get_buffer_desc(name);
	}

	result<rgtexture_id> rgpass_builder::get_texture_id(const rgname& name) const
	{
		return m_graph.get_texture_id(name);
	}
	result<rgbuffer_id> rgpass_builder::get_buffer_id(const rgname& name) const
	{
		return m_graph.get_buffer_id(name);
	}

#pragma region impl
	result<rgtexture_readonly_id> rgpass_builder::read_texture_impl(const rgname& name, rgread_access read_access, const texture_view_desc& view_desc)
	{
		auto read_id = m_graph.read_texture(name, view_desc);
		if (read_id.is_unex())
		{
			register_error_to_current_pass();
			return result<rgtexture_readonly_id>::make_error("read_texture failed!");
		}

		rgtexture_id res_id = read_id.get().get_resource_id();

		// register the state transition
		switch (m_pass.m_type)
		{
		case e_rgpass_type::graphics:
			switch (read_access)
			{
			case rgread_access::ps: m_pass.m_texture_state_map[res_id] = rhi_resource_state::ps_srv; break;
			case rgread_access::non_ps: m_pass.m_texture_state_map[res_id] = rhi_resource_state::cs_srv; break;
			case rgread_access::all_shader: m_pass.m_texture_state_map[res_id] = rhi_resource_state::all_srv; break;
			}
			break;

		case e_rgpass_type::compute:
		case e_rgpass_type::async_compute:
			m_pass.m_texture_state_map[res_id] = rhi_resource_state::cs_srv;
			break;
		}

		// register the tex-read
		m_pass.m_texture_reads.push_back(res_id);

		return read_id;
	}
	result<rgtexture_readwrite_id> rgpass_builder::write_texture_impl(const rgname& name, const texture_view_desc& view_desc)
	{
		using result_type = result<rgtexture_readwrite_id>;

		auto rw_id = m_graph.write_texture(name, view_desc);
		if (rw_id.is_unex())
		{
			register_error_to_current_pass();
			return result_type::make_error("write_texture failed!");
		}

		rgtexture_id res_id = rw_id.get().get_resource_id();
		rgtexture* texture = m_graph.get_texture(res_id);

		if (texture->m_desc.m_allow_uav == false)
		{
			return result_type::make_error("error: failed to register a write-texture for a texture that's flagged !allowUAV");
		}

		// register the state transition
		m_pass.m_texture_state_map[res_id] = rhi_resource_state::cs_uav;
		
		// register tex-write
		m_pass.m_texture_writes.push_back(res_id);

		// ...
		if (!m_pass.has_create(res_id))
		{
			dummy_read_texture(name);
		}

		// write to imported resource should not be culled
		if (texture->is_imported())
		{
			m_pass.m_flags |= e_rgpass_flags::force_no_cull;
		}

		return rw_id;
	}
	result<rgrendertarget_id> rgpass_builder::write_rendertarget_impl(const rgname& name, rgaccess load_store_op, const texture_view_desc& view_desc)
	{
		auto rt_id = m_graph.rendertarget(name, view_desc);
		if (rt_id.is_unex())
		{
			register_error_to_current_pass();
			return result<rgrendertarget_id>::make_error("rendertarget failed!");
		}

		rgtexture_id res_id = rt_id.get().get_resource_id();
		rgtexture* texture = m_graph.get_texture(res_id);

		// register state transition
		m_pass.m_texture_state_map[res_id] = rhi_resource_state::render_target;
		
		// register tex-write
		m_pass.m_texture_writes.push_back(res_id);

		// register rtv
		m_pass.m_rtvs.push_back(rgpass::render_target{ .m_texture_id = res_id, .m_access = load_store_op });
		m_pass.m_rtvs.back().m_dimensions = { texture->m_desc.m_width, texture->m_desc.m_heigth };

		if (!m_pass.has_create(res_id))
		{
			dummy_read_texture(name);
		}

		// write to imported resource should not be culled
		if (texture->is_imported()) 
			m_pass.m_flags |= e_rgpass_flags::force_no_cull;

		return rt_id;
	}
	result<rgdepthtarget_id> rgpass_builder::write_depthtarget_impl(const rgname& name, rgaccess load_store_op, rgaccess stencil_load_store_op, const texture_view_desc& view_desc)
	{
		auto dt_id = m_graph.depthtarget(name, view_desc);
		if (dt_id.is_unex())
		{
			register_error_to_current_pass();
			return result<rgdepthtarget_id>::make_error("depthtarget_write failed!");
		}

		rgtexture_id res_id = dt_id.get().get_resource_id();
		rgtexture* texture = m_graph.get_texture(res_id);

		// register the state transition
		m_pass.m_texture_state_map[res_id] = rhi_resource_state::depth_target;

		// register tex-write
		m_pass.m_texture_writes.push_back(res_id);

		// register dsv
		m_pass.m_dsv = rgpass::depth_stencil{ .m_texture_id = res_id, .m_depth_access = load_store_op,
			.m_stencil_access = stencil_load_store_op, .m_depth_read_only = false, .m_is_enabled = true };
		m_pass.m_dsv.m_dimensions = { texture->m_desc.m_width, texture->m_desc.m_heigth };

		if (!m_pass.has_create(res_id))
		{
			dummy_read_texture(name);
		}

		// write to imported resource should not be culled
		if (texture->is_imported()) 
			m_pass.m_flags |= e_rgpass_flags::force_no_cull;

		return dt_id;
	}
	result<rgdepthtarget_id> rgpass_builder::read_depthtarget_impl(const rgname& name, rgaccess load_store_op, rgaccess stencil_load_store_op, const texture_view_desc& view_desc)
	{
		auto dt_id = m_graph.depthtarget(name, view_desc);
		if (dt_id.is_unex())
		{
			register_error_to_current_pass();
			return result<rgdepthtarget_id>::make_error("depthtarget_read failed!");
		}

		rgtexture_id res_id = dt_id.get().get_resource_id();

		m_pass.m_texture_state_map[res_id] = rhi_resource_state::depth_readonly;
		m_pass.m_dsv = rgpass::depth_stencil{ .m_texture_id = res_id, .m_depth_access = load_store_op,
			.m_stencil_access = stencil_load_store_op, .m_depth_read_only = true, .m_is_enabled = true };

		m_pass.m_texture_reads.push_back(res_id);
		rgtexture* texture = m_graph.get_texture(res_id);

		if (texture->is_imported()) 
			m_pass.m_flags |= e_rgpass_flags::force_no_cull;

		return dt_id;
	}
	result<rgbuffer_readonly_id> rgpass_builder::read_buffer_impl(const rgname& name, rgread_access read_access, const buffer_view_desc& view_desc)
	{
		auto read_id = m_graph.read_buffer(name, view_desc);
		if (read_id.is_unex())
		{
			register_error_to_current_pass();
			return result<rgbuffer_readonly_id>::make_error("read_buffer failed!");
		}

		rgbuffer_id res_id = read_id.get().get_resource_id();

		if (m_pass.is_compute_any())
		{
			read_access = rgread_access::non_ps;
			m_pass.m_buffer_state_map[res_id] = rhi_resource_state::cs_srv;
		}
		else
		{
			switch (read_access)
			{
			case rgread_access::ps: m_pass.m_buffer_state_map[res_id] = rhi_resource_state::ps_srv; break;
			case rgread_access::non_ps: m_pass.m_buffer_state_map[res_id] = rhi_resource_state::cs_srv; break;
			case rgread_access::all_shader: m_pass.m_buffer_state_map[res_id] = rhi_resource_state::all_srv; break;
			}
		}

		m_pass.m_buffer_reads.push_back(res_id);
		return read_id;
	}
	result<rgbuffer_readwrite_id> rgpass_builder::write_buffer_impl(const rgname& name, const buffer_view_desc& view_desc)
	{
		auto rw_id = m_graph.write_buffer(name, view_desc);
		if (rw_id.is_unex())
		{
			register_error_to_current_pass();
			return result<rgbuffer_readwrite_id>::make_error("write_buffer failed!");
		}

		rgbuffer_id res_id = rw_id.get().get_resource_id();

		m_pass.m_buffer_state_map[res_id] = rhi_resource_state::cs_uav;

		if (!m_pass.has_create(res_id))
		{
			dummy_read_buffer(name);
		}

		m_pass.m_buffer_writes.push_back(res_id);
		rgbuffer* buffer = m_graph.get_buffer(res_id);
		if (buffer->is_imported()) m_pass.m_flags |= e_rgpass_flags::force_no_cull;
		return rw_id;
	}
	result<rgbuffer_readwrite_id> rgpass_builder::write_buffer_impl(const rgname& name, const rgname& counter_name, const buffer_view_desc& view_desc)
	{
		auto rw_id = m_graph.write_buffer(name, counter_name, view_desc);
		if (rw_id.is_unex())
		{
			register_error_to_current_pass();
			return result<rgbuffer_readwrite_id>::make_error("write_buffer failed!");
		}

		rgbuffer_id counter_id = m_graph.get_buffer_id(counter_name);
		rgbuffer_id res_id = rw_id.get().get_resource_id();

		m_pass.m_buffer_state_map[res_id] = rhi_resource_state::cs_uav;
		m_pass.m_buffer_state_map[counter_id] = rhi_resource_state::cs_uav;

		dummy_write_buffer(counter_name);

		if (!m_pass.has_create(res_id))
		{
			dummy_read_buffer(name);
			dummy_read_buffer(counter_name);
		}

		m_pass.m_buffer_writes.push_back(res_id);
		rgbuffer* buffer = m_graph.get_buffer(res_id);
		if (buffer->is_imported()) m_pass.m_flags |= e_rgpass_flags::force_no_cull;
		return rw_id;
	}
#pragma endregion
}