#include "Matrix.h"

#define ___CORE_MATRIX_USE_CORE_ 1
#if ___CORE_MATRIX_USE_CORE_
#include "Core/Assert.h"
#include "Core/Math/Math.h"
#else
#include <cassert>
#define FLX_ASSERT(expr) assert(expr)

namespace influx::Math
{
	template <typename _t>
	constexpr inline _t DegreesToRadians(_t degrees)
	{
		return degrees * (3.1415 / 180);
	}
}


#endif

#include <cmath>

namespace influx::Math
{
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline float Matrix<_t, _C, _R>::Determinant(const Matrix<_t, 2, 2>& m)
	{
		return m[0][0] * m[1][1] - m[0][1] * m[1][0];
	}

	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline float Matrix<_t, _C, _R>::Determinant(const Matrix<_t, 3, 3>& m)
	{
		// From wikipedia
		return m[0][0] * m[1][1] * m[2][2] +
			m[0][1] * m[1][2] * m[2][0] +
			m[0][2] * m[1][0] * m[2][1] -
			m[0][2] * m[1][1] * m[2][0] -
			m[0][1] * m[1][0] * m[2][2] -
			m[0][0] * m[1][2] * m[2][1];
	}
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline float Matrix<_t, _C, _R>::Determinant(const Matrix<_t, 4, 4>& m)
	{

	}

	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline float Matrix<_t, _C, _R>::Determinant() const
	{
		return Determinant(*this);
	}

#pragma region Inversion
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, 4, 4> Matrix<_t, _C, _R>::Inverse(const Matrix<_t, 4, 4>& m)
	{
		Matrix<_t, 4, 4> cpy = m;
		Invert(cpy);
		return cpy;
	}
	// _The real one ;)
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline float Matrix<_t, _C, _R>::Invert(Matrix<_t, 4, 4>& m)
	{
		Matrix<_t, 4, 4> inverse{};
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

		for (MatrixSizeType i = 0; i < 16; i++)
			m.m_data[i] = inverse.m_data[i] * det;

		return det;
	}
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, 4, 4> Matrix<_t, _C, _R>::Inverted() const
	{
		return Inverse(*this);
	}
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline float Matrix<_t, _C, _R>::Invert()
	{
		return Invert(*this);
	}
#pragma endregion

	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline _t Matrix<_t, _C, _R>::Sum() const
	{
		_t sum{};
		OnEachElement([&sum](const _t& element)
			{
				sum += element;
			});

		return sum;
	}
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline _t Matrix<_t, _C, _R>::Sum(const Matrix& matrix)
	{
		return matrix.Sum();
	}
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline bool Matrix<_t, _C, _R>::IsNull() const
	{
		for (MatrixSizeType r{}; r < _R; ++r)
			for (MatrixSizeType c{}; c < _C; ++c)
			{
				if (Element(c, r) != (_t)0)
					return false;
			}

		return true;
	}
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline bool Matrix<_t, _C, _R>::IsNull(const Matrix& matrix)
	{
		return matrix.IsNull();
	}
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, _C, _R> Matrix<_t, _C, _R>::Identity()
	{
		Matrix<_t, _C, _R> result{};
		for (MatrixSizeType r{}; r < _R; ++r)
			for (MatrixSizeType c{}; c < _C; ++c)
			{
				MatrixSizeType i = c + (_C * r);
				if (r == c) result.m_data[i] = (_t)1;
			}

		return result;
	}

#pragma region _Transformation

	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, 3, 3> Matrix<_t, _C, _R>::MakeTranslation(const vector<_t, 2>& t)
	{
		return
		{
			1, 0, 0,
			0, 1, 0,
			t.x, t.y, 1
		};
	}
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, 4, 4> Matrix<_t, _C, _R>::MakeTranslation(const vector<_t, 3>& t)
	{
		return
		{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			t.x, t.y, t.z, 1
		};
	}
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, 3, 3> Matrix<_t, _C, _R>::MakeRotation(float angle)
	{
		return
		{
			cosf(angle), -sinf(angle), 0,
			sinf(angle), cosf(angle), 0,
			0, 0, 1
		};
	}
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, 4, 4> Matrix<_t, _C, _R>::MakeRotation(const vector<_t, 3>& axis, float angle)
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
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, 3, 3> Matrix<_t, _C, _R>::MakeScale(const vector<_t, 2>& s)
	{
		return
		{
			s.x, 0, 0,
			0, s.y, 0,
			0, 0, 1
		};
	}
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, 4, 4> Matrix<_t, _C, _R>::MakeScale(const vector<_t, 3>& s)
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
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	template <typename Other_T>
	Matrix<_t, _C, _R>::Matrix(const Matrix<Other_T, _C, _R>& other)
	{
		OnEachElement([&other](_t& element, MatrixSizeType idx) { element = static_cast<_t>(other.Element(idx)); });
	}
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	template <typename Other_T>
	Matrix<_t, _C, _R>::Matrix(Matrix<Other_T, _C, _R>&& other)
	{
		OnEachElement([&other](_t& element, MatrixSizeType idx) { element = static_cast<_t>(other.Element(idx)); });
	}

	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline const vector<_t, _C>& Matrix<_t, _C, _R>::operator[](MatrixSizeType r) const
	{
		FLX_ASSERT(r < _R);
		return this->rows[r];
	}

	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline vector<_t, _C>& Matrix<_t, _C, _R>::operator[](MatrixSizeType r)
	{
		FLX_ASSERT(r < _R);
		return this->rows[r];
	}

	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline const vector<_t, _C>& Matrix<_t, _C, _R>::Row(MatrixSizeType r) const
	{
		FLX_ASSERT(r < _R);
		return this->rows[r];
	}

	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline vector<_t, _C>& Matrix<_t, _C, _R>::Row(MatrixSizeType r)
	{
		FLX_ASSERT(r < _R);
		return this->rows[r];
	}

	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline vector<_t, _R> Matrix<_t, _C, _R>::Collumn(MatrixSizeType c) const
	{
		FLX_ASSERT(c < _C);
		vector<_t, _R> collumn{};
		for (MatrixSizeType r{}; r < _R; ++r)
			collumn[r] = this->rows[r][c];

		return collumn;
	}

	// Data Access:
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline _t& Matrix<_t, _C, _R>::Element(MatrixSizeType c, MatrixSizeType r)
	{
		FLX_ASSERT(r < _R&& c < _C);
		return (*this)[r][c];
	}
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline const _t& Matrix<_t, _C, _R>::Element(MatrixSizeType c, MatrixSizeType r) const
	{
		FLX_ASSERT(r < _R&& c < _C);
		return (*this)[r][c];
	}
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline _t& Matrix<_t, _C, _R>::Element(MatrixSizeType idx)
	{
		FLX_ASSERT(idx < _C* _R);
		return Element(idx % _C, idx / _R);
	}
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline const _t& Matrix<_t, _C, _R>::Element(MatrixSizeType idx) const
	{
		FLX_ASSERT(idx < _C* _R);
		return Element(idx % _C, idx / _R);
	}


	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, _C, _R>& Matrix<_t, _C, _R>::operator*=(const float scalar)
	{
		OnEachElement([](_t& element)
			{
				element *= scalar;
			});

		return *this;
	}
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, _C, _R>& Matrix<_t, _C, _R>::operator/=(const float scalar)
	{
		OnEachElement([](_t& element)
			{
				element /= scalar;
			});

		return *this;
	}

	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, _C, _R>& Matrix<_t, _C, _R>::MemberMultiply(const Matrix<_t, _C, _R>& other)
	{
		for (MatrixSizeType i{}; i < _R * _C; ++i)
			this->m_data[i] = other.m_data[i];

		return *this;
	}

	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, _C, _R> Matrix<_t, _C, _R>::MemberMultiply(const Matrix<_t, _C, _R>& a, const Matrix<_t, _C, _R>& b)
	{
		Matrix<_t, _C, _R> res = a;
		res.MemberMultiply(b);
		return res;
	}


	// Operations: Matrix - Scalar
	template <typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, _C, _R> operator*(const Matrix<_t, _C, _R>& a, float b)
	{
		Matrix<_t, _C, _R> result = a;
		result.ForEachElement([](_t& el) { el *= b; });
		return result;
	}
	template <typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, _C, _R> operator/(const Matrix<_t, _C, _R>& a, float b)
	{
		Matrix<_t, _C, _R> result = a;
		result.ForEachElement([](_t& el) { el /= b; });
		return result;
	}
	template <typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, _C, _R> operator*(float a, const Matrix<_t, _C, _R>& b)
	{
		Matrix<_t, _C, _R> result = b;
		result.ForEachElement([](_t& el) { el *= a; });
		return result;
	}
	template <typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, _C, _R> operator/(float a, const Matrix<_t, _C, _R>& b)
	{
		Matrix<_t, _C, _R> result = b;
		result.ForEachElement([](_t& el) { el /= a; });
		return result;
	}


	// Operations: Matrix - Matrix
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, _C, _R> operator+(const Matrix<_t, _C, _R>& a, const Matrix<_t, _C, _R>& b)
	{
		Matrix<_t, _C, _R> result = a;
		result.ForEachElement([=, &result](_t& element, MatrixSizeType idx)
			{
				result.Element(idx) += b.Element(idx);
			});

		return result;
	}
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, _C, _R> operator-(const Matrix<_t, _C, _R>& a, const Matrix<_t, _C, _R>& b)
	{
		Matrix<_t, _C, _R> result = a;
		result.ForEachElement([=, &result](_t& element, MatrixSizeType idx)
			{
				result.Element(idx) -= b.Element(idx);
			});

		return result;
	}

	template <typename _t, MatrixSizeType _C, MatrixSizeType _R, MatrixSizeType _OC, MatrixSizeType _OR>
	inline Matrix<_t, _R, _OR> operator*(const Matrix<_t, _C, _R>& a, const Matrix<_t, _OC, _OR>& b)
	{
		FLX_ASSERT(_C == _OR);

		Matrix<_t, _R, _OC> result{};

		for (MatrixSizeType r{}; r < _R; ++r)
			for (MatrixSizeType c{}; c < _OC; ++c)
			{
				for (MatrixSizeType i = 0; i < _OR; ++i)
				{
					result[r][c] += a[r][i] * b[i][c];
				}
			}

		return result;
	}

	// Operations: Matrix - Vector
	template<typename _t>
	inline vector<_t, 2> operator*(const Matrix<_t, 3, 3>& mat, const vector<_t, 2>& v)
	{
		vector<_t, 3> result{ v.x, v.y, 1.f };
		for (MatrixSizeType c{}; c < 3; ++c)
		{
			for (MatrixSizeType r{}; r < 3; ++r)
				result[c] += mat[r][c] * result[c];
		}
		return { result.x, result.y };
	}
	template<typename _t>
	inline vector<_t, 3u> operator*(const Matrix<_t, 4u, 4u>& mat, const vector<_t, 3u>& v)
	{
		vector<_t, 4u> cpy = { v.x, v.y, v.z, 1.f };
		vector<_t, 4u> res{};
		for (MatrixSizeType c{}; c < 4u; ++c)
			res[c] = vector<_t, 4u>::Dot(mat.Collumn(c), cpy);

		return { res.x, res.y, res.z };
	}

	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, 4, 4> Matrix<_t, _C, _R>::MakeTransformMatrixLH(const vector<_t, 3u>& pos, const vector<_t, 3u>& forward, const vector<_t, 3u>& up)
	{
		vector<_t, 3u> lRight = vector<_t, 3u>::Cross(up, forward);
		vector<_t, 3u> lUp = vector<_t, 3u>::Cross(forward, lRight);
		vector<_t, 3u> lForward = forward;

		return
		{
			lRight.x, lRight.y, lRight.z, 0.0f,
			lUp.x, lUp.y, lUp.z, 0.0f,
			lForward.x, lForward.y, lForward.z, 0.0f,
			pos.x, pos.y, pos.z, 1
		};
	}
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, 4, 4> Matrix<_t, _C, _R>::MakeTransformMatrixRH(const vector<_t, 3u>& pos, const vector<_t, 3u>& forward, const vector<_t, 3u>& up)
	{
		vector<_t, 3u> lRight = vector<_t, 3u>::Cross(forward, up);
		vector<_t, 3u> lUp = vector<_t, 3u>::Cross(lRight, forward);
		vector<_t, 3u> lForward = forward;

		return
		{
			lRight.x, lRight.y, lRight.z, 0.0f,
			lUp.x, lUp.y, lUp.z, 0.0f,
			lForward.x, lForward.y, lForward.z, 0.0f,
			pos.x, pos.y, pos.z, 1
		};
	}

	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, 4u, 4u> Matrix<_t, _C, _R>::MakeViewMatrixLH(const vector<_t, 3u>& pos, const vector<_t, 3u>& forward, const vector<_t, 3u>& up)
	{
		return MakeTransformMatrixLH(pos, forward, up).Inverted();
	}

	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, 4u, 4u> Matrix<_t, _C, _R>::MakeViewMatrixRH(const vector<_t, 3u>& pos, const vector<_t, 3u>& forward, const vector<_t, 3u>& up)
	{
		return MakeTransformMatrixRH(pos, forward, up).Inverted();
	}

	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, 4u, 4u> Matrix<_t, _C, _R>::MakeProjectionMatrixRH(const float fov, const float ar, const float n, const float f)
	{
		float y = 1.0f / tanf(influx::Math::DegreesToRadians(fov) / 2.f);
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

	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_t, 4u, 4u> Matrix<_t, _C, _R>::MakeProjectionMatrixLH(const float fov, const float ar, const float n, const float f)
	{
		float y = 1.0f / tanf(influx::Math::DegreesToRadians(fov) / 2.f);
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

	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline void Matrix<_t, _C, _R>::ForEachElement(std::function<void(_t&)> operation)
	{
		for (MatrixSizeType r{}; r < _R; ++r)
			for (MatrixSizeType c{}; c < _C; ++c)
			{
				MatrixSizeType i = c + (_C * r);
				operation(this->m_data[i]);
			}
	}
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline void Matrix<_t, _C, _R>::ForEachElement(std::function<void(_t&, MatrixSizeType idx)> operation)
	{
		for (MatrixSizeType r{}; r < _R; ++r)
			for (MatrixSizeType c{}; c < _C; ++c)
			{
				MatrixSizeType i = c + (_C * r);
				operation(this->m_data[i], i);
			}
	}
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline void Matrix<_t, _C, _R>::ForEachElement(std::function<void(const _t&)> operation) const
	{
		for (MatrixSizeType r{}; r < _R; ++r)
			for (MatrixSizeType c{}; c < _C; ++c)
			{
				MatrixSizeType i = c + (_C * r);
				operation(this->m_data[i]);
			}
	}
	template<typename _t, MatrixSizeType _C, MatrixSizeType _R>
	inline void Matrix<_t, _C, _R>::ForEachElement(std::function<void(const _t&, MatrixSizeType idx)> operation) const
	{
		for (MatrixSizeType r{}; r < _R; ++r)
			for (MatrixSizeType c{}; c < _C; ++c)
			{
				MatrixSizeType i = c + (_C * r);
				operation(this->m_data[i], i);
			}
	}
}