#include "graphics_pch.h"

// influx::core
#include "core/log.h"
#include "core/scope.h"

// influx::graphics
#include "influx_graphics/commandlist.h"
#include "influx_graphics/device.h"

namespace influx::graphics
{
    const bool g_mute = true;

    void commandlist::start(device* device, detail::pipeline* init_state)
    {
        const e_state state = get_state();
        influx_assert(state == e_state::created || state == e_state::completed);

        // create fence if this commandlist is just created
        if (state == e_state::created && m_fence == nullptr)
        {
            const uint32 incomplete_value = (m_complete_value == 1u) ? 0u : 1u;
            m_fence = device->create_fence(incomplete_value);
        }
        
        // if we've completed previous, time to flip the value to wait for
        if (state == e_state::completed)
        {
            m_complete_value = 1 - m_complete_value;
        }

        // starts allocator
        m_state = e_state::recording;
        start_impl(device, init_state);

        if (!g_mute) logwar("commandlist start: {}", m_name.get().c_str());
    }

    void commandlist::submit(queue* queue)
    {
        queue->submit({ this });
        if (!g_mute) logwar("commandlist submit: {}", m_name.get().c_str());
    }

    void commandlist::wait_for_completion()
    {
        influx_scope("wait_for_gpu");
        while (get_state() == e_state::submitted)
        {
            // ...
        }

        if (!g_mute) logwar("commandlist complete: {}", m_name.get().c_str());
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

    void commandlist::post_submit(queue* queue)
    {
        m_state = e_state::submitted;
        queue->queue_signal(m_fence, m_complete_value);
        if (!g_mute) logwar("commandlist signal: {}", m_name.get().c_str());
    }
}