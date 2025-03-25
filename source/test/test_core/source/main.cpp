
// test math against glm
#include "glm/glm.hpp"
namespace glm
{
	template <typename _t, uint32_t _n>
	inline glm::vec<_n, _t> fill(const _t& value)
	{
		glm::vec<_n, _t> result{};
		for (uint32_t i = 0u; i < _n; ++i)
		{
			result[i] = value;
		}
		return result;
	}
}

#include "core/debug.h"	// influx_assert
#include "core/basetypes.h"
#include "core/math/vector.h"
#include "core/math/matrix.h"
#include "core/math/quaternion.h"
#include "core/math/rotor.h"
#include "core/result.h"

using namespace influx;

template <typename _t>
static bool is_almost_equal(const _t& a, const _t& b)
{
	return math::abs(a - b) < (_t)0.0001f;
}

#pragma region test_math
template <typename _t, uint32 _n>
static bool operator==(const math::vector<_t, _n>& a, const glm::vec<_n, _t>& b)
{
	for (uint32 i = 0u; i < _n; ++i)
	{
		if (a[i] != b[i]) return false;
	}
	return true;
}
template <typename _t, uint32 _n>
void test_math_vector()
{
	using value_type = _t;
	constexpr uint32 size = _n;
	using vector = math::vector<_t, _n>;
	using glmvec = glm::vec<_n, _t>;

	// basic vec x vec
	influx_assert(vector::fill(1) + vector::fill(1) == vector::fill(2));
	influx_assert(vector::fill(1) - vector::fill(1) == vector::fill(0));
	influx_assert(vector::fill(4) * vector::fill(4) == vector::fill(16));
	influx_assert(vector::fill(8) / vector::fill(4) == vector::fill(2));

	influx_assert(vector::dot(vector::fill(1), vector::fill(1)) == size);

	// 3D section
	if constexpr (size == 3u)
	{
		// cross
		const vector cross = vector::cross(vector{ 1, 0, 0 }, vector{ 0, 1, 0 });
		// influx_assert(is_almost_equal(cross, vector(0, 0, 1)));
		// influx_assert(cross == glm::vec3(0, 0, 1));
	}
}
void test_math_vector_all()
{
	// static_assert(_dim != 0u, "influx::vector<_t, _dim> � Cannot instantiate zero-sized vector (_dim == 0)! ");
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

template <typename _t>
void test_math_quaternion()
{

}
void test_math_quaternion_all()
{
	test_math_quaternion<bool>();
	test_math_quaternion<int>();
	test_math_quaternion<float>();
	test_math_quaternion<double>();
	test_math_quaternion<long double>();
}

template <typename _t, uint32 _d>
void test_math_rotor()
{
	using value_t = _t;
	using rotor = math::rotor<_t, _d>;
}
void test_math_rotor_all()
{
	test_math_rotor<float, 2u>();
	test_math_rotor<double, 2u>();
	test_math_rotor<int, 2u>();
	test_math_rotor<uint32, 2u>();
}
#pragma endregion

void test_result()
{
	result<float> float_result = { 1.0f };
	result<bool> bool_result = { false };
	result<int> int_result = e_result::error;

	result<> nothing = {};

	assert(float_result);
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
	test_result();
	test_cache();

	test_math_vector_all();
	test_math_matrix_all();
	test_math_quaternion_all();
}