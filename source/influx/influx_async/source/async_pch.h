#pragma once

#include "influx_async.h"

// influx core
#include "core/singleton.h"
#include "core/container/queue.h"
#include "core/container/vector.h"
#include "core/container/list.h"
#include "core/container/pool.h"
#include "core/container/ringBuffer.h"
#include "core/math/math.h"
#include "core/time.h"
#include "core/debug.h"

// STL
#include <thread>
#include <mutex>