#pragma once

#include "influx_application.h"

// core includes
#include "core/singleton.h"
#include "core/container/vector.h"
#include "core/container/ringBuffer.h"
#include "core/math/matrix.h"
#include "core/math/transform.h"
#include "core/math/random.h"
#include "core/geometry/quad.h"
#include "core/geometry/geometry.h"
#include "core/time.h"
#include "core/platform/platform.h"
#include "application/constants.h"

// common
struct frame_time final
{
	float m_delta_seconds;
	float m_time_seconds;
};
