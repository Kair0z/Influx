#include "rendergraph_pch.h"

#include "rendergraph.h"
#include "rgpass.h"

#include "core/enum.h"
#include "rgcommon.h"

#include "influx_graphics/renderpass.h"

namespace influx::rendergraph
{
	uint32 rgpass::get_num_reads() const
	{
		return static_cast<uint32>(m_buffer_reads.size() + m_texture_reads.size());
	}

	uint32 rgpass::get_num_writes() const
	{
		return static_cast<uint32>(m_buffer_writes.size() + m_texture_writes.size());
	}

	void rgpass::set_name(const rgname& name)
	{
		m_name = name;
	}

	rgpass::rgpass(const rgpass_builder_clb& builder_clb, const rgpass_process_clb& process_clb, e_rgpass_type type, e_rgpass_flags flags)
		: m_process_clb{ process_clb }
		, m_builder_clb{ builder_clb }
		, m_type{ type }
		, m_flags{ flags }
	{
	}

	void rgpass::build(rgpass_builder& builder)
	{
		if (m_builder_clb)
		{
			m_builder_clb(builder);
		}
	}

	void rgpass::execute(rgpass_context& ctx) const
	{
		if (m_process_clb)
		{
			m_process_clb(ctx);
		}
	}

	bool rgpass::is_culled() const
	{
		return m_is_culled && can_be_culled();
	}

	bool rgpass::can_be_culled() const
	{
		return !has_any_flag(m_flags, e_rgpass_flags::force_no_cull);
	}

	bool rgpass::allow_uav_writes() const
	{
		return false;
	}

	void rgpass::set_id(rgpass_id id)
	{
		m_id = id;
	}

	e_rgpass_type rgpass::get_type() const
	{
		return m_type;
	}

	bool rgpass::depends_on(const rgpass& other) const
	{
		return has_dependency(other, *this);
	}

	bool rgpass::has_dependency(const rgpass& a, const rgpass& b)
	{
		// if 'b' reads a texture that 'a' writes, then we have a dependency
		for (auto other_node_read : b.m_texture_reads)
		{
			if (a.m_texture_writes.find(other_node_read) != a.m_texture_writes.end())
			{
				return true;
			}
		}

		// if 'b' reads a buffer that 'a' writes, then we have a dependency
		for (auto other_node_read : b.m_buffer_reads)
		{
			if (a.m_buffer_writes.find(other_node_read) != a.m_buffer_writes.end())
			{
				return true;
			}
		}

		return false;
	}

	bool rgpass::is_graphics() const
	{
		return m_type == e_rgpass_type::graphics;
	}

	bool rgpass::is_compute() const
	{
		return m_type == e_rgpass_type::compute;
	}

	bool rgpass::is_compute_any() const
	{
		return is_compute() || is_async_compute();
	}

	bool rgpass::is_async_compute() const
	{
		return m_type == e_rgpass_type::async_compute;
	}

	uint32 rgpass::get_width() const
	{
		return m_width;
	}

	uint32 rgpass::get_height() const
	{
		return m_height;
	}
}

