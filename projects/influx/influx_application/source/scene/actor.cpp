#include "app_pch.h"
#include "actor.h"

namespace influx::scene
{
    actor_id actor::get_actor_id() const
    {
        return m_id;
    }
}