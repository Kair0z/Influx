#pragma once

#ifndef __APP_COMMON_H_
#define __APP_COMMON_H_

#define PLATFORM_WINDOWS 1

#if PLATFORM_WINDOWS
#include "Core/Platform/WindowsPlatform.h"
#endif

#define FLX_APP_RENDERER_D3D12 PLATFORM_WINDOWS
#define FLX_APP_RENDERER_DEBUG _DEBUG

#include "Core/String.h"
#include "Core/Math/Vector.h"

#endif