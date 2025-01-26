#include "core/math/vector.h"

#define __CORE_VECTOR_USECORE_ 1

#if __CORE_VECTOR_USECORE_
#include "core/debug.h"
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
#include <algorithm>
#include <limits>

namespace influx::math
{
	// Constructors:
	template<typename _t, _vector_dim_t _dim> // Initializer list...
	template<typename ..._V> inline vector<_t, _dim>::vector(const _V& ...components)
		: detail::base_vector<_t, _dim>(static_cast<_t>(components) ...) 
	{

	}

	template<typename _t, _vector_dim_t _dim> // Typecast...
	template <typename _U> inline vector<_t, _dim>::vector(const vector<_U, _dim>& other)
	{
		for (size_t i{}; i < _dim; ++i)
			this->m_data[i] = other.m_data[i];
	}

	template<typename _t, _vector_dim_t _dim> // Sizecast constructor
	template <_vector_dim_t _D> inline vector<_t, _dim>::vector(const vector<_t, _D>& other)
	{
		for (_vector_dim_t i{}; i < _dim; ++i)
			this->m_data[i] = (i < _D) ? other[i] : static_cast<_t>(0);

		// If smaller, copy all data
		// If bigger, copy data & init 0 leftover...
	}

	template<typename _t, _vector_dim_t _dim>
	constexpr _vector_dim_t vector<_t, _dim>::dimension()
	{
		return _dim;
	}

	// Data
#pragma region data Access
	template<typename _t, _vector_dim_t _dim>
	inline _t& vector<_t, _dim>::operator[](_vector_dim_t i)
	{
		influx_assert(i < _dim);
		return this->m_data[i];
	}
	template<typename _t, _vector_dim_t _dim>
	inline const _t& vector<_t, _dim>::operator[](_vector_dim_t i) const
	{
		influx_assert(i < _dim);
		return this->m_data[i];
	}
	template<typename _t, _vector_dim_t _dim>
	inline _t& vector<_t, _dim>::at(_vector_dim_t i)
	{
		influx_assert(i < _dim);
		return (*this)[i];
	}
	template<typename _t, _vector_dim_t _dim>
	inline const _t& vector<_t, _dim>::at(_vector_dim_t i) const
	{
		influx_assert(i < _dim);
		return (*this)[i];
	}
	template<typename _t, _vector_dim_t _dim>
	const _t* vector<_t, _dim>::data() const
	{
		return this->m_data;
	}
	template<typename _t, _vector_dim_t _dim>
	_t* vector<_t, _dim>::data()
	{
		return this->m_data;
	}
#pragma endregion

	// Normalize:
#pragma region Normalize
	template<typename _t, _vector_dim_t _dim>
	inline vector<_t, _dim> vector<_t, _dim>::normalized() const { return vector<_t, _dim>::normalized(*this); }
	template<typename _t, _vector_dim_t _dim>
	inline void vector<_t, _dim>::normalize() { normalize(*this); }
	template<typename _t, _vector_dim_t _dim>
	inline void vector<_t, _dim>::normalize(vector& vec) { vec = normalized(vec); }
	template<typename _t, _vector_dim_t _dim>
	inline vector<_t, _dim> vector<_t, _dim>::normalized(const vector<_t, _dim>& vec)
	{
		return vec / vec.magnitude();
	}
#pragma endregion

	// Clamp:
#pragma region clamp
	template<typename _t, _vector_dim_t _dim>
	vector<_t, _dim> clamped(float min, float max) { return vector<_t, _dim>::clamped(*this, min, max); }
	template<typename _t, _vector_dim_t _dim>
	void clamp(float min, float max) { return clamp(*this, min, max); }
	template<typename _t, _vector_dim_t _dim>
	static void clamp(vector<_t, _dim>& vec, float min, float max) { vec = clamped(vec); }
	template<typename _t, _vector_dim_t _dim>
	static vector<_t, _dim> clamped(const vector<_t, _dim>& vec, float min, float max)
	{
		influx_assert(false); // Noimpl
	}
	template<typename _t, _vector_dim_t _dim>
	inline void vector<_t, _dim>::clamp_length(float length)
	{
		if (sqr_magnitude() > (length * length))
		{
			scale(length);
		}
	}
#pragma endregion

	// Angle:
#pragma region Angle
	template<typename _t, _vector_dim_t _dim>
	inline float vector<_t, _dim>::radians_between(const vector& other) const { return angle_between(*this, other); }

	template <typename _t, _vector_dim_t _dim>
	inline float vector<_t, _dim>::radians_between(const vector<_t, _dim>& a, const vector<_t, _dim>& b)
	{
		float magA = magnitude(a);
		float magB = magnitude(b);
		if (magA == 0.0f || magB == 0.0f) return 0.0f;

		return std::acosf(Dot(a, b) / (magA * magB));
	}
#pragma endregion

	// Magnitude:
#pragma region Magnitude
	template<typename _t, _vector_dim_t _dim>
	inline float vector<_t, _dim>::magnitude() const { return magnitude(*this); }
	template<typename _t, _vector_dim_t _dim>
	inline float vector<_t, _dim>::sqr_magnitude() const { return sqr_magnitude(*this); }
	template<typename _t, _vector_dim_t _dim>
	inline float vector<_t, _dim>::magnitude(const vector& other) { return sqrtf(sqr_magnitude(other)); }
	template<typename _t, _vector_dim_t _dim>
	inline float vector<_t, _dim>::sqr_magnitude(const vector& other) 
	{ 
		return dot(other, other); 
	}
	template<typename _t, _vector_dim_t _dim>
	inline float vector<_t, _dim>::distance(const vector& a, const vector& b)
	{
		return magnitude(a - b);
	}
	template<typename _t, _vector_dim_t _dim>
	inline float vector<_t, _dim>::sqr_distance(const vector& a, const vector& b)
	{
		return sqr_magnitude(a - b);
	}
#pragma endregion

	// Scaling
#pragma region Scaling
	template<typename _t, _vector_dim_t _dim>
	inline void vector<_t, _dim>::scale(float mag)
	{
		scale(*this, mag);
	}
	template<typename _t, _vector_dim_t _dim>
	inline vector<_t, _dim> vector<_t, _dim>::scaled(float mag) const
	{
		return scaled(*this, mag);
	}
	template<typename _t, _vector_dim_t _dim>
	inline void vector<_t, _dim>::scale(vector& vec, float mag)
	{
		vec = vec.normalized() * mag;
	}
	template<typename _t, _vector_dim_t _dim>
	inline vector<_t, _dim> vector<_t, _dim>::scaled(const vector& vec, float mag)
	{
		vector<_t, _dim> res = vec;
		scale(res, mag);
		return res;
	}
#pragma endregion
	
	// Dot & Cross
#pragma region Dot & Cross
	template<typename _t, _vector_dim_t _dim>
	inline float vector<_t, _dim>::dot(const vector& other) const
	{
		return dot(*this, other);
	}
	template<typename _t, _vector_dim_t _dim>
	inline float vector<_t, _dim>::dot(const vector& a, const vector& b)
	{
		float result{};
		for (_vector_dim_t i{}; i < _dim; ++i) result += a[i] * b[i];
		return result;
	}
	template<typename _t, _vector_dim_t _dim>
	inline float vector<_t, _dim>::cross(const vector<_t, 2>& other) const
	{
		return cross(*this, other);
	}
	template<typename _t, _vector_dim_t _dim>
	inline vector<_t, 3> vector<_t, _dim>::cross(const vector<_t, 3>& other) const
	{
		return cross(*this, other);
	}
	template<typename _t, _vector_dim_t _dim> // 2D
	inline float vector<_t, _dim>::cross(const vector<_t, 2>& a, const vector<_t, 2>& b)
	{
		return { a.x * b.y - a.y * b.x };
	}
	template<typename _t, _vector_dim_t _dim> // 3D
	inline vector<_t, 3> vector<_t, _dim>::cross(const vector<_t, 3>& a, const vector<_t, 3>& b)
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
	template<typename _t, _vector_dim_t _dim>
	inline const vector<_t, _dim>& vector<_t, _dim>::inverted() const
	{
		return inverted(*this);
	}
	template<typename _t, _vector_dim_t _dim>
	inline void vector<_t, _dim>::inverse()
	{
		inverse(*this);
	}
	template<typename _t, _vector_dim_t _dim>
	inline void vector<_t, _dim>::inverse(vector& vec)
	{
		vec = inverted(vec);
	}
	template<typename _t, _vector_dim_t _dim>
	inline vector<_t, _dim> vector<_t, _dim>::inverted(const vector& vec)
	{
		vector<_t, _dim> res = vec;
		for (_vector_dim_t i{}; i < _dim; ++i)
		{
			res[i] = -res[i];
		}

		return res;
	}
#pragma endregion

	// Reflection
#pragma region Reflection
	template<typename _t, _vector_dim_t _dim>
	inline const vector<_t, 2u>& vector<_t, _dim>::reflected(const vector<_t, 2u>& hitNormal) const
	{
		return reflection(*this, hitNormal);
	}
	template<typename _t, _vector_dim_t _dim>
	inline const vector<_t, 3u>& vector<_t, _dim>::reflected(const vector<_t, 3u>& hitNormal) const
	{
		return reflection(*this, hitNormal);
	}
	template<typename _t, _vector_dim_t _dim>

	inline vector<_t, 2u> vector<_t, _dim>::reflection(const vector<_t, 2u>& vec, const vector<_t, 2u>& hitNormal)
	{
		return vec - 2.0f * (dot(vec, hitNormal)) * hitNormal;
	}
	template<typename _t, _vector_dim_t _dim>
	inline vector<_t, 3u> vector<_t, _dim>::reflection(const vector<_t, 3u>& vec, const vector<_t, 3u>& hitNormal)
	{
		return vec - 2.0f * (dot(vec, hitNormal)) * hitNormal;
	}
#pragma endregion

	// Lerp:
#pragma region lerp
	template<typename _t, _vector_dim_t _dim>
	inline vector<_t, _dim> vector<_t, _dim>::lerp(const vector& a, const vector& b, const float t)
	{
		float clamped_t = t;

		// clamp [0-1]
		if (clamped_t < 0.0f) clamped_t = 0.0f;
		if (clamped_t > 1.0f) clamped_t = 1.0f;

		return (b * clamped_t) + (a * (1.0f - clamped_t));
	}

	template<typename _t, _vector_dim_t _dim>
	void vector<_t, _dim>::lerp_towards(const vector& b, const float t)
	{
		*this = lerp(*this, b, t);
	}
#pragma endregion

	// Zero:
#pragma region Null
	template<typename _t, _vector_dim_t _dim>
	inline bool vector<_t, _dim>::is_zero() const
	{
		return is_zero(*this);
	}
	template<typename _t, _vector_dim_t _dim>
	inline bool vector<_t, _dim>::is_zero(const vector& v)
	{
		for (_vector_dim_t i{}; i < _dim; ++i)
			if (v[i] != static_cast<_t>(0)) return false;

		return true;
	}
	template<typename _t, _vector_dim_t _dim>
	inline constexpr vector<_t, 3u> vector<_t, _dim>::up()
	{
		return vector<_t, 3u>(static_cast<_t>(0), static_cast<_t>(1), static_cast<_t>(0));
	}
	template<typename _t, _vector_dim_t _dim>
	inline constexpr vector<_t, 3u> vector<_t, _dim>::forward()
	{
		return vector<_t, 3u>(static_cast<_t>(0), static_cast<_t>(0), static_cast<_t>(1.0f));
	}
	template<typename _t, _vector_dim_t _dim>
	inline constexpr vector<_t, 3u> vector<_t, _dim>::right()
	{
		return vector<_t, 3u>(static_cast<_t>(1), static_cast<_t>(0), static_cast<_t>(0));
	}

	template <typename _t, _vector_dim_t _dim>
	inline vector<_t, 2u> get_look_at(const vector<_t, 2u>& from, const vector<_t, 2u>& to)
	{
		return (to - from).normalized();
	}

	template <typename _t, _vector_dim_t _dim>
	inline vector<_t, 3u> get_look_at(const vector<_t, 3u>& from, const vector<_t, 3u>& to)
	{
		return (to - from).normalized();
	}

	template <typename _t, _vector_dim_t _dim>
	inline vector<_t, 2u> vector<_t, _dim>::get_xy() const
	{
		return vector<_t, 2u>{ this->x, this->y };
	}

	template <typename _t, _vector_dim_t _dim>
	inline vector<_t, 3u> vector<_t, _dim>::get_xyz() const
	{
		return vector<_t, 3u>{ this->x, this->y, this->z };
	}

	template <typename _t, _vector_dim_t _dim>
	inline vector<_t, 2u> vector<_t, _dim>::get_rg() const
	{
		return vector<_t, 2u>{ this->x, this->y };
	}

	template <typename _t, _vector_dim_t _dim>
	inline vector<_t, 3u> vector<_t, _dim>::get_rgb() const
	{
		return vector<_t, 3u>{ this->x, this->y, this->z };
	}

	template <typename _t, _vector_dim_t _dim>
	vector<_t, _dim> vector<_t, _dim>::abs(const vector<_t, _dim>& vec)
	{
		vector<_t, _dim> result = vec;
		for (_vector_dim_t i = 0u; i < _dim; ++i)
		{
			result[i] = std::abs(vec[i]);
		}
		return result;
	}

	template<typename _t, _vector_dim_t _dim>
	inline vector<_t, _dim> vector<_t, _dim>::zero()
	{
		return vector<_t, _dim>();
	}
	template<typename _t, _vector_dim_t _dim>
	inline vector<_t, _dim> vector<_t, _dim>::one()
	{
		vector<_t, _dim> result{};
		for (_vector_dim_t i{}; i < _dim; ++i)
		{
			result[i] = static_cast<_t>(1);
		}
		return result;
	}
	template<typename _t, _vector_dim_t _dim>
	inline vector<_t, _dim> vector<_t, _dim>::get_max()
	{
		vector<_t, _dim> result{};
		for (_vector_dim_t i{}; i < _dim; ++i)
		{
			result[i] = static_cast<_t>(std::numeric_limits<_t>::max());
		}
		return result;
	}
#pragma endregion

	// Comparison:
#pragma region Comparison
	template<typename _t, _vector_dim_t _dim>
	inline bool vector<_t, _dim>::operator==(const vector& other) const
	{
		bool same = true;
		for (_vector_dim_t i{}; i < _dim; ++i) same = same && (at(i) == other.at(i));

		return same;
	}
	template<typename _t, _vector_dim_t _dim>
	inline bool vector<_t, _dim>::operator!=(const vector& other) const
	{
		return !(*this == other);
	}
#pragma endregion

	// Arithmatic:
#pragma region Arithmetic
	template<typename _t, _vector_dim_t _dim>
	inline vector<_t, _dim>& vector<_t, _dim>::operator+=(const vector& other)
	{
		return *this = *this + other;
	}
	template<typename _t, _vector_dim_t _dim>
	inline vector<_t, _dim>& vector<_t, _dim>::operator-=(const vector& other)
	{
		return *this = *this - other;
	}
	template<typename _t, _vector_dim_t _dim>
	inline vector<_t, _dim>& vector<_t, _dim>::operator*=(const vector& other)
	{
		return *this = *this * other;
	}
	template<typename _t, _vector_dim_t _dim>
	inline vector<_t, _dim>& vector<_t, _dim>::operator*=(const float scalar)
	{
		return *this = *this * scalar;
	}
	template<typename _t, _vector_dim_t _dim>
	inline vector<_t, _dim>& vector<_t, _dim>::operator/=(const vector& other)
	{
		return *this = *this / other;
	}
	template<typename _t, _vector_dim_t _dim>
	inline vector<_t, _dim>& vector<_t, _dim>::operator/=(const float scalar)
	{
		assert(scalar != static_cast<_t>(0));

		return *this = *this / scalar;
	}
	template<typename _t, _vector_dim_t _dim>
	vector<_t, _dim> operator+(const vector<_t, _dim>& a, const vector<_t, _dim>& b)
	{
		vector<_t, _dim> res{};
		for (_vector_dim_t i{}; i < _dim; ++i)
			res[i] = a[i] + b[i];

		return res;
	}
	template<typename _t, _vector_dim_t _dim>
	vector<_t, _dim> operator-(const vector<_t, _dim>& a, const vector<_t, _dim>& b)
	{
		vector<_t, _dim> res{};
		for (_vector_dim_t i{}; i < _dim; ++i)
			res[i] = a[i] - b[i];

		return res;
	}
	template<typename _t, _vector_dim_t _dim>
	vector<_t, _dim> operator*(const vector<_t, _dim>& a, const vector<_t, _dim>& b)
	{
		vector<_t, _dim> res{};
		for (_vector_dim_t i{}; i < _dim; ++i)
			res[i] = a[i] * b[i];

		return res;
	}
	template<typename _t, _vector_dim_t _dim>
	vector<_t, _dim> operator/(const vector<_t, _dim>& a, const vector<_t, _dim>& b)
	{
		FLX_ASSERT(b.x != static_cast<_t>(0) && b.y != static_cast<_t>(0) && b.z != static_cast<_t>(0));

		vector<_t, _dim> res{};
		for (_vector_dim_t i{}; i < _dim; ++i)
			res[i] = a[i] / b[i];

		return res;
	}
	template<typename _t, _vector_dim_t _dim>
	vector<_t, _dim> operator*(const vector<_t, _dim>& a, const float b)
	{
		vector<_t, _dim> res{};
		for (_vector_dim_t i{}; i < _dim; ++i)
			res[i] = a[i] * b;

		return res;
	}
	template<typename _t, _vector_dim_t _dim>
	vector<_t, _dim> operator/(const vector<_t, _dim>& a, const float b)
	{
		vector<_t, _dim> res{};
		for (_vector_dim_t i{}; i < _dim; ++i)
			res[i] = a[i] / b;

		return res;
	}
	template<typename _t, _vector_dim_t _dim>
	vector<_t, _dim> operator*(const float a, const vector<_t, _dim>& b)
	{
		vector<_t, _dim> res{};
		for (_vector_dim_t i{}; i < _dim; ++i)
			res[i] = a * b[i];

		return res;
	}
	template<typename _t, _vector_dim_t _dim>
	vector<_t, _dim> operator-(const vector<_t, _dim>& v)
	{
		vector<_t, _dim> res{};
		for (_vector_dim_t i{}; i < _dim; ++i)
			res[i] = -v[i];

		return res;
	}

	template <typename _t, _vector_dim_t _dim>
	vector<_t, _dim> operator/(const float a, const vector<_t, _dim>& b)
	{
		vector<_t, _dim> res{};
		for (_vector_dim_t i{}; i < _dim; ++i)
		{
			influx_assert(b[i] != 0);
			res[i] = a / b[i];
		}

		return res;
	}
#pragma endregion
}