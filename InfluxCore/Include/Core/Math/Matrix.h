#pragma once

#include "core/math/vector.h"
#include <functional>

#pragma warning(disable : 4201)

namespace influx::math
{
	using matrix_dim_t = size_t;

	namespace detail
	{
		template <typename _t, matrix_dim_t _C, matrix_dim_t _R>
		struct base_matrix
		{
			constexpr matrix_dim_t get_num_collumns() { return _C; };
			constexpr matrix_dim_t get_num_rows() { return _R; };

			union
			{
				struct { _t m_data[_C * _R]; };
				struct { math::vector<_t, _C> m_rows[_R]; };
			};


			template <class... _Init>
			base_matrix(_Init...values) : m_data{ static_cast<_t>(values)... } {}
		};
	}

	template <typename _t, matrix_dim_t _C, matrix_dim_t _R>
	struct matrix : public detail::base_matrix<_t, _C, _R>
	{
	public:
		matrix() = default;
		matrix(const matrix& other) = default;
		matrix(matrix && other) = default;
		matrix& operator=(const matrix & other) = default;
		matrix& operator=(matrix && other) = default;

		template <class... _I>	matrix(_I... values) : detail::base_matrix<_t, _C, _R>(values...) {} // Initializer list
		template <typename _D>	matrix(const matrix<_D, _C, _R>& other); // Typecasting
		template <typename _D>	matrix(matrix<_D, _C, _R>&& other); // Typecasting

		// Data Access:
		const vector<_t, _C>& operator[](matrix_dim_t r) const;
		const vector<_t, _C>& get_row(matrix_dim_t r) const;
		vector<_t, _R> get_collumn(matrix_dim_t c) const;

		vector<_t, _C>& operator[](matrix_dim_t r);
		vector<_t, _C>& get_row(matrix_dim_t r);

		_t& get_element(matrix_dim_t c, matrix_dim_t r);
		_t& get_element(matrix_dim_t idx);

		const _t& get_element(matrix_dim_t c, matrix_dim_t r) const;
		const _t& get_element(matrix_dim_t idx) const;

		// Basic Operations:
		matrix& operator*=(const float scalar);
		matrix& operator/=(const float scalar);
		matrix& operator+=(const matrix& other);
		matrix& operator-=(const matrix& other);

		matrix& member_multiply(const matrix& other);
		static matrix member_multiply(const matrix& a, const matrix& b);

		// Transpose:
		matrix& transpose();
		matrix transposed() const;
		static void transpose(matrix& matrix);
		static matrix transposed(const matrix& matrix);

		// Determinant:
		static float determinant(const matrix<_t, 2u, 2u>& m);
		static float determinant(const matrix<_t, 3u, 3u>& m);
		static float determinant(const matrix<_t, 4u, 4u>& m);
		float determinant() const;

		// Inverse:
		static matrix<_t, 4u, 4u> inverse(const matrix<_t, 4u, 4u>& m);
		static float invert(matrix<_t, 4u, 4u>& m);
		static matrix<_t, 3u, 3u> inverse(const matrix<_t, 3u, 3u>& m);
		static float invert(matrix<_t, 3u, 3u>& m);
		matrix<_t, 4u, 4u> inverted() const;
		float invert();

		// Summation:
		_t get_sum() const;
		static _t get_sum(const matrix& matrix);

		// IsNull:
		bool is_null() const;
		static bool is_null(const matrix& matrix);

		static matrix identity();

		// Transformation:
		static matrix<_t, 3u, 3u> make_rotation(float angle);
		static matrix<_t, 4u, 4u> make_rotation(const vector<_t, 3u>& axis, float angle);
		static matrix<_t, 3u, 3u> make_translation(const vector<_t, 2u>& translation);
		static matrix<_t, 4u, 4u> make_translation(const vector<_t, 3u>& translation);
		static matrix<_t, 3u, 3u> make_scale(const vector<_t, 2u>& scale);
		static matrix<_t, 4u, 4u> make_scale(const vector<_t, 3u>& scale);

		// More...
		static matrix<_t, 4u, 4u> make_transform_LH(const vector<_t, 3u>& pos, const vector<_t, 3u>& forward, const vector<_t, 3u>& up = vector<_t, 3u>::up());
		static matrix<_t, 4u, 4u> make_transform_RH(const vector<_t, 3u>& pos, const vector<_t, 3u>& forward, const vector<_t, 3u>& up = vector<_t, 3u>::up());
		static matrix<_t, 4u, 4u> make_view_LH(const vector<_t, 3u>& pos, const vector<_t, 3u>& forward, const vector<_t, 3u>& up = vector<_t, 3u>::up());
		static matrix<_t, 4u, 4u> make_view_RH(const vector<_t, 3u>& pos, const vector<_t, 3u>& forward, const vector<_t, 3u>& up = vector<_t, 3u>::up());
		static matrix<_t, 4u, 4u> make_projection_LH(const float fov, const float ar, const float n, const float f);
		static matrix<_t, 4u, 4u> make_projection_RH(const float fov, const float ar, const float n, const float f); // Todo: [Orthographic vs Perspective]

		void for_each_element(std::function<void(_t&)> operation);
		void for_each_element(std::function<void(_t&, matrix_dim_t idx)> operation);
		void for_each_element(std::function<void(const _t&)> operation) const;
		void for_each_element(std::function<void(const _t&, matrix_dim_t idx)> operation) const;
	};

	// Matrix - Matrix
	template <typename _t, matrix_dim_t _C, matrix_dim_t _R> matrix<_t, _C, _R> operator+(const matrix<_t, _C, _R>& a, const matrix<_t, _C, _R>& b);
	template <typename _t, matrix_dim_t _C, matrix_dim_t _R> matrix<_t, _C, _R> operator-(const matrix<_t, _C, _R>& a, const matrix<_t, _C, _R>& b);

	template <typename _t, matrix_dim_t _C, matrix_dim_t _R, matrix_dim_t _OC, matrix_dim_t _OR>
	matrix<_t, _R, _OR> operator*(const matrix<_t, _C, _R>& a, const matrix<_t, _OC, _OR>& b);

	// Matrix - Vector
	template<typename _t> math::vector<_t, 2u> operator*(const matrix<_t, 3u, 3u>& mat, const vector<_t, 2u>& v);
	template<typename _t> math::vector<_t, 3u> operator*(const matrix<_t, 4u, 4u>& mat, const vector<_t, 3u>& v);

	// Matrix - scalar
	template <typename _t, matrix_dim_t _C, matrix_dim_t _R> matrix<_t, _C, _C> operator*(const matrix<_t, _C, _R>& a, float b);
	template <typename _t, matrix_dim_t _C, matrix_dim_t _R> matrix<_t, _C, _C> operator/(const matrix<_t, _C, _R>& a, float b);
	template <typename _t, matrix_dim_t _C, matrix_dim_t _R> matrix<_t, _C, _C> operator*(float a, const matrix<_t, _C, _R>& b);
	template <typename _t, matrix_dim_t _C, matrix_dim_t _R> matrix<_t, _C, _C> operator/(float a, const matrix<_t, _C, _R>& b);

	// Aliases:
	using matrix2x2f = matrix<float, 2u, 2u>;
	using matrix2x2d = matrix<double, 2u, 2u>;
	using matrix2x2i = matrix<int, 2u, 2u>;
	using matrix2x2ui = matrix<uint32_t, 2u, 2u>;

	using matrix3x3f = matrix<float, 3u, 3u>;
	using matrix3x3d = matrix<double, 3u, 3u>;
	using matrix3x3i = matrix<int, 3u, 3u>;
	using matrix3x3ui = matrix<uint32_t, 3u, 3u>;

	using matrix4x4f = matrix<float, 4u, 4u>;
	using matrix4x4d = matrix<double, 4u, 4u>;
	using matrix4x4i = matrix<int, 4u, 4u>;
	using matrix4x4ui = matrix<uint32_t, 4u, 4u>;
}

#include "Matrix.inl"

#pragma warning(default : 4201)