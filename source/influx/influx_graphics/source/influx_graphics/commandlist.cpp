#include "graphics_pch.h"

// influx::core
#include "core/log.h"
#include "core/scope.h"

// influx::graphics
#include "influx_graphics/commandlist.h"
#include "influx_graphics/device.h"

namespace influx::graphics
{
    result<> commandlist::start(device* device, detail::base_pipeline* init_state)
    {
        result<> res = {};

        wait_for_completion();

        const e_state state = get_state();
        const bool is_ready_to_start = state == e_state::created || state == e_state::completed;
        if (is_ready_to_start == false)
        {
            return result<>::make_error("error: starting an in-flight / recording commandlist.");
        }

        // create fence if this commandlist is newly created
        if (state == e_state::created && m_fence == nullptr)
        {
            const uint32 incomplete_value = (m_complete_value == 1u) ? 0u : 1u;
            m_fence = device->create_fence(incomplete_value);
            if (m_fence == nullptr)
            {
                return result<>::make_error("error: failed creating fence for this commandlist.");
            }
        }

        // if we've completed previous, time to flip the value to wait for
        if (state == e_state::completed)
        {
            m_complete_value = 1 - m_complete_value;
        }

        // starts allocator
        m_state = e_state::recording;

        start_impl(device, init_state);
        return res;
    }

    result<> commandlist::submit(queue* queue)
    {
        result<> res = {};
        
        res = end();
        if (!res.is_success())
        {
            return result<>::make_error("error: failed ending current commandlist!");
        }

        res = queue->submit({ this });
        if (!res.is_success())
        {
            return result<>::make_error("error: failed submitting current commandlist!");
        }

        return res;
    }

    void commandlist::wait_for_completion()
    {
        influx_scope("wait_for_gpu");
        while (get_state() == e_state::submitted)
        {
            // ...
        }
    }

    bool commandlist::is_completed()
    {
        return get_state() == e_state::completed;
    }

    commandlist::e_state commandlist::get_state()
    {
        if (m_fence != nullptr && m_fence->query_value() == m_complete_value)
        {
            m_state = e_state::completed;
        }

        return m_state;
    }

    void commandlist::set_name(const debug_name& name)
    {
        m_name = name;
    }

    const debug_name& commandlist::get_name() const
    {
        return m_name;
    }

    result<> commandlist::post_submit(queue* queue)
    {
        result<> res = {};
        m_state = e_state::submitted;
        res = queue->queue_signal(m_fence, m_complete_value);
        return res;
    }

    result<> commandlist::set_vp_and_rect(const math::float2& min, const math::float2& max)
    {
        result<> res = {};
        res = set(graphics::viewport
		{
			.m_left = min.x,
			.m_top = min.y,
			.m_width = max.x - min.x,
			.m_height = max.y - min.y
		});
        if (!res.is_success()) return result<>::make_error("error: failed setting viewport");

		res = set(graphics::rect
		{
			.m_left = math::round<uint32>(min.x),
			.m_top = math::round<uint32>(min.y),
			.m_right = math::round<uint32>(max.x - min.x),
			.m_bottom = math::round<uint32>(max.y - min.y)
		});
        if (!res.is_success()) return result<>::make_error("error: failed setting scissor rect");
        
        return res;
    }

    result<> commandlist::set(const viewport& viewport)
    {
        m_viewport = viewport;
        return {};
    }

    result<> commandlist::set(const rect& rect)
    {
        m_scissor_rect = rect;
        return {};
    }

    void commandlist::pre_draw()
    {
        const bool is_viewport_valid = m_viewport.m_width > 0.0f && m_viewport.m_height > 0.0f;
        const bool is_rect_valid = m_scissor_rect.m_right > 0u && m_scissor_rect.m_bottom > 0u;
        // todo..
    }
}