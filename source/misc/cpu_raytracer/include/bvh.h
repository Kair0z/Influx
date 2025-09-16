#pragma once

namespace influx
{
	using uint32 = unsigned int;
	template <typename _t, uint32 _n>
	struct multi final
	{
		_t m_data[_n];

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

	struct aabb final
	{

	};

	namespace detail
	{
		template <uint32 _n>
		struct bvh_node final
		{
			using bounds_type = multi<float, _n>;
			static constexpr uint32 k_dim = _n;
			node_id m_parent = 0u;
			float2 m_bounds[k_dim]{};

			inline void set_bounds(const bounds_type& min, const bounds_type& max)
			{
				for (uint32 i = 0u; i < k_dim; ++i)
				{
					m_bounds[i][0] = min[i];
					m_bounds[i][1] = max[i];
				}
			}
			inline bounds_type get_min() const
			{
				bounds_type result{};
				for (uint32 i = 0u; i < k_dim; ++i) result[i] = m_bounds[i][0];
				return result;
			}
			inline bounds_type get_max() const
			{
				bounds_type result{};
				for (uint32 i = 0u; i < k_dim; ++i) result[i] = m_bounds[i][1];
				return result;
			}
		};
	}

	template <uint32 _c, uint32 _n>
	class bvh final
	{
		static constexpr uint32 k_dim = _n;
		static constexpr uint32 k_max_num_triangles = _c;
		static constexpr uint32 k_max_num_nodes = (k_max_num_triangles * 2) - 1u;
		static constexpr node_id k_root_node = 0u;

		detail::bvh_node<_n> m_nodes[k_max_num_nodes];

	public:
		inline void rebuild()
		{
			// todo
		}

		inline detail::bvh_node& mod_root()
		{
			return m_nodes[k_root_node];
		}
	};
}