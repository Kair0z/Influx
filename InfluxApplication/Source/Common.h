#pragma once

#ifndef __APP_COMMON_H_
#define __APP_COMMON_H_

#define PLATFORM_WINDOWS 1
#define PLATFORM_TESTNULL 0

#if PLATFORM_WINDOWS
#include "Core/Platform/WindowsPlatform.h"
#else
#include "Core/Platform/NullPlatform.h"
#endif

#define FLX_APP_RENDERER_D3D12 PLATFORM_WINDOWS
#define FLX_APP_RENDERER_DEBUG _DEBUG
#define FLX_APP_KEEP_TIMING_STATS 0

#include "Core/String.h"
#include "Core/Math/Vector.h"
#include "Core/Container/List.h"
#include "Core/Container/Vector.h"
#include "Core/Container/Array.h"
#include "Core/Time.h"
#endif