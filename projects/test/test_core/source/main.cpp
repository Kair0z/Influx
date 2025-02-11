
#include "core/debug.h"	// influx_assert
#include "core/basetypes.h"
#include "core/math/vector.h"
#include "core/math/matrix.h"

using namespace influx;

template <typename _t, uint32 _n>
void test_math_vector()
{
	using value_type = _t;
	constexpr uint32 dimensions = _n;
	using vector = math::vector<_t, _n>;

	// addition
	influx_assert(vector::fill(1) + vector::fill(1) == vector::fill(2));

	// subtraction
	influx_assert(vector::fill(1) - vector::fill(1) == vector::fill(0));
	
	// multiplication
	influx_assert(vector::fill(4) * vector::fill(4) == vector::fill(16));

	// division
	influx_assert(vector::fill(8) / vector::fill(4) == vector::fill(2));

	// dot
	influx_assert(vector::dot(vector::fill(1), vector::fill(1)) == dimensions);

	// 3D section
	if constexpr (dimensions == 3u)
	{
		// cross
		const vector cross = vector::cross(vector{ 1, 0, 0 }, vector{ 0, 1, 0 });
		influx_assert(cross == vector(0, 0, 1));
	}
}
void test_math_vector_all()
{
	// static_assert(_dim != 0u, "influx::vector<_t, _dim> ¬ Cannot instantiate zero-sized vector (_dim == 0)! ");
	// test_math_vector<float, 0u>();

	test_math_vector<double, 1u>();
	test_math_vector<double, 2u>();
	test_math_vector<double, 3u>();
	test_math_vector<double, 4u>();
	test_math_vector<float, 1u>();
	test_math_vector<float, 2u>();
	test_math_vector<float, 3u>();
	test_math_vector<float, 4u>();
	test_math_vector<int, 1u>();
	test_math_vector<int, 2u>();
	test_math_vector<int, 3u>();
	test_math_vector<int, 4u>();
	test_math_vector<uint32, 1u>();
	test_math_vector<uint32, 2u>();
	test_math_vector<uint32, 3u>();
	test_math_vector<uint32, 4u>();
}

template <typename _t, uint32 _x, uint32 _y>
void test_math_matrix()
{
	using value_type = _t;
	constexpr uint32 num_rows = _x;
	constexpr uint32 num_cols = _y;

	if constexpr (num_rows == num_cols)
	{
		// uniform matrix tests

	}
}
void test_math_matrix_all()
{
	test_math_matrix<double, 1u, 1u>();
	test_math_matrix<double, 2u, 2u>();
	test_math_matrix<double, 3u, 3u>();
	test_math_matrix<double, 4u, 4u>();
	test_math_matrix<float, 1u, 1u>();
	test_math_matrix<float, 2u, 2u>();
	test_math_matrix<float, 3u, 3u>();
	test_math_matrix<float, 4u, 4u>();
	test_math_matrix<int, 1u, 1u>();
	test_math_matrix<int, 2u, 2u>();
	test_math_matrix<int, 3u, 3u>();
	test_math_matrix<int, 4u, 4u>();
	test_math_matrix<uint32, 1u, 1u>();
	test_math_matrix<uint32, 2u, 2u>();
	test_math_matrix<uint32, 3u, 3u>();
	test_math_matrix<uint32, 4u, 4u>();
}

void test_basetypes()
{
	// todo
}

void test_cache()
{
	// todo
}

int main()
{
	test_basetypes();
	test_cache();
	test_math_vector_all();
	test_math_matrix_all();
}