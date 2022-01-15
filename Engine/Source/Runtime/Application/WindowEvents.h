#pragma once

#include "Runtime/Events/Event.h"

namespace Influx
{
	struct WindowResizeEvent final : public Event { int NewWidth{}; int NewHeight{}; };
}