#include "Matrix.h"

#define ___CORE_MATRIX_USE_CORE_ 1
#if ___CORE_MATRIX_USE_CORE_
#include "Core/Assert.h"
#include "Core/Math/Math.h"
#else
#include <cassert>
#define FLX_ASSERT(expr) assert(expr)

namespace Influx::Math
{
	template <typename _T>
	constexpr inline _T DegreesToRadians(_T degrees)
	{
		return degrees * (3.1415 / 180);
	}
}


#endif

#include <cmath>

namespace Influx::Math
{
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline float Matrix<_T, _C, _R>::Determinant(const Matrix<_T, 2, 2>& m)
	{
		return m[0][0] * m[1][1] - m[0][1] * m[1][0];
	}

	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline float Matrix<_T, _C, _R>::Determinant(const Matrix<_T, 3, 3>& m)
	{
		// From wikipedia
		return m[0][0] * m[1][1] * m[2][2] +
			m[0][1] * m[1][2] * m[2][0] +
			m[0][2] * m[1][0] * m[2][1] -
			m[0][2] * m[1][1] * m[2][0] -
			m[0][1] * m[1][0] * m[2][2] -
			m[0][0] * m[1][2] * m[2][1];
	}
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline float Matrix<_T, _C, _R>::Determinant(const Matrix<_T, 4, 4>& m)
	{

	}

	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline float Matrix<_T, _C, _R>::Determinant() const
	{
		return Determinant(*this);
	}

#pragma region Inversion
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, 4, 4> Matrix<_T, _C, _R>::Inverse(const Matrix<_T, 4, 4>& m)
	{
		Matrix<_T, 4, 4> cpy = m;
		Invert(cpy);
		return cpy;
	}
	// _The real one ;)
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline float Matrix<_T, _C, _R>::Invert(Matrix<_T, 4, 4>& m)
	{
		Matrix<_T, 4, 4> inverse{};
		float det{};

		inverse.data[0] = m.data[5] * m.data[10] * m.data[15] -
			m.data[5] * m.data[11] * m.data[14] -
			m.data[9] * m.data[6] * m.data[15] +
			m.data[9] * m.data[7] * m.data[14] +
			m.data[13] * m.data[6] * m.data[11] -
			m.data[13] * m.data[7] * m.data[10];

		inverse.data[4] = -m.data[4] * m.data[10] * m.data[15] +
			m.data[4] * m.data[11] * m.data[14] +
			m.data[8] * m.data[6] * m.data[15] -
			m.data[8] * m.data[7] * m.data[14] -
			m.data[12] * m.data[6] * m.data[11] +
			m.data[12] * m.data[7] * m.data[10];

		inverse.data[8] = m.data[4] * m.data[9] * m.data[15] -
			m.data[4] * m.data[11] * m.data[13] -
			m.data[8] * m.data[5] * m.data[15] +
			m.data[8] * m.data[7] * m.data[13] +
			m.data[12] * m.data[5] * m.data[11] -
			m.data[12] * m.data[7] * m.data[9];

		inverse.data[12] = -m.data[4] * m.data[9] * m.data[14] +
			m.data[4] * m.data[10] * m.data[13] +
			m.data[8] * m.data[5] * m.data[14] -
			m.data[8] * m.data[6] * m.data[13] -
			m.data[12] * m.data[5] * m.data[10] +
			m.data[12] * m.data[6] * m.data[9];

		inverse.data[1] = -m.data[1] * m.data[10] * m.data[15] +
			m.data[1] * m.data[11] * m.data[14] +
			m.data[9] * m.data[2] * m.data[15] -
			m.data[9] * m.data[3] * m.data[14] -
			m.data[13] * m.data[2] * m.data[11] +
			m.data[13] * m.data[3] * m.data[10];

		inverse.data[5] = m.data[0] * m.data[10] * m.data[15] -
			m.data[0] * m.data[11] * m.data[14] -
			m.data[8] * m.data[2] * m.data[15] +
			m.data[8] * m.data[3] * m.data[14] +
			m.data[12] * m.data[2] * m.data[11] -
			m.data[12] * m.data[3] * m.data[10];

		inverse.data[9] = -m.data[0] * m.data[9] * m.data[15] +
			m.data[0] * m.data[11] * m.data[13] +
			m.data[8] * m.data[1] * m.data[15] -
			m.data[8] * m.data[3] * m.data[13] -
			m.data[12] * m.data[1] * m.data[11] +
			m.data[12] * m.data[3] * m.data[9];

		inverse.data[13] = m.data[0] * m.data[9] * m.data[14] -
			m.data[0] * m.data[10] * m.data[13] -
			m.data[8] * m.data[1] * m.data[14] +
			m.data[8] * m.data[2] * m.data[13] +
			m.data[12] * m.data[1] * m.data[10] -
			m.data[12] * m.data[2] * m.data[9];

		inverse.data[2] = m.data[1] * m.data[6] * m.data[15] -
			m.data[1] * m.data[7] * m.data[14] -
			m.data[5] * m.data[2] * m.data[15] +
			m.data[5] * m.data[3] * m.data[14] +
			m.data[13] * m.data[2] * m.data[7] -
			m.data[13] * m.data[3] * m.data[6];

		inverse.data[6] = -m.data[0] * m.data[6] * m.data[15] +
			m.data[0] * m.data[7] * m.data[14] +
			m.data[4] * m.data[2] * m.data[15] -
			m.data[4] * m.data[3] * m.data[14] -
			m.data[12] * m.data[2] * m.data[7] +
			m.data[12] * m.data[3] * m.data[6];

		inverse.data[10] = m.data[0] * m.data[5] * m.data[15] -
			m.data[0] * m.data[7] * m.data[13] -
			m.data[4] * m.data[1] * m.data[15] +
			m.data[4] * m.data[3] * m.data[13] +
			m.data[12] * m.data[1] * m.data[7] -
			m.data[12] * m.data[3] * m.data[5];

		inverse.data[14] = -m.data[0] * m.data[5] * m.data[14] +
			m.data[0] * m.data[6] * m.data[13] +
			m.data[4] * m.data[1] * m.data[14] -
			m.data[4] * m.data[2] * m.data[13] -
			m.data[12] * m.data[1] * m.data[6] +
			m.data[12] * m.data[2] * m.data[5];

		inverse.data[3] = -m.data[1] * m.data[6] * m.data[11] +
			m.data[1] * m.data[7] * m.data[10] +
			m.data[5] * m.data[2] * m.data[11] -
			m.data[5] * m.data[3] * m.data[10] -
			m.data[9] * m.data[2] * m.data[7] +
			m.data[9] * m.data[3] * m.data[6];

		inverse.data[7] = m.data[0] * m.data[6] * m.data[11] -
			m.data[0] * m.data[7] * m.data[10] -
			m.data[4] * m.data[2] * m.data[11] +
			m.data[4] * m.data[3] * m.data[10] +
			m.data[8] * m.data[2] * m.data[7] -
			m.data[8] * m.data[3] * m.data[6];

		inverse.data[11] = -m.data[0] * m.data[5] * m.data[11] +
			m.data[0] * m.data[7] * m.data[9] +
			m.data[4] * m.data[1] * m.data[11] -
			m.data[4] * m.data[3] * m.data[9] -
			m.data[8] * m.data[1] * m.data[7] +
			m.data[8] * m.data[3] * m.data[5];

		inverse.data[15] = m.data[0] * m.data[5] * m.data[10] -
			m.data[0] * m.data[6] * m.data[9] -
			m.data[4] * m.data[1] * m.data[10] +
			m.data[4] * m.data[2] * m.data[9] +
			m.data[8] * m.data[1] * m.data[6] -
			m.data[8] * m.data[2] * m.data[5];

		det = m.data[0] * inverse.data[0] + m.data[1] * inverse.data[4] + m.data[2] * inverse.data[8] + m.data[3] * inverse.data[12];
		det = 1.0f / det;

		for (MatrixSizeType i = 0; i < 16; i++)
			m.data[i] = inverse.data[i] * det;

		return det;
	}
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, 4, 4> Matrix<_T, _C, _R>::Inverted() const
	{
		return Inverse(*this);
	}
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline float Matrix<_T, _C, _R>::Invert()
	{
		return Invert(*this);
	}
#pragma endregion

	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline _T Matrix<_T, _C, _R>::Sum() const
	{
		_T sum{};
		OnEachElement([&sum](const _T& element)
			{
				sum += element;
			});

		return sum;
	}
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline _T Matrix<_T, _C, _R>::Sum(const Matrix& matrix)
	{
		return matrix.Sum();
	}
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline bool Matrix<_T, _C, _R>::IsNull() const
	{
		for (MatrixSizeType r{}; r < _R; ++r)
			for (MatrixSizeType c{}; c < _C; ++c)
			{
				if (Element(c, r) != (_T)0)
					return false;
			}

		return true;
	}
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline bool Matrix<_T, _C, _R>::IsNull(const Matrix& matrix)
	{
		return matrix.IsNull();
	}
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, _C, _R> Matrix<_T, _C, _R>::Identity()
	{
		Matrix<_T, _C, _R> result{};
		for (MatrixSizeType r{}; r < _R; ++r)
			for (MatrixSizeType c{}; c < _C; ++c)
			{
				MatrixSizeType i = c + (_C * r);
				if (r == c) result.data[i] = (_T)1;
			}

		return result;
	}

#pragma region _Transformation

	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, 3, 3> Matrix<_T, _C, _R>::MakeTranslation(const Vector<_T, 2>& t)
	{
		return
		{
			1, 0, 0,
			0, 1, 0,
			t.x, t.y, 1
		};
	}
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, 4, 4> Matrix<_T, _C, _R>::MakeTranslation(const Vector<_T, 3>& t)
	{
		return
		{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			t.x, t.y, t.z, 1
		};
	}
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, 3, 3> Matrix<_T, _C, _R>::MakeRotation(float angle)
	{
		return
		{
			cosf(angle), -sinf(angle), 0,
			sinf(angle), cosf(angle), 0,
			0, 0, 1
		};
	}
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, 4, 4> Matrix<_T, _C, _R>::MakeRotation(const Vector<_T, 3>& axis, float angle)
	{
		// http://www.fastgraph.com/makegames/3drotation/3dsrce.html

		float x{ axis.x }, y{ axis.y }, z{ axis.z };
		Vector<_T, 3> norm = axis.Normalized();
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
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, 3, 3> Matrix<_T, _C, _R>::MakeScale(const Vector<_T, 2>& s)
	{
		return
		{
			s.x, 0, 0,
			0, s.y, 0,
			0, 0, 1
		};
	}
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, 4, 4> Matrix<_T, _C, _R>::MakeScale(const Vector<_T, 3>& s)
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
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	template <typename Other_T>
	Matrix<_T, _C, _R>::Matrix(const Matrix<Other_T, _C, _R>& other)
	{
		OnEachElement([&other](_T& element, MatrixSizeType idx) { element = static_cast<_T>(other.Element(idx)); });
	}
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	template <typename Other_T>
	Matrix<_T, _C, _R>::Matrix(Matrix<Other_T, _C, _R>&& other)
	{
		OnEachElement([&other](_T& element, MatrixSizeType idx) { element = static_cast<_T>(other.Element(idx)); });
	}

	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline const Vector<_T, _C>& Matrix<_T, _C, _R>::operator[](MatrixSizeType r) const
	{
		FLX_ASSERT(r < _R);
		return this->rows[r];
	}

	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Vector<_T, _C>& Matrix<_T, _C, _R>::operator[](MatrixSizeType r)
	{
		FLX_ASSERT(r < _R);
		return this->rows[r];
	}

	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline const Vector<_T, _C>& Matrix<_T, _C, _R>::Row(MatrixSizeType r) const
	{
		FLX_ASSERT(r < _R);
		return this->rows[r];
	}

	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Vector<_T, _C>& Matrix<_T, _C, _R>::Row(MatrixSizeType r)
	{
		FLX_ASSERT(r < _R);
		return this->rows[r];
	}

	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Vector<_T, _R> Matrix<_T, _C, _R>::Collumn(MatrixSizeType c) const
	{
		FLX_ASSERT(c < _C);
		Vector<_T, _R> collumn{};
		for (MatrixSizeType r{}; r < _R; ++r)
			collumn[r] = this->rows[r][c];

		return collumn;
	}

	// Data Access:
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline _T& Matrix<_T, _C, _R>::Element(MatrixSizeType c, MatrixSizeType r)
	{
		FLX_ASSERT(r < _R&& c < _C);
		return (*this)[r][c];
	}
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline const _T& Matrix<_T, _C, _R>::Element(MatrixSizeType c, MatrixSizeType r) const
	{
		FLX_ASSERT(r < _R&& c < _C);
		return (*this)[r][c];
	}
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline _T& Matrix<_T, _C, _R>::Element(MatrixSizeType idx)
	{
		FLX_ASSERT(idx < _C* _R);
		return Element(idx % _C, idx / _R);
	}
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline const _T& Matrix<_T, _C, _R>::Element(MatrixSizeType idx) const
	{
		FLX_ASSERT(idx < _C* _R);
		return Element(idx % _C, idx / _R);
	}


	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, _C, _R>& Matrix<_T, _C, _R>::operator*=(const float scalar)
	{
		OnEachElement([](_T& element)
			{
				element *= scalar;
			});

		return *this;
	}
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, _C, _R>& Matrix<_T, _C, _R>::operator/=(const float scalar)
	{
		OnEachElement([](_T& element)
			{
				element /= scalar;
			});

		return *this;
	}

	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, _C, _R>& Matrix<_T, _C, _R>::MemberMultiply(const Matrix<_T, _C, _R>& other)
	{
		for (MatrixSizeType i{}; i < _R * _C; ++i)
			this->data[i] = other.data[i];

		return *this;
	}

	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, _C, _R> Matrix<_T, _C, _R>::MemberMultiply(const Matrix<_T, _C, _R>& a, const Matrix<_T, _C, _R>& b)
	{
		Matrix<_T, _C, _R> res = a;
		res.MemberMultiply(b);
		return res;
	}


	// Operations: Matrix - Scalar
	template <typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, _C, _R> operator*(const Matrix<_T, _C, _R>& a, float b)
	{
		Matrix<_T, _C, _R> result = a;
		result.ForEachElement([](_T& el) { el *= b; });
		return result;
	}
	template <typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, _C, _R> operator/(const Matrix<_T, _C, _R>& a, float b)
	{
		Matrix<_T, _C, _R> result = a;
		result.ForEachElement([](_T& el) { el /= b; });
		return result;
	}
	template <typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, _C, _R> operator*(float a, const Matrix<_T, _C, _R>& b)
	{
		Matrix<_T, _C, _R> result = b;
		result.ForEachElement([](_T& el) { el *= a; });
		return result;
	}
	template <typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, _C, _R> operator/(float a, const Matrix<_T, _C, _R>& b)
	{
		Matrix<_T, _C, _R> result = b;
		result.ForEachElement([](_T& el) { el /= a; });
		return result;
	}


	// Operations: Matrix - Matrix
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, _C, _R> operator+(const Matrix<_T, _C, _R>& a, const Matrix<_T, _C, _R>& b)
	{
		Matrix<_T, _C, _R> result = a;
		result.ForEachElement([=, &result](_T& element, MatrixSizeType idx)
			{
				result.Element(idx) += b.Element(idx);
			});

		return result;
	}
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, _C, _R> operator-(const Matrix<_T, _C, _R>& a, const Matrix<_T, _C, _R>& b)
	{
		Matrix<_T, _C, _R> result = a;
		result.ForEachElement([=, &result](_T& element, MatrixSizeType idx)
			{
				result.Element(idx) -= b.Element(idx);
			});

		return result;
	}
	template <typename _T, MatrixSizeType _C, MatrixSizeType _R, MatrixSizeType _OC, MatrixSizeType _OR>
	inline Matrix<_T, _R, _OR> operator*(const Matrix<_T, _C, _R>& a, const Matrix<_T, _OC, _OR>& b)
	{
		FLX_ASSERT(_C == _OR); // Amount of collumns must be equal
		Matrix<_T, _R, _OC> result{};

		for (MatrixSizeType r{}; r < _R; ++r)
			for (MatrixSizeType c{}; c < _OC; ++c)
				result[r][c] = Vector<_T, _OC>::Dot(a.Row(r), b.Collumn(c));


		return result;
	}

	// Operations: Matrix - Vector
	template<typename _T>
	inline Vector<_T, 2> operator*(const Matrix<_T, 3, 3>& mat, const Vector<_T, 2>& v)
	{
		Vector<_T, 3> result{ v.x, v.y, 1.f };
		for (MatrixSizeType c{}; c < 3; ++c)
		{
			for (MatrixSizeType r{}; r < 3; ++r)
				result[c] += mat[r][c] * result[c];
		}
		return { result.x, result.y };
	}
	template<typename _T>
	inline Vector<_T, 3u> operator*(const Matrix<_T, 4u, 4u>& mat, const Vector<_T, 3u>& v)
	{
		Vector<_T, 4u> cpy = { v.x, v.y, v.z, 1.f };
		Vector<_T, 4u> res{};
		for (MatrixSizeType c{}; c < 4u; ++c)
			res[c] = Vector<_T, 4u>::Dot(mat.Collumn(c), cpy);

		return { res.x, res.y, res.z };
	}

	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, 4, 4> Matrix<_T, _C, _R>::MakeTransformMatrixLH(const Vector<_T, 3u>& pos, const Vector<_T, 3u>& forward, const Vector<_T, 3u>& up)
	{
		Vector<_T, 3u> lRight = Vector<_T, 3u>::Cross(up, forward);
		Vector<_T, 3u> lUp = Vector<_T, 3u>::Cross(forward, lRight);
		Vector<_T, 3u> lForward = forward;

		return
		{
			lRight.x, lRight.y, lRight.z, 0.0f,
			lUp.x, lUp.y, lUp.z, 0.0f,
			lForward.x, lForward.y, lForward.z, 0.0f,
			pos.x, pos.y, pos.z, 1
		};
	}
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, 4, 4> Matrix<_T, _C, _R>::MakeTransformMatrixRH(const Vector<_T, 3u>& pos, const Vector<_T, 3u>& forward, const Vector<_T, 3u>& up)
	{
		Vector<_T, 3u> lRight = Vector<_T, 3u>::Cross(forward, up);
		Vector<_T, 3u> lUp = Vector<_T, 3u>::Cross(lRight, forward);
		Vector<_T, 3u> lForward = forward;

		return
		{
			lRight.x, lRight.y, lRight.z, 0.0f,
			lUp.x, lUp.y, lUp.z, 0.0f,
			lForward.x, lForward.y, lForward.z, 0.0f,
			pos.x, pos.y, pos.z, 1
		};
	}
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, 4u, 4u> Matrix<_T, _C, _R>::MakeViewMatrixLH(const Vector<_T, 3u>& pos, const Vector<_T, 3u>& forward, const Vector<_T, 3u>& up)
	{
		return MakeTransformMatrixLH(pos, forward, up).Inverted();
	}
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, 4u, 4u> Matrix<_T, _C, _R>::MakeViewMatrixRH(const Vector<_T, 3u>& pos, const Vector<_T, 3u>& forward, const Vector<_T, 3u>& up)
	{
		return MakeTransformMatrixRH(pos, forward, up).Inverted();
	}

	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, 4u, 4u> Matrix<_T, _C, _R>::MakeProjectionMatrixRH(const float fov, const float ar, const float n, const float f)
	{
		float y = 1.0f / tanf(Influx::Math::DegreesToRadians(fov) / 2.f);
		float x = y / ar;
		float intv = n - f;

		return
		{
			(_T)x, (_T)0, (_T)0, (_T)0,
			(_T)0, (_T)y, (_T)0, (_T)0,
			(_T)0, (_T)0, (_T)f / intv, (_T)-1,
			(_T)0, (_T)0, (_T)(f * n) / intv,	(_T)0
		};
	}

	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline Matrix<_T, 4u, 4u> Matrix<_T, _C, _R>::MakeProjectionMatrixLH(const float fov, const float ar, const float n, const float f)
	{
		float y = 1.0f / tanf(Influx::Math::DegreesToRadians(fov) / 2.f);
		float x = y / ar;
		float intv = f - n;

		return
		{
			(_T)x, (_T)0, (_T)0, (_T)0,
			(_T)0, (_T)y, (_T)0, (_T)0,
			(_T)0, (_T)0, (_T)f / intv, (_T)1,
			(_T)0, (_T)0, static_cast<_T>(-(f * n) / intv), (_T)0
		};
	}

	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline void Matrix<_T, _C, _R>::ForEachElement(std::function<void(_T&)> operation)
	{
		for (MatrixSizeType r{}; r < _R; ++r)
			for (MatrixSizeType c{}; c < _C; ++c)
			{
				MatrixSizeType i = c + (_C * r);
				operation(this->data[i]);
			}
	}
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline void Matrix<_T, _C, _R>::ForEachElement(std::function<void(_T&, MatrixSizeType idx)> operation)
	{
		for (MatrixSizeType r{}; r < _R; ++r)
			for (MatrixSizeType c{}; c < _C; ++c)
			{
				MatrixSizeType i = c + (_C * r);
				operation(this->data[i], i);
			}
	}
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline void Matrix<_T, _C, _R>::ForEachElement(std::function<void(const _T&)> operation) const
	{
		for (MatrixSizeType r{}; r < _R; ++r)
			for (MatrixSizeType c{}; c < _C; ++c)
			{
				MatrixSizeType i = c + (_C * r);
				operation(this->data[i]);
			}
	}
	template<typename _T, MatrixSizeType _C, MatrixSizeType _R>
	inline void Matrix<_T, _C, _R>::ForEachElement(std::function<void(const _T&, MatrixSizeType idx)> operation) const
	{
		for (MatrixSizeType r{}; r < _R; ++r)
			for (MatrixSizeType c{}; c < _C; ++c)
			{
				MatrixSizeType i = c + (_C * r);
				operation(this->data[i], i);
			}
	}
}