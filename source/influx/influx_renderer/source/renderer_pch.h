#pragma once

// influx::core
#include "influx_core.h"
#include "core/scope.h"
#include "core/container/vector.h"
#include "core/container/ringBuffer.h"
#include "influx_renderer/common.h"

// influx::renderer
#include "influx_renderer.h"
#include "influx_renderer/pipeline/pipeline.h"
namespace influx::renderer
{
	class renderer_backend;
	struct scene_debug;
	class target;
}
#include "influx_renderer/rhi.h"


