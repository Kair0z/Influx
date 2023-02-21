#include "Vector.h"

#define __CORE_VECTOR_USECORE_ 1
#if __CORE_VECTOR_USECORE_
#include "Core/Assert.h"
#else
#include <cassert>
#define FLX_ASSERT(expr) assert(expr)
#endif

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include <cmath>

namespace Influx::Math
{
	// Constructors:
	template<typename _T, VectorSizeType _N> // Initializer list...
	template<typename ..._V> inline Vector<_T, _N>::Vector(const _V& ...components)
		: Internal::VectorBase<_T, _N>(static_cast<_T>(components) ...) 
	{

	}

	template<typename _T, VectorSizeType _N> // Typecast...
	template <typename _U> inline Vector<_T, _N>::Vector(const Vector<_U, _N>& other)
	{
		for (size_t i{}; i < _N; ++i)
			this->data[i] = other.data[i];
	}

	template<typename _T, VectorSizeType _N> // Sizecast constructor
	template <VectorSizeType _D> inline Vector<_T, _N>::Vector(const Vector<_T, _D>& other)
	{
		for (VectorSizeType i{}; i < _N; ++i)
			this->data[i] = (i < _D) ? other[i] : static_cast<_T>(0);

		// If smaller, copy all data
		// If bigger, copy data & init 0 leftover...
	}

	template<typename _T, VectorSizeType _N>
	constexpr VectorSizeType Vector<_T, _N>::Size()
	{
		return _N;
	}

	// Data
#pragma region Data Access
	template<typename _T, VectorSizeType _N>
	inline _T& Vector<_T, _N>::operator[](VectorSizeType i)
	{
		FLX_ASSERT(i < _N);
		return this->data[i];
	}
	template<typename _T, VectorSizeType _N>
	inline const _T& Vector<_T, _N>::operator[](VectorSizeType i) const
	{
		FLX_ASSERT(i < _N);
		return this->data[i];
	}
	template<typename _T, VectorSizeType _N>
	inline _T& Vector<_T, _N>::At(VectorSizeType i)
	{
		FLX_ASSERT(i < _N);
		return (*this)[i];
	}
	template<typename _T, VectorSizeType _N>
	inline const _T& Vector<_T, _N>::At(VectorSizeType i) const
	{
		FLX_ASSERT(i < _N);
		return (*this)[i];
	}
	template<typename _T, VectorSizeType _N>
	const _T* Vector<_T, _N>::Data() const
	{
		return this->data;
	}
#pragma endregion

	// Normalize:
#pragma region Normalize
	template<typename _T, VectorSizeType _N>
	inline Vector<_T, _N> Vector<_T, _N>::Normalized() const { return Vector<_T, _N>::Normalized(*this); }
	template<typename _T, VectorSizeType _N>
	inline void Vector<_T, _N>::Normalize() { Normalize(*this); }
	template<typename _T, VectorSizeType _N>
	inline void Vector<_T, _N>::Normalize(Vector& vec) { vec = Normalized(vec); }

	template<typename _T, VectorSizeType _N>
	inline Vector<_T, _N> Vector<_T, _N>::Normalized(const Vector<_T, _N>& vec)
	{
		return vec / vec.Magnitude();
	}
#pragma endregion

	// Clamp:
#pragma region Clamp
	template<typename _T, VectorSizeType _N>
	Vector<_T, _N> Clamped(float min, float max) { return Vector<_T, _N>::Clamped(*this, min, max); }
	template<typename _T, VectorSizeType _N>
	void Clamp(float min, float max) { return Clamp(*this, min, max); }
	template<typename _T, VectorSizeType _N>
	static void Clamp(Vector<_T, _N>& vec, float min, float max) { vec = Clamped(vec); }

	template<typename _T, VectorSizeType _N>
	static Vector<_T, _N> Clamped(const Vector<_T, _N>& vec, float min, float max)
	{
		FLX_ASSERT(false); // Noimpl
	}
#pragma endregion

	// Angle:
#pragma region Angle
	template<typename _T, VectorSizeType _N>
	inline float Vector<_T, _N>::RadiansBetween(const Vector& other) const { return AngleBetween(*this, other); }

	template <typename _T, VectorSizeType _N>
	inline float Vector<_T, _N>::RadiansBetween(const Vector<_T, _N>& a, const Vector<_T, _N>& b)
	{
		float magA = Magnitude(a);
		float magB = Magnitude(b);
		if (magA == 0.0f || magB == 0.0f) return 0.0f;

		return std::acosf(Dot(a, b) / (magA * magB));
	}
#pragma endregion

	// Magnitude:
#pragma region Magnitude
	template<typename _T, VectorSizeType _N>
	inline float Vector<_T, _N>::Magnitude() const { return Magnitude(*this); }
	template<typename _T, VectorSizeType _N>
	inline float Vector<_T, _N>::SqrMagnitude() const { return SqrMagnitude(*this); }
	template<typename _T, VectorSizeType _N>
	inline float Vector<_T, _N>::Magnitude(const Vector& other) { return sqrtf(SqrMagnitude(other)); }

	template<typename _T, VectorSizeType _N>
	inline float Vector<_T, _N>::SqrMagnitude(const Vector& other) 
	{ 
		return Dot(other, other); 
	}

	template<typename _T, VectorSizeType _N>
	inline float Vector<_T, _N>::Distance(const Vector& a, const Vector& b)
	{
		return Magnitude(a - b);
	}

	template<typename _T, VectorSizeType _N>
	inline float Vector<_T, _N>::SqrDistance(const Vector& a, const Vector& b)
	{
		return SqrMagnitude(a - b);
	}
#pragma endregion

	// Scaling
#pragma region Scaling
	template<typename _T, VectorSizeType _N>
	inline void Vector<_T, _N>::Scale(float mag)
	{
		Scale(*this, mag);
	}
	template<typename _T, VectorSizeType _N>
	inline Vector<_T, _N> Vector<_T, _N>::Scaled(float mag) const
	{
		return Scaled(*this, mag);
	}
	template<typename _T, VectorSizeType _N>
	inline void Vector<_T, _N>::Scale(Vector& vec, float mag)
	{
		vec = vec.Normalized() * mag;
	}
	template<typename _T, VectorSizeType _N>
	inline Vector<_T, _N> Vector<_T, _N>::Scaled(const Vector& vec, float mag)
	{
		Vector<_T, _N> res = vec;
		Scale(res, mag);
		return res;
	}
#pragma endregion
	
	// Dot & Cross
#pragma region Dot & Cross
	template<typename _T, VectorSizeType _N>
	inline float Vector<_T, _N>::Dot(const Vector& other) const
	{
		return Dot(*this, other);
	}
	template<typename _T, VectorSizeType _N>
	inline float Vector<_T, _N>::Dot(const Vector& a, const Vector& b)
	{
		float result{};
		for (VectorSizeType i{}; i < _N; ++i) result += a[i] * b[i];
		return result;
	}
	template<typename _T, VectorSizeType _N>
	inline float Vector<_T, _N>::Cross(const Vector<_T, 2>& other) const
	{
		return Cross(*this, other);
	}
	template<typename _T, VectorSizeType _N>
	inline Vector<_T, 3> Vector<_T, _N>::Cross(const Vector<_T, 3>& other) const
	{
		return Cross(*this, other);
	}
	template<typename _T, VectorSizeType _N> // 2D
	inline float Vector<_T, _N>::Cross(const Vector<_T, 2>& a, const Vector<_T, 2>& b)
	{
		return { a.x * b.y - a.y * b.x };
	}
	template<typename _T, VectorSizeType _N> // 3D
	inline Vector<_T, 3> Vector<_T, _N>::Cross(const Vector<_T, 3>& a, const Vector<_T, 3>& b)
	{
		return
		{
			a[1] * b[2] - a[2] * b[1],
			a[2] * b[0] - a[0] * b[2],
			a[0] * b[1] - a[1] * b[0]
		};
	}
#pragma endregion

	// Inversion
#pragma region Inversion
	template<typename _T, VectorSizeType _N>
	inline const Vector<_T, _N>& Vector<_T, _N>::Inverted() const
	{
		return Inverted(*this);
	}
	template<typename _T, VectorSizeType _N>
	inline void Vector<_T, _N>::Inverse()
	{
		Inverse(*this);
	}
	template<typename _T, VectorSizeType _N>
	inline void Vector<_T, _N>::Inverse(Vector& vec)
	{
		vec = Inverted(vec);
	}
	template<typename _T, VectorSizeType _N>
	inline Vector<_T, _N> Vector<_T, _N>::Inverted(const Vector& vec)
	{
		Vector<_T, _N> res = vec;
		for (VectorSizeType i{}; i < _N; ++i)
		{
			res[i] = -res[i];
		}

		return res;
	}
#pragma endregion

	// Reflection
#pragma region Reflection
	template<typename _T, VectorSizeType _N>
	inline const Vector<_T, 2>& Vector<_T, _N>::Reflected(const Vector<_T, 2>& hitNormal) const
	{
		return Reflection(*this, hitNormal);

	}
	template<typename _T, VectorSizeType _N>
	inline const Vector<_T, 3>& Vector<_T, _N>::Reflected(const Vector<_T, 3>& hitNormal) const
	{
		return Reflection(*this, hitNormal);
	}

	template<typename _T, VectorSizeType _N>
	inline Vector<_T, 2> Vector<_T, _N>::Reflection(const Vector<_T, 2>& vector, const Vector<_T, 2>& hitNormal)
	{
		return vector - 2 * (Dot(vector, hitNormal)) * hitNormal;
	}
	template<typename _T, VectorSizeType _N>
	inline Vector<_T, 3> Vector<_T, _N>::Reflection(const Vector<_T, 3>& vector, const Vector<_T, 3>& hitNormal)
	{
		return vector - 2 * (Dot(vector, hitNormal)) * hitNormal;
	}
#pragma endregion

	// Lerp:
#pragma region Lerp
	template<typename _T, VectorSizeType _N>
	inline Vector<_T, _N> Vector<_T, _N>::Lerp(const Vector& a, const Vector& b, const float t)
	{
		return a * t + b * (1.0f - t);
	}
#pragma endregion

	// Zero:
#pragma region Null
	template<typename _T, VectorSizeType _N>
	inline bool Vector<_T, _N>::IsZero() const
	{
		return IsNull(*this);
	}
	template<typename _T, VectorSizeType _N>
	inline bool Vector<_T, _N>::IsZero(const Vector& v)
	{
		for (size_t i{}; i < _N; ++i)
			if (v[i] != static_cast<_T>(0)) return false;

		return true;
	}

	template<typename _T, VectorSizeType _N>
	inline constexpr Vector<_T, 3u> Vector<_T, _N>::Up()
	{
		return Vector<_T, 3u>(static_cast<_T>(0), static_cast<_T>(1), static_cast<_T>(0));
	}

	template<typename _T, VectorSizeType _N>
	inline constexpr Vector<_T, 3u> Vector<_T, _N>::Forward()
	{
		return Vector<_T, 3u>(static_cast<_T>(0), static_cast<_T>(0), static_cast<_T>(1));
	}

	template<typename _T, VectorSizeType _N>
	inline constexpr Vector<_T, 3u> Vector<_T, _N>::Right()
	{
		return Vector<_T, 3u>(static_cast<_T>(1), static_cast<_T>(0), static_cast<_T>(0));
	}

	template<typename _T, VectorSizeType _N>
	inline Vector<_T, _N> Vector<_T, _N>::Zero()
	{
		return Vector<_T, _N>();
	}

	template<typename _T, VectorSizeType _N>
	inline Vector<_T, _N> Vector<_T, _N>::One()
	{
		Vector<_T, _N> result{};

		for (size_t i{}; i < _N; ++i)
			result[i] = static_cast<_T>(1);

		return result;
	}
#pragma endregion

	// Comparison:
#pragma region Comparison
	template<typename _T, VectorSizeType _N>
	inline bool Vector<_T, _N>::operator==(const Vector& other) const
	{
		bool same = true;
		for (VectorSizeType i{}; i < _N; ++i) same = same && (At(i) == other.At(i));

		return same;
	}
	template<typename _T, VectorSizeType _N>
	inline bool Vector<_T, _N>::operator!=(const Vector& other) const
	{
		return !(*this == other);
	}
#pragma endregion

	// Arithmatic:
#pragma region Arithmetic
	template<typename _T, VectorSizeType _N>
	inline Vector<_T, _N>& Vector<_T, _N>::operator+=(const Vector& other)
	{
		return *this = *this + other;
	}
	template<typename _T, VectorSizeType _N>
	inline Vector<_T, _N>& Vector<_T, _N>::operator-=(const Vector& other)
	{
		return *this = *this - other;
	}
	template<typename _T, VectorSizeType _N>
	inline Vector<_T, _N>& Vector<_T, _N>::operator*=(const Vector& other)
	{
		return *this = *this * other;
	}
	template<typename _T, VectorSizeType _N>
	inline Vector<_T, _N>& Vector<_T, _N>::operator*=(const float scalar)
	{
		return *this = *this * scalar;
	}
	template<typename _T, VectorSizeType _N>
	inline Vector<_T, _N>& Vector<_T, _N>::operator/=(const Vector& other)
	{
		return *this = *this / other;
	}
	template<typename _T, VectorSizeType _N>
	inline Vector<_T, _N>& Vector<_T, _N>::operator/=(const float scalar)
	{
		assert(scalar != static_cast<_T>(0));

		return *this = *this / scalar;
	}
	template<typename _T, VectorSizeType _N>
	Vector<_T, _N> operator+(const Vector<_T, _N>& a, const Vector<_T, _N>& b)
	{
		Vector<_T, _N> res{};
		for (VectorSizeType i{}; i < _N; ++i)
			res[i] = a[i] + b[i];

		return res;
	}
	template<typename _T, VectorSizeType _N>
	Vector<_T, _N> operator-(const Vector<_T, _N>& a, const Vector<_T, _N>& b)
	{
		Vector<_T, _N> res{};
		for (VectorSizeType i{}; i < _N; ++i)
			res[i] = a[i] - b[i];

		return res;
	}
	template<typename _T, VectorSizeType _N>
	Vector<_T, _N> operator*(const Vector<_T, _N>& a, const Vector<_T, _N>& b)
	{
		Vector<_T, _N> res{};
		for (VectorSizeType i{}; i < _N; ++i)
			res[i] = a[i] * b[i];

		return res;
	}
	template<typename _T, VectorSizeType _N>
	Vector<_T, _N> operator/(const Vector<_T, _N>& a, const Vector<_T, _N>& b)
	{
		FLX_ASSERT(b.x != static_cast<_T>(0) && b.y != static_cast<_T>(0) && b.z != static_cast<_T>(0));

		Vector<_T, _N> res{};
		for (VectorSizeType i{}; i < _N; ++i)
			res[i] = a[i] / b[i];

		return res;
	}
	template<typename _T, VectorSizeType _N>
	Vector<_T, _N> operator*(const Vector<_T, _N>& a, const float b)
	{
		Vector<_T, _N> res{};
		for (VectorSizeType i{}; i < _N; ++i)
			res[i] = a[i] * b;

		return res;
	}
	template<typename _T, VectorSizeType _N>
	Vector<_T, _N> operator/(const Vector<_T, _N>& a, const float b)
	{
		Vector<_T, _N> res{};
		for (VectorSizeType i{}; i < _N; ++i)
			res[i] = a[i] / b;

		return res;
	}
	template<typename _T, VectorSizeType _N>
	Vector<_T, _N> operator*(const float a, const Vector<_T, _N>& b)
	{
		Vector<_T, _N> res{};
		for (VectorSizeType i{}; i < _N; ++i)
			res[i] = a * b[i];

		return res;
	}
	template<typename _T, VectorSizeType _N>
	Vector<_T, _N> operator-(const Vector<_T, _N>& v)
	{
		Vector<_T, _N> res{};
		for (VectorSizeType i{}; i < _N; ++i)
			res[i] = -v[i];

		return res;
	}
#pragma endregion
}