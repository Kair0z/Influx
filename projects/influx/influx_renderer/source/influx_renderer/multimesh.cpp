#include "renderer_pch.h"
#include "multimesh.h"

// influx::renderer
#include "influx_renderer/renderer_backend.h"
#include "influx_renderer/descriptor_manager.h"

namespace influx::renderer
{
	void multimesh::add_mesh(const string& name, const vector<vertex_type>& vertices, const vector<index>& indices)
	{
		if (!m_name_to_indexbuffer.contains(name))
		{
			m_name_to_indexbuffer[name] = indices;
			m_name_to_vertexbuffer[name] = vertices;

			m_is_multiresource_dirty = true;
		}
	}
	void multimesh::remove_mesh(const string& name)
	{
		if (m_name_to_indexbuffer.contains(name))
		{

			m_is_multiresource_dirty = true;
		}
	}

	uint64 multimesh::get_base_vertex(const string& name)
	{
		return m_name_to_basevertex_map[name];
	}

	uint64 multimesh::get_base_index(const string& name)
	{
		return m_name_to_baseindex_map[name];
	}

	void multimesh::update_multiresource()
	{
		if (m_is_multiresource_dirty)
		{
			// prepass
			uint64 total_num_vertices = 0u;
			uint64 total_num_indices = 0u;
			for (const auto& pair : m_name_to_vertexbuffer)
			{
				const string& name = pair.first;
				const auto& vertices = pair.second;
				const auto& indices = m_name_to_indexbuffer[name];
				total_num_vertices += vertices.size();
				total_num_indices += indices.size();
			}
			const bool vertex_needs_rebuild = m_multi_vertex_content.size() < total_num_vertices;
			const bool index_needs_rebuild = m_multi_index_content.size() < total_num_indices;

			// resize the cpu buffers if need
			if (vertex_needs_rebuild) m_multi_vertex_content.resize(total_num_vertices);
			if (index_needs_rebuild) m_multi_index_content.resize(total_num_indices);

			// record & compile all offsets
			uint64 vertex_offset = 0u;
			uint64 index_offset = 0u;
			for (const auto& pair : m_name_to_vertexbuffer)
			{
				const string& name = pair.first;
				const auto& vertices = pair.second;
				const auto& indices = m_name_to_indexbuffer[name];

				m_name_to_basevertex_map[name] = vertex_offset;
				m_name_to_baseindex_map[name] = index_offset;

				memcpy(&m_multi_vertex_content[vertex_offset], vertices.data(), vertices.size() * sizeof(vertex_type));
				memcpy(&m_multi_index_content[index_offset], indices.data(), indices.size() * sizeof(index));

				vertex_offset += vertices.size();
				index_offset += indices.size();
			}

			// build gpu resources
			renderer_backend& backend = renderer_backend::get_instance();
			if (vertex_needs_rebuild)
			{
				m_multi_vertexbuffer = backend.create_vertexbuffer("multi_mesh", m_multi_vertex_content, true);
			}
			if (index_needs_rebuild)
			{
				m_multi_indexbuffer = backend.create_indexbuffer("multi_mesh", m_multi_index_content, true);
			}

			m_multidescriptor = backend.get_descriptor_manager()->create_buffer_srv(m_multi_vertexbuffer);
		}

		m_is_multiresource_dirty = false;
	}

	graphics::descriptor_handle multimesh::get_vertexbuffer_srv() const
	{
		return m_multidescriptor;
	}

	graphics::resource* multimesh::get_index_buffer() const
	{
		return m_multi_indexbuffer;
	}

	bool multimesh::is_render_read() const
	{
		return m_multidescriptor != nullptr;
	}
}