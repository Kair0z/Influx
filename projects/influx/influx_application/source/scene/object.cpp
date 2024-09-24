#include "app_pch.h"
#include "object.h"

namespace influx::scene
{
    obj_id object::get_id() const
    {
        return m_id;
    }
}