// choose your test:
#define TEST_MATH		0
#define TEST_BASETYPES	0
#define TEST_STRING		0
#define TEST_RESULT		0
#define TEST_CONTAINER	0
#define TEST_POINTER	1
#define TEST_ASCII_ART	1

// common
#include "core/basetypes.h"
#include "core/debug.h"	// influx_assert
#include "core/ascii_art.h"

void print(const char* fmt, ...)
{
	// printf(fmt, ...);
}
using namespace influx;

// includes
#if TEST_BASETYPES
#include "core/basetypes.h"
#endif
#if TEST_MATH
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
#include "core/math/vector.h"
#include "core/math/matrix.h"
#include "core/math/quaternion.h"
#include "core/math/rotor.h"
#endif
#if TEST_RESULT
#include "core/result.h"
#endif
#if TEST_STRING
#include "core/string.h"
#endif
#if TEST_CONTAINER
#include "core/containers.h"
#endif
#if TEST_POINTER
#include "core/pointer.h"
#endif

// tests
#if TEST_MATH
template <typename _t>
static bool is_almost_equal(const _t& a, const _t& b)
{
	return math::abs(a - b) < (_t)0.0001f;
}

template <typename _t, uint32 _n>
static bool operator==(const math::vector<_t, _n>& a, const glm::vec<_n, _t>& b)
{
	for (uint32 i = 0u; i < _n; ++i)
	{
		if (a[i] != b[i]) return false;
	}
	return true;
}

template <typename _t, uint32 _x, uint32 _y = 1u>
void test_math_vector()
{
	using value_type = _t;
	constexpr uint32 k_num = _x * _y;
	using vector = math::vector<_t, _x, _y>;
	using glmvec = glm::vec<_x, _t>;

	// common
	influx_assert(vector::fill(1) + vector::fill(1) == vector::fill(2));
	influx_assert(vector::fill(1) - vector::fill(1) == vector::fill(0));
	influx_assert(vector::fill(4) * vector::fill(4) == vector::fill(16));
	influx_assert(vector::fill(8) / vector::fill(4) == vector::fill(2));

	// vector section
	if constexpr (vector::k_is_vector)
	{
		influx_assert(vector::dot(vector::fill(1), vector::fill(1)) == k_num);

		if constexpr (_x == 3u)
		{
			const vector cross = vector::cross(vector{ 1, 0, 0 }, vector{ 0, 1, 0 });
			// influx_assert(is_almost_equal(cross, vector(0, 0, 1)));
			// influx_assert(cross == glm::vec3(0, 0, 1));
		}
	}

	// matrix section
	if constexpr (vector::k_is_matrix)
	{
		if constexpr (_x == 2u) // 2x2
		{
			vector a = vector::make_one();
			a *= 2.0f;
		}
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
	test_math_vector<float, 2u, 2u>();
	test_math_vector<float, 3u, 3u>();
	test_math_vector<float, 4u, 4u>();
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
#endif
#if TEST_CONTAINER
void test_containers()
{
	using namespace influx;

	static_array<int, 3u> stat_array{};
	static_array<int, 3u> stat_array2{};
	// ctr::push(stat_array, 3); // this should never compile!
	ctr::merge(stat_array2, stat_array);
	influx_assert(ctr::contains(stat_array, 2));

	dynamic_array<int> dyn_array{};
	dynamic_array<int> dyn_array2{};
	ctr::push(dyn_array, 34);
	ctr::merge(dyn_array2, dyn_array);

	bool contains = ctr::contains(dyn_array, 2);
}
#endif
#if TEST_RESULT
void test_result()
{
	result<float> float_result = { 1.0f };
	result<bool> bool_result = { false };
	result<int> int_result = e_result::error;

	result<> nothing = {};

	assert(float_result);
}
#endif
#if TEST_BASETYPES
void test_basetypes()
{
	// todo
}
#endif
#if TEST_POINTER
template <typename _t>
using shptr		= pointers::shared_ptr<_t>;
template <typename _t>
using uniptr	= pointers::unique_ptr<_t>;
template <typename _t>
using wptr		= pointers::weak_ptr<_t>;

#include <chrono>
#include <thread>
void test_pointers()
{
	print("==== testing (smart) pointers ...");

	shptr<uint32> shared = pointers::make_shared<uint32>(3u);
	shptr<uint32> other = shared;
	wptr<uint32> wother = shared;
}
#endif
#if TEST_ASCII_ART
void test_asciiart()
{
	for (uint32 j = 0u; j < 10u; ++j)
	{
		artscii::progress_bar progress{};
		
		// [======C . . . .]
		progress.get_settings()
			.set_length(50u)
			.set_done('=')
			.set_cursor('C')
			.set_todo('.')
			.set_todo(' ', true);

		for (uint32 i = 0u; i < progress.bar_length(); ++i)
		{
			progress += 1u;
			std::cout << "\r[loading] " << progress.get_cstr() << " " << math::round<int, float>(progress.pc() * 100) << "%";
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
		std::cout << "\n";
	}
}
#endif

int main()
{
	// test_containers();
	// test_basetypes();
	// test_result();
	// test_cache();
	
#if TEST_MATH
	test_math_vector_all();
	test_math_matrix_all();
	test_math_quaternion_all();
#endif
#if TEST_POINTER
	test_pointers();
#endif
#if TEST_ASCII_ART
	test_asciiart();
#endif
}