#pragma once

#include "core/basetypes.h"
#include "core/math/vector.h"

#pragma warning(disable : 4201)

namespace influx::math
{
	using matsize = uint32;
	#define INFLUX_MATRIX_COLUMN_MAJOR 1

	template <typename _t, matsize _x, matsize _y>
	struct matrix final
	{
	private:
		using value_type = _t;
		using row 		= vector<_t, _x>;
		using column 	= vector<_t, _y>;
		using vector3 	= vector<_t, 3u>;
		using vector4 	= vector<_t, 4u>;

		static constexpr matsize k_num_columns = _x;
		static constexpr matsize k_num_rows = _y;
		static constexpr matsize k_num_elements = k_num_columns * k_num_rows;
		union
		{
			struct { value_type m_data[_x * _y]; };
			struct { row m_rows[k_num_columns]; };
		};
		
	public:
		// constructors
		matrix() = default;
		matrix(const matrix&) = default;
		matrix(matrix&&) = default;
		matrix& operator=(const matrix&) = default;
		matrix& operator=(matrix&&) = default;

		// typecasting
		template <typename _o> matrix(const matrix<_o, _x, _y>& other); 
		template <typename _o> matrix(matrix<_o, _x, _y>&& other);

		template <class... _init>
		matrix(_init...values) : m_data{ static_cast<_t>(values)... } {}

		static constexpr matsize get_num_collumns() { return k_num_columns; };
		static constexpr matsize get_num_rows() { return k_num_rows; };

		// data access
		const row& operator[](matsize index) const;
		row& operator[](matsize index);
		const row& get_row(matsize index) const;
		row& get_row(matsize index);
		column get_column(matsize index) const;
		void set_column(matsize index, const column&);

		inline void set_row(matsize r_index, const vector<_t, k_num_columns>& row)
		{
			influx_assert(r_index < k_num_rows);
			for (matsize c{}; c < k_num_columns; ++c)
			{
				const uint32 index = (r_index * k_num_columns) + c;
				this->m_data[index] = row[c];
			}
		}

		_t& get_element(matsize c, matsize r);
		_t& get_element(matsize idx);

		const _t& get_element(matsize c, matsize r) const;
		const _t& get_element(matsize idx) const;

		bool operator==(const matrix& other) const;
		bool operator!=(const matrix& other) const;

		// Basic Operations:
		matrix& operator*=(const float scalar);
		matrix& operator/=(const float scalar);
		matrix& operator*=(const matrix& other);
		matrix& operator+=(const matrix& other);
		matrix& operator-=(const matrix& other);

		template <matsize _c, matsize _r>
		matrix& operator*=(const matrix<_t, _c, _r>& other);

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
		static matrix<_t, 3u, 3u> make_rotation(const vector<_t, 3u>& axis, float angle);
		static matrix<_t, 4u, 4u> make_rotation_RH(float euler_x, float euler_y, float euler_z);
		static matrix<_t, 4u, 4u> make_rotation_LH(float euler_x, float euler_y, float euler_z);
		static matrix<_t, 3u, 3u> make_translation(const vector<_t, 2u>& translation);
		static matrix<_t, 4u, 4u> make_translation(const vector<_t, 3u>& translation);
		static matrix<_t, 3u, 3u> make_scale(const vector<_t, 2u>& scale);
		static matrix<_t, 4u, 4u> make_scale(const vector<_t, 3u>& scale);

		// More...
		static matrix<_t, 4u, 4u> make_transform_LH(const vector<_t, 3u>& pos, const vector<_t, 3u>& forward, const vector3& up = vector3::up());
		static matrix<_t, 4u, 4u> make_transform_RH(const vector<_t, 3u>& pos, const vector<_t, 3u>& forward, const vector<_t, 3u>& up = vector3::up());
		static matrix<_t, 4u, 4u> make_view_LH(const vector<_t, 3u>& pos, const vector<_t, 3u>& forward, const vector<_t, 3u>& up = vector3::up());
		static matrix<_t, 4u, 4u> make_view_RH(const vector<_t, 3u>& pos, const vector<_t, 3u>& forward, const vector<_t, 3u>& up = vector3::up());
		static matrix<_t, 4u, 4u> make_projection_LH(const float fov, const float ar, const float n, const float f);
		static matrix<_t, 4u, 4u> make_projection_RH(const float fov, const float ar, const float n, const float f); // Todo: [Orthographic vs Perspective]

		static matrix<_t, 4u, 4u> make_viewprojection_RH(
			const vector3& pos,
			const vector3& forward,
			const float fov,
			const float aspect_ratio,
			const float near = 0.001f,
			const float far = 1000.0f,
			const vector3& up = vector3::up());

		static matrix make_diagonal(const _t& x, const _t& y, const _t& z, const _t& w = 1.0f);

		// decomposition
		void set_translation(const vector<_t, 3u>& translation);
		vector<_t, 3u> get_translation() const;
		vector<_t, 3u> get_scale() const;
		vector<_t, 3u> get_scale_sqr() const;
		vector<_t, 3u> get_euler_angles() const;

		void decompose(vector<_t, 3u>& out_translation, matrix<_t, 3u, 3u>& out_rotation, vector<_t, 3u>& out_scale) const;
		matrix<_t, 3u, 3u> get_rotation_matrix() const;
	};

	// Matrix - Matrix
	template <typename _t, matsize k_num_columns, matsize num_rows> matrix<_t, k_num_columns, num_rows> operator+(const matrix<_t, k_num_columns, num_rows>& a, const matrix<_t, k_num_columns, num_rows>& b);
	template <typename _t, matsize k_num_columns, matsize num_rows> matrix<_t, k_num_columns, num_rows> operator-(const matrix<_t, k_num_columns, num_rows>& a, const matrix<_t, k_num_columns, num_rows>& b);

	template <typename _t, matsize k_num_columns, matsize num_rows, matsize _OC, matsize _OR>
	matrix<_t, num_rows, _OR> operator*(const matrix<_t, k_num_columns, num_rows>& a, const matrix<_t, _OC, _OR>& b);

	// Matrix - Vector
	template<typename _t> math::vector<_t, 2u> operator*(const matrix<_t, 3u, 3u>& mat, const vector<_t, 2u>& v);
	template<typename _t> math::vector<_t, 3u> operator*(const matrix<_t, 3u, 3u>& mat, const vector<_t, 3u>& v);
	template<typename _t> math::vector<_t, 3u> operator*(const matrix<_t, 4u, 4u>& mat, const vector<_t, 3u>& v);
	template<typename _t> math::vector<_t, 4u> operator*(const matrix<_t, 4u, 4u>& mat, const vector<_t, 4u>& v);

	// Matrix - scalar
	template <typename _t, matsize k_num_columns, matsize num_rows> matrix<_t, k_num_columns, k_num_columns> operator*(const matrix<_t, k_num_columns, num_rows>& a, float b);
	template <typename _t, matsize k_num_columns, matsize num_rows> matrix<_t, k_num_columns, k_num_columns> operator/(const matrix<_t, k_num_columns, num_rows>& a, float b);
	template <typename _t, matsize k_num_columns, matsize num_rows> matrix<_t, k_num_columns, k_num_columns> operator*(float a, const matrix<_t, k_num_columns, num_rows>& b);
	template <typename _t, matsize k_num_columns, matsize num_rows> matrix<_t, k_num_columns, k_num_columns> operator/(float a, const matrix<_t, k_num_columns, num_rows>& b);

	
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

#include "matrix.inl"

#pragma warning(default : 4201)