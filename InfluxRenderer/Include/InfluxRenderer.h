#pragma once

#ifndef __INFLUX_RENDERER_H_
#define __INFLUX_RENDERER_H_

#pragma comment (lib, "InfluxRenderer.lib")

// Defines
#pragma region Defines

#ifndef INFLUX_RENDERER_API
#define INFLUX_RENDERER_API
#endif

#define INFLUX_RENDERER_USE_CORE		1
#define INFLUX_RENDERER_USE_STL			1

#define INFLUX_RENDERER_INCLUDE_DX12	1
#define INFLUX_RENDERER_INCLUDE_VULKAN	1

// Defining Debug
#ifdef _DEBUG
#define INFLUX_RENDERER_DEBUG 1
#else
#define INFLUX_RENDERER_DEBUG 0
#endif

// Defining Assert & Todo
#if INFLUX_RENDERER_DEBUG
#include <cassert>

#define INFLUX_GRAPHICS_TODO __debugbreak();
#define INFLUX_GRAPHICS_ASSERT(x) assert(x);

#else
#define INFLUX_GRAPHICS_TODO
#define INFLUX_GRAPHICS_ASSERT
#endif

#pragma endregion

// Types
#pragma region Types

#pragma region Types - Core Types
#if INFLUX_RENDERER_USE_CORE
#include "Core/BasicTypes.h"
#include "Core/Math/Vector.h"
#include "Core/Math/Matrix.h"
#include "Core/Container/Vector.h"
#include "Core/Container/Array.h"
#include "Core/String.h"
#include "Core/Function.h"
#endif // INFLUX_RENDERER_USE_CORE

#if INFLUX_RENDERER_USE_STL
#include <vector>
#include <array>
#include <string>
#endif // INFLUX_RENDERER_USE_STL

namespace influx::Renderer
{
#if INFLUX_RENDERER_USE_CORE
	using uint8 = influx::uint8;
	using byte = influx::byte;
	using uint16 = influx::uint16;
	using uint32 = influx::uint32;
	using uint64 = influx::uint64;

	using int8 = influx::int8;
	using int16 = influx::int16;
	using int32 = influx::int32;
	using int64 = influx::int64;

	using f32 = influx::f32;
	using f64 = influx::f64;

	constexpr uint64 u64_max = influx::u64_max;
	constexpr uint32 u32_max = influx::u32_max;
	constexpr uint16 u16_max = influx::u16_max;
	constexpr uint8  u8_max = influx::u8_max;

	using Vectorf2 = influx::Math::Vectorf2;
	using Vectorf3 = influx::Math::Vectorf3;
	using Vectorf4 = influx::Math::Vectorf4;

	using Vectoru2 = influx::Math::Vectoru2;
	using Vectoru3 = influx::Math::Vectoru3;
	using Vectoru4 = influx::Math::Vectoru4;

	using Matrix4x4f = influx::Math::Matrix4x4f;

	template <typename _t>
	using vector = influx::vector<_t>;

	template <typename _t, uint64 _dim>
	using Array = influx::Array<_t, _dim>;

	using String = influx::string;

	template <typename _func>
	using Function = influx::function<_func>;

#else
	using uint8 = unsigned char;
	using byte = unsigned char;
	using uint16 = unsigned short;
	using uint32 = unsigned int;
	using uint64 = unsigned long long;

	using int8 = char;
	using int16 = short;
	using int32 = int;
	using int64 = long;

	using f32 = float;
	using f64 = double;

	constexpr uint64 u64_max = { 0xffff'ffff'ffff'ffffui64 };
	constexpr uint32 u32_max = { 0xffff'ffffui32 };
	constexpr uint16 u16_max = { 0xffffui16 };
	constexpr uint8  u8_max = { 0xffui8 };

#if INFLUX_RENDERER_USE_STL
	template <typename _t>
	using Vector = std::vector<_t>;

	template <typename _t, uint64 _N>
	using Array = std::array<_t, _N>;

	using String = std::string;
#endif
#endif // INFLUX_RENDERER_USE_CORE
}
#pragma endregion

#pragma endregion

#if INFLUX_RENDERER_USE_CORE
#include "Core/Platform/Platform.h"
#endif

namespace influx::Renderer
{
	struct Result
	{

	};
}

namespace influx::Renderer
{
	INFLUX_RENDERER_API Result Initialize();

	INFLUX_RENDERER_API bool IsInitialized();

	INFLUX_RENDERER_API Result Cleanup();

	INFLUX_RENDERER_API Result AttachToWindow(platform::window_handle window);

	INFLUX_RENDERER_API bool IsAttachedToWindow(platform::window_handle window);

	
	// Do Render work...
	INFLUX_RENDERER_API Result Render();

	// Do Present to swapchain...
	INFLUX_RENDERER_API Result Present();
}

#endif