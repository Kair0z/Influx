#pragma once

#ifndef __CORE_SCENE_MESH_H_
#define __CORE_SCENE_MESH_H_

#define CORE_SCENE_MESH_DEBUG 1

#include "Core/Geometry/Vertex.h"
#include "Core/Container/Containers.h"
#include "Core/String.h"

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
			AddVertex(a);
			AddIndex((Index)m_vertices.size() - 1u);

			AddVertex(b);
			AddIndex((Index)m_vertices.size() - 1u);

			AddVertex(c);
			AddIndex((Index)m_vertices.size() - 1u);
		}

		inline void AddVertex(const Vertex& v)
		{
			m_vertices.push_back(v);
		}

		inline void AddIndex(const Index& i)
		{
			m_indices.push_back(i);
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

#if CORE_SCENE_MESH_DEBUG
	public:
		inline void SetName(const String& name)
		{
			m_name = name;
		}

		inline const String& GetName() const
		{
			return m_name;
		}

	private:
		String m_name = "";
#endif
	};
}

#endif