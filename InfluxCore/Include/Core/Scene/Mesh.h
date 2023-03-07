#pragma once

#ifndef __CORE_SCENE_MESH_H_
#define __CORE_SCENE_MESH_H_

#include "Core/Geometry/Vertex.h"
#include "Core/Container/Containers.h"

namespace Influx::Scene
{
	struct Mesh final
	{
		using Index = uint32;
		using Vertex = Math::Vertex;
		using Triangle = Vertex[3u];

	public:
		Mesh() = default;

		inline Mesh(const Vector<Vertex>& vertices, const Vector<Index>& indices)
			: m_vertices{vertices}
			, m_indices{indices}
		{
		}

		inline Mesh(const Vector<Triangle>& triangles)
		{
			m_vertices.reserve(triangles.size() * 3u);
			m_indices.reserve(triangles.size() * 3u);

			for (size_t i = 0; i < triangles.size(); ++i)
			{
				AddTriangle(triangles[i]);
			}
		}

		inline void AddTriangle(const Triangle& triangle)
		{
			AddTriangle(triangle[0], triangle[1], triangle[2]);
		}

		inline void AddTriangle(const Vertex& a, const Vertex& b, const Vertex& c)
		{
			m_vertices.push_back(a);
			m_indices.push_back((Index)m_vertices.size() - 1u);

			m_vertices.push_back(b);
			m_indices.push_back((Index)m_vertices.size() - 1u);

			m_vertices.push_back(c);
			m_indices.push_back((Index)m_vertices.size() - 1u);
		}

		const Vector<Vertex>& GetVertices() const
		{
			return m_vertices;
		}

		const Vector<Index>& GetIndices() const
		{
			return m_indices;
		}

	private:
		Vector<Vertex> m_vertices;
		Vector<Index> m_indices;
	};
}

#endif