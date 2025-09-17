#pragma once

namespace influx
{
	using uint32 = unsigned int;
	template <typename _t, uint32 _n>
	struct multi final
	{
		_t m_data[_n];

		multi() = default;
		multi(const _t& x, const _t& y, const _t& z)
			: m_data[0]{ x }, m_data[1]{ y }, m_data{ z }{}

		static multi make_fill(const _t& val)
		{
			multi res{};
			for (uint32 i = 0u; i < _n; ++i) res.m_data[i] = val;
			return res;
		}

		_t& mod(uint32 index)
		{
			return m_data[index];
		}
		_t& operator[](uint32 index)
		{
			return mod(index);
		}
	};
	using float2 = multi<float, 2u>;
	using float3 = multi<float, 3u>;
	
	using node_id = uint32;

	namespace detail
	{
		template <uint32 _n>
		struct bvh_node final
		{
			using bounds = multi<float, _n>;
			static constexpr uint32 k_dim = _n;
			node_id m_parent = 0u;

			// aabb
			bounds m_bounds_min{};
			bounds m_bounds_max{};

			inline void set_bounds(const bounds& min, const bounds& max)
			{
				for (uint32 i = 0u; i < k_dim; ++i)
				{
					m_bounds_min = min[i];
					m_bounds_max = max[i];
				}
			}
			inline bounds get_min() const
			{
				return m_bounds_min;
			}
			inline bounds get_max() const
			{
				return m_bounds_max;
			}
		};

		template <uint32 _n>
		struct bvh_primitive final
		{
			using point = multi<float, _n>;
			point m_centroid = {};
		};
	}

	/// <summary>
	/// BVH implementation
	/// </summary>
	/// <typeparam name="_c"> _c - (capacity)	max num primitives in the BVH </typeparam>
	/// <typeparam name="_n"> _n - (dimension)	2 in 2D, 3 in 3D etc..</typeparam>
	/// <typeparam name="_d"> _d - (depth)		max recursion depth in the BVH </typeparam>
	template <uint32 _c, uint32 _n, uint32 _d = 8u>
	class bvh final
	{
		static constexpr uint32 k_dim = _n;
		static constexpr uint32 k_max_num_primitives = _c;
		static constexpr uint32 k_max_num_nodes = (k_max_num_primitives * 2) - 1u;
		static constexpr node_id k_root_node = 0u;
		static constexpr float k_max_bounds = 1e30f;
		static constexpr uint32 k_max_depth = _d;

		using node = detail::bvh_node<_n>;
		node m_nodes[k_max_num_nodes];
		
	public:
		inline void rebuild()
		{
			node& root = root_node();
			root.set_bounds(node::bounds::make_fill(-k_max_bounds), node::bounds::make_fill(k_max_bounds));

			for (uint32 i = 0u; i < k_max_depth; ++i)
			{

			}
		}

		inline node& root_node()
		{
			return m_nodes[k_root_node];
		}
	};

	template <uint32 _c, uint32 _d>
	using bvh_2D = bvh<_c, 2u, _d>;
	template <uint32 _c, uint32 _d>
	using bvh_3D = bvh<_c, 3u, _d>;
}