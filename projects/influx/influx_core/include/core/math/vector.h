#pragma once

#ifndef _CORE_MATH_VECTOR_H_
#define _CORE_MATH_VECTOR_H_

#include "core/basetypes.h"

#pragma warning(disable : 4201) // Union warning...

namespace influx::math
{
	using _vector_dim_t = size_t;

	namespace detail
	{
		template <typename _t, _vector_dim_t _dim>
		struct base_vector
		{
			union
			{
				struct { _t x, y, z, w; };
				struct { _t r, g, b, a; };
				_t m_data[_dim];
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

	template <typename _t, _vector_dim_t _dim>
	class vector final : public detail::base_vector<_t, _dim>
	{
		static_assert(_dim != 0u, "influx::vector<_t, _dim> ¬ Cannot instantiate zero-sized vector (_dim == 0)! ");

	public:
		// Constructors:
		vector() = default;
		vector(const vector& other) = default;
		vector(vector&& other) = default;
		vector& operator=(const vector& other) = default;
		vector& operator=(vector&& other) = default;

		template <typename... _V>		vector(const _V&... components);		// Initializer list
		template <typename _U>			vector(const vector<_U, _dim>& other);	// Typecasting
		template <_vector_dim_t _D>		vector(const vector<_t, _D>& other);	// Sizecasting

		constexpr static _vector_dim_t dimension();

		// Accessing data:
		_t& operator[](_vector_dim_t i);
		const _t& operator[](_vector_dim_t i) const;
		_t& at(_vector_dim_t i);
		const _t& at(_vector_dim_t i) const;

		const _t* data() const;
		_t* data();

		// Normalizing:
		vector normalized() const;
		void normalize();
		static void normalize(vector& vec);
		static vector normalized(const vector& vec);

		// Clamp:
		vector clamped(float min, float max);
		void clamp(float min, float max);
		static void clamp(vector& vec, float min, float max);
		static vector clamped(const vector& vec, float min, float max);

		void clamp_length(float length);

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
		float dot(const vector& other) const;
		static float dot(const vector& a, const vector& b);

		float cross(const vector<_t, 2u>& other) const;
		vector<_t, 3u> cross(const vector<_t, 3u>& other) const;
		static float cross(const vector<_t, 2u>& a, const vector<_t, 2u>& b);
		static vector<_t, 3u> cross(const vector<_t, 3u>& a, const vector<_t, 3u>& b);

		// Comparison
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
		bool is_zero() const;
		static bool is_zero(const vector& v);

		// 3D:
		constexpr static vector<_t, 3u> up();
		constexpr static vector<_t, 3u> forward();
		constexpr static vector<_t, 3u> right();

		static vector<_t, 2u> get_look_at(const vector<_t, 2u>& from, const vector<_t, 2u>& to);
		static vector<_t, 3u> get_look_at(const vector<_t, 3u>& from, const vector<_t, 3u>& to);

		vector<_t, 2u> get_xy() const;

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
	template <typename _t, _vector_dim_t _dim>
	vector<_t, _dim> operator+(const vector<_t, _dim>& a, const vector<_t, _dim>& b);
	template <typename _t, _vector_dim_t _dim>
	vector<_t, _dim> operator-(const vector<_t, _dim>& a, const vector<_t, _dim>& b);
	template <typename _t, _vector_dim_t _dim>
	vector<_t, _dim> operator*(const vector<_t, _dim>& a, const vector<_t, _dim>& b);
	template <typename _t, _vector_dim_t _dim>
	vector<_t, _dim> operator/(const vector<_t, _dim>& a, const vector<_t, _dim>& b);

	// Scalar operators:
	template <typename _t, _vector_dim_t _dim>
	vector<_t, _dim> operator*(const vector<_t, _dim>& a, const float b);
	template <typename _t, _vector_dim_t _dim>
	vector<_t, _dim> operator/(const vector<_t, _dim>& a, const float b);
	template <typename _t, _vector_dim_t _dim>
	vector<_t, _dim> operator*(const float a, const vector<_t, _dim>& b);
	template <typename _t, _vector_dim_t _dim>
	vector<_t, _dim> operator-(const vector<_t, _dim>& v);

#pragma region Aliases
	template <typename _vector_dim_t _dim>
	using vectoru8 = vector<uint8, _dim>;

	template <typename _vector_dim_t _dim>
	using vectoru32 = vector<uint32, _dim>;

	template <typename _vector_dim_t _dim>
	using vectoru64 = vector<uint64, _dim>;

	template <typename _vector_dim_t _dim>
	using vectori = vector<int, _dim>;

	template <typename _vector_dim_t _dim>
	using vectorf = vector<float, _dim>;

	template <typename _vector_dim_t _dim>
	using vectorl = vector<long, _dim>;

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