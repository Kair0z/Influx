#pragma once

// influx::core
#include "core/container/map.h"
#include "core/function.h"

// influx::rendergraph
#include "rgcommon.h"

// influx::graphics
#include "influx_graphics/renderpass.h"
#include "influx_graphics/resource.h"

namespace influx::rendergraph
{
	class rgbuilder;

	enum class e_rgpass_type : uint8
	{
		graphics,
		compute,
		async_compute,
		count
	};

	enum class e_rgpass_flags : uint32
	{
		none = 0x00,
		force_no_cull = 0x01,
		allow_uav_write = 0x02
	};

	class rgpass
	{
	public:
		uint32 get_num_reads() const;
		uint32 get_num_writes() const;

	protected:
		rgpass(e_rgpass_type type, e_rgpass_flags flags);

	private:
		// inherit from this:
		virtual void setup(rgbuilder& builder) = 0;
		virtual void execute() = 0;

	private:
		bool is_culled() const;
		bool can_be_culled() const;
		bool allow_uav_writes() const;
		void set_id(rgpass_id id);
		e_rgpass_type get_type() const;
		static bool has_dependency(rgpass* a, rgpass* b);

		uint32 get_width() const;
		uint32 get_height() const;

		e_rgpass_type m_type;
		e_rgpass_flags m_flags;
		rgpass_id m_id;
		uint32 m_width;
		uint32 m_height;
		uint32 m_refcount;
		bool m_is_culled;

		uset<rgtexture_id> m_texture_creates;
		uset<rgtexture_id> m_texture_reads;
		uset<rgtexture_id> m_texture_writes;
		uset<rgtexture_id> m_texture_destroys;
		umap<rgtexture_id, graphics::e_resource_state> m_texture_state_map;

		uset<rgtexture_id> m_buffer_creates;
		uset<rgtexture_id> m_buffer_reads;
		uset<rgtexture_id> m_buffer_writes;
		uset<rgtexture_id> m_buffer_destroys;
		umap<rgtexture_id, graphics::e_resource_state> m_buffer_state_map;

		struct render_target final
		{
			rgtexture_id m_texture_id;
			rgaccess m_access;
		};
		vector<render_target> m_rtvs{};

		struct depth_stencil final
		{
			rgtexture_id m_texture_id;
			rgaccess m_depth_access;
			rgaccess m_stencil_access;
			bool m_depth_read_only;
		};
		depth_stencil m_dsv{};

		friend class rendergraph;
		friend class rglayer;
	};
}