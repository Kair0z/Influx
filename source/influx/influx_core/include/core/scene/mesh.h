#pragma once

#include "core/basetypes.h"
#include "core/container/vector.h"
#include "core/math/vector.h"
#include "core/string.h"

namespace influx::scene
{
	enum class e_vertex_attribute : uint8
	{
		position,
		colour,
		normal,
		texcoord_array,
		count
	};
	static constexpr uint8 k_max_num_vertex_attributes = static_cast<uint8>(e_vertex_attribute::count);
	static constexpr uint8 k_max_num_texcoords_per_vertex = 8u;
	static constexpr uint8 k_max_num_colours_per_vertex = 8u;
	static const uint8 k_num_floats_per_attribute[k_max_num_vertex_attributes]
	{
		3u,									// position
		k_max_num_colours_per_vertex * 4u,  // colour
		3u,									// normal
		k_max_num_texcoords_per_vertex * 2u // texcoord_array
	};

	namespace detail
	{
		template <class _vertex_t, class _index_t = uint32>
		class mesh final
		{
		public:
			using triangle = _vertex_t[3u];
			using vertex = _vertex_t;
			using index = _index_t;
			mesh() = default;

		private:
			vector<_vertex_t> m_vertices;
			vector<_index_t> m_indices;

		public:
			/* construct a mesh from vertices + indices */
			inline mesh(const vector<_vertex_t>& vertices, const vector<_index_t>& indices)
			{
				m_vertices = vertices;
				m_indices = indices;
			}

			/* construct a mesh from a list of triangles */
			inline mesh(const vector<triangle>& triangles)
			{
				m_vertices.reserve(triangles.dimension() * 3u);
				m_indices.reserve(triangles.dimension() * 3u);

				for (size_t i = 0; i < triangles.dimension(); ++i)
				{
					add_triangle(triangles[i]);
				}
			}

			inline void add_triangle(const triangle& triangle)
			{
				add_triangle(triangle[0], triangle[1], triangle[2]);
			}

			inline void add_triangle(const _vertex_t& a, const _vertex_t& b, const _vertex_t& c)
			{
				add_vertex(a);
				add_index((_index_t)m_vertices.dimension() - 1u);

				add_vertex(b);
				add_index((_index_t)m_vertices.dimension() - 1u);

				add_vertex(c);
				add_index((_index_t)m_vertices.dimension() - 1u);
			}

			inline void add_vertex(const _vertex_t& v)
			{
				m_vertices.push_back(v);
			}

			inline void add_index(const _index_t& i)
			{
				m_indices.push_back(i);
			}

			const vector<_vertex_t>& get_vertices() const
			{
				return m_vertices;
			}

			const vector<_index_t>& get_indices() const
			{
				return m_indices;
			}
		};
	}

	using mesh = detail::mesh<math::vectorf3>;
}