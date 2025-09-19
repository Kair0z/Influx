#pragma once

#include <iostream>

namespace influx
{
	namespace detail
	{
		using uint32 = unsigned int;
		template <typename _t, uint32 _n>
		struct multi final
		{
			_t m_data[_n];

			multi() = default;
			multi(const multi& other) { *this = other; }
			multi(const _t& x, const _t& y, const _t& z)
			{
				_t* values[3u]{ &x, &y, &z };
				for (uint32 i = 0u; i < _n; ++i) m_data[i] = *values[i];
			}
			multi& operator=(const multi& other)
			{
				for (uint32 i = 0u; i < _n; ++i) m_data[i] = other[i];
				return *this;
			}

			static multi make_fill(const _t& val)
			{
				multi res{};
				for (uint32 i = 0u; i < _n; ++i) res.m_data[i] = val;
				return res;
			}

			_t& mod(uint32 index) { return m_data[index]; };
			const _t& get(uint32 index) const { return m_data[index]; }
			_t& operator[](uint32 index) { return mod(index); }
			const _t& operator[](uint32 index) const { return get(index); }

			multi& operator+=(const multi& other) { *this = *this + other; return *this; }
			multi& operator/=(const _t& scalar) { *this = *this / scalar; return *this; }
		};
		template <typename _t, uint32 _n>
		multi<_t, _n> operator+(const multi<_t, _n>& a, const multi<_t, _n>& b)
		{
			using multi = multi<_t, _n>;
			multi result{};
			for (uint32 i = 0u; i < _n; ++i) result[i] = a[i] + b[i];
			return result;
		}
		template <typename _t, uint32 _n>
		multi<_t, _n> operator/(const multi<_t, _n>& a, const _t& scalar)
		{
			using multi = multi<_t, _n>;
			multi result{};
			for (uint32 i = 0u; i < _n; ++i) result[i] = a[i] / scalar;
			return result;
		}

		using float2 = multi<float, 2u>;
		using float3 = multi<float, 3u>;

		using node_id = uint32;
		using prim_id = uint32;
		struct prim_range final
		{
			prim_id m_start = 0u;
			uint32 m_num = 0u;
		};

		template <typename _t>
		using vector = std::vector<_t>;

		template <uint32 _n>
		struct ray final
		{
			multi<float, _n> m_origin{};
			multi<float, _n> m_direction{};
		};

		template <uint32 _n>
		struct aabb final
		{
			multi<float, _n> m_min{};
			multi<float, _n> m_max{};
		};

		template <uint32 _n>
		struct bvh_node final
		{
			using bounds = multi<float, _n>;
			static constexpr uint32 k_dim = _n;
			node_id m_parent = 0u;

			aabb<_n> m_bounds;
			vector<prim_range> m_primitive_ranges{};

			inline void set_bounds(const bounds& min, const bounds& max)
			{
				for (uint32 i = 0u; i < k_dim; ++i)
				{
					m_bounds.m_min = min;
					m_bounds.m_max = max;
				}
			}
			inline bounds get_min() const
			{
				return m_bounds.m_min;
			}
			inline bounds get_max() const
			{
				return m_bounds.m_max;
			}
		};

		template <uint32 _n>
		struct bvh_primitive final
		{
			using point = multi<float, _n>;
			point m_centroid = {};
		};

		template <uint32 _n>
		struct triangle final
		{
			multi<float, _n> m_points[3u]{};
		};

		template <uint32 _n>
		struct sphere final
		{
			multi<float, _n> m_center{};
			float m_radius = 1.0f;
		};

		template <uint32 _n>
		struct rayhit final
		{
			bool m_is_hit = false;
			multi<float, _n> m_point;
		};

		template <typename _t, uint32 _n> multi<float, _n> get_centroid(const _t& prim);
		template <typename _t, uint32 _n> rayhit<_n> intersect_ray(const ray<_n>& ray, const _t& prim);

		template <typename _t, uint32 _n>
		inline multi<float, _n> get_centroid(const triangle<_n>& tri)
		{
			using mfloat = multi<float, _n>;
			mfloat avg_sum;
			for (uint32 i = 0u; i < 3u; ++i) avg_sum += tri.m_points[i];
			avg_sum /= 3u;
			return avg_sum;
		}
		
		template <>
		inline rayhit<2u> intersect_ray(const ray<2u>& ray, const triangle<2u>& tri)
		{
			using hit_type = rayhit<2u>;
			hit_type res{};
			res.m_is_hit = false;
			res.m_point;
			return res;
		}
		
		template <>
		inline rayhit<3u> intersect_ray(const ray<3u>& ray, const triangle<3u>& tri)
		{
			using hit_type = rayhit<3u>;
			hit_type res{};
			res.m_is_hit = false;
			res.m_point;
			return res;
		}
		
		template <typename _t, uint32 _n>
		inline multi<float, _n> get_centroid(const sphere<_n>& sphere)
		{
			return sphere.m_center;
		}
		
		template <>
		inline rayhit<2u> intersect_ray(const ray<2u>& ray, const sphere<2u>& sphere)
		{
			using hit_type = rayhit<2u>;
			hit_type res;
			return res;
		}
		
		template <>
		inline rayhit<3u> intersect_ray(const ray<3u>& ray, const sphere<3u>& sphere)
		{
			using hit_type = rayhit<3u>;
			hit_type res;
			return res;
		}
	}

	enum class e_dimension : unsigned char
	{
		_2D = 2u,
		_3D = 3u,
		num
	};

	/// <summary>
	/// BVH implementation
	/// </summary>
	/// <typeparam name="_c"> _c - (capacity)	max num primitives in the BVH </typeparam>
	/// <typeparam name="_n"> _n - (dimension)	2 in 2D, 3 in 3D etc..</typeparam>
	/// <typeparam name="_d"> _d - (depth)		max recursion depth in the BVH </typeparam>
	template <uint32 _c, e_dimension _n, uint32 _d = 8u>
	class bvh final
	{
		static constexpr uint32 k_dim = static_cast<uint32>(_n);
		static constexpr uint32 k_max_num_primitives = _c;
		static constexpr uint32 k_max_num_nodes = (k_max_num_primitives * 2) + 1u;
		static constexpr detail::node_id k_root_node = 0u;
		static constexpr float k_max_bounds = 1e30f;
		static constexpr uint32 k_max_depth = _d;

		using mfloat = detail::multi<float, k_dim>;
		using node = detail::bvh_node<k_dim>;
		node m_nodes[k_max_num_nodes]{};

		struct primitive_data final
		{
			mfloat m_centroid;
		};
		primitive_data	m_primdata[k_max_num_primitives]{};

	public:
		using ray = detail::ray<k_dim>;
		using rayhit = detail::rayhit<k_dim>;
		using triangle = detail::triangle<k_dim>;
		using sphere = detail::sphere<k_dim>;

		template <typename _p>
		inline void rebuild(const vector<_p>& primitives)
		{
			node& root = root_node();
			root.set_bounds(node::bounds::make_fill(-k_max_bounds), node::bounds::make_fill(k_max_bounds));

			// store all centroids
			for (uint32 i = 0u; i < primitives.size(); ++i)
			{
				m_primdata[i].m_centroid = detail::get_centroid<_p, k_dim>(primitives[i]);
			}

			auto split_recursive = [](node& parent, uint32 depth)
			{
			};
			split_recursive(root, 0u);
		}

		template <typename _p>
		inline rayhit test_hit(const ray& ray, const vector<_p>& primitives)
		{
			vector<rayhit> all_hits{};
			for (uint32 i = 0u; i < primitives.size(); ++i)
			{
				auto this_hit = detail::intersect_ray<_p, k_dim>(ray, primitives[i]);
				if (this_hit.m_is_hit) all_hits.push_back(this_hit);
			}
			return all_hits[0];
		}

		inline node& root_node()
		{
			return m_nodes[k_root_node];
		}
	};

	template <uint32 _c, uint32 _d>
	using bvh_2D = bvh<_c, e_dimension::_2D, _d>;
	template <uint32 _c, uint32 _d>
	using bvh_3D = bvh<_c, e_dimension::_3D, _d>;
	
	using float3	= detail::multi<float, 3u>;
	using ray3D		= detail::ray<3u>;
	using rayhit3D	= detail::rayhit<3u>;

	//======================================================================================
	// want to use your own primitives? (non-triangles)
	// these are the 2 function templates you need to instantiate..
	// - detail::get_centroid(const _t& primitive) -> float3
	// - detail::intersect_ray(const ray3D& ray, const _t& primitive) -> rayhit3D
#if 0
	// example primitive
	struct mysphere { float3 m_center; float m_radius; };

	// get_centroid(const mysphere&) ...
	template <> inline float3 detail::get_centroid(const mysphere& sphere)
	{
		return sphere.m_center;
	}

	// intersect_ray(const mysphere&) ...
	template <> inline rayhit3D detail::intersect_ray(const ray3D& ray, const mysphere& sphere)
	{
		rayhit3D hit{};
		hit.m_is_hit = true;
		return hit;
	}
#endif
}