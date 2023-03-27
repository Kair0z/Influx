#pragma once

#ifndef __APP_COMMON_H_
#define __APP_COMMON_H_

#define PLATFORM_WINDOWS 1
#include "Core/Platform/WindowsPlatform.h"

#define FLX_APP_RENDERER_D3D12		PLATFORM_WINDOWS
#define FLX_APP_RENDERER_DEBUG		_DEBUG
#define FLX_APP_KEEP_TIMING_STATS	0

#define FLX_APP_USE_CORE 1
#if FLX_APP_USE_CORE
#include "Core/BasicTypes.h"
#include "Core/String.h"
#include "Core/Math/Vector.h"
#include "Core/Container/List.h"
#include "Core/Container/Vector.h"
#include "Core/Container/Array.h"
#include "Core/Time.h"
#include "Core/Pointer.h"
#include "Core/Scene/Scene.h"
#else
static_assert(false, "Error: Application requires using the Influx Core-header-library! ")
#endif

#endif