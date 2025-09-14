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
		_t m_real{};
		_t m_x{};
		_t m_y{};
		_t m_z{};

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
		quaternion(_t r = 0, _t x = 0, _t y = 0, _t z = 0)
		: m_real{r}, m_x{x}, m_y{y}, m_z{z}{}
		// todo more constructors...

		template <typename _t2>
		quaternion& operator=(const quaternion<_t2>& other)
		{ m_real = other.m_real; m_x = other.m_x; m_y = other.m_y; m_z = other.m_z; return *this; }

		_t& get_real() { return m_real; }	const _t& get_real() const { return m_real; }
		_t& get_x() { return m_x; }		const _t& get_x() const { return m_x; }
		_t& get_y() { return m_y; }		const _t& get_y() const { return m_y; }
		_t& get_z() { return m_z; }		const _t& get_z() const { return m_z; }

		quaternion get_imaginary() const
		{ return {0, m_x, m_y, m_z}; }

		/**
		* The square of the norm of the Quaternion.
		* (The square is sometimes useful, and it avoids paying for a sqrt).
		*/
		_t get_norm_sqr() const
		{ return m_real * m_real + m_x * m_x + m_y * m_y + m_z * m_z; }

		inline void normalize()
		{
			_t norm = std::sqrt(m_real * m_real + m_x * m_x + m_y * m_y + m_z * m_z);
			m_real /= norm;
			m_x /= norm;
			m_y /= norm;
			m_z /= norm;
		}

		quaternion get_normalized() const
		{
			quaternion cpy = *this; cpy.normalize(); return cpy;
		}

		_t get_imaginary_norm_sqr() const
		{ return m_x * m_x + m_y * m_y + m_z * m_z; }

		_t get_abs() const
		{ return sqrt(get_norm_sqr()); }

		void normalize_real()
		{ m_real = sqrt((_t)1 - get_imaginary_norm_sqr()); }

		// 0 + 0i + 0j + 0k
		template <typename _t2 = _t>
		bool is_zero(const _t2& eps = 0) const
		{ 
			return
				is_scalar_zero(m_real, eps) &&
				is_scalar_zero(m_x, eps) &&
				is_scalar_zero(m_y, eps) &&
				is_scalar_zero(m_z, eps);
		}

		template <typename _t2 = _t>
		bool is_unit(const _t2& eps = 0) const
		{ return is_scalar_zero(get_norm_sqr() - _t(1), eps); }

		// a + 0i + 0j + 0k
		template <typename _t2 = _t>
		bool is_real(const _t2& eps = 0) const
		{ 
			return
				is_scalar_zero(m_x) &&
				is_scalar_zero(m_y) &&
				is_scalar_zero(m_z);
		}

		// a + bi + 0j + 0k
		template <typename _t2 = _t>
		bool is_complex(const _t2& eps = 0) const
		{ return is_scalar_zero(m_y, eps) && is_scalar_zero(m_z, eps); }

		// m_real == 0, xyz != 0
		template <typename _t2 = _t>
		bool is_imaginary(const _t2& eps = 0) const
		{
			return is_scalar_zero(m_real, eps) && 
				!(is_scalar_zero(m_x, eps) && is_scalar_zero(m_y, eps) && is_scalar_zero(m_z, eps));
		}

		// +q = q
		quaternion operator+() const
		{ return *this; }

		// -q = -a + (-bi) + (-cj) + (-dk)
		quaternion operator-() const
		{ return {-m_real, -m_x, -m_y, -m_z}; }

		// quaternion x real (scalar)
		// a + bi + cj + dk + y = (a+y) + bi + cj + dk
		quaternion operator+=(const _t& y)
		{ m_real += y; return *this; }
		quaternion operator-=(const _t& y)
		{ m_real -= y; return *this; }
		quaternion operator*=(const _t& k)
		{ m_real *= k; m_x *= k; m_y *= k; m_z *= k; return *this; }
		quaternion operator/=(const _t& k)
		{
			const _t inv_k = 1 / k; return (*this)*= inv_k;
		}

		// quaternion x complex
		// (a + bi + cj + dk) + (x + yi) = (a+x) + (b+y)i + cj + dk
		template <typename _t2>
		quaternion operator+=(const complex<_t2>& y)
		{ m_real += y.real(); m_x += y.imag(); return *this; }
		template <typename _t2>
		quaternion operator-=(const complex<_t2>& y)
		{ m_real -= y.real(); m_x -= y.imag(); return *this; }

		template <typename _t2>
		quaternion operator*=(const complex<_t2>& y)
		{
			*this = *this * y;
			return *this;
		}
		template <typename _t2>
		quaternion operator/=(const complex<_t2>& y)
		{
			*this = *this / y;
			return *this;
		}

		// quaternion x quaternion
		template <typename _t2>
		quaternion operator+=(const quaternion<_t2>& y)
		{
			*this = *this + y;
			return *this;
		}
		template <typename _t2>
		quaternion operator-=(const quaternion<_t2>& y)
		{ return *this += -y; }
		template<typename _t2>
		quaternion operator*=(const quaternion<_t2>& y) 
		{
			*this = *this * y;
			return *this;
		}
		template <typename _t2>
		quaternion operator/=(const quaternion<_t2>& y)
		{
			*this = (*this) / y;
			return *this;
		}

		static quaternion make_conjugate(const quaternion& quat)
		{
			return { quat.m_real, -quat.m_x, -quat.m_y, -quat.m_z };
		}
		
		static quaternion make_angleaxis(const _t& delta_degrees, const math::vector<_t, 3>& axis)
		{
			const auto norm_axis = axis.normalized();
			const float half_angle = to_radians(delta_degrees) * 0.5f;
			const _t sin = math::sin(half_angle);
			return {
				math::cos(half_angle),
				norm_axis.x * sin,
				norm_axis.y * sin,
				norm_axis.z * sin
			};
		}

		static vec3 quat_to_vec3(const quaternion& quat)
		{
			return { quat.m_x, quat.m_y, quat.m_z };
		}
		
		static quaternion vec3_to_quat(const math::vector<_t, 3u>& vec)
		{
			return { 0.0f, vec.x, vec.y, vec.z };
		}

		// 3D operations
		static vec3 rotate(const vec3& vec, const quaternion& quat)
		{
			const auto& vec_as_quat = vec3_to_quat(vec);
			const auto conj = make_conjugate(quat);
			const quaternion rotated = quat * vec_as_quat * conj;
			return quat_to_vec3(rotated);
		}

		template <typename _t2>
		static matrix<_t2, 4u, 4u> quat_to_matrix(const quaternion<_t2>& quat)
		{
			// 21 operations?
			_t2 a2 = quat.get_real() * quat.get_real(), b2 = quat.get_x() * quat.get_x(), c2 = quat.get_y() * quat.get_y(), d2 = quat.get_z() * quat.get_z();
			_t2 ab = quat.get_real() * quat.get_x(), ac = quat.get_real() * quat.get_y(), ad = quat.get_real() * quat.get_z();
			_t2 bc = quat.get_x() * quat.get_y(), bd = quat.get_x() * quat.get_z();
			_t2 cd = quat.get_y() * quat.get_z();

			matrix<_t2, 4u, 4u> mat{};
			mat.set_row(0, float3{a2 + b2 - c2 - d2	, 2 * (bc - ad)		, 2 * (bd + ac)} );
			mat.set_row(1, float3{2 * (bc + ad)		, a2 - b2 + c2 - d2	, 2 * (cd - ab)} );
			mat.set_row(2, float3{2 * (bd - ac)		, 2 * (cd + ab)		, a2 - b2 - c2 + d2} );
			return mat;
		}

		static quaternion identity()
		{ static quaternion g_identity{ 1,0,0,0 }; return g_identity; }

		template <typename _t2>
		static quaternion<_t2> matrix_to_quat(const matrix<_t2, 4u, 4u>& mat)
		{
			_t fourXSquaredMinus1 = mat[0][0] - mat[1][1] - mat[2][2];
			_t fourYSquaredMinus1 = mat[1][1] - mat[0][0] - mat[2][2];
			_t fourZSquaredMinus1 = mat[2][2] - mat[0][0] - mat[1][1];
			_t fourWSquaredMinus1 = mat[0][0] + mat[1][1] + mat[2][2];

			int biggestIndex = 0;
			_t fourBiggestSquaredMinus1 = fourWSquaredMinus1;
			if (fourXSquaredMinus1 > fourBiggestSquaredMinus1)
			{
				fourBiggestSquaredMinus1 = fourXSquaredMinus1;
				biggestIndex = 1;
			}
			if (fourYSquaredMinus1 > fourBiggestSquaredMinus1)
			{
				fourBiggestSquaredMinus1 = fourYSquaredMinus1;
				biggestIndex = 2;
			}
			if (fourZSquaredMinus1 > fourBiggestSquaredMinus1)
			{
				fourBiggestSquaredMinus1 = fourZSquaredMinus1;
				biggestIndex = 3;
			}

			_t biggestVal = std::sqrt(fourBiggestSquaredMinus1 + static_cast<_t>(1)) * static_cast<_t>(0.5);
			_t mult = static_cast<_t>(0.25) / biggestVal;

			switch (biggestIndex)
			{
			case 0: return quaternion<_t2>(biggestVal, (mat[1][2] - mat[2][1]) * mult, (mat[2][0] - mat[0][2]) * mult, (mat[0][1] - mat[1][0]) * mult).get_normalized();
			case 1: return quaternion<_t2>((mat[1][2] - mat[2][1]) * mult, biggestVal, (mat[0][1] + mat[1][0]) * mult, (mat[2][0] + mat[0][2]) * mult).get_normalized();
			case 2: return quaternion<_t2>((mat[2][0] - mat[0][2]) * mult, (mat[0][1] + mat[1][0]) * mult, biggestVal, (mat[1][2] + mat[2][1]) * mult).get_normalized();
			case 3: return quaternion<_t2>((mat[0][1] - mat[1][0]) * mult, (mat[2][0] + mat[0][2]) * mult, (mat[1][2] + mat[2][1]) * mult, biggestVal).get_normalized();
			}
			return {};
		}
	};
	
	// (a + bi + cj + dk) + (x + yi + zj + wk)
	template <typename _t> 
	static quaternion<_t> operator*(const quaternion<_t>& a, const quaternion<_t>& b)
	{
		const _t& areal = a.get_real(); const _t& ax = a.get_x(); const _t& ay = a.get_y(); const _t& az = a.get_z();
		const _t& breal = b.get_real(); const _t& bx = b.get_x(); const _t& by = b.get_y(); const _t& bz = b.get_z();
		return
		{
			areal * breal - ax * bx		- ay * by		- az * bz,
			areal * bx	+ ax * breal	+ ay * bz		- az * by,
			areal * by	- ax * bz		+ ay * breal	+ az * bx,
			areal * bz	+ ax * by		- ay * bx		+ az * breal
		};
	}
	template <typename _t>
	static quaternion<_t> operator/(const quaternion<_t>& a, const quaternion<_t>& b)
	{
		const _t& areal = a.get_real(); const _t& ax = a.get_x(); const _t& ay = a.get_y(); const _t& az = a.get_z();
		const _t& breal = b.get_real(); const _t& bx = b.get_x(); const _t& by = b.get_y(); const _t& bz = b.get_z();

		/**
		* Unary division with other Quaternion.
		*
		* Warning: if the norm of y is zero, the result is
		* 4 NaNs, but maybe it should be inf.
		*/

		_t n2 = b.get_norm_sqr();
		_t at = areal	* breal + ax * bx		+ ay * by		+ az * bz;
		_t bt = -areal	* bx	+ ax * breal	- ay * bz		+ az * by;
		_t ct = -areal	* by	+ ax * bz		+ ay * breal	- az * bx;
		_t dt = -areal	* bz	- ax * by		+ ay * bx		+ az * breal;
		return
		{
			at / n2,
			bt / n2,
			ct / n2,
			dt / n2
		};
	}
	template <typename _t>
	static quaternion<_t> operator+(const quaternion<_t>& a, const quaternion<_t>& b)
	{
		const _t& areal = a.get_real(); const _t& ax = a.get_x(); const _t& ay = a.get_y(); const _t& az = a.get_z();
		const _t& breal = b.get_real(); const _t& bx = b.get_x(); const _t& by = b.get_y(); const _t& bz = b.get_z();
		
		return  {
			 areal + breal,
			 ax + bx,
			 by + by,
			 az + bz
		};
	}

	template <typename _t>
	static quaternion<_t> operator*(const quaternion<_t>& a, const complex<_t>& b)
	{
		const _t& areal = a.get_real(); const _t& ax = a.get_x(); const _t& ay = a.get_y(); const _t& az = a.get_z();

		return {
			areal * b.real() - ax * b.imag(),
			areal * b.imag() + ax * b.real(),
			ay * b.real() + az * b.imag(),
			-ay * b.imag() + az * b.real()
		};
	}
	template <typename _t>
	static quaternion<_t> operator/(const quaternion<_t>& a, const complex<_t>& b)
	{
		const _t& areal = a.get_real(); const _t& ax = a.get_x(); const _t& ay = a.get_y(); const _t& az = a.get_z();

		_t n2 = b.real() * b.real() + b.imag() * b.imag();
		_t at = areal	* b.real() + ax * b.imag();
		_t bt = -areal	* b.imag() + ax * b.real();
		_t ct = ay		* b.real() - az * b.imag();
		_t dt = ay		* b.imag() + az * b.real();

		return {
			at / n2,
			bt / n2,
			ct / n2,
			dt / n2
		};
	}

	template <typename _t>
	static vector<_t, 3u> operator*(const quaternion<_t>& quat, const vector<_t, 3u>& vec)
	{
		using quatt = quaternion<_t>;
		return quatt::quat_to_vec3(quat * quatt::vec3_to_quat(vec));
	}


	using quatf = quaternion<float>;
	using quatd = quaternion<double>;

	/* 
	*  quaternion-based rotation representation
	*/
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

		/* converts the rotation encoded in a 3x3 rotation matrix to our m_quaternion */
		void set_rotation_matrix(const math::matrix4x4f& mat)
		{
			m_quaternion = quatf::matrix_to_quat(mat);
		}

		/* converts the rotation encoded in 3 euler angles (degrees) to our m_quaternion */
		void set_euler_angles(float x, float y, float z)
		{
			m_quaternion;
		}

		void set_euler_angles(const float3& eulers)
		{
			return set_euler_angles(eulers.x, eulers.y, eulers.z);
		}

		/* converts the rotation encoded in our quaternion into a 3x3 rotation matrix */
		const math::matrix4x4f get_rotation_matrix() const
		{
			return quatf::quat_to_matrix(m_quaternion);
		}

		/* converts the rotation encoded in our quaternion into a vec3 euler angle representation (degrees) */
		vectorf3 get_euler_angles() const
		{
			return {};
		}

		/* returns a vector rotated [delta_degrees] around [axis] by our m_quaternion */
		math::float3 rotate_vector(const math::float3& vector, float delta_degrees, const vectorf3& axis) const
		{
			return quatf::rotate( vector, quatf::make_angleaxis(delta_degrees, axis) );
		}
		
		void add_euler_angles(float delta_x, float delta_y, float delta_z)
		{
			const float3 prev_euler = get_euler_angles();
			set_euler_angles(prev_euler + float3(delta_x, delta_y, delta_z));
		}

		float get_pitch_degrees() const
		{
			const float3 eulers = get_euler_angles();
			return eulers.x;
		}

		float get_yaw_degrees() const
		{
			const float3 eulers = get_euler_angles();
			return eulers.y;
		}

		float get_roll_degrees() const
		{
			const float3 eulers = get_euler_angles();
			return eulers.z;
		}

		vectorf3 get_forward() const
		{
			return quatf::rotate(float3::make_forward(), m_quaternion);
		}

		vectorf3 get_right() const
		{
			return quatf::rotate(float3::make_right(), m_quaternion);
		}

		vectorf3 get_up() const
		{
			return quatf::rotate(float3::make_up(), m_quaternion);
		}
		
		void set_up(const float3& up) {}

		void set_forward(const float3& forward)
		{
			const float3 up = get_up();
			const auto mat = math::matrix4x4f::make_rotation_RH(forward, up);
			m_quaternion = quatf::matrix_to_quat(mat);
		}

		void set_right(const float3& right) {}
	};


}

#endif