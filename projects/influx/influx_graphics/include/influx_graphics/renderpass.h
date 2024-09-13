#pragma once

#include "core/basetypes.h"
#include "core/container/vector.h"
#include "core/math/colour.h"

#include "influx_graphics/descriptorheap.h"

namespace influx::graphics
{
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

    struct depth_attachment final
    {
        descriptor_handle* m_rtv_descriptor;

        e_load_op m_load;
        e_store_op m_store;
        math::colour_rgba m_clear = {};
    };

    struct depth_attachment final
    {
        descriptor_handle* m_dsv_descriptor;

        e_load_op m_depth_load;
        e_load_op m_stencil_load = e_load_op::no_access;
        e_store_op m_depth_store;
        e_store_op m_stencil_store = e_store_op::no_access;
        
        float m_depth_clear = 0.0f;
        uint8 m_stencil_clear = 0u;
    };

	struct renderpass_args final
	{
        vector<color_attachment> m_color_attachments;
        depth_attachment m_depth_attachment;

		uint32 m_width = 0u;
		uint32 m_height = 0u;
        e_renderpass_flags m_flags = e_renderpass_flags::none;
		bool m_legacy = false;
	};
}