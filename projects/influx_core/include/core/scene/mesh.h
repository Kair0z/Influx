#pragma once

#ifndef __CORE_SCENE_MESH_H_
#define __CORE_SCENE_MESH_H_

#include "core/basetypes.h"
#include "core/geometry/vertex.h"
#include "core/container/vector.h"
#include "core/string.h"

namespace influx::scene
{
	namespace detail
	{
		struct i_mesh
		{

		};

		template <class _vertex_t, class _index_t = uint32>
		struct mesh final
		{
			using triangle = vertex_t[3u];

		public:
			mesh() = default;

			inline mesh(const vector<_vertex_t>& vertices, const vector<_index_t>& indices)
				: m_vertices{ vertices }
				, m_indices{ indices }
			{
			}

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

		private:
			vector<_vertex_t> m_vertices;
			vector<_index_t> m_indices;
		};
	}
	
	
}

#endif