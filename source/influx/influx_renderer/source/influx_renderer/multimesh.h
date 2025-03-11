#pragma once

// influx::core
#include "core/basetypes.h"

// influx::graphics
#include "influx_graphics/descriptors.h"

// influx::renderer
#include "influx_renderer.h"

namespace influx::graphics
{
	class resource;
}

namespace influx::renderer
{
	class multimesh final
	{
		using vertex_type = vertex_data;

	public:
		void add_mesh(const string& name,
			const vector<vertex_type>& vertices,
			const vector<index>& indices,
			const math::matrix4x4f& transform = math::matrix4x4f::identity());

		void remove_mesh(const string& name);

		uint64 get_base_vertex(const string& name) const;
		uint64 get_base_index(const string& name) const;
		uint64 get_num_vertices(const string& name) const;
		uint64 get_num_indices(const string& name) const;
		uint64 get_num_meshes() const;

		void update_multiresource();

		graphics::descriptor_handle get_vertexbuffer_srv() const;

		graphics::resource* get_index_buffer() const;

		bool is_render_read() const;

	private:
		graphics::resource* m_multi_indexbuffer = nullptr;
		graphics::resource* m_multi_vertexbuffer = nullptr;
		graphics::descriptor_handle m_multidescriptor = nullptr;

		map<string, uint64> m_name_to_basevertex_map{};
		map<string, uint64> m_name_to_baseindex_map{};
		map<string, vector<vertex_type>> m_name_to_vertexbuffer{};
		map<string, vector<index>> m_name_to_indexbuffer{};

		vector<vertex_type> m_multi_vertex_content;
		vector<index> m_multi_index_content{};

		bool m_is_size_dirty = false;
		bool m_is_content_dirty = false;
	};
}