#pragma once
#define _CORE_MATH_VECTOR_VERSION_ 1

#ifndef _CORE_MATH_VECTOR_H_
#define _CORE_MATH_VECTOR_H_

// influx::core
#include "core/basetypes.h"

// STL
#include <cmath> // std::sqrt

namespace influx::math { using vecsize = uint32; }

#if _CORE_MATH_VECTOR_VERSION_ == 1
namespace influx::math
{
    namespace detail
    {
        template <typename _t, uint32 _x, uint32 _y>
        struct base_vector
        {
            static constexpr uint32 _n = _x * _y;
            union
            {
                struct { _t x, y, z, w; };
                struct { _t r, g, b, a; };
                _t m_data[_n];
            };
            constexpr base_vector() = default;
#if 0
            template <typename... _args>
            constexpr base_vector(const _args&... components) 
                : m_data{ static_cast<_t>(std::forward<const _args&>(args))... }
#else
            template <typename... _V>
            base_vector(const _V&... components)
                : m_data{ components... }
            {

            }
#endif
        };
        template <typename _t>
        struct base_vector<_t, 1u, 1u>
        {
            static constexpr uint32 _n = 1u * 1u;
            union
            {
                struct { _t x; };
                struct { _t r; };
                _t m_data[_n];
            };
            constexpr base_vector() = default;
            constexpr base_vector<_t, 1u, 1u>(const _t _x) : x{ _x } {}
        };
        template <typename _t>
        struct base_vector<_t, 2u, 1u>
        {
            static constexpr uint32 _n = 2u * 1u;
            union
            {
                struct { _t x, y; };
                struct { _t r, g; };
                _t m_data[_n];
            };
            constexpr base_vector() = default;
            constexpr base_vector<_t, 2u, 1u>(const _t _x, const _t _y = 0) : x{ _x }, y{ _y } {}
        };
        template <typename _t>
        struct base_vector<_t, 3u, 1u>
        {
            static constexpr uint32 _n = 3u * 1u;
            union
            {
                struct { _t x, y, z; };
                struct { _t r, g, b; };
                _t m_data[_n];
            };
            constexpr base_vector() = default;
            constexpr base_vector<_t, 3u, 1u>(const _t _x, const _t _y = 0, const _t _z = 0) : x{ _x }, y{ _y }, z{ _z } {}
        };
        template <typename _t>
        struct base_vector<_t, 4u, 1u>
        {
            static constexpr uint32 _n = 4u * 1u;
            union
            {
                struct { _t x, y, z, w; };
                struct { _t r, g, b, a; };
                _t m_data[_n];
            };
            constexpr base_vector() = default;
            constexpr base_vector<_t, 4u, 1u>(const _t _x, const _t _y = 0, const _t _z = 0, const _t _w = 0) : x{ _x }, y{ _y }, z{ _z }, w{ _w } {}
        };
        template <typename _t>
        struct base_vector<_t, 2u, 2u>
        {
            static constexpr uint32 _n = 2u * 2u;
            union
            {
                struct { _t _00, _01, _10, _11; };
                _t m_data[_n];
            };
        };
    }

    template <typename _t, uint32 _x, uint32 _y = 1u>
    struct vector final : public detail::base_vector<_t, _x, _y>
    {
        static constexpr uint32 _n = _y * _x;
        using size_t = uint32;
        using data_t = _t;

        // helpers
        constexpr static bool is_zero(const _t& value)
        {
            return value == 0.0f;
        }
        constexpr static bool is_equal(const _t& a, const _t& b)
        {
            return a == b;
        }
        template <typename _tt, uint32 _nn>
        constexpr static void copyarr(const _tt* src, _tt* dest)
        {
            for (uint32 i = 0u; i < _nn; ++i)
                dest[i] = src[i];
        }
        constexpr static _t sqrt(const _t& val)
        {
            return std::sqrt(val);
        }

        // constructors
        vector() = default;

        // init list
        template <typename... _args>
        vector(const _args&... args) : detail::base_vector<_t, _x, _y>({ static_cast<_t>(args) ...}) {}

        // > typecasting
        template <typename _u>
        vector(const vector<_u, _x, _y>& other)
        {
            for (uint32 i{}; i < _n; ++i)
                this->m_data[i] = static_cast<_t>(other.m_data[i]);
        }

        // > sizecasting
        // if other smaller, overwrite first elements, set rest to zero
        // if other bigger, write until our limit
        template <uint32 _ox, uint32 _oy>
        vector(const vector<_t, _ox, _oy>& other)
        {
            static constexpr uint32 _on = _ox * _oy;
            for (uint32 i{}; i < _n; ++i)
                this->m_data[i] = (i < _on) ? other[i] : static_cast<_t>(0);
        }

        constexpr vector(const vector& other) { copyarr<_t, _n>(other.m_data, this->m_data); }
        constexpr vector(vector&& other) noexcept { copyarr<_t, _n>(other.m_data, this->m_data); }
        constexpr vector& operator=(const vector& other) { copyarr<_t, _n>(other.m_data, this->m_data); return *this; }
        constexpr vector& operator=(vector&& other) noexcept { copyarr<_t, _n>(other.m_data, this->m_data); return *this; }

        // access
        constexpr _t& operator[](uint32 i) { return this->m_data[i]; }
        constexpr const _t& operator[](uint32 i) const { return this->m_data[i]; }
        constexpr _t& at(uint32 x, uint32 y) { return this->m_data[xy_to_index(x, y)]; }
        constexpr const _t& at(uint32 x, uint32 y) const { return this->m_data[xy_to_index(x, y)]; }
        constexpr static uint32 xy_to_index(uint32 x, uint32 y) { return (y * _x) + x; }
        constexpr static uint32 index_to_x(uint32 index) { return index % _x; }
        constexpr static uint32 index_to_y(uint32 index) { return index / _y; }
        constexpr const _t* data() const { return this->m_data; }
#if 0
        constexpr _t& x() { return m_data[0]; } constexpr const _t& x() const { return m_data[0]; }
        constexpr _t& y() { return m_data[1]; } constexpr const _t& y() const { return m_data[1]; }
        constexpr _t& z() { return m_data[2]; } constexpr const _t& z() const { return m_data[2]; }
        constexpr _t& w() { return m_data[3]; } constexpr const _t& w() const { return m_data[3]; }
#endif

        // math operations
        friend constexpr vector operator+(const vector& a, const vector& b)
        {
            return add_impl(a, b, std::make_index_sequence<_n>{});
        }
        friend constexpr vector operator+(const vector& a, const _t& sc)
        {
            return add_impl(a, fill(sc), std::make_index_sequence<_n>{});
        }
        friend constexpr vector operator+(const _t& sc, const vector& a)
        {
            return add_impl(a, fill(sc), std::make_index_sequence<_n>{});
        }
        friend constexpr vector operator-(const vector& a, const vector& b)
        {
            return det_impl(a, b, std::make_index_sequence<_n>{});
        }
        friend constexpr vector operator-(const vector& a, const _t& sc)
        {
            return det_impl(a, fill(sc), std::make_index_sequence<_n>{});
        }
        friend constexpr vector operator-(const _t& sc, const vector& a)
        {
            return det_impl(a, fill(sc), std::make_index_sequence<_n>{});
        }
        friend constexpr vector operator*(const vector& a, const vector& b)
        {
            return mult_impl(a, b, std::make_index_sequence<_n>{});
        }
        friend constexpr vector operator*(const vector& a, const _t& sc)
        {
            return mult_impl(a, fill(sc), std::make_index_sequence<_n>{});
        }
        friend constexpr vector operator*(const _t& sc, const vector& a)
        {
            return mult_impl(a, fill(sc), std::make_index_sequence<_n>{});
        }
        friend constexpr vector operator/(const vector& a, const vector& b)
        {
            return div_impl(a, b, std::make_index_sequence<_n>{});
        }
        friend constexpr vector operator/(const vector& a, const _t& sc)
        {
            return div_impl(a, fill(sc), std::make_index_sequence<_n>{});
        }
        friend constexpr vector operator/(const _t& sc, const vector& a)
        {
            return div_impl(a, fill(sc), std::make_index_sequence<_n>{});
        }
        friend constexpr vector operator-(const vector& a)
        {
            return neg_impl(a, std::make_index_sequence<_n>{});
        }

        constexpr vector& operator+=(const vector& other)   { *this = *this + other; return *this; }
        constexpr vector& operator+=(const _t& other)       { *this = *this + other; return *this; }
        constexpr vector& operator-=(const vector& other)   { *this = *this - other; return *this; }
        constexpr vector& operator-=(const _t& other)       { *this = *this - other; return *this; }
        constexpr vector& operator*=(const vector& other)   { *this = *this * other; return *this; }
        constexpr vector& operator*=(const _t& other)       { *this = *this * other; return *this; }
        constexpr vector& operator/=(const vector& other)   { *this = *this / other; return *this; }
        constexpr vector& operator/=(const _t& other)       { *this = *this / other; return *this; }

        // comparison
        constexpr bool operator==(const vector& other) const
        {
            for (uint32 i = 0u; i < _n; ++i)
                if (!is_equal(this->m_data[i], other.m_data[i])) return false;
            return true;
        }
        constexpr bool operator!=(const vector& other) const
        {
            return !(*this == other);
        }
        
        // misc
        constexpr static vector fill(const _t& value)
        {
            vector result{};
            for (uint32 i = 0u; i < _n; ++i) result[i] = value;
            return result;
        }
        constexpr static vector make_one() { return fill(1); }
        constexpr static vector make_zero() { return fill(0); }
        constexpr bool is_zero() const
        {
            for (uint32 i = 0u; i < _n; ++i) 
                if (!is_zero(this->m_data[i])) return false;
            return true;
        }
        constexpr static vector make_identity()
        {
            vector result;
            for (uint32 y = 0u; y < _y; ++y)
                for (uint32 x = 0u; x < _x; ++x)
                    result[xy_to_index(x, y)] = (_t)(x == y);

            return result;
        }
        constexpr bool is_identity() const
        {
            for (uint32 y = 0u; y < _y; ++y)
                for (uint32 x = 0u; x < _x; ++x)
                    if (x == y && at(x, y) != (_t)1) return false;

            return true;
        }
        constexpr vector<_t, 3u, 1u> get_xyz() const
        {
            return { this->m_data[0], this->m_data[1], this->m_data[2] };
        }

        // vector operations
        constexpr static bool k_is_vector       = _y == 1u;
        constexpr static bool k_supports_dot    = k_is_vector;
        constexpr static bool k_supports_cross  = k_is_vector;

        constexpr _t get_length_sq() const
        {
            static_assert(k_is_vector);

            _t sum = 0u;
            for (uint32 i = 0u; i < _n; ++i)
                sum += this->m_data[i] * this->m_data[i];
            return sum;
        }
        constexpr _t get_length() const
        {
            static_assert(k_is_vector);
            return sqrt(get_length_sq());
        }
        constexpr _t get_magnitude() const
        {
            return get_length();
        }
        constexpr _t get_magnitude_sq() const
        {
            return get_length_sq();
        }
        constexpr vector& normalize()
        {
            static_assert(k_is_vector);
            if (!is_zero())
            {
                *this = normalized();
            }
            return *this;
        }
        constexpr vector normalized() const
        {
            static_assert(k_is_vector);
            if (is_zero()) return make_zero();

            return *this / get_length();
        }
        constexpr static vector make_up()
        {
            static_assert(k_is_vector);
            return { 0,1,0 };
        }
        constexpr static vector make_right()
        {
            static_assert(k_is_vector);
            return { 1,0,0 };
        }
        constexpr static vector make_forward()
        {
            static_assert(k_is_vector);
            return { 0,0,1 };
        }
        constexpr static vector cross(const vector& a, const vector& b)
        {
            static_assert(k_supports_cross);
            if constexpr (_x == 2u)
            {
                return {};
            }
            else if constexpr (_x == 3u) 
            {
                return {a.y * b.z - a.z * b.y,
                        a.z * b.x - a.x * b.z,
                        a.x * b.y - a.y * b.x };
            }
            else
            {
                static_assert(false, "not implemented!");
            }
            return {};
        }
        constexpr static _t dot(const vector& a, const vector& b)
        {
            static_assert(k_supports_dot);
            _t sum = {};
            for (uint32 i = 0u; i < _n; ++i) sum += (a[i] * b[i]);
            return sum;
        }
        constexpr vector& clamp_length(const _t& max)
        {
            static_assert(k_is_vector);
            if (get_magnitude_sq() > (max * max))
            {
                *this = this->normalized() * max;
            }
            return *this;
        }

        // matrix operations
        constexpr static bool k_is_matrix = _y > 1u;
        constexpr static bool k_is_square_matrix = k_is_matrix && _y == _x;
        constexpr static bool k_supports_transpose = k_is_matrix;

        template <uint32 _ox, uint32 _oy>
        constexpr vector<_t, _ox, _y> mat_mul(const vector<_t, _ox, _oy>& other)
        {
            static_assert(k_is_matrix);
            // matrix multiplication requires Ax (cols) == By (rows)
            static_assert(_x == _oy);

            vector<_t, _ox, _y> result{};
            for (uint32 y = 0u; y < _y; ++y)
                for (uint32 x = 0u; x < _ox; ++x)
                {
                    _t sum{};
                    for (uint32 i = 0u; i < _x; ++i)
                    {
                        const _t& at0 = this->at(i, y);
                        const _t& at1 = other.at(x, i);
                        _t add = (at0 * at1);
                        sum += add;
                    }
                    result.at(x, y) = sum;
                }
            return result;
        }
        constexpr vector<_t, _y, _x> get_transposed() const
        {
            static_assert(k_supports_transpose);

            vector<_t, _y, _x> result{};
            for (uint32 y = 0u; y < _y; ++y)
                for (uint32 x = 0u; x < _x; ++x)
                    result.at(y, x) = at(x, y);

            return result;
        }
        constexpr bool get_inverse(vector& out_inverse) const
        {
            static_assert(k_is_square_matrix);

            // 2D matrix inverse
            if constexpr (_x == 2u)
            {
                _t a = at(0, 0);
                _t b = at(1, 0);
                _t c = at(0, 1);
                _t d = at(1, 1);
                _t det = a * d - b * c;

                if (is_zero(det))
                    return false;

                out_inverse.at(0, 0) = d / det;
                out_inverse.at(1, 0) = -b / det;
                out_inverse.at(0, 1) = -c / det;
                out_inverse.at(1, 1) = a / det;
                return true;
            }
            else
            {
                static_assert("not implemented!");
            }

            return false;
        }
        constexpr _t get_determinant() const
        {
            static_assert(k_is_matrix);

            // 2D matrix determinant
            if constexpr (_x == 2u && _y == 2u)
            {
                _t a = at(0, 0);
                _t b = at(1, 0);
                _t c = at(0, 1);
                _t d = at(1, 1);
                return a * d - b * c;
            }
            else
            {
                static_assert(false, "not implemented!");
            }
        }

    private:
        template <typename _t, uint32 _x, uint32 _y, uint32... _is>
        static constexpr vector<_t, _x, _y> add_impl(const vector<_t, _x, _y>& a, const vector<_t, _x, _y>& b, std::index_sequence<_is...>) {
            return vector<_t, _x, _y>{ (a[_is] + b[_is]) ... };
        }
        template <typename _t, uint32 _x, uint32 _y, uint32... _is>
        static constexpr vector<_t, _x, _y> det_impl(const vector<_t, _x, _y>& a, const vector<_t, _x, _y>& b, std::index_sequence<_is...>) {
            return vector<_t, _x, _y>{ (a[_is] - b[_is]) ... };
        }
        template <typename _t, uint32 _x, uint32 _y, uint32... _is>
        static constexpr vector<_t, _x, _y> mult_impl(const vector<_t, _x, _y>& a, const vector<_t, _x, _y>& b, std::index_sequence<_is...>) {
            return vector<_t, _x, _y>{ (a[_is] * b[_is]) ... };
        }
        template <typename _t, uint32 _x, uint32 _y, uint32... _is>
        static constexpr vector<_t, _x, _y> div_impl(const vector<_t, _x, _y>& a, const vector<_t, _x, _y>& b, std::index_sequence<_is...>) {
            return vector<_t, _x, _y>{ (a[_is] / b[_is]) ... };
        }
        template <typename _t, uint32 _x, uint32 _y, uint32... _is>
        static constexpr vector<_t, _x, _y> neg_impl(const vector<_t, _x, _y>& a, std::index_sequence<_is...>) {
            return vector<_t, _x, _y>{ (-a[_is]) ... };
        }
    };
}
#elif _CORE_MATH_VECTOR_VERSION_ == 0
// disable union-related warning
#pragma warning(disable : 4201) 
namespace influx::math
{
	namespace detail
	{
		template <typename _t, vecsize _s>
		struct base_vector
		{
			union
			{
				struct { _t x, y, z, w; };
				struct { _t r, g, b, a; };
				_t m_data[_s];
			};

			base_vector() = default;
			template <typename... _V>
			base_vector(const _V&... components)
				: m_data{components...}
			{

			}
		};

		template <typename _t>
		struct base_vector<_t, 1u>
		{
			union
			{
				struct { _t x; };
				struct { _t r; };
				_t m_data[1];
			};
			base_vector() = default;
			base_vector<_t, 1u>(const _t _x) : x{ _x } {}
		};

		template <typename _t>
		struct base_vector<_t, 2u>
		{
			union
			{
				struct { _t x, y; };
				struct { _t r, g; };
				_t m_data[2];
			};
			base_vector() = default;
			base_vector<_t, 2u>(const _t _x, const _t _y = 0) : x{ _x }, y{ _y } {}
		};

		template <typename _t>
		struct base_vector<_t, 3u>
		{
			union
			{
				struct { _t x, y, z; };
				struct { _t r, g, b; };
				_t m_data[3];
			};
			base_vector() = default;
			base_vector<_t, 3u>(const _t _x, const _t _y = 0, const _t _z = 0) : x{ _x }, y{ _y }, z{ _z } {}
		};

		template <typename _t>
		struct base_vector<_t, 4u>
		{
			union
			{
				struct { _t x, y, z, w; };
				struct { _t r, g, b, a; };
				_t m_data[4];
			};
			base_vector() = default;
			base_vector<_t, 4u>(const _t _x, const _t _y = 0, const _t _z = 0, const _t _w = 0) : x{ _x }, y{ _y }, z{ _z }, w{ _w } {}
		};
	}

	template <typename _t, vecsize _s>
	class vector final : public detail::base_vector<_t, _s>
	{
		static_assert(_s != 0u, "influx::vector<_t, _s> � Cannot instantiate zero-sized vector (_s == 0)! ");
		static_assert(_s <= 4u, "influx::vector<_t, _s> � Cannot instantiate vector bigger than 4 (_s > 4)! ");

		static constexpr vecsize k_size = _s;
		using value_type = _t;

	public:
		// constructors
		vector() = default;
		vector(const vector&) = default;
		vector(vector&&) = default;
		vector& operator=(const vector& other) = default;
		vector& operator=(vector&& other) = default;
		template <typename... _v>		vector(const _v&... components);		// Initializer list
		template <typename _u>			vector(const vector<_u, _s>& other);	// Typecasting
		template <vecsize _d>			vector(const vector<_t, _d>& other);	// Sizecasting

		constexpr static vecsize size();

		// Accessing data:
		_t& operator[](vecsize i);
		const _t& operator[](vecsize i) const;
		_t& at(vecsize i);
		const _t& at(vecsize i) const;

		const _t* data() const;
		_t* data();

		// Normalizing:
		vector normalized() const;
		void normalize();
		static void normalize(vector& vec);
		static vector normalized(const vector& vec);

		// clamp:
		vector&			clamp_values(_t min, _t max);
		vector			get_clamped_values(_t min, _t max) const;
		static void		clamp_values(vector& vec, _t min, _t max);
		static vector	get_clamped_values(const vector& vec, _t min, _t max);

		vector&			clamp_length(_t length);
		vector			get_clamped_length(_t length) const;
		static void		clamp_length(vector& vec, _t length);
		static vector	get_clamped_length(const vector& vec, _t length);

		// Angle:
		float radians_between(const vector& other) const;
		static float radians_between(const vector& a, const vector& b);

		// Magnitude:
		float magnitude() const;
		float sqr_magnitude() const;
		static float magnitude(const vector& other);
		static float sqr_magnitude(const vector& other);
		static float distance(const vector& a, const vector& b);
		static float sqr_distance(const vector& a, const vector& b);

		// Scaling:
		void scale(float mag);
		vector scaled(float mag) const;
		static void scale(vector& vec, float mag);
		static vector scaled(const vector& vec, float mag);

		// Cross & dot:
		_t dot(const vector& other) const;
		static _t dot(const vector& a, const vector& b);

		_t cross(const vector<_t, 2u>& other) const;
		vector<_t, 3u> cross(const vector<_t, 3u>& other) const;
		static float cross(const vector<_t, 2u>& a, const vector<_t, 2u>& b);
		static vector<_t, 3u> cross(const vector<_t, 3u>& a, const vector<_t, 3u>& b);

		// Comparison+		a	{...}	const influx::math::vector<float,3> &

		bool operator==(const vector& other) const;
		bool operator!=(const vector& other) const;

		// Inverting:
		const vector& inverted() const;
		void inverse();
		static void inverse(vector& vec);
		static vector inverted(const vector& vec);

		// Reflect:
		const	vector<_t, 2u>&	reflected(const vector<_t, 2u>& hitNormal) const;
		const	vector<_t, 3u>& reflected(const vector<_t, 3u>& hitNormal) const;
		static	vector<_t, 2u>	reflection(const vector<_t, 2u>& vec, const vector<_t, 2u>& hitNormal);
		static	vector<_t, 3u>	reflection(const vector<_t, 3u>& vec, const vector<_t, 3u>& hitNormal);

		// Lerp:
		static vector lerp(const vector& a, const vector& b, const float t);
		void lerp_towards(const vector& b, const float t);

		// Zero:
		static vector zero();
		static vector one();
		static vector get_max();

		// makes a vector filled with value
		static vector fill(const _t&);

		bool is_zero() const;
		static bool is_zero(const vector& v);

		// 3D:
		constexpr static vector<_t, 3u> up();
		constexpr static vector<_t, 3u> forward();
		constexpr static vector<_t, 3u> right();

		static vector<_t, 2u> get_look_at(const vector<_t, 2u>& from, const vector<_t, 2u>& to);
		static vector<_t, 3u> get_look_at(const vector<_t, 3u>& from, const vector<_t, 3u>& to);

		vector<_t, 2u> get_xy() const;
		vector<_t, 3u> get_xyz() const;
		vector<_t, 2u> get_rg() const;
		vector<_t, 3u> get_rgb() const;

		static vector abs(const vector& vec);
		static _t get_summed(const vector& vec);

		// Arithmatics:
		vector& operator+=(const vector& other);
		vector& operator-=(const vector& other);
		vector& operator*=(const vector& other);
		vector& operator*=(const float scalar);
		vector& operator/=(const vector& other);
		vector& operator/=(const float scalar);
	};

	// operators:
	template <typename _t, vecsize _s>
	vector<_t, _s> operator+(const vector<_t, _s>& a, const vector<_t, _s>& b);
	template <typename _t, vecsize _s>
	vector<_t, _s> operator-(const vector<_t, _s>& a, const vector<_t, _s>& b);
	template <typename _t, vecsize _s>
	vector<_t, _s> operator*(const vector<_t, _s>& a, const vector<_t, _s>& b);
	template <typename _t, vecsize _s>
	vector<_t, _s> operator/(const vector<_t, _s>& a, const vector<_t, _s>& b);
	template <typename _t, vecsize _s>
	vector<_t, _s> operator*(const vector<_t, _s>& a, const float b);
	template <typename _t, vecsize _s>
	vector<_t, _s> operator/(const vector<_t, _s>& a, const float b);
	template <typename _t, vecsize _s>
	vector<_t, _s> operator*(const float a, const vector<_t, _s>& b);
	template <typename _t, vecsize _s>
	vector<_t, _s> operator-(const vector<_t, _s>& v);
	template <typename _t, vecsize _s>
	vector<_t, _s> operator/(const float a, const vector<_t, _s>& b);
}
#include "vector.inl"
#pragma warning(default : 4201)
#endif

// aliases
namespace influx::math
{
    template <typename vecsize _s>
    using vectoru8 = vector<uint8, _s>;
    template <typename vecsize _s>
    using vectoru32 = vector<uint32, _s>;
    template <typename vecsize _s>
    using vectoru64 = vector<uint64, _s>;
    template <typename vecsize _s>
    using vectori = vector<int, _s>;
    template <typename vecsize _s>
    using vectorf = vector<float, _s>;
    template <typename vecsize _s>
    using vectorl = vector<long, _s>;

    using vectorf2 = vectorf<2u>;
    using vectorf3 = vectorf<3u>;
    using vectorf4 = vectorf<4u>;
    using vectoru2 = vectoru32<2u>;
    using vectoru3 = vectoru32<3u>;
    using vectoru4 = vectoru32<4u>;
    using vectori2 = vectori<2u>;
    using vectori3 = vectori<3u>;
    using vectori4 = vectori<4u>;

    using float2 = vectorf2;
    using float3 = vectorf3;
    using float4 = vectorf4;
    using uint2 = vectoru2;
    using uint3 = vectoru3;
    using uint4 = vectoru4;
    using int2 = vectori2;
    using int3 = vectori3;
    using int4 = vectori4;

    using mat23 = vector<float, 2u, 3u>;
    using mat32 = vector<float, 3u, 2u>;
    using mat22 = vector<float, 2u, 2u>;
    using mat33 = vector<float, 3u, 3u>;
    using mat44 = vector<float, 4u, 4u>;
}



#endif