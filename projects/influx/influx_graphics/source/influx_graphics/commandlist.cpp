#include "graphics_pch.h"

// influx::core
#include "core/log.h"

// influx::graphics
#include "influx_graphics/commandlist.h"
#include "influx_graphics/device.h"

namespace influx::graphics
{
    void commandlist::start(device* device, pipeline* init_state)
    {
        if (m_fence == nullptr)
        {
            m_fence = device->create_fence(0u);
        }

        const e_state state = get_state();
        influx_assert(state == e_state::created || state == e_state::completed);

        m_state = e_state::recording;
        start_impl(device, init_state);
    }

    void commandlist::submit(queue* queue)
    {
        queue->submit_commandlists({ this });
        post_submit(queue);
    }

    bool commandlist::is_completed()
    {
        return get_state() == e_state::completed;
    }

    commandlist::e_state commandlist::get_state()
    {
        if (m_fence->query_value() == 1u)
        {
            m_state = e_state::completed;
        }

        return m_state;
    }

    void commandlist::post_submit(queue* queue)
    {
        m_state = e_state::submitted;
        queue->queue_signal(m_fence, 1u);
    }
}