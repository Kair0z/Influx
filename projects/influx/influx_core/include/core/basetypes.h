#pragma once

namespace influx
{
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
}