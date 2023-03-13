#pragma once

#include <functional>

#include "Vector.h"

#pragma warning(disable : 4201)

namespace Influx::Math
{
	using MatrixSizeType = size_t;

	namespace Internal
	{
		template <typename _T, MatrixSizeType _C, MatrixSizeType _R>
		struct MatrixBase
		{
			constexpr MatrixSizeType GetNumCollumns() { return _C; };
			constexpr MatrixSizeType GetNumRows() { return _R; };

			union
			{
				struct { _T data[_C * _R]; };
				struct { Vector<_T, _C> rows[_R]; };
			};


			template <class... _Init>
			MatrixBase(_Init...values) : data{ static_cast<_T>(values)... } {}
		};
	}

	template <typename _T, MatrixSizeType _C, MatrixSizeType _R>
	struct Matrix : public Internal::MatrixBase<_T, _C, _R>
	{
	public:
		Matrix() = default;
		Matrix(const Matrix& other) = default;
		Matrix(Matrix && other) = default;
		Matrix& operator=(const Matrix & other) = default;
		Matrix& operator=(Matrix && other) = default;

		template <class... _I>	Matrix(_I... values) : Internal::MatrixBase<_T, _C, _R>(values...) {} // Initializer list
		template <typename _D>	Matrix(const Matrix<_D, _C, _R>& other); // Typecasting
		template <typename _D>	Matrix(Matrix<_D, _C, _R>&& other); // Typecasting

		// Data Access:
		const Vector<_T, _C>& operator[](MatrixSizeType r) const;
		const Vector<_T, _C>& Row(MatrixSizeType r) const;
		Vector<_T, _R> Collumn(MatrixSizeType c) const;

		Vector<_T, _C>& operator[](MatrixSizeType r);
		Vector<_T, _C>& Row(MatrixSizeType r);

		_T& Element(MatrixSizeType c, MatrixSizeType r);
		_T& Element(MatrixSizeType idx);

		const _T& Element(MatrixSizeType c, MatrixSizeType r) const;
		const _T& Element(MatrixSizeType idx) const;

		// Basic Operations:
		Matrix& operator*=(const float scalar);
		Matrix& operator/=(const float scalar);
		Matrix& operator+=(const Matrix& other);
		Matrix& operator-=(const Matrix& other);

		Matrix& MemberMultiply(const Matrix& other);
		static Matrix MemberMultiply(const Matrix& a, const Matrix& b);

		// Transpose:
		Matrix& Transpose();
		Matrix Transposed() const;
		static void Transpose(Matrix& matrix);
		static Matrix Transposed(const Matrix& matrix);

		// Determinant:
		static float Determinant(const Matrix<_T, 2u, 2u>& m);
		static float Determinant(const Matrix<_T, 3u, 3u>& m);
		static float Determinant(const Matrix<_T, 4u, 4u>& m);
		float Determinant() const;

		// Inverse:
		static Matrix<_T, 4u, 4u> Inverse(const Matrix<_T, 4u, 4u>& m);
		static float Invert(Matrix<_T, 4u, 4u>& m);
		static Matrix<_T, 3u, 3u> Inverse(const Matrix<_T, 3u, 3u>& m);
		static float Invert(Matrix<_T, 3u, 3u>& m);
		Matrix<_T, 4u, 4u> Inverted() const;
		float Invert();

		// Summation:
		_T Sum() const;
		static _T Sum(const Matrix& matrix);

		// IsNull:
		bool IsNull() const;
		static bool IsNull(const Matrix& matrix);

		static Matrix Identity();

		// Transformation:
		static Matrix<_T, 3u, 3u> MakeRotation(float angle);
		static Matrix<_T, 4u, 4u> MakeRotation(const Vector<_T, 3u>& axis, float angle);
		static Matrix<_T, 3u, 3u> MakeTranslation(const Vector<_T, 2u>& translation);
		static Matrix<_T, 4u, 4u> MakeTranslation(const Vector<_T, 3u>& translation);
		static Matrix<_T, 3u, 3u> MakeScale(const Vector<_T, 2u>& scale);
		static Matrix<_T, 4u, 4u> MakeScale(const Vector<_T, 3u>& scale);

		// More...
		static Matrix<_T, 4u, 4u> MakeTransformMatrixLH(const Vector<_T, 3u>& pos, const Vector<_T, 3u>& forward, const Vector<_T, 3u>& up);
		static Matrix<_T, 4u, 4u> MakeTransformMatrixRH(const Vector<_T, 3u>& pos, const Vector<_T, 3u>& forward, const Vector<_T, 3u>& up);
		static Matrix<_T, 4u, 4u> MakeViewMatrixLH(const Vector<_T, 3u>& pos, const Vector<_T, 3u>& forward, const Vector<_T, 3u>& up);
		static Matrix<_T, 4u, 4u> MakeViewMatrixRH(const Vector<_T, 3u>& pos, const Vector<_T, 3u>& forward, const Vector<_T, 3u>& up);
		static Matrix<_T, 4u, 4u> MakeProjectionMatrixLH(const float fov, const float ar, const float n, const float f);
		static Matrix<_T, 4u, 4u> MakeProjectionMatrixRH(const float fov, const float ar, const float n, const float f); // Todo: [Orthographic vs Perspective]

		void ForEachElement(std::function<void(_T&)> operation);
		void ForEachElement(std::function<void(_T&, MatrixSizeType idx)> operation);
		void ForEachElement(std::function<void(const _T&)> operation) const;
		void ForEachElement(std::function<void(const _T&, MatrixSizeType idx)> operation) const;
	};

	// Matrix - Matrix
	template <typename _T, MatrixSizeType _C, MatrixSizeType _R> Matrix<_T, _C, _R> operator+(const Matrix<_T, _C, _R>& a, const Matrix<_T, _C, _R>& b);
	template <typename _T, MatrixSizeType _C, MatrixSizeType _R> Matrix<_T, _C, _R> operator-(const Matrix<_T, _C, _R>& a, const Matrix<_T, _C, _R>& b);

	template <typename _T, MatrixSizeType _C, MatrixSizeType _R, MatrixSizeType _OC, MatrixSizeType _OR>
	Matrix<_T, _R, _OR> operator*(const Matrix<_T, _C, _R>& a, const Matrix<_T, _OC, _OR>& b);

	// Matrix - Vector
	template<typename _T> Vector<_T, 2u> operator*(const Matrix<_T, 3u, 3u>& mat, const Vector<_T, 2u>& v);
	template<typename _T> Vector<_T, 3u> operator*(const Matrix<_T, 4u, 4u>& mat, const Vector<_T, 3u>& v);

	// Matrix - scalar
	template <typename _T, MatrixSizeType _C, MatrixSizeType _R> Matrix<_T, _C, _C> operator*(const Matrix<_T, _C, _R>& a, float b);
	template <typename _T, MatrixSizeType _C, MatrixSizeType _R> Matrix<_T, _C, _C> operator/(const Matrix<_T, _C, _R>& a, float b);
	template <typename _T, MatrixSizeType _C, MatrixSizeType _R> Matrix<_T, _C, _C> operator*(float a, const Matrix<_T, _C, _R>& b);
	template <typename _T, MatrixSizeType _C, MatrixSizeType _R> Matrix<_T, _C, _C> operator/(float a, const Matrix<_T, _C, _R>& b);

	// Aliases:
	using Matrix2x2f = Matrix<float, 2u, 2u>;
	using Matrix2x2d = Matrix<double, 2u, 2u>;
	using Matrix2x2i = Matrix<int, 2u, 2u>;
	using Matrix2x2ui = Matrix<uint32_t, 2u, 2u>;

	using Matrix3x3f = Matrix<float, 3u, 3u>;
	using Matrix3x3d = Matrix<double, 3u, 3u>;
	using Matrix3x3i = Matrix<int, 3u, 3u>;
	using Matrix3x3ui = Matrix<uint32_t, 3u, 3u>;

	using Matrix4x4f = Matrix<float, 4u, 4u>;
	using Matrix4x4d = Matrix<double, 4u, 4u>;
	using Matrix4x4i = Matrix<int, 4u, 4u>;
	using Matrix4x4ui = Matrix<uint32_t, 4u, 4u>;
}

#include "Matrix.inl"

#pragma warning(default : 4201)