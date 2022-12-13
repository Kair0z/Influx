#pragma once

#ifndef _CORE_TYPES_H_
#define _CORE_TYPES_H_
#include <stdint.h>

namespace Influx
{
	using u64 = uint64_t;
	using u32 = uint32_t;
	using u16 = uint16_t;
	using u8 = uint8_t;

	using s64 = int64_t;
	using s32 = int32_t;
	using s16 = int16_t;
	using s8 = int8_t;

	constexpr u64 u64_max = { 0xffff'ffff'ffff'ffffui64 };
	constexpr u32 u32_max = { 0xffff'ffffui32 };
	constexpr u16 u16_max = { 0xffffui16 };
	constexpr u8 u8_max = { 0xffui8 };
}

#endif