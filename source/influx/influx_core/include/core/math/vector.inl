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

#ifndef TVEC
#define TVEC vector<_t, _s>

namespace influx::math
{
	// Constructors:
	template<typename _t, vecsize _s> // Initializer list...
	template<typename ..._V> 
	inline TVEC::vector(const _V& ...components)
		: detail::base_vector<_t, _s>(static_cast<_t>(components) ...) 
	{

	}

	template<typename _t, vecsize _s> // Typecast...
	template <typename _u> inline TVEC::vector(const vector<_u, _s>& other)
	{
		for (size_t i{}; i < _s; ++i)
			this->m_data[i] = static_cast<_t>(other.m_data[i]);
	}

	template<typename _t, vecsize _s> // Sizecast constructor
	template <vecsize _D> inline TVEC::vector(const vector<_t, _D>& other)
	{
		for (vecsize i{}; i < _s; ++i)
			this->m_data[i] = (i < _D) ? other[i] : static_cast<_t>(0);

		// If smaller, copy all data
		// If bigger, copy data & init 0 leftover...
	}

	template<typename _t, vecsize _s>
	constexpr vecsize TVEC::size()
	{
		return _s;
	}

	// Data
#pragma region data Access
	template<typename _t, vecsize _s>
	inline _t& TVEC::operator[](vecsize i)
	{
		influx_assert(i < _s);
		return this->m_data[i];
	}
	template<typename _t, vecsize _s>
	inline const _t& TVEC::operator[](vecsize i) const
	{
		influx_assert(i < _s);
		return this->m_data[i];
	}
	template<typename _t, vecsize _s>
	inline _t& TVEC::at(vecsize i)
	{
		influx_assert(i < _s);
		return (*this)[i];
	}
	template<typename _t, vecsize _s>
	inline const _t& TVEC::at(vecsize i) const
	{
		influx_assert(i < _s);
		return (*this)[i];
	}
	template<typename _t, vecsize _s>
	const _t* TVEC::data() const
	{
		return this->m_data;
	}
	template<typename _t, vecsize _s>
	_t* TVEC::data()
	{
		return this->m_data;
	}
#pragma endregion

	// Normalize:
#pragma region Normalize
	template<typename _t, vecsize _s>
	inline vector<_t, _s> TVEC::normalized() const { return TVEC::normalized(*this); }
	template<typename _t, vecsize _s>
	inline void TVEC::normalize() { normalize(*this); }
	template<typename _t, vecsize _s>
	inline void TVEC::normalize(vector& vec) { vec = normalized(vec); }
	template<typename _t, vecsize _s>
	inline vector<_t, _s> TVEC::normalized(const vector<_t, _s>& vec)
	{
		return vec / vec.magnitude();
	}
#pragma endregion

	// clamp:
#pragma region clamp
	template<typename _t, vecsize _s>
	inline TVEC& TVEC::clamp_values(_t min, _t max)
	{
		for (uint32 i = 0u; i < size(); ++i)
		{
			this->m_data[i] = clamp(this->m_data[i], min, max);
		}
		return *this;
	}

	template<typename _t, vecsize _s>
	inline TVEC TVEC::get_clamped_values(_t min, _t max) const
	{ TVEC copy = *this; return copy.clamp_values(min, max); }
	template<typename _t, vecsize _s>
	inline void TVEC::clamp_values(TVEC& vec, _t min, _t max)
	{ vec.clamp_values(min, max); }
	template<typename _t, vecsize _s>
	inline TVEC	TVEC::get_clamped_values(const TVEC& vec, _t min, _t max)
	{ return vec.get_clamped_values(min, max); }

	template<typename _t, vecsize _s>
	TVEC& TVEC::clamp_length(_t length)
	{
		const _t sqr_mag = sqr_magnitude();
		const _t sqr_length = length * length;
		if (sqr_mag < sqr_length || sqr_mag > sqr_length)
		{
			scale(length);
		}
		return *this;
	}

	template<typename _t, vecsize _s>
	TVEC TVEC::get_clamped_length(_t length) const
	{ return TVEC(*this).clamp_length(length); }
	template<typename _t, vecsize _s>
	inline void TVEC::clamp_length(TVEC& vec, _t length)
	{ vec.clamp_length(length); }
	template<typename _t, vecsize _s>
	inline TVEC	TVEC::get_clamped_length(const TVEC& vec,_t length)
	{ return vec.get_clamped_length(length); }
#pragma endregion

	// Angle:
#pragma region Angle
	template<typename _t, vecsize _s>
	inline float TVEC::radians_between(const vector& other) const { return angle_between(*this, other); }

	template <typename _t, vecsize _s>
	inline float TVEC::radians_between(const vector<_t, _s>& a, const vector<_t, _s>& b)
	{
		float magA = magnitude(a);
		float magB = magnitude(b);
		if (magA == 0.0f || magB == 0.0f) return 0.0f;

		return std::acosf(Dot(a, b) / (magA * magB));
	}
#pragma endregion

	// Magnitude:
#pragma region Magnitude
	template<typename _t, vecsize _s>
	inline float TVEC::magnitude() const { return magnitude(*this); }
	template<typename _t, vecsize _s>
	inline float TVEC::sqr_magnitude() const { return sqr_magnitude(*this); }
	template<typename _t, vecsize _s>
	inline float TVEC::magnitude(const vector& other) { return sqrtf(sqr_magnitude(other)); }
	template<typename _t, vecsize _s>
	inline float TVEC::sqr_magnitude(const vector& other) 
	{ 
		return dot(other, other); 
	}
	template<typename _t, vecsize _s>
	inline float TVEC::distance(const vector& a, const vector& b)
	{
		return magnitude(a - b);
	}
	template<typename _t, vecsize _s>
	inline float TVEC::sqr_distance(const vector& a, const vector& b)
	{
		return sqr_magnitude(a - b);
	}
#pragma endregion

	// Scaling
#pragma region Scaling
	template<typename _t, vecsize _s>
	inline void TVEC::scale(float mag)
	{
		scale(*this, mag);
	}
	template<typename _t, vecsize _s>
	inline vector<_t, _s> TVEC::scaled(float mag) const
	{
		return scaled(*this, mag);
	}
	template<typename _t, vecsize _s>
	inline void TVEC::scale(vector& vec, float mag)
	{
		vec = vec.normalized() * mag;
	}
	template<typename _t, vecsize _s>
	inline vector<_t, _s> TVEC::scaled(const vector& vec, float mag)
	{
		vector<_t, _s> res = vec;
		scale(res, mag);
		return res;
	}
#pragma endregion
	
	// Dot & Cross
#pragma region Dot & Cross
	template<typename _t, vecsize _s>
	inline _t TVEC::dot(const vector& other) const
	{
		_t result{};
		for (vecsize i{}; i < size(); ++i)
		{
			result += this->m_data[i] * other.m_data[i];
		}
		return result;
	}
	template<typename _t, vecsize _s>
	inline _t TVEC::dot(const vector& a, const vector& b)
	{ return a.dot(b); }

	template<typename _t, vecsize _s>
	inline _t TVEC::cross(const vector<_t, 2>& other) const
	{
		return cross(*this, other);
	}
	template<typename _t, vecsize _s>
	inline vector<_t, 3> TVEC::cross(const vector<_t, 3>& other) const
	{
		return cross(*this, other);
	}
	template<typename _t, vecsize _s> // 2D
	inline float TVEC::cross(const vector<_t, 2>& a, const vector<_t, 2>& b)
	{
		return { a.x * b.y - a.y * b.x };
	}
	template<typename _t, vecsize _s> // 3D
	inline vector<_t, 3> TVEC::cross(const vector<_t, 3>& a, const vector<_t, 3>& b)
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
	template<typename _t, vecsize _s>
	inline const vector<_t, _s>& TVEC::inverted() const
	{
		return inverted(*this);
	}
	template<typename _t, vecsize _s>
	inline void TVEC::inverse()
	{
		inverse(*this);
	}
	template<typename _t, vecsize _s>
	inline void TVEC::inverse(vector& vec)
	{
		vec = inverted(vec);
	}
	template<typename _t, vecsize _s>
	inline vector<_t, _s> TVEC::inverted(const vector& vec)
	{
		vector<_t, _s> res = vec;
		for (vecsize i{}; i < _s; ++i)
		{
			res[i] = -res[i];
		}

		return res;
	}
#pragma endregion

	// Reflection
#pragma region Reflection
	template<typename _t, vecsize _s>
	inline const vector<_t, 2u>& TVEC::reflected(const vector<_t, 2u>& hitNormal) const
	{
		return reflection(*this, hitNormal);
	}
	template<typename _t, vecsize _s>
	inline const vector<_t, 3u>& TVEC::reflected(const vector<_t, 3u>& hitNormal) const
	{
		return reflection(*this, hitNormal);
	}
	template<typename _t, vecsize _s>

	inline vector<_t, 2u> TVEC::reflection(const vector<_t, 2u>& vec, const vector<_t, 2u>& hitNormal)
	{
		return vec - 2.0f * (dot(vec, hitNormal)) * hitNormal;
	}
	template<typename _t, vecsize _s>
	inline vector<_t, 3u> TVEC::reflection(const vector<_t, 3u>& vec, const vector<_t, 3u>& hitNormal)
	{
		return vec - 2.0f * (dot(vec, hitNormal)) * hitNormal;
	}
#pragma endregion

	// Lerp:
#pragma region lerp
	template<typename _t, vecsize _s>
	inline vector<_t, _s> TVEC::lerp(const vector& a, const vector& b, const float t)
	{
		float clamped_t = t;

		// clamp [0-1]
		if (clamped_t < 0.0f) clamped_t = 0.0f;
		if (clamped_t > 1.0f) clamped_t = 1.0f;

		return (b * clamped_t) + (a * (1.0f - clamped_t));
	}

	template<typename _t, vecsize _s>
	void TVEC::lerp_towards(const vector& b, const float t)
	{
		*this = lerp(*this, b, t);
	}
#pragma endregion

	// Zero:
#pragma region Null
	template<typename _t, vecsize _s>
	inline bool TVEC::is_zero() const
	{
		return is_zero(*this);
	}
	template<typename _t, vecsize _s>
	inline bool TVEC::is_zero(const vector& v)
	{
		for (vecsize i{}; i < _s; ++i)
			if (v[i] != static_cast<_t>(0)) return false;

		return true;
	}
	template<typename _t, vecsize _s>
	inline constexpr vector<_t, 3u> TVEC::up()
	{
		return vector<_t, 3u>(static_cast<_t>(0), static_cast<_t>(1), static_cast<_t>(0));
	}
	template<typename _t, vecsize _s>
	inline constexpr vector<_t, 3u> TVEC::forward()
	{
		return vector<_t, 3u>(static_cast<_t>(0), static_cast<_t>(0), static_cast<_t>(1.0f));
	}
	template<typename _t, vecsize _s>
	inline constexpr vector<_t, 3u> TVEC::right()
	{
		return vector<_t, 3u>(static_cast<_t>(1), static_cast<_t>(0), static_cast<_t>(0));
	}

	template <typename _t, vecsize _s>
	inline vector<_t, 2u> get_look_at(const vector<_t, 2u>& from, const vector<_t, 2u>& to)
	{
		return (to - from).normalized();
	}

	template <typename _t, vecsize _s>
	inline vector<_t, 3u> get_look_at(const vector<_t, 3u>& from, const vector<_t, 3u>& to)
	{
		return (to - from).normalized();
	}

	template <typename _t, vecsize _s>
	inline vector<_t, 2u> TVEC::get_xy() const
	{
		return vector<_t, 2u>{ this->x, this->y };
	}

	template <typename _t, vecsize _s>
	inline vector<_t, 3u> TVEC::get_xyz() const
	{
		return vector<_t, 3u>{ this->x, this->y, this->z };
	}

	template <typename _t, vecsize _s>
	inline vector<_t, 2u> TVEC::get_rg() const
	{
		return vector<_t, 2u>{ this->x, this->y };
	}

	template <typename _t, vecsize _s>
	inline vector<_t, 3u> TVEC::get_rgb() const
	{
		return vector<_t, 3u>{ this->x, this->y, this->z };
	}

	template <typename _t, vecsize _s>
	vector<_t, _s> TVEC::abs(const vector<_t, _s>& vec)
	{
		vector<_t, _s> result = vec;
		for (vecsize i = 0u; i < _s; ++i)
		{
			result[i] = std::abs(vec[i]);
		}
		return result;
	}

	template <typename _t, vecsize _s>
	static _t TVEC::get_summed(const vector& vec)
	{
		_t result{};
		for (vecsize i = 0u; i < _s; ++i)
		{
			result += vec[i];
		}
		return result;
	}

	template<typename _t, vecsize _s>
	inline vector<_t, _s> TVEC::zero()
	{
		return vector<_t, _s>();
	}
	template<typename _t, vecsize _s>
	inline vector<_t, _s> TVEC::one()
	{
		vector<_t, _s> result{};
		for (vecsize i{}; i < _s; ++i)
		{
			result[i] = static_cast<_t>(1);
		}
		return result;
	}
	template<typename _t, vecsize _s>
	inline vector<_t, _s> TVEC::get_max()
	{
		vector<_t, _s> result{};
		for (vecsize i{}; i < _s; ++i)
		{
			result[i] = static_cast<_t>(std::numeric_limits<_t>::max());
		}
		return result;
	}
	template<typename _t, vecsize _s>
	inline vector<_t, _s> TVEC::fill(const _t& value)
	{
		vector<_t, _s> result{};
		for (vecsize i{}; i < _s; ++i)
		{
			result[i] = value;
		}
		return result;
	}
#pragma endregion

	// Comparison:
#pragma region Comparison
	template<typename _t, vecsize _s>
	inline bool TVEC::operator==(const vector& other) const
	{
		bool same = true;
		for (vecsize i{}; i < _s; ++i) same = same && (at(i) == other.at(i));

		return same;
	}
	template<typename _t, vecsize _s>
	inline bool TVEC::operator!=(const vector& other) const
	{
		return !(*this == other);
	}
#pragma endregion

	// Arithmatic:
#pragma region Arithmetic
	template<typename _t, vecsize _s>
	inline vector<_t, _s>& TVEC::operator+=(const vector& other)
	{
		return *this = *this + other;
	}
	template<typename _t, vecsize _s>
	inline vector<_t, _s>& TVEC::operator-=(const vector& other)
	{
		return *this = *this - other;
	}
	template<typename _t, vecsize _s>
	inline vector<_t, _s>& TVEC::operator*=(const vector& other)
	{
		return *this = *this * other;
	}
	template<typename _t, vecsize _s>
	inline vector<_t, _s>& TVEC::operator*=(const float scalar)
	{
		return *this = *this * scalar;
	}
	template<typename _t, vecsize _s>
	inline vector<_t, _s>& TVEC::operator/=(const vector& other)
	{
		return *this = *this / other;
	}
	template<typename _t, vecsize _s>
	inline vector<_t, _s>& TVEC::operator/=(const float scalar)
	{
		assert(scalar != static_cast<_t>(0));

		return *this = *this / scalar;
	}
	template<typename _t, vecsize _s>
	vector<_t, _s> operator+(const vector<_t, _s>& a, const vector<_t, _s>& b)
	{
		vector<_t, _s> res{};
		for (vecsize i{}; i < _s; ++i)
			res[i] = a[i] + b[i];

		return res;
	}
	template<typename _t, vecsize _s>
	vector<_t, _s> operator-(const vector<_t, _s>& a, const vector<_t, _s>& b)
	{
		vector<_t, _s> res{};
		for (vecsize i{}; i < _s; ++i)
			res[i] = a[i] - b[i];

		return res;
	}
	template<typename _t, vecsize _s>
	vector<_t, _s> operator*(const vector<_t, _s>& a, const vector<_t, _s>& b)
	{
		vector<_t, _s> res{};
		for (vecsize i{}; i < _s; ++i)
			res[i] = a[i] * b[i];

		return res;
	}
	template<typename _t, vecsize _s>
	vector<_t, _s> operator/(const vector<_t, _s>& a, const vector<_t, _s>& b)
	{
		for (vecsize i =0u; i < _s; ++i)
			influx_assert(b[i] != static_cast<_t>(0));

		vector<_t, _s> res{};
		for (vecsize i{}; i < _s; ++i)
			res[i] = a[i] / b[i];

		return res;
	}
	template<typename _t, vecsize _s>
	vector<_t, _s> operator*(const vector<_t, _s>& a, const float b)
	{
		vector<_t, _s> res{};
		for (vecsize i{}; i < _s; ++i)
			res[i] = a[i] * b;

		return res;
	}
	template<typename _t, vecsize _s>
	vector<_t, _s> operator/(const vector<_t, _s>& a, const float b)
	{
		vector<_t, _s> res{};
		for (vecsize i{}; i < _s; ++i)
			res[i] = static_cast<_t>(a[i] / b);

		return res;
	}
	template<typename _t, vecsize _s>
	vector<_t, _s> operator*(const float a, const vector<_t, _s>& b)
	{
		vector<_t, _s> res{};
		for (vecsize i{}; i < _s; ++i)
			res[i] = a * b[i];

		return res;
	}
	template<typename _t, vecsize _s>
	vector<_t, _s> operator-(const vector<_t, _s>& v)
	{
		vector<_t, _s> res{};
		for (vecsize i{}; i < _s; ++i)
			res[i] = -v[i];

		return res;
	}

	template <typename _t, vecsize _s>
	vector<_t, _s> operator/(const float a, const vector<_t, _s>& b)
	{
		vector<_t, _s> res{};
		for (vecsize i{}; i < _s; ++i)
		{
			influx_assert(b[i] != 0);
			res[i] = a / b[i];
		}

		return res;
	}
#pragma endregion
}

#undef TVEC
#endif