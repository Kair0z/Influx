#pragma once

#ifndef _CORE_MATH_VECTOR_H_
#define _CORE_MATH_VECTOR_H_

#include "core/basetypes.h"

#pragma warning(disable : 4201) // Union warning...

namespace influx::math
{
	using vecsize = uint32;

	namespace detail
	{
		template <typename _t, vecsize _s>
		struct base_vector
		{
			union
			{
				struct { _t x, y, z, w; };
				struct { _t r, g, b, a; };
				_t m_data[_s];
			};

			template <typename... _V>
			base_vector(const _V&... components)
				: m_data{components...}
			{

			}
		};

		template <typename _t>
		struct base_vector<_t, 1u>
		{
			union
			{
				struct { _t x; };
				struct { _t r; };
				_t m_data[1];
			};
			base_vector<_t, 1u>(const _t _x = 0) : x{ _x } {}
		};

		template <typename _t>
		struct base_vector<_t, 2u>
		{
			union
			{
				struct { _t x, y; };
				struct { _t r, g; };
				_t m_data[2];
			};

			base_vector<_t, 2u>(const _t _x = 0, const _t _y = 0) : x{ _x }, y{ _y } {}
		};

		template <typename _t>
		struct base_vector<_t, 3u>
		{
			union
			{
				struct { _t x, y, z; };
				struct { _t r, g, b; };
				_t m_data[3];
			};

			base_vector<_t, 3u>(const _t _x = 0, const _t _y = 0, const _t _z = 0) : x{ _x }, y{ _y }, z{ _z } {}
		};

		template <typename _t>
		struct base_vector<_t, 4u>
		{
			union
			{
				struct { _t x, y, z, w; };
				struct { _t r, g, b, a; };
				_t m_data[4];
			};

			base_vector<_t, 4u>(const _t _x = 0, const _t _y = 0, const _t _z = 0, const _t _w = 0) : x{ _x }, y{ _y }, z{ _z }, w{ _w } {}
		};
	}

	template <typename _t, vecsize _s>
	class vector final : public detail::base_vector<_t, _s>
	{
		static_assert(_s != 0u, "influx::vector<_t, _s> � Cannot instantiate zero-sized vector (_s == 0)! ");
		static_assert(_s <= 4u, "influx::vector<_t, _s> � Cannot instantiate vector bigger than 4 (_s > 4)! ");

		static constexpr vecsize k_size = _s;
		using value_type = _t;

	public:
		// constructors
		vector() = default;
		vector(const vector& other) = default;
		vector(vector&& other) = default;
		vector& operator=(const vector& other) = default;
		vector& operator=(vector&& other) = default;
		template <typename... _v>		vector(const _v&... components);		// Initializer list
		template <typename _u>			vector(const vector<_u, _s>& other);	// Typecasting
		template <vecsize _d>			vector(const vector<_t, _d>& other);	// Sizecasting

		constexpr static vecsize size();

		// Accessing data:
		_t& operator[](vecsize i);
		const _t& operator[](vecsize i) const;
		_t& at(vecsize i);
		const _t& at(vecsize i) const;

		const _t* data() const;
		_t* data();

		// Normalizing:
		vector normalized() const;
		void normalize();
		static void normalize(vector& vec);
		static vector normalized(const vector& vec);

		// clamp:
		vector&			clamp_values(_t min, _t max);
		vector			get_clamped_values(_t min, _t max) const;
		static void		clamp_values(vector& vec, _t min, _t max);
		static vector	get_clamped_values(const vector& vec, _t min, _t max);

		vector&			clamp_length(_t length);
		vector			get_clamped_length(_t length) const;
		static void		clamp_length(vector& vec, _t length);
		static vector	get_clamped_length(const vector& vec, _t length);

		// Angle:
		float radians_between(const vector& other) const;
		static float radians_between(const vector& a, const vector& b);

		// Magnitude:
		float magnitude() const;
		float sqr_magnitude() const;
		static float magnitude(const vector& other);
		static float sqr_magnitude(const vector& other);
		static float distance(const vector& a, const vector& b);
		static float sqr_distance(const vector& a, const vector& b);

		// Scaling:
		void scale(float mag);
		vector scaled(float mag) const;
		static void scale(vector& vec, float mag);
		static vector scaled(const vector& vec, float mag);

		// Cross & dot:
		_t dot(const vector& other) const;
		static _t dot(const vector& a, const vector& b);

		_t cross(const vector<_t, 2u>& other) const;
		vector<_t, 3u> cross(const vector<_t, 3u>& other) const;
		static float cross(const vector<_t, 2u>& a, const vector<_t, 2u>& b);
		static vector<_t, 3u> cross(const vector<_t, 3u>& a, const vector<_t, 3u>& b);

		// Comparison+		a	{...}	const influx::math::vector<float,3> &

		bool operator==(const vector& other) const;
		bool operator!=(const vector& other) const;

		// Inverting:
		const vector& inverted() const;
		void inverse();
		static void inverse(vector& vec);
		static vector inverted(const vector& vec);

		// Reflect:
		const	vector<_t, 2u>&	reflected(const vector<_t, 2u>& hitNormal) const;
		const	vector<_t, 3u>& reflected(const vector<_t, 3u>& hitNormal) const;
		static	vector<_t, 2u>	reflection(const vector<_t, 2u>& vec, const vector<_t, 2u>& hitNormal);
		static	vector<_t, 3u>	reflection(const vector<_t, 3u>& vec, const vector<_t, 3u>& hitNormal);

		// Lerp:
		static vector lerp(const vector& a, const vector& b, const float t);
		void lerp_towards(const vector& b, const float t);

		// Zero:
		static vector zero();
		static vector one();
		static vector get_max();

		// makes a vector filled with value
		static vector fill(const _t&);

		bool is_zero() const;
		static bool is_zero(const vector& v);

		// 3D:
		constexpr static vector<_t, 3u> up();
		constexpr static vector<_t, 3u> forward();
		constexpr static vector<_t, 3u> right();

		static vector<_t, 2u> get_look_at(const vector<_t, 2u>& from, const vector<_t, 2u>& to);
		static vector<_t, 3u> get_look_at(const vector<_t, 3u>& from, const vector<_t, 3u>& to);

		vector<_t, 2u> get_xy() const;
		vector<_t, 3u> get_xyz() const;
		vector<_t, 2u> get_rg() const;
		vector<_t, 3u> get_rgb() const;

		static vector abs(const vector& vec);
		
		// Arithmatics:
		vector& operator+=(const vector& other);
		vector& operator-=(const vector& other);
		vector& operator*=(const vector& other);
		vector& operator*=(const float scalar);
		vector& operator/=(const vector& other);
		vector& operator/=(const float scalar);
	};

	// Per-Component operators:
	template <typename _t, vecsize _s>
	vector<_t, _s> operator+(const vector<_t, _s>& a, const vector<_t, _s>& b);
	template <typename _t, vecsize _s>
	vector<_t, _s> operator-(const vector<_t, _s>& a, const vector<_t, _s>& b);
	template <typename _t, vecsize _s>
	vector<_t, _s> operator*(const vector<_t, _s>& a, const vector<_t, _s>& b);
	template <typename _t, vecsize _s>
	vector<_t, _s> operator/(const vector<_t, _s>& a, const vector<_t, _s>& b);

	// Scalar operators:
	template <typename _t, vecsize _s>
	vector<_t, _s> operator*(const vector<_t, _s>& a, const float b);
	template <typename _t, vecsize _s>
	vector<_t, _s> operator/(const vector<_t, _s>& a, const float b);
	template <typename _t, vecsize _s>
	vector<_t, _s> operator*(const float a, const vector<_t, _s>& b);
	template <typename _t, vecsize _s>
	vector<_t, _s> operator-(const vector<_t, _s>& v);

	template <typename _t, vecsize _s>
	vector<_t, _s> operator/(const float a, const vector<_t, _s>& b);


#pragma region Aliases
	template <typename vecsize _s>
	using vectoru8 = vector<uint8, _s>;

	template <typename vecsize _s>
	using vectoru32 = vector<uint32, _s>;

	template <typename vecsize _s>
	using vectoru64 = vector<uint64, _s>;

	template <typename vecsize _s>
	using vectori = vector<int, _s>;

	template <typename vecsize _s>
	using vectorf = vector<float, _s>;

	template <typename vecsize _s>
	using vectorl = vector<long, _s>;

	using vectorf2 = vectorf<2u>;
	using vectorf3 = vectorf<3u>;
	using vectorf4 = vectorf<4u>;

	using vectoru2 = vectoru32<2u>;
	using vectoru3 = vectoru32<3u>;
	using vectoru4 = vectoru32<4u>;

	using vectori2 = vectori<2u>;
	using vectori3 = vectori<3u>;
	using vectori4 = vectori<4u>;

	using float2 = vectorf2;
	using float3 = vectorf3;
	using float4 = vectorf4;

	using uint2 = vectoru2;
	using uint3 = vectoru3;
	using uint4 = vectoru4;

	using int2 = vectori2;
	using int3 = vectori3;
	using int4 = vectori4;
#pragma endregion
}

#include "vector.inl"

#pragma warning(default : 4201)

#endif