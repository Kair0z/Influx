#pragma once

#ifndef _CORE_MATH_VECTOR_H_
#define _CORE_MATH_VECTOR_H_

#include <stdint.h>

#pragma warning(disable : 4201) // Union warning...

namespace Influx::Math
{
	using VectorSizeType = size_t;

	namespace Internal
	{
		template <typename _T, VectorSizeType _N>
		struct VectorBase
		{
			union
			{
				struct { _T x, y, z, w; };
				struct { _T r, g, b, a; };
				_T data[_N];
			};

			template <typename... _V>
			VectorBase(const _V&... components);
		};

		template <typename _T>
		struct VectorBase<_T, 1u>
		{
			union
			{
				struct { _T x; };
				struct { _T r; };
				_T data[1];
			};
			VectorBase<_T, 1u>(const _T _x = 0) : x{ _x } {}
		};

		template <typename _T>
		struct VectorBase<_T, 2u>
		{
			union
			{
				struct { _T x, y; };
				struct { _T r, g; };
				_T data[2];
			};

			VectorBase<_T, 2u>(const _T _x = 0, const _T _y = 0) : x{ _x }, y{ _y } {}
		};

		template <typename _T>
		struct VectorBase<_T, 3u>
		{
			union
			{
				struct { _T x, y, z; };
				struct { _T r, g, b; };
				_T data[3];
			};

			VectorBase<_T, 3u>(const _T _x = 0, const _T _y = 0, const _T _z = 0) : x{ _x }, y{ _y }, z{ _z } {}
		};

		template <typename _T>
		struct VectorBase<_T, 4u>
		{
			union
			{
				struct { _T x, y, z, w; };
				struct { _T r, g, b, a; };
				_T data[4];
			};

			VectorBase<_T, 4u>(const _T _x = 0, const _T _y = 0, const _T _z = 0, const _T _w = 0) : x{ _x }, y{ _y }, z{ _z }, w{ _w } {}
		};
	}

	template <typename _T, VectorSizeType _N>
	class Vector : public Internal::VectorBase<_T, _N>
	{
	public:
		// Constructors:
		Vector() = default;
		Vector(const Vector& other) = default;
		Vector(Vector && other) = default;
		Vector& operator=(const Vector & other) = default;
		Vector& operator=(Vector && other) = default;

		template <typename... _V>		Vector(const _V&... components);		// Initializer list
		template <typename _U>			Vector(const Vector<_U, _N>& other);	// Typecasting
		template <VectorSizeType _D>	Vector(const Vector<_T, _D>& other);	// Sizecasting

		constexpr static VectorSizeType Size();

		// Accessing data:
		_T& operator[](VectorSizeType i);
		const _T& operator[](VectorSizeType i) const;
		_T& Data(VectorSizeType i);
		const _T& Data(VectorSizeType i) const;

		// Normalizing:
		Vector Normalized() const;
		void Normalize();
		static void Normalize(Vector& vec);
		static Vector Normalized(const Vector& vec);

		// Clamp:
		Vector Clamped(float min, float max);
		void Clamp(float min, float max);
		static void Clamp(Vector& vec, float min, float max);
		static Vector Clamped(const Vector& vec, float min, float max);

		// Angle:
		float RadiansBetween(const Vector& other) const;
		static float RadiansBetween(const Vector& a, const Vector& b);

		// Magnitude:
		float Magnitude() const;
		float SqrMagnitude() const;
		static float Magnitude(const Vector& other);
		static float SqrMagnitude(const Vector& other);
		static float Distance(const Vector& a, const Vector& b);
		static float SqrDistance(const Vector& a, const Vector& b);

		// Scaling:
		void Scale(float mag);
		Vector Scaled(float mag) const;
		static void Scale(Vector& vec, float mag);
		static Vector Scaled(const Vector& vec, float mag);

		// Cross & dot:
		float Dot(const Vector& other) const;
		static float Dot(const Vector& a, const Vector& b);

		float Cross(const Vector<_T, 2u>& other) const;
		Vector<_T, 3u> Cross(const Vector<_T, 3u>& other) const;
		static float Cross(const Vector<_T, 2u>& a, const Vector<_T, 2u>& b);
		static Vector<_T, 3u> Cross(const Vector<_T, 3u>& a, const Vector<_T, 3u>& b);

		// Comparison
		bool operator==(const Vector& other) const;
		bool operator!=(const Vector& other) const;

		// Inverting:
		const Vector& Inverted() const;
		void Inverse();
		static void Inverse(Vector& vec);
		static Vector Inverted(const Vector& vec);

		// Reflect:
		const	Vector<_T, 2u>&	Reflected(const Vector<_T, 2u>& hitNormal) const;
		static	Vector<_T, 2u>	Reflection(const Vector<_T, 2u>& vector, const Vector<_T, 2>& hitNormal);
		const	Vector<_T, 3u>&	Reflected(const Vector<_T, 3u>& hitNormal) const;
		static	Vector<_T, 3u>	Reflection(const Vector<_T, 3u>& vector, const Vector<_T, 3>& hitNormal);

		// Lerp:
		static Vector Lerp(const Vector& a, const Vector& b, const float t);

		// Zero:
		static Vector Zero();
		static Vector One();
		bool IsZero() const;
		static bool IsZero(const Vector& v);

		// Arithmatics:
		Vector& operator+=(const Vector& other);
		Vector& operator-=(const Vector& other);
		Vector& operator*=(const Vector& other);
		Vector& operator*=(const float scalar);
		Vector& operator/=(const Vector& other);
		Vector& operator/=(const float scalar);
	};

	// Per-Component operators:
	template <typename _T, VectorSizeType _N>
	Vector<_T, _N> operator+(const Vector<_T, _N>& a, const Vector<_T, _N>& b);
	template <typename _T, VectorSizeType _N>
	Vector<_T, _N> operator-(const Vector<_T, _N>& a, const Vector<_T, _N>& b);
	template <typename _T, VectorSizeType _N>
	Vector<_T, _N> operator*(const Vector<_T, _N>& a, const Vector<_T, _N>& b);
	template <typename _T, VectorSizeType _N>
	Vector<_T, _N> operator/(const Vector<_T, _N>& a, const Vector<_T, _N>& b);

	// Scalar operators:
	template <typename _T, VectorSizeType _N>
	Vector<_T, _N> operator*(const Vector<_T, _N>& a, const float b);
	template <typename _T, VectorSizeType _N>
	Vector<_T, _N> operator/(const Vector<_T, _N>& a, const float b);
	template <typename _T, VectorSizeType _N>
	Vector<_T, _N> operator*(const float a, const Vector<_T, _N>& b);
	template <typename _T, VectorSizeType _N>
	Vector<_T, _N> operator-(const Vector<_T, _N>& v);

#pragma region Aliases
	template <VectorSizeType _N>
	using Vectoru8 = Vector<VectorSizeType, _N>;

	template <VectorSizeType _N>
	using Vectoru32 = Vector<VectorSizeType, _N>;

	template <VectorSizeType _N>
	using Vectoru64 = Vector<VectorSizeType, _N>;

	template <VectorSizeType _N>
	using Vectori = Vector<int, _N>;

	template <VectorSizeType _N>
	using Vectorf = Vector<float, _N>;

	template <VectorSizeType _N>
	using Vectorl = Vector<long, _N>;

	using Vectorf2 = Vectorf<2u>;
	using Vectorf3 = Vectorf<3u>;
	using Vectorf4 = Vectorf<4u>;

	using Vectoru2 = Vectoru32<2u>;
	using Vectoru3 = Vectoru32<3u>;
	using Vectoru4 = Vectoru32<4u>;

	using Vectori2 = Vectori<2u>;
	using Vectori3 = Vectori<3u>;
	using Vectori4 = Vectori<4u>;
#pragma endregion
}

#include "Vector.inl"

#pragma warning(default : 4201)

#endif