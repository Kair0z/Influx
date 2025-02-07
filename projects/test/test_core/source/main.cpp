
#include "core/debug.h"
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
template <typename _t, typename _x, typename _y>
void test_math_matrix()
{
	using value_type = _t;
	using num_rows = _x;
	using num_cols = _y;

	// addition
		
	// subtraction
	
	// multiplication
	
	// division
	
	// translation

	// scaling

	// rotation
}
void test_math_matrix_all()
{

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