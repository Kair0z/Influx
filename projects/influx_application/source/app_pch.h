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
#if INFLUX_PLATFORM_WINDOWS
#include "core/platform/windows_platform.h"
#endif

#include "application/constants.h"