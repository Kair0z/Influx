#pragma once

#ifndef __CORE_MATH_QUATERNION_H_
#define __CORE_MATH_QUATERNION_H_

#include <complex>

#include "core/math/math.h"
#include "core/math/vector.h"
#include "core/math/matrix.h"

template<typename T1, typename T2>
inline bool is_scalar_zero(T1 x, T2 eps = 0) {
	typedef typename std::common_type<T1, T2>::type T;
	T xx = static_cast<T>(x);
	T ee = static_cast<T>(eps);
	return std::abs(xx) <= ee;
}

namespace influx::math
{
	template <typename _t>
	using complex = std::complex<_t>;

	template <typename _t>
	class quaternion final
	{
		_t m_a{};
		_t m_b{};
		_t m_c{};
		_t m_d{};

		using value_type = _t;
		using vec3 = math::vector<_t, 3u>;

		// these are the supported types
		static_assert(std::is_same<_t, bool>()
			|| std::is_same<_t, int>()
			|| std::is_same<_t, long>()
			|| std::is_same<_t, long long>()
			|| std::is_same<_t, float>()
			|| std::is_same<_t, double>()
			|| std::is_same<_t, long double>(),
			"Invalid scalar type for Quaternion");

	public:
		quaternion(_t a = 0, _t b = 0, _t c = 0, _t d = 0)
		: m_a{a}, m_b{b}, m_c{c}, m_d{d}{}
		// todo more constructors...

		template <typename _t2>
		quaternion& operator=(const quaternion<_t2>& other)
		{ m_a = other.m_a; m_b = other.m_b; m_c = other.m_c; m_d = other.m_d; return *this; }

		const _t& get_a() const { return m_a; }
		const _t& get_b() const { return m_b; }
		const _t& get_c() const { return m_c; }
		const _t& get_d() const { return m_d; }

		const _t& get_real() const
		{ return m_a; }

		quaternion get_unreal() const
		{ return {0, m_b, m_c, m_d}; }

		/**
		* The square of the norm of the Quaternion.
		* (The square is sometimes useful, and it avoids paying for a sqrt).
		*/
		_t get_norm_sqr() const
		{ return m_a * m_a + m_b * m_b + m_c * m_c + m_d * m_d; }

		_t get_unreal_norm_sqr() const
		{ return m_b * m_b + m_c * m_c + m_d * m_d; }

		_t get_abs() const
		{ return sqrt(get_norm_sqr()); }

		void normalize_real()
		{ m_a = sqrt((_t)1 - get_unreal_norm_sqr()); }

		template <typename _t2 = _t>
		bool is_zero(const _t2& eps = 0) const
		{ 
			return
				is_scalar_zero(m_a, eps) &&
				is_scalar_zero(m_b, eps) &&
				is_scalar_zero(m_c, eps) &&
				is_scalar_zero(m_d, eps);
		}

		template <typename _t2 = _t>
		bool is_unit(const _t2& eps = 0) const
		{ return is_scalar_zero(get_norm_sqr() - _t(1), eps); }

		template <typename _t2 = _t>
		bool is_real(const _t2& eps = 0) const
		{ 
			return
				is_scalar_zero(m_b) &&
				is_scalar_zero(m_c) &&
				is_scalar_zero(m_d);
		}

		template <typename _t2 = _t>
		bool is_complex(const _t2& eps = 0) const
		{ return is_scalar_zero(m_c, eps) && is_scalar_zero(m_d, eps); }

		template <typename _t2 = _t>
		bool is_unreal(const _t2& eps = 0) const
		{
			return is_scalar_zero(m_a, eps) && 
				!(is_scalar_zero(m_b, eps) && is_scalar_zero(m_c, eps) && is_scalar_zero(m_d, eps));
		}

		// unary operators
		quaternion operator+() const
		{ return *this; }
		quaternion operator-() const
		{ return {-m_a, -m_b, -m_c, -m_d}; }

		// quaternion x scalar
		quaternion operator+=(const _t& y) 
		{ m_a += y; return *this; }
		quaternion operator-=(const _t& y)
		{ m_a -= y; return *this; }
		quaternion operator*=(const _t& k)
		{ m_a *= k; m_b *= k; m_c *= k; m_d *= k; return *this; }
		quaternion operator/=(const _t& k)
		{
			const _t inv_k = 1 / k; return (*this)*= inv_k;
		}

		// quaternion x complex
		template <typename _t2>
		quaternion operator+=(const complex<_t2>& y)
		{ m_a += y.real(); m_b += y.imag(); return *this; }
		template <typename _t2>
		quaternion operator-=(const complex<_t2>& y)
		{ m_a -= y.real(); m_b -= y.imag(); return *this; }
		template <typename _t2>
		quaternion operator*=(const complex<_t2>& y)
		{
			m_a = m_a * y.real() - m_b * y.imag();
			m_b = m_a * y.imag() + m_b * y.real();
			m_c = m_c * y.real() + m_d * y.imag();
			m_d = -m_c * y.imag()	+ m_d * y.real();
			return *this;
		}
		template <typename _t2>
		quaternion operator/=(const complex<_t2>& y)
		{
			_t n2 = y.real() * y.real() + y.imag() * y.imag();
			_t at = m_a * y.real() + m_b * y.imag();
			_t bt = -m_a * y.imag() + m_b * y.real();
			_t ct = m_c * y.real() - m_d * y.imag();
			_t dt = m_c * y.imag() + m_d * y.real();

			m_a = at / n2;
			m_b = bt / n2;
			m_c = ct / n2;
			m_d = dt / n2;
			return *this;
		}

		// quaternion x quaternion
		template <typename _t2>
		quaternion operator+=(const quaternion<_t2>& y)
		{
			m_a += y.get_a();
			m_b += y.get_b();
			m_c += y.get_c();
			m_d += y.get_d();
			return *this;
		}
		template <typename _t2>
		quaternion operator-=(const quaternion<_t2>& y)
		{ return *this += -y; }
		template<typename _t2>
		quaternion operator*=(const quaternion<_t2>& y) 
		{

			m_a = m_a * y.a() - m_b * y.b() - m_c * y.c() - m_d * y.d();
			m_b = m_a * y.b() + m_b * y.a() + m_c * y.d() - m_d * y.c();
			m_c = m_a * y.c() - m_b * y.d() + m_c * y.a() + m_d * y.b();
			m_d = m_a * y.d() + m_b * y.c() - m_c * y.b() + m_d * y.a();
			return *this;
		}
		template <typename _t2>
		quaternion operator/=(const quaternion<_t2>& y)
		{
			/**
			* Unary division with other Quaternion.
			*
			* Warning: if the norm of y is zero, the result is
			* 4 NaNs, but maybe it should be inf.
			*/

			_t n2 = y.get_norm_sqr();
			_t at = m_a * y.a() + m_b * y.b() + m_c * y.c() + m_d * y.d();
			_t bt = -m_a * y.b() + m_b * y.a() - m_c * y.d() + m_d * y.c();
			_t ct = -m_a * y.c() + m_b * y.d() + m_c * y.a() - m_d * y.b();
			_t dt = -m_a * y.d() - m_b * y.c() + m_c * y.b() + m_d * y.a();

			m_a = at / n2;
			m_b = bt / n2;
			m_c = ct / n2;
			m_d = dt / n2;
			return *this;
		}

		static quaternion make_conjugate(const quaternion& quat)
		{
			return { quat.m_d, -quat.m_a, -quat.m_b, -quat.m_c };
		}

		static quaternion make_angleaxis(const _t& delta_degrees, const math::vector<_t, 3>& axis)
		{
			const auto norm_axis = axis.normalized();
			const float half_angle = to_radians(delta_degrees) * 0.5f;
			const _t sin_angle = math::sin(half_angle);
			return {
				math::cos(half_angle),
				norm_axis.x * sin_angle,
				norm_axis.y * sin_angle,
				norm_axis.z * sin_angle
			};
		}
		
		static quaternion vec3_to_quat(const math::vector<_t, 3u>& vec)
		{
			return { 0.0f, vec.x, vec.y, vec.z };
		}

		static vec3 rotate(const vec3& vec, const quaternion& quat)
		{
			quaternion rotated = {}; // quat* vec3_to_quat(vec)* make_conjugate(quat);
			return { rotated.m_a, rotated.m_b, rotated.m_c };
		}

		template <typename _t2>
		static matrix<_t2, 3u, 3u> quat_to_matrix(const quaternion<_t2>& quat)
		{
			// 21 operations?
			_t2 a2 = quat.get_a() * quat.get_a(), b2 = quat.get_b() * quat.get_b(), c2 = quat.get_c() * quat.get_c(), d2 = quat.get_d() * quat.get_d();
			_t2 ab = quat.get_a() * quat.get_b(), ac = quat.get_a() * quat.get_c(), ad = quat.get_a() * quat.get_d();
			_t2 bc = quat.get_b() * quat.get_c(), bd = quat.get_b() * quat.get_d();
			_t2 cd = quat.get_c() * quat.get_d();

			matrix<_t2, 3u, 3u> mat{};
			mat.set_row(0, float3{a2 + b2 - c2 - d2	, 2 * (bc - ad)		, 2 * (bd + ac)} );
			mat.set_row(1, float3{2 * (bc + ad)		, a2 - b2 + c2 - d2	, 2 * (cd - ab)} );
			mat.set_row(2, float3{2 * (bd - ac)		, 2 * (cd + ab)		, a2 - b2 - c2 + d2} );
			return mat;
		}

		static quaternion identity() 
		{ static quaternion g_identity{ 1,0,0,0 }; return g_identity; }

		template <typename _t2>
		static quaternion<_t2> matrix_to_quat(const matrix<_t2, 3u, 3u>& mat)
		{
			const _t2 tee = mat[0][0] + mat[1][1] + mat[2][2];

			if (tee > 0)
			{
				_t2 s = (_t2)0.5 / std::sqrt(tee + 1);
				return quaternion<_t2>{ (_t2)0.25 / s,
						(mat[2][1] - mat[1][2]) * s,
						(mat[0][2] - mat[2][0]) * s,
						(mat[1][0] - mat[0][1]) * s };
			}
			else 
			{
				if (mat[0][0] > mat[1][1] && mat[0][0] > mat[2][2]) 
				{
					_t2 s = (_t)2.0 * std::sqrt((_t2)1.0 + mat[0][0] - mat[1][1] - mat[2][2]);
					return { (mat[2][1] - mat[1][2]) / s,
							(_t2)0.25 * s,
							(mat[0][1] + mat[1][0]) / s,
							(mat[0][2] + mat[2][0]) / s };
				}
				else if (mat[1][1] > mat[2][2]) 
				{
					_t2 s = (_t2)2.0 * std::sqrt((_t2)1.0 + mat[1][1] - mat[0][0] - mat[2][2]);
					return quaternion<_t2>{ 
						(mat[0][2] - mat[2][0]) / s,
						(mat[0][1] + mat[1][0]) / s,
						(_t)0.25 * s,
						(mat[1][2] + mat[2][1]) / s };
				}
				else 
				{
					_t2 s = (_t2)2.0 * std::sqrt((_t2)1.0 + mat[2][2] - mat[0][0] - mat[1][1]);
					return quaternion<_t2>{ 
						(mat[1][0] - mat[0][1]) / s,
						(mat[0][2] + mat[2][0]) / s,
						(mat[1][2] + mat[2][1]) / s,
						(_t2)0.25 * s };
				}
			}
		}
	};

	using quatf = quaternion<float>;
	using quatd = quaternion<double>;

	class rotation final
	{
		quatf m_quaternion;

	public:
		rotation() = default;
		rotation(const quatf& quat) : m_quaternion{ quat } {}

		static rotation identity()
		{
			static rotation rot{ quatf::identity() };
			return rot;
		}

		void set_matrix(const math::matrix3x3f& mat)
		{
			m_quaternion = quatf::matrix_to_quat(mat);
		}

		const math::matrix3x3f get_matrix() const
		{
			return quatf::quat_to_matrix(m_quaternion);
		}

		bool is_gimbal_locked() const
		{
			return false;
		}

		math::float3 rotate(const math::float3& vector, float delta_degrees, const vectorf3& axis)
		{
			return quatf::rotate( vector, quatf::make_angleaxis(delta_degrees, axis) );
		}

		vectorf3 get_euler_angles() const
		{
			influx_assert(false);
			return {};
		}

		float get_pitch() const
		{
			return 0.0f;
		}

		float get_yaw() const
		{
			return 0.0f;
		}

		float get_roll() const
		{
			return 0.0f;
		}

		vectorf3 get_forward() const
		{
			return vectorf3::make_one();
		}

		vectorf3 get_right() const
		{
			return vectorf3::make_one();
		}

		vectorf3 get_up() const
		{
			return vectorf3::make_one();
		}

		void set_forward(const vectorf3& newForward)
		{
		}

		void set_right(const vectorf3& newRight)
		{
		}

		void set_up(const vectorf3& newUp)
		{
		}
	};


}

#endif