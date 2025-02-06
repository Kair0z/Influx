#pragma once

// influx::core
#include "influx_core.h"
#include "core/scope.h"
#include "core/container/vector.h"
#include "core/container/ringBuffer.h"

// influx::renderer
#include "influx_renderer.h"
#include "influx_renderer/pipeline/pipeline.h"
namespace influx::renderer
{
	class renderer_backend;
	struct scene_debug;
	class target;
}

// influx::graphics
#include "influx_graphics/descriptors.h"

namespace influx::graphics
{
	class device;
	class commandlist;
	class descriptor_heap;
	class resource;
	class descriptor_heap;
	class render_target_view;
	class shader_resource_view;
}