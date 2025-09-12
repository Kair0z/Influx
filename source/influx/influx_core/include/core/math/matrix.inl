#include "core/math/matrix.h"
#include "core/debug.h"
#include "core/math/math.h"

#include <algorithm>
#include <cmath>

namespace influx
{
	template <typename _t>
	inline void do_swap(_t& a, _t& b)
	{
		std::swap<_t>(a, b);
	}

	template <typename _t>
	inline void to_radians(const _t& degrees)
	{
		return math::to_radians(degrees);
	}
}

namespace influx::math
{
#pragma region transpose
	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, _x, _y>& matrix<_t, _x, _y>::transpose()
	{
		// todo: how about non-uniform matrices?
		static_assert(_x == _y);

		for (matsize y = 0u; y < _y; ++y)
		{
			for (matsize x = 0u; x < _x; ++x)
			{
				do_swap(this->m_rows[y][x], this->m_rows[x][y]);
			}
		}
		return *this;
	}
	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, _x, _y> matrix<_t, _x, _y>::transposed() const
	{
		matrix<_t, _x, _y> copy = *this;
		return transpose(copy);
	}
	template<typename _t, matsize _x, matsize _y>
	inline void matrix<_t, _x, _y>::transpose(matrix& matrix)
	{
		matrix.transpose();
	}
	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, _x, _y> matrix<_t, _x, _y>::transposed(const matrix& matrix)
	{
		return matrix.transposed();
	}
#pragma endregion

#pragma region determinant
	template<typename _t, matsize _x, matsize _y>
	inline float matrix<_t, _x, _y>::determinant(const matrix<_t, 2, 2>& m)
	{
		return m[0][0] * m[1][1] - m[0][1] * m[1][0];
	}

	template<typename _t, matsize _x, matsize _y>
	inline float matrix<_t, _x, _y>::determinant(const matrix<_t, 3, 3>& m)
	{
		// From wikipedia
		return m[0][0] * m[1][1] * m[2][2] +
			m[0][1] * m[1][2] * m[2][0] +
			m[0][2] * m[1][0] * m[2][1] -
			m[0][2] * m[1][1] * m[2][0] -
			m[0][1] * m[1][0] * m[2][2] -
			m[0][0] * m[1][2] * m[2][1];
	}
	template<typename _t, matsize _x, matsize _y>
	inline float matrix<_t, _x, _y>::determinant(const matrix<_t, 4, 4>& m)
	{
		return 0.0f;
	}

	template<typename _t, matsize _x, matsize _y>
	inline float matrix<_t, _x, _y>::determinant() const
	{
		return determinant(*this);
	}
#pragma endregion

#pragma region Inversion
	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, 4, 4> matrix<_t, _x, _y>::inverse(const matrix<_t, 4, 4>& m)
	{
		matrix<_t, 4, 4> cpy = m;
		invert(cpy);
		return cpy;
	}
	template<typename _t, matsize _x, matsize _y>
	inline float matrix<_t, _x, _y>::invert(matrix<_t, 4, 4>& m)
	{
		matrix<_t, 4, 4> inverse{};
		float det{};

		inverse.m_data[0] = m.m_data[5] * m.m_data[10] * m.m_data[15] -
			m.m_data[5] * m.m_data[11] * m.m_data[14] -
			m.m_data[9] * m.m_data[6] * m.m_data[15] +
			m.m_data[9] * m.m_data[7] * m.m_data[14] +
			m.m_data[13] * m.m_data[6] * m.m_data[11] -
			m.m_data[13] * m.m_data[7] * m.m_data[10];

		inverse.m_data[4] = -m.m_data[4] * m.m_data[10] * m.m_data[15] +
			m.m_data[4] * m.m_data[11] * m.m_data[14] +
			m.m_data[8] * m.m_data[6] * m.m_data[15] -
			m.m_data[8] * m.m_data[7] * m.m_data[14] -
			m.m_data[12] * m.m_data[6] * m.m_data[11] +
			m.m_data[12] * m.m_data[7] * m.m_data[10];

		inverse.m_data[8] = m.m_data[4] * m.m_data[9] * m.m_data[15] -
			m.m_data[4] * m.m_data[11] * m.m_data[13] -
			m.m_data[8] * m.m_data[5] * m.m_data[15] +
			m.m_data[8] * m.m_data[7] * m.m_data[13] +
			m.m_data[12] * m.m_data[5] * m.m_data[11] -
			m.m_data[12] * m.m_data[7] * m.m_data[9];

		inverse.m_data[12] = -m.m_data[4] * m.m_data[9] * m.m_data[14] +
			m.m_data[4] * m.m_data[10] * m.m_data[13] +
			m.m_data[8] * m.m_data[5] * m.m_data[14] -
			m.m_data[8] * m.m_data[6] * m.m_data[13] -
			m.m_data[12] * m.m_data[5] * m.m_data[10] +
			m.m_data[12] * m.m_data[6] * m.m_data[9];

		inverse.m_data[1] = -m.m_data[1] * m.m_data[10] * m.m_data[15] +
			m.m_data[1] * m.m_data[11] * m.m_data[14] +
			m.m_data[9] * m.m_data[2] * m.m_data[15] -
			m.m_data[9] * m.m_data[3] * m.m_data[14] -
			m.m_data[13] * m.m_data[2] * m.m_data[11] +
			m.m_data[13] * m.m_data[3] * m.m_data[10];

		inverse.m_data[5] = m.m_data[0] * m.m_data[10] * m.m_data[15] -
			m.m_data[0] * m.m_data[11] * m.m_data[14] -
			m.m_data[8] * m.m_data[2] * m.m_data[15] +
			m.m_data[8] * m.m_data[3] * m.m_data[14] +
			m.m_data[12] * m.m_data[2] * m.m_data[11] -
			m.m_data[12] * m.m_data[3] * m.m_data[10];

		inverse.m_data[9] = -m.m_data[0] * m.m_data[9] * m.m_data[15] +
			m.m_data[0] * m.m_data[11] * m.m_data[13] +
			m.m_data[8] * m.m_data[1] * m.m_data[15] -
			m.m_data[8] * m.m_data[3] * m.m_data[13] -
			m.m_data[12] * m.m_data[1] * m.m_data[11] +
			m.m_data[12] * m.m_data[3] * m.m_data[9];

		inverse.m_data[13] = m.m_data[0] * m.m_data[9] * m.m_data[14] -
			m.m_data[0] * m.m_data[10] * m.m_data[13] -
			m.m_data[8] * m.m_data[1] * m.m_data[14] +
			m.m_data[8] * m.m_data[2] * m.m_data[13] +
			m.m_data[12] * m.m_data[1] * m.m_data[10] -
			m.m_data[12] * m.m_data[2] * m.m_data[9];

		inverse.m_data[2] = m.m_data[1] * m.m_data[6] * m.m_data[15] -
			m.m_data[1] * m.m_data[7] * m.m_data[14] -
			m.m_data[5] * m.m_data[2] * m.m_data[15] +
			m.m_data[5] * m.m_data[3] * m.m_data[14] +
			m.m_data[13] * m.m_data[2] * m.m_data[7] -
			m.m_data[13] * m.m_data[3] * m.m_data[6];

		inverse.m_data[6] = -m.m_data[0] * m.m_data[6] * m.m_data[15] +
			m.m_data[0] * m.m_data[7] * m.m_data[14] +
			m.m_data[4] * m.m_data[2] * m.m_data[15] -
			m.m_data[4] * m.m_data[3] * m.m_data[14] -
			m.m_data[12] * m.m_data[2] * m.m_data[7] +
			m.m_data[12] * m.m_data[3] * m.m_data[6];

		inverse.m_data[10] = m.m_data[0] * m.m_data[5] * m.m_data[15] -
			m.m_data[0] * m.m_data[7] * m.m_data[13] -
			m.m_data[4] * m.m_data[1] * m.m_data[15] +
			m.m_data[4] * m.m_data[3] * m.m_data[13] +
			m.m_data[12] * m.m_data[1] * m.m_data[7] -
			m.m_data[12] * m.m_data[3] * m.m_data[5];

		inverse.m_data[14] = -m.m_data[0] * m.m_data[5] * m.m_data[14] +
			m.m_data[0] * m.m_data[6] * m.m_data[13] +
			m.m_data[4] * m.m_data[1] * m.m_data[14] -
			m.m_data[4] * m.m_data[2] * m.m_data[13] -
			m.m_data[12] * m.m_data[1] * m.m_data[6] +
			m.m_data[12] * m.m_data[2] * m.m_data[5];

		inverse.m_data[3] = -m.m_data[1] * m.m_data[6] * m.m_data[11] +
			m.m_data[1] * m.m_data[7] * m.m_data[10] +
			m.m_data[5] * m.m_data[2] * m.m_data[11] -
			m.m_data[5] * m.m_data[3] * m.m_data[10] -
			m.m_data[9] * m.m_data[2] * m.m_data[7] +
			m.m_data[9] * m.m_data[3] * m.m_data[6];

		inverse.m_data[7] = m.m_data[0] * m.m_data[6] * m.m_data[11] -
			m.m_data[0] * m.m_data[7] * m.m_data[10] -
			m.m_data[4] * m.m_data[2] * m.m_data[11] +
			m.m_data[4] * m.m_data[3] * m.m_data[10] +
			m.m_data[8] * m.m_data[2] * m.m_data[7] -
			m.m_data[8] * m.m_data[3] * m.m_data[6];

		inverse.m_data[11] = -m.m_data[0] * m.m_data[5] * m.m_data[11] +
			m.m_data[0] * m.m_data[7] * m.m_data[9] +
			m.m_data[4] * m.m_data[1] * m.m_data[11] -
			m.m_data[4] * m.m_data[3] * m.m_data[9] -
			m.m_data[8] * m.m_data[1] * m.m_data[7] +
			m.m_data[8] * m.m_data[3] * m.m_data[5];

		inverse.m_data[15] = m.m_data[0] * m.m_data[5] * m.m_data[10] -
			m.m_data[0] * m.m_data[6] * m.m_data[9] -
			m.m_data[4] * m.m_data[1] * m.m_data[10] +
			m.m_data[4] * m.m_data[2] * m.m_data[9] +
			m.m_data[8] * m.m_data[1] * m.m_data[6] -
			m.m_data[8] * m.m_data[2] * m.m_data[5];

		det = m.m_data[0] * inverse.m_data[0] + m.m_data[1] * inverse.m_data[4] + m.m_data[2] * inverse.m_data[8] + m.m_data[3] * inverse.m_data[12];
		det = 1.0f / det;

		for (matsize i = 0; i < 16; i++)
			m.m_data[i] = inverse.m_data[i] * det;

		return det;
	}
	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, 4, 4> matrix<_t, _x, _y>::inverted() const
	{
		return inverse(*this);
	}
	template<typename _t, matsize _x, matsize _y>
	inline float matrix<_t, _x, _y>::invert()
	{
		return invert(*this);
	}
#pragma endregion

	template<typename _t, matsize _x, matsize _y>
	inline _t matrix<_t, _x, _y>::get_sum() const
	{
		_t sum{};
		for_each_element([&sum](const _t& element)
		{
			sum += element;
		});

		return sum;
	}
	template<typename _t, matsize _x, matsize _y>
	inline _t matrix<_t, _x, _y>::get_sum(const matrix& matrix)
	{
		return matrix.get_sum();
	}
	template<typename _t, matsize _x, matsize _y>
	inline bool matrix<_t, _x, _y>::is_null() const
	{
		for (matsize r{}; r < _y; ++r)
			for (matsize c{}; c < _x; ++c)
			{
				if (get_element(c, r) != (_t)0)
					return false;
			}

		return true;
	}
	template<typename _t, matsize _x, matsize _y>
	inline bool matrix<_t, _x, _y>::is_null(const matrix& matrix)
	{
		return matrix.is_null();
	}
	template<typename _t, matsize _x, matsize _y>
	inline const matrix<_t, _x, _y>& matrix<_t, _x, _y>::identity()
	{
		static matrix<_t, _x, _y> result{};
		static bool once = true;
		if (once == true)
		{
			for (matsize r{}; r < _y; ++r)
				for (matsize c{}; c < _x; ++c)
				{
					matsize i = c + (_x * r);
					if (r == c) 
						result.m_data[i] = (_t)1;
				}
			once = false;
		}
		
		return result;
	}

#pragma region _Transformation

	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, 3, 3> matrix<_t, _x, _y>::make_translation(const vector<_t, 2>& t)
	{
		return
		{
			1, 0, 0,
			0, 1, 0,
			t.x, t.y, 1
		};
	}
	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, 4, 4> matrix<_t, _x, _y>::make_translation(const vector<_t, 3>& t)
	{
		return
		{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			t.x, t.y, t.z, 1
		};
	}
	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, 3u, 3u> matrix<_t, _x, _y>::make_rotation(const vector<_t, 3>& axis, float angle)
	{
		// http://www.fastgraph.com/makegames/3drotation/3dsrce.html

		float x{ axis.x }, y{ axis.y }, z{ axis.z };
		vector<_t, 3> norm = axis.normalized();
		float c, s, t;
		c = cos(angle);
		s = sin(angle);
		t = 1 - c;

		return
		{
			t * x * x + c,	 t * x * y - s * z,	 t * x * z + s * y,
			t * x * y + s * z, t * y * y + c, t * y * z - s * x,
			t * x * z - s * y, t * y * z + s * x, t * z * z + c
		};
	}
	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, 4u, 4u> matrix<_t, _x, _y>::make_rotation_RH(float euler_x, float euler_y, float euler_z)
	{
		float cp = cosf(euler_x); // cos(Pitch)
		float sp = sinf(euler_x); // sin(Pitch)
		float cy = cosf(euler_y);   // cos(Yaw)
		float sy = sinf(euler_y);   // sin(Yaw)
		float cr = cosf(euler_z);  // cos(Roll)
		float sr = sinf(euler_z);  // sin(Roll)

		matrix<_t, 4u, 4u> result{};
		result[0][0] = cy * cr;
		result[0][1] = -cy * sr;
		result[0][2] = sy;
		result[1][0] = sp * sy * cr + cp * sr;
		result[1][1] = -sp * sy * sr + cp * cr;
		result[1][2] = -sp * cy;
		result[2][0] = -cp * sy * cr + sp * sr;
		result[2][1] = cp * sy * sr + sp * cr;
		result[2][2] = cp * cy;
		result[3][3] = 1.0f;
		return result;
	}
	template<typename _t, matsize _x, matsize _y>
	static matrix<_t, 4u, 4u> make_rotation_LH(float euler_x, float euler_y, float euler_z)
	{
		float cp = cosf(euler_x); // cos(Pitch)
		float sp = sinf(euler_x); // sin(Pitch)
		float cy = cosf(euler_y);   // cos(Yaw)
		float sy = -sinf(euler_y);   // sin(Yaw)
		float cr = cosf(euler_z);  // cos(Roll)
		float sr = -sinf(euler_z);  // sin(Roll)

		matrix<_t, 4u, 4u> result{};
		result[0][0] = cy * cr;
		result[0][1] = -cy * sr;
		result[0][2] = sy;
		result[1][0] = sp * sy * cr + cp * sr;
		result[1][1] = -sp * sy * sr + cp * cr;
		result[1][2] = -sp * cy;
		result[2][0] = -cp * sy * cr + sp * sr;
		result[2][1] = cp * sy * sr + sp * cr;
		result[2][2] = cp * cy;
		result[3][3] = 1.0f;
		return result;
	}
	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, 3, 3> matrix<_t, _x, _y>::make_scale(const vector<_t, 2>& s)
	{
		return
		{
			s.x, 0, 0,
			0, s.y, 0,
			0, 0, 1
		};
	}
	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, 4, 4> matrix<_t, _x, _y>::make_scale(const vector<_t, 3>& s)
	{
		return
		{
			s.x, 0, 0, 0,
			0, s.y, 0, 0,
			0, 0, s.z, 0,
			0, 0, 0, 1
		};
	}
	template<typename _t, matsize _x, matsize _y>
	inline vector<_t, 3u> matrix<_t, _x, _y>::get_euler_angles() const
	{
#if 1
		float r21 = this->m_yows[1][0];
		float r11 = this->m_yows[0][0];
		float r31 = this->m_yows[2][0];
		float r32 = this->m_yows[2][1];
		float r33 = this->m_yows[2][2];
		return
		{
			atan2f(r32, r33),
			atan2f(-r31, sqrtf(powf(r32,2) + powf(r33,2))),
			atan2f(r21, r11)
		};
#else
		if (math::abs(this->m_yows[2][0]) < 1.0) 
		{
			return vector<_t, 3u>
			{
				asinf(-this->m_yows[2][0]),
				atan2f(this->m_yows[2][1], this->m_yows[2][2]),
				atan2f(this->m_yows[1][0], this->m_yows[0][0])
			};	
		}
#endif
	}

#pragma endregion
	// constructor:
	template<typename _t, matsize _x, matsize _y>
	template <typename Other_T>
	matrix<_t, _x, _y>::matrix(const matrix<Other_T, _x, _y>& other)
	{
		// for_each_element([&other](_t& element, matsize idx) { element = static_cast<_t>(other.get_element(idx)); });
	}
	template<typename _t, matsize _x, matsize _y>
	template <typename Other_T>
	matrix<_t, _x, _y>::matrix(matrix<Other_T, _x, _y>&& other)
	{
		// for_each_element([&other](_t& element, matsize idx) { element = static_cast<_t>(other.get_element(idx)); });
	}

	template<typename _t, matsize _x, matsize _y>
	inline const vector<_t, _x>& matrix<_t, _x, _y>::operator[](matsize index) const
	{
		influx_assert(index < _y);
		return this->m_rows[index];
	}

	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, _x, _y>::row& matrix<_t, _x, _y>::operator[](matsize index)
	{
		influx_assert(index < _y);
		return this->m_rows[index];
	}

	template<typename _t, matsize _x, matsize _y>
	inline const vector<_t, _x>& matrix<_t, _x, _y>::get_row(matsize index) const
	{
		influx_assert(index < _y);
		return this->m_rows[index];
	}

	template<typename _t, matsize _x, matsize _y>
	inline vector<_t, _x>& matrix<_t, _x, _y>::get_row(matsize index)
	{
		influx_assert(index < _y);
		return this->m_rows[index];
	}

	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, _x, _y>::column matrix<_t, _x, _y>::get_column(matsize index) const
	{
		influx_assert(index < _x);
		vector<_t, _y> collumn{};
		for (matsize r{}; r < _y; ++r)
			collumn[r] = this->m_rows[r][index];

		return collumn;
	}

	template<typename _t, matsize _x, matsize _y>
	inline void matrix<_t, _x, _y>::set_column(matsize index, const matrix<_t, _x, _y>::column& column)
	{
		influx_assert(index < k_num_columns);
		for (matsize r{}; r < k_num_rows; ++r)
		{
			this->m_rows[r][index] = column[r];
		}
	}

	// Data Access:
	template<typename _t, matsize _x, matsize _y>
	inline _t& matrix<_t, _x, _y>::get_element(matsize c, matsize r)
	{
		influx_assert(r < _y&& c < _x);
		return (*this)[r][c];
	}
	template<typename _t, matsize _x, matsize _y>
	inline const _t& matrix<_t, _x, _y>::get_element(matsize c, matsize r) const
	{
		influx_assert(r < _y&& c < _x);
		return (*this)[r][c];
	}
	template<typename _t, matsize _x, matsize _y>
	inline _t& matrix<_t, _x, _y>::get_element(matsize idx)
	{
		influx_assert(idx < _x* _y);
		return get_element(idx % _x, idx / _y);
	}
	template<typename _t, matsize _x, matsize _y>
	inline const _t& matrix<_t, _x, _y>::get_element(matsize idx) const
	{
		influx_assert(idx < _x* _y);
		return get_element(idx % _x, idx / _y);
	}

	template<typename _t, matsize _x, matsize _y>
	bool matrix<_t, _x, _y>::operator==(const matrix& other) const
	{
		for (matsize r{}; r < _y; ++r)
			for (matsize c{}; c < _x; ++c)
			{
				const matsize idx = c + (_x * r);
				if (this->m_data[idx] != other.m_data[idx]) return false;
			}

		return true;
	}
	template<typename _t, matsize _x, matsize _y>
	bool matrix<_t, _x, _y>::operator!=(const matrix& other) const
	{
		return !(*this == other);
	}

	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, _x, _y>& matrix<_t, _x, _y>::operator*=(const float scalar)
	{
		for_each_element([](_t& element)
		{
			element *= scalar;
		});

		return *this;
	}
	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, _x, _y>& matrix<_t, _x, _y>::operator/=(const float scalar)
	{
		for_each_element([](_t& element)
		{
			element /= scalar;
		});

		return *this;
	}

	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, _x, _y>& matrix<_t, _x, _y>::operator*=(const matrix& other)
	{
		*this = *this * other;
		return *this;
	}

	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, _x, _y>& matrix<_t, _x, _y>::member_multiply(const matrix<_t, _x, _y>& other)
	{
		for (matsize i{}; i < _y * _x; ++i)
			this->m_data[i] = other.m_data[i];

		return *this;
	}
	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, _x, _y> matrix<_t, _x, _y>::member_multiply(const matrix<_t, _x, _y>& a, const matrix<_t, _x, _y>& b)
	{
		matrix<_t, _x, _y> res = a;
		res.member_multiply(b);
		return res;
	}

	// Operations: matrix - Scalar
	template <typename _t, matsize _x, matsize _y>
	inline matrix<_t, _x, _y> operator*(const matrix<_t, _x, _y>& a, float b)
	{
		matrix<_t, _x, _y> result = a;
		result.for_each_element([](_t& el) { el *= b; });
		return result;
	}
	template <typename _t, matsize _x, matsize _y>
	inline matrix<_t, _x, _y> operator/(const matrix<_t, _x, _y>& a, float b)
	{
		matrix<_t, _x, _y> result = a;
		result.for_each_element([](_t& el) { el /= b; });
		return result;
	}
	template <typename _t, matsize _x, matsize _y>
	inline matrix<_t, _x, _y> operator*(float a, const matrix<_t, _x, _y>& b)
	{
		matrix<_t, _x, _y> result = b;
		result.for_each_element([](_t& el) { el *= a; });
		return result;
	}
	template <typename _t, matsize _x, matsize _y>
	inline matrix<_t, _x, _y> operator/(float a, const matrix<_t, _x, _y>& b)
	{
		matrix<_t, _x, _y> result = b;
		result.for_each_element([](_t& el) { el /= a; });
		return result;
	}

	// Operations: matrix - matrix
	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, _x, _y> operator+(const matrix<_t, _x, _y>& a, const matrix<_t, _x, _y>& b)
	{
		matrix<_t, _x, _y> result = a;
		result.for_each_element([=, &result](_t& element, matsize idx)
		{
			result.get_element(idx) += b.get_element(idx);
		});

		return result;
	}
	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, _x, _y> operator-(const matrix<_t, _x, _y>& a, const matrix<_t, _x, _y>& b)
	{
		matrix<_t, _x, _y> result = a;
		result.for_each_element([=, &result](_t& element, matsize idx)
		{
			result.get_element(idx) -= b.get_element(idx);
		});

		return result;
	}

#pragma warning(push)
#pragma warning(disable : 4267)
	template <typename _t, matsize _x, matsize _y, matsize _OC, matsize _OR>
	inline matrix<_t, _y, _OR> operator*(const matrix<_t, _x, _y>& a, const matrix<_t, _OC, _OR>& b)
	{
		influx_assert(_x == _OR);

		matrix<_t, _y, _OC> result{};

		for (matsize r{}; r < _y; ++r)
			for (matsize c{}; c < _OC; ++c)
				for (matsize i = 0; i < _OR; ++i)
				{
					result[r][c] += a[r][i] * b[i][c];
				}

		return result;
	}
#pragma warning(pop) 

	// Operations: matrix - Vector
	template<typename _t>
	inline vector<_t, 2u> operator*(const matrix<_t, 3u, 3u>& mat, const vector<_t, 2>& v)
	{
		vector<_t, 3> result{ v.x, v.y, 1.f };
		for (matsize c{}; c < 3; ++c)
		{
			for (matsize r{}; r < 3; ++r)
				result[c] += mat[r][c] * result[c];
		}
		return { result.x, result.y };
	}
	template<typename _t>
	inline vector<_t, 3u> operator*(const matrix<_t, 3u, 3u>& mat, const vector<_t, 3u>& v)
	{
		vector<_t, 3u> cpy = { v.x, v.y, v.z };
		vector<_t, 3u> res{};
		for (matsize c{}; c < 3u; ++c)
		{
			res[c] = vector<_t, 3u>::dot(mat.get_column(c), cpy);
		}

		return { res.x, res.y, res.z };
	}
	template<typename _t>
	inline vector<_t, 3u> operator*(const matrix<_t, 4u, 4u>& mat, const vector<_t, 3u>& v)
	{
		vector<_t, 4u> cpy = { v.x, v.y, v.z, 1.f };
		vector<_t, 4u> res{};
		for (matsize c{}; c < 4u; ++c)
			res[c] = vector<_t, 4u>::dot(mat.get_column(c), cpy);

		return { res.x, res.y, res.z };
	}
	template<typename _t> 
	inline vector<_t, 4u> operator*(const matrix<_t, 4u, 4u>& mat, const vector<_t, 4u>& v)
	{
		vector<_t, 4u> cpy = { v.x, v.y, v.z, v.w };
		vector<_t, 4u> res{};
		for (matsize c{}; c < 4u; ++c)
			res[c] = vector<_t, 4u>::dot(mat.get_column(c), cpy);

		return { res.x, res.y, res.z, res.w };
	}
	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, 4u, 4u> matrix<_t, _x, _y>::make_transform_LH(const vector<_t, 3u>& pos, const vector<_t, 3u>& forward, const vector<_t, 3u>& up)
	{
		vector<_t, 3u> lRight = vector<_t, 3u>::cross(up, forward);
		vector<_t, 3u> lUp = vector<_t, 3u>::cross(forward, lRight);
		vector<_t, 3u> lForward = forward;

		return
		{
			lRight.x, lRight.y, lRight.z, 0.0f,
			lUp.x, lUp.y, lUp.z, 0.0f,
			lForward.x, lForward.y, lForward.z, 0.0f,
			pos.x, pos.y, pos.z, 1
		};
	}
	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, 4u, 4u> matrix<_t, _x, _y>::make_transform_RH(const vector<_t, 3u>& pos, const vector<_t, 3u>& forward, const vector<_t, 3u>& up)
	{
		vector<_t, 3u> lRight = vector<_t, 3u>::cross(forward, up).normalized();
		vector<_t, 3u> lUp = vector<_t, 3u>::cross(lRight, forward).normalized();
		vector<_t, 3u> lForward = forward.normalized();

		return
		{
			lRight.x, lRight.y, lRight.z, 0.0f,
			lUp.x, lUp.y, lUp.z, 0.0f,
			lForward.x, lForward.y, lForward.z, 0.0f,
			pos.x, pos.y, pos.z, 1
		};
	}

	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, 4u, 4u> matrix<_t, _x, _y>::make_view_LH(const vector<_t, 3u>& pos, const vector<_t, 3u>& forward, const vector<_t, 3u>& up)
	{
		return make_transform_LH(pos, forward, up).inverted();
	}

	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, 4u, 4u> matrix<_t, _x, _y>::make_view_RH(const vector<_t, 3u>& pos, const vector<_t, 3u>& forward, const vector<_t, 3u>& up)
	{
		return make_transform_RH(pos, forward, up).inverted();
	}

	// assuming [0, 1] depth range
	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, 4u, 4u> matrix<_t, _x, _y>::make_projection_LH(const float fov, const float ar, const float n, const float f)
	{
		float y = 1.0f / tanf(math::to_radians(fov) / 2.f);
		float x = y / ar;
		float intv = f - n;
		return
		{
			(_t)x, (_t)0, (_t)0, (_t)0,
			(_t)0, (_t)y, (_t)0, (_t)0,
			(_t)0, (_t)0, (_t)f / intv, (_t)1,
			(_t)0, (_t)0, static_cast<_t>(-(f * n) / intv), (_t)0
		};
	}
	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, 4u, 4u> matrix<_t, _x, _y>::make_projection_RH(
		const float fov, const float ar, 
		const float pnear, const float pfar,
		const float dmin, const float dmax)
	{
		const float fov_radians = to_radians(fov);
		const float plane_delta = pfar - pnear;
		const float depth_delta = dmax - dmin;

		const float y = 1.0f / tanf(fov_radians * 0.5f);
		const float x = y / ar;
		const float a = -(pfar * dmax - pnear * dmin) / (plane_delta);
		const float b = -(pfar * pnear * depth_delta) / (plane_delta);
		const float c = -(depth_delta);
		const float d = -dmin;

		return
		{
			(_t)x, (_t)0, (_t)0,	(_t)0,
			(_t)0, (_t)y, (_t)0,	(_t)0,
			(_t)0, (_t)0, (_t)a,	(_t)c,
			(_t)0, (_t)0, (_t)b,	(_t)d
		};
	}

	template<typename _t, matsize _x, matsize _y>
	inline matrix<_t, 4u, 4u> matrix<_t, _x, _y>::make_viewprojection_RH(
		const vector3& pos,
		const vector3& forward,
		const float fov,
		const float aspect_ratio,
		const float _near,
		const float _far,
		const vector3& up)
	{
		const math::matrix4x4f mat_view = math::matrix4x4f::make_view_RH(pos, forward, up);
		const math::matrix4x4f mat_proj = math::matrix4x4f::make_projection_RH(fov, aspect_ratio, _near, _far);
		return mat_view * mat_proj;
	}

	template<typename _t, matsize _x, matsize _y>
	matrix<_t, _x, _y> matrix<_t, _x, _y>::make_diagonal(const _t& x, const _t& y, const _t& z, const _t& w)
	{
		return
		{
			x, 0, 0, 0,
			0, y, 0, 0,
			0, 0, z, 0,
			0, 0, 0, w
		};
	}
	template<typename _t, matsize _x, matsize _y>
	inline void matrix<_t, _x, _y>::set_translation(const vector<_t, 3u>& translation)
	{
		static_assert(_x == 4u && _y == 4u);
		this->m_rows[3u].x = translation.x;
		this->m_rows[3u].y = translation.y;
		this->m_rows[3u].z = translation.z;
	}
	template<typename _t, matsize _x, matsize _y>
	inline vector<_t, 3u> matrix<_t, _x, _y>::get_translation() const
	{
		static_assert(_x == 4u && _y == 4u);
		const auto& row = get_row(3u);
		return { row.x, row.y, row.z };
	}

	template<typename _t, matsize _x, matsize _y>
	vector<_t, 3u> matrix<_t, _x, _y>::get_scale() const
	{
		static_assert(_x == 4u && _y == 4u);
		vector<_t, 3u> scale_sqr = get_scale_sqr();
		return 
		{
			sqrtf(scale_sqr.x),
			sqrtf(scale_sqr.y),
			sqrtf(scale_sqr.z),
		};

		return scale_sqr;
	}

	template<typename _t, matsize _x, matsize _y>
	vector<_t, 3u> matrix<_t, _x, _y>::get_scale_sqr() const
	{
		static_assert(_x == 4u && _y == 4u);
		const auto& rowX = get_row(0u);
		const auto& rowY = get_row(1u);
		const auto& rowZ = get_row(2u);

		return { 
			rowX.get_magnitude_sq(),
			rowY.get_magnitude_sq(),
			rowZ.get_magnitude_sq()};
	}

	template<typename _t, matsize _x, matsize _y>
	void matrix<_t, _x, _y>::decompose(vector<_t, 3u>& out_translation, matrix<_t, 3u, 3u>& out_yotation, vector<_t, 3u>& out_scale) const
	{
		static_assert(_x == 4u && _y == 4u);

		out_translation = get_translation();
		out_yotation	= get_rotation_matrix();
		out_scale		= get_scale();
	}

	template<typename _t, matsize _x, matsize _y>
	matrix<_t, 3u, 3u> matrix<_t, _x, _y>::get_rotation_matrix() const
	{
		static_assert(_x == 4u && _y == 4u);
		auto rowX = get_row(0u);
		auto rowY = get_row(1u);
		auto rowZ = get_row(2u);

		rowX.normalize();
		rowY.normalize();
		rowZ.normalize();

		return
		{
			rowX.x, rowX.y, rowX.z,
			rowY.x, rowY.y, rowY.z,
			rowZ.x, rowZ.y, rowZ.z,
		};
	}
}