#pragma once

// influx::core
#include "core/basetypes.h"
#include "core/container/vector.h"
#include "core/math/colour.h"

// influx::graphics
#include "influx_graphics/descriptors.h"

namespace influx::graphics
{
    class resource;

    enum e_renderpass_flags : uint32
    {
        none = 0x0,
        read_only_depth = 0x1,
        read_only_stencil = 0x2,
        allow_uav_write = 0x4,
        suspending = 0x8,
        resuming = 0x10,
    };

    enum class e_load_op : uint8
    {
        discard,
        preserve,
        clear,
        no_access,
        count
    };

    enum class e_store_op : uint8
    {
        discard,
        preserve,
        resolve,
        no_access,
        count
    };

    struct color_attachment final
    {
        descriptor_handle m_rtv_descriptor;

        e_load_op m_load;
        e_store_op m_store;
        math::colour_rgba m_clear = {};

        struct resolve_params final
        {
            e_format m_format;
            resource* m_source;
            resource* m_dest;
            bool m_keep_source = false;
            // todo... subresources
            // todo... D3D12_RESOLVE_MODE
        } m_resolve{};
    };

    struct depth_attachment final
    {
        descriptor_handle m_dsv_descriptor;

        e_load_op m_depth_load;
        e_store_op m_depth_store;
        float m_depth_clear = 0.0f;

        e_load_op m_stencil_load = e_load_op::no_access;
        e_store_op m_stencil_store = e_store_op::no_access;
        uint8 m_stencil_clear = 0u;

        bool m_is_enabled = false;
    };

	struct renderpass_args final
	{
        vector<color_attachment> m_color_attachments;
        depth_attachment m_depth_attachment;

		uint32 m_width = 0u;
		uint32 m_height = 0u;
        e_renderpass_flags m_flags = e_renderpass_flags::none;
		bool m_legacy = false;

        /* 
            renderpasses by default call cmdlist.set_viewport() based on width & height implicitly.
        */
        bool m_allow_implicit_viewport_set = true;
        bool m_allow_implicit_viewrect_set = true;
	};
}