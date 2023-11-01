#include "core/Math/Matrix.h"

#include "Core/Assert.h"
#include "Core/Math/Math.h"

#include <cmath>

namespace influx::math
{
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline float matrix<_t, _C, _R>::determinant(const matrix<_t, 2, 2>& m)
	{
		return m[0][0] * m[1][1] - m[0][1] * m[1][0];
	}

	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline float matrix<_t, _C, _R>::determinant(const matrix<_t, 3, 3>& m)
	{
		// From wikipedia
		return m[0][0] * m[1][1] * m[2][2] +
			m[0][1] * m[1][2] * m[2][0] +
			m[0][2] * m[1][0] * m[2][1] -
			m[0][2] * m[1][1] * m[2][0] -
			m[0][1] * m[1][0] * m[2][2] -
			m[0][0] * m[1][2] * m[2][1];
	}
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline float matrix<_t, _C, _R>::determinant(const matrix<_t, 4, 4>& m)
	{
		return 0.0f;
	}

	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline float matrix<_t, _C, _R>::determinant() const
	{
		return determinant(*this);
	}

#pragma region Inversion
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, 4, 4> matrix<_t, _C, _R>::inverse(const matrix<_t, 4, 4>& m)
	{
		matrix<_t, 4, 4> cpy = m;
		invert(cpy);
		return cpy;
	}
	// _The real one ;)
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline float matrix<_t, _C, _R>::invert(matrix<_t, 4, 4>& m)
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

		for (matrix_dim_t i = 0; i < 16; i++)
			m.m_data[i] = inverse.m_data[i] * det;

		return det;
	}
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, 4, 4> matrix<_t, _C, _R>::inverted() const
	{
		return inverse(*this);
	}
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline float matrix<_t, _C, _R>::invert()
	{
		return invert(*this);
	}
#pragma endregion

	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline _t matrix<_t, _C, _R>::get_sum() const
	{
		_t sum{};
		for_each_element([&sum](const _t& element)
		{
			sum += element;
		});

		return sum;
	}
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline _t matrix<_t, _C, _R>::get_sum(const matrix& matrix)
	{
		return matrix.get_sum();
	}
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline bool matrix<_t, _C, _R>::is_null() const
	{
		for (matrix_dim_t r{}; r < _R; ++r)
			for (matrix_dim_t c{}; c < _C; ++c)
			{
				if (get_element(c, r) != (_t)0)
					return false;
			}

		return true;
	}
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline bool matrix<_t, _C, _R>::is_null(const matrix& matrix)
	{
		return matrix.is_null();
	}
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, _C, _R> matrix<_t, _C, _R>::identity()
	{
		matrix<_t, _C, _R> result{};
		for (matrix_dim_t r{}; r < _R; ++r)
			for (matrix_dim_t c{}; c < _C; ++c)
			{
				matrix_dim_t i = c + (_C * r);
				if (r == c) result.m_data[i] = (_t)1;
			}

		return result;
	}

#pragma region _Transformation

	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, 3, 3> matrix<_t, _C, _R>::make_translation(const vector<_t, 2>& t)
	{
		return
		{
			1, 0, 0,
			0, 1, 0,
			t.x, t.y, 1
		};
	}
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, 4, 4> matrix<_t, _C, _R>::make_translation(const vector<_t, 3>& t)
	{
		return
		{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			t.x, t.y, t.z, 1
		};
	}
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, 3, 3> matrix<_t, _C, _R>::make_rotation(float angle)
	{
		return
		{
			cosf(angle), -sinf(angle), 0,
			sinf(angle), cosf(angle), 0,
			0, 0, 1
		};
	}
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, 4, 4> matrix<_t, _C, _R>::make_rotation(const vector<_t, 3>& axis, float angle)
	{
		// http://www.fastgraph.com/makegames/3drotation/3dsrce.html

		float x{ axis.x }, y{ axis.y }, z{ axis.z };
		vector<_t, 3> norm = axis.Normalized();
		float c, s, t;
		c = cos(angle);
		s = sin(angle);
		t = 1 - c;

		return
		{
			t * x * x + c,	 t * x * y - s * z,	 t * x * z + s * y, 0,
			t * x * y + s * z, t * y * y + c, t * y * z - s * x, 0,
			t * x * z - s * y, t * y * z + s * x, t * z * z + c, 0,
			0, 0, 0, 1
		};
	}
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, 3, 3> matrix<_t, _C, _R>::make_scale(const vector<_t, 2>& s)
	{
		return
		{
			s.x, 0, 0,
			0, s.y, 0,
			0, 0, 1
		};
	}
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, 4, 4> matrix<_t, _C, _R>::make_scale(const vector<_t, 3>& s)
	{
		return
		{
			s.x, 0, 0, 0,
			0, s.y, 0, 0,
			0, 0, s.z, 0,
			0, 0, 0, 1
		};
	}

#pragma endregion
	// _Constructor:
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	template <typename Other_T>
	matrix<_t, _C, _R>::matrix(const matrix<Other_T, _C, _R>& other)
	{
		for_each_element([&other](_t& element, matrix_dim_t idx) { element = static_cast<_t>(other.get_element(idx)); });
	}
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	template <typename Other_T>
	matrix<_t, _C, _R>::matrix(matrix<Other_T, _C, _R>&& other)
	{
		for_each_element([&other](_t& element, matrix_dim_t idx) { element = static_cast<_t>(other.get_element(idx)); });
	}

	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline const vector<_t, _C>& matrix<_t, _C, _R>::operator[](matrix_dim_t r) const
	{
		FLX_ASSERT(r < _R);
		return this->m_rows[r];
	}

	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline vector<_t, _C>& matrix<_t, _C, _R>::operator[](matrix_dim_t r)
	{
		FLX_ASSERT(r < _R);
		return this->m_rows[r];
	}

	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline const vector<_t, _C>& matrix<_t, _C, _R>::get_row(matrix_dim_t r) const
	{
		FLX_ASSERT(r < _R);
		return this->m_rows[r];
	}

	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline vector<_t, _C>& matrix<_t, _C, _R>::get_row(matrix_dim_t r)
	{
		FLX_ASSERT(r < _R);
		return this->m_rows[r];
	}

	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline vector<_t, _R> matrix<_t, _C, _R>::get_collumn(matrix_dim_t c) const
	{
		FLX_ASSERT(c < _C);
		vector<_t, _R> collumn{};
		for (matrix_dim_t r{}; r < _R; ++r)
			collumn[r] = this->rows[r][c];

		return collumn;
	}

	// Data Access:
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline _t& matrix<_t, _C, _R>::get_element(matrix_dim_t c, matrix_dim_t r)
	{
		FLX_ASSERT(r < _R&& c < _C);
		return (*this)[r][c];
	}
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline const _t& matrix<_t, _C, _R>::get_element(matrix_dim_t c, matrix_dim_t r) const
	{
		FLX_ASSERT(r < _R&& c < _C);
		return (*this)[r][c];
	}
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline _t& matrix<_t, _C, _R>::get_element(matrix_dim_t idx)
	{
		FLX_ASSERT(idx < _C* _R);
		return get_element(idx % _C, idx / _R);
	}
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline const _t& matrix<_t, _C, _R>::get_element(matrix_dim_t idx) const
	{
		FLX_ASSERT(idx < _C* _R);
		return get_element(idx % _C, idx / _R);
	}


	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, _C, _R>& matrix<_t, _C, _R>::operator*=(const float scalar)
	{
		for_each_element([](_t& element)
		{
			element *= scalar;
		});

		return *this;
	}
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, _C, _R>& matrix<_t, _C, _R>::operator/=(const float scalar)
	{
		for_each_element([](_t& element)
		{
			element /= scalar;
		});

		return *this;
	}

	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, _C, _R>& matrix<_t, _C, _R>::member_multiply(const matrix<_t, _C, _R>& other)
	{
		for (matrix_dim_t i{}; i < _R * _C; ++i)
			this->m_data[i] = other.m_data[i];

		return *this;
	}

	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, _C, _R> matrix<_t, _C, _R>::member_multiply(const matrix<_t, _C, _R>& a, const matrix<_t, _C, _R>& b)
	{
		matrix<_t, _C, _R> res = a;
		res.member_multiply(b);
		return res;
	}


	// Operations: matrix - Scalar
	template <typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, _C, _R> operator*(const matrix<_t, _C, _R>& a, float b)
	{
		matrix<_t, _C, _R> result = a;
		result.for_each_element([](_t& el) { el *= b; });
		return result;
	}
	template <typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, _C, _R> operator/(const matrix<_t, _C, _R>& a, float b)
	{
		matrix<_t, _C, _R> result = a;
		result.for_each_element([](_t& el) { el /= b; });
		return result;
	}
	template <typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, _C, _R> operator*(float a, const matrix<_t, _C, _R>& b)
	{
		matrix<_t, _C, _R> result = b;
		result.for_each_element([](_t& el) { el *= a; });
		return result;
	}
	template <typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, _C, _R> operator/(float a, const matrix<_t, _C, _R>& b)
	{
		matrix<_t, _C, _R> result = b;
		result.for_each_element([](_t& el) { el /= a; });
		return result;
	}


	// Operations: matrix - matrix
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, _C, _R> operator+(const matrix<_t, _C, _R>& a, const matrix<_t, _C, _R>& b)
	{
		matrix<_t, _C, _R> result = a;
		result.for_each_element([=, &result](_t& element, matrix_dim_t idx)
		{
			result.get_element(idx) += b.get_element(idx);
		});

		return result;
	}
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, _C, _R> operator-(const matrix<_t, _C, _R>& a, const matrix<_t, _C, _R>& b)
	{
		matrix<_t, _C, _R> result = a;
		result.for_each_element([=, &result](_t& element, matrix_dim_t idx)
		{
			result.get_element(idx) -= b.get_element(idx);
		});

		return result;
	}

	template <typename _t, matrix_dim_t _C, matrix_dim_t _R, matrix_dim_t _OC, matrix_dim_t _OR>
	inline matrix<_t, _R, _OR> operator*(const matrix<_t, _C, _R>& a, const matrix<_t, _OC, _OR>& b)
	{
		FLX_ASSERT(_C == _OR);

		matrix<_t, _R, _OC> result{};

		for (matrix_dim_t r{}; r < _R; ++r)
			for (matrix_dim_t c{}; c < _OC; ++c)
				for (matrix_dim_t i = 0; i < _OR; ++i)
				{
					result[r][c] += a[r][i] * b[i][c];
				}

		return result;
	}

	// Operations: matrix - Vector
	template<typename _t>
	inline vector<_t, 2> operator*(const matrix<_t, 3, 3>& mat, const vector<_t, 2>& v)
	{
		vector<_t, 3> result{ v.x, v.y, 1.f };
		for (matrix_dim_t c{}; c < 3; ++c)
		{
			for (matrix_dim_t r{}; r < 3; ++r)
				result[c] += mat[r][c] * result[c];
		}
		return { result.x, result.y };
	}
	template<typename _t>
	inline vector<_t, 3u> operator*(const matrix<_t, 4u, 4u>& mat, const vector<_t, 3u>& v)
	{
		vector<_t, 4u> cpy = { v.x, v.y, v.z, 1.f };
		vector<_t, 4u> res{};
		for (matrix_dim_t c{}; c < 4u; ++c)
			res[c] = vector<_t, 4u>::dot(mat.Collumn(c), cpy);

		return { res.x, res.y, res.z };
	}

	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, 4, 4> matrix<_t, _C, _R>::make_transform_LH(const vector<_t, 3u>& pos, const vector<_t, 3u>& forward, const vector<_t, 3u>& up)
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
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, 4, 4> matrix<_t, _C, _R>::make_transform_RH(const vector<_t, 3u>& pos, const vector<_t, 3u>& forward, const vector<_t, 3u>& up)
	{
		vector<_t, 3u> lRight = vector<_t, 3u>::cross(forward, up);
		vector<_t, 3u> lUp = vector<_t, 3u>::cross(lRight, forward);
		vector<_t, 3u> lForward = forward;

		return
		{
			lRight.x, lRight.y, lRight.z, 0.0f,
			lUp.x, lUp.y, lUp.z, 0.0f,
			lForward.x, lForward.y, lForward.z, 0.0f,
			pos.x, pos.y, pos.z, 1
		};
	}

	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, 4u, 4u> matrix<_t, _C, _R>::make_view_LH(const vector<_t, 3u>& pos, const vector<_t, 3u>& forward, const vector<_t, 3u>& up)
	{
		return make_transform_LH(pos, forward, up).inverted();
	}

	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, 4u, 4u> matrix<_t, _C, _R>::make_view_RH(const vector<_t, 3u>& pos, const vector<_t, 3u>& forward, const vector<_t, 3u>& up)
	{
		return make_transform_RH(pos, forward, up).inverted();
	}

	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, 4u, 4u> matrix<_t, _C, _R>::make_projection_RH(const float fov, const float ar, const float n, const float f)
	{
		float y = 1.0f / tanf(math::degrees_to_radians(fov) / 2.f);
		float x = y / ar;
		float intv = n - f;

		return
		{
			(_t)x, (_t)0, (_t)0, (_t)0,
			(_t)0, (_t)y, (_t)0, (_t)0,
			(_t)0, (_t)0, (_t)f / intv,			(_t)-1,
			(_t)0, (_t)0, (_t)(f * n) / intv,	(_t)0
		};
	}

	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline matrix<_t, 4u, 4u> matrix<_t, _C, _R>::make_projection_LH(const float fov, const float ar, const float n, const float f)
	{
		float y = 1.0f / tanf(math::degrees_to_radians(fov) / 2.f);
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

	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline void matrix<_t, _C, _R>::for_each_element(std::function<void(_t&)> operation)
	{
		for (matrix_dim_t r{}; r < _R; ++r)
			for (matrix_dim_t c{}; c < _C; ++c)
			{
				matrix_dim_t i = c + (_C * r);
				operation(this->m_data[i]);
			}
	}
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline void matrix<_t, _C, _R>::for_each_element(std::function<void(_t&, matrix_dim_t idx)> operation)
	{
		for (matrix_dim_t r{}; r < _R; ++r)
			for (matrix_dim_t c{}; c < _C; ++c)
			{
				matrix_dim_t i = c + (_C * r);
				operation(this->m_data[i], i);
			}
	}
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline void matrix<_t, _C, _R>::for_each_element(std::function<void(const _t&)> operation) const
	{
		for (matrix_dim_t r{}; r < _R; ++r)
			for (matrix_dim_t c{}; c < _C; ++c)
			{
				matrix_dim_t i = c + (_C * r);
				operation(this->m_data[i]);
			}
	}
	template<typename _t, matrix_dim_t _C, matrix_dim_t _R>
	inline void matrix<_t, _C, _R>::for_each_element(std::function<void(const _t&, matrix_dim_t idx)> operation) const
	{
		for (matrix_dim_t r{}; r < _R; ++r)
			for (matrix_dim_t c{}; c < _C; ++c)
			{
				matrix_dim_t i = c + (_C * r);
				operation(this->m_data[i], i);
			}
	}
}