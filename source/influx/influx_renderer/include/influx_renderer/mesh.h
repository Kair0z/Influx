#pragma once

// influx::core
#include "core/basetypes.h"
#include "core/math/vector.h"
#include "core/container/vector.h"
#include "core/math/colour.h"

namespace influx::renderer
{
	using index = uint32;

	struct vertex_data final
	{
		math::vectorf3 m_position{};
		math::vectorf4 m_colour{};
		math::vectorf3 m_normal{};
		math::vectorf2 m_texcoords{};
	};

	enum class e_mesh : uint8
	{
		plane,
		box,
		sphere,
		triangle,
		quad,
		count
	};

	static constexpr uint8 k_num_internal_meshes = static_cast<uint32>(e_mesh::count);
	static const char* k_internal_mesh_names[k_num_internal_meshes] =
	{
		"internal_plane",
		"internal_box",
		"internal_sphere",
		"internal_triangle",
		"internal_quad",
	};

	inline constexpr const char* get_internal_mesh_name(const e_mesh& mesh)
	{
		return k_internal_mesh_names[static_cast<uint32>(mesh)];
	}

	static mesh_id get_internal_mesh_id(const e_mesh& mesh)
	{
		return static_cast<uint32>(mesh);
	}

	static bool is_internal_mesh(const mesh_id id)
	{
		return static_cast<uint32>(id) < k_num_internal_meshes;
	}

	namespace detail
	{
		class base_mesh_data
		{
		public:
			virtual uint64 get_vert_bytesize() const = 0;
			virtual uint64 get_vert_bytestride() const = 0;
			virtual void const* get_vert_data() const = 0;

			virtual uint64 get_indx_bytesize() const = 0;
			virtual uint64 get_indx_bytestride() const = 0;
			virtual void const* get_indx_data() const = 0;
		};
	}

	template <typename _vt = vertex_data>
	class mesh_data final : public detail::base_mesh_data
	{
		virtual uint64 get_vert_bytesize() const override { return m_vertices.size() * get_vert_bytestride(); }
		virtual uint64 get_vert_bytestride() const override { return sizeof(_vt); }
		virtual void const* get_vert_data() const override { return reinterpret_cast<void const*>(m_vertices.data()); }
		
		virtual uint64 get_indx_bytesize() const override { return m_indices.size() * get_indx_bytestride(); }
		virtual uint64 get_indx_bytestride() const override { return sizeof(index); }
		virtual void const* get_indx_data() const override { return reinterpret_cast<void const*>(m_indices.data()); }

	public:
		vector<_vt>		m_vertices{};
		vector<index>	m_indices{};
	};

	static const mesh_data<vertex_data>& get_inline_mesh_plane()
	{
		static mesh_data<vertex_data> inline_mesh{};
		if (inline_mesh.m_vertices.size() == 0u)
		{
			const static math::vectorf3 positions[4u]
			{
				{ 1.0f, 0.0f, 1.0f },
				{ -1.0f, 0.0f, 1.0f },
				{ 1.0f, 0.0f, -1.0f },
				{ -1.0f, 0.0f, -1.0f }
			};
			const static math::vectorf4 colours[4u]
			{
				{ 1.0f, 0.0f, 0.0f, 1.0f },
				{ 0.0f, 1.0f, 0.0f, 1.0f },
				{ 0.0f, 0.0f, 1.0f, 1.0f },
				{ 1.0f, 1.0f, 0.0f, 1.0f }
			};
			const static math::vectorf2 uvs[4u]
			{
				{ 0.0f, 0.0f },
				{ 1.0f, 0.0f },
				{ 1.0f, 1.0f },
				{ 0.0f, 1.0f }
			};
			const static math::vectorf3 normals[4u]
			{
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f }
			};

			inline_mesh.m_vertices.resize(4u);
			inline_mesh.m_indices.resize(6u);

			for (uint8 i = 0u; i < 4u; ++i)
			{
				inline_mesh.m_vertices[i] = {
					.m_position{positions[i]},
					.m_colour{colours[i]},
					.m_normal{normals[i]},
					.m_texcoords{uvs[i]} };
			}

			inline_mesh.m_indices[0] = 0u;
			inline_mesh.m_indices[1] = 2u;
			inline_mesh.m_indices[2] = 1u;
			inline_mesh.m_indices[3] = 2u;
			inline_mesh.m_indices[4] = 3u;
			inline_mesh.m_indices[5] = 1u;
		}
		
		return inline_mesh;
	}

	static const mesh_data<vertex_data>& get_inline_mesh_box()
	{
		static mesh_data<vertex_data> inline_mesh{};
		if (inline_mesh.m_vertices.size() == 0u)
		{
			const static float offset = 0.5f;
			const static uint32 num_vertices = 24u;
			const static uint32 num_triangles = 12u;
			const static uint32 num_indices = num_triangles * 3u;
			const static math::vectorf3 positions[num_vertices]
			{
				// Front face
				{- offset, -offset,  offset, },  // Bottom-left (0)
				{ offset, -offset,  offset,  }, // Bottom-right (1)
				{ offset,  offset,  offset,  }, // Top-right (2)
				{-offset,  offset,  offset,  }, // Top-left (3)
				// Back face
				{-offset, -offset, -offset,  }, // Bottom-left (4)
				{ offset, -offset, -offset,  }, // Bottom-right (5)
				{ offset,  offset, -offset,  }, // Top-right (6)
				{-offset,  offset, -offset,  }, // Top-left (7)
				// Top face
				{-offset,  offset, -offset,  }, // Back-left (8)
				{ offset,  offset, -offset,  }, // Back-right (9)
				{ offset,  offset,  offset,  }, // Front-right (10)
				{-offset,  offset,  offset,  }, // Front-left (11)
				// Bottom face 
				{-offset, -offset, -offset,  }, // Back-left (12)
				{ offset, -offset, -offset,  }, // Back-right (13)
				{ offset, -offset,  offset,  }, // Front-right (14)
				{-offset, -offset,  offset,  }, // Front-left (15)
				// Left face
				{-offset, -offset, -offset,  }, // Back-bottom (16)
				{-offset,  offset, -offset,  }, // Back-top (17)
				{-offset,  offset,  offset,  }, // Front-top (18)
				{-offset, -offset,  offset,  }, // Front-bottom (19)
				// Right face
				{ offset, -offset, -offset,  }, // Back-bottom (20)
				{ offset,  offset, -offset,  }, // Back-top (21)
				{ offset,  offset,  offset,  }, // Front-top (22)
				{ offset, -offset,  offset   } // Front-bottom (23)
			};
			const static math::vectorf4 colours[num_vertices]
			{
				{1,0,0},
				{1,0,0},
				{1,0,0},
				{1,0,0},
				{1,0,0},
				{1,0,0},
				{1,0,0},
				{1,0,0},
				{1,0,0},
				{1,0,0},
				{1,0,0},
				{1,0,0},
				{1,0,0},
				{1,0,0},
				{1,0,0},
				{1,0,0},
				{1,0,0},
				{1,0,0},
				{1,0,0},
				{1,0,0},
				{1,0,0},
				{1,0,0},
				{1,0,0},
				{1,0,0}
			};
			const static math::vectorf2 uvs[num_vertices]
			{
				{0.0, 0.0},
				{1.0, 0.0},
				{1.0, 1.0},
				{0.0, 1.0},
				{0.0, 0.0},
				{1.0, 0.0},
				{1.0, 1.0},
				{0.0, 1.0},
				{0.0, 0.0},
				{1.0, 0.0},
				{1.0, 1.0},
				{0.0, 1.0},
				{0.0, 0.0},
				{1.0, 0.0},
				{1.0, 1.0},
				{0.0, 1.0},
				{0.0, 0.0},
				{1.0, 0.0},
				{1.0, 1.0},
				{0.0, 1.0},
				{0.0, 0.0},
				{1.0, 0.0},
				{1.0, 1.0},
				{0.0, 1.0}
			};
			const static math::vectorf3 normals[num_vertices]
			{
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f }
			};

			inline_mesh.m_vertices.resize(num_vertices);
			for (uint8 i = 0u; i < num_vertices; ++i)
			{
				inline_mesh.m_vertices[i] = {
					.m_position{positions[i]},
					.m_colour{colours[i]},
					.m_normal{normals[i]},
					.m_texcoords{uvs[i]} };
			}

			inline_mesh.m_indices =
			{
				0, 1, 2,		2, 3, 0,
				4, 5, 6,		6, 7, 4,
				8, 9, 10,		10, 11, 8,
				12, 13, 14,		14, 15, 12,
				16, 17, 18,		18, 19, 16,
				20, 21, 22,		22, 23, 20
			};
		}
		return inline_mesh;
	}

	static const mesh_data<vertex_data>& get_inline_mesh_sphere()
	{
		static mesh_data<vertex_data> inline_mesh{};
		if (inline_mesh.m_vertices.size() == 0u)
		{
			inline_mesh.m_vertices;
			inline_mesh.m_indices;

			const static uint32 resolution = 18u;
			const static float radius = 1.0f;
			const static uint32 num_stacks = resolution;
			const static uint32 num_slices = resolution;

			// generate vertices
			for (uint32 st = 0u; st < num_stacks; ++st)
			{
				const float stack_angle = math::k_PIHalf - st * math::k_PI / num_stacks;
				const float xy = radius * math::cosf(stack_angle);
				const float z = radius * math::sinf(stack_angle);

				for (uint32 sl = 0u; sl < num_slices; ++sl)
				{
					const float sliceAngle = sl * 2 * math::k_PI / num_slices; // From 0 to 2*PI
					const float x = xy * math::cosf(sliceAngle); // X position
					const float y = xy * math::sinf(sliceAngle); // Y position

					const float inv_length = 1.0f / sqrtf(x * x + y * y + z * z);
					vertex_data new_vertex{};
					new_vertex.m_position = { x, y, z };
					new_vertex.m_normal = { x * inv_length, y * inv_length, z * inv_length };
					new_vertex.m_texcoords = { (float)sl / num_slices, (float)st / num_stacks };
					inline_mesh.m_vertices.push_back(new_vertex);
				}
			}

			// generate indices
			for (uint32 st = 0; st < num_stacks; ++st) 
			{
				for (uint32 sl = 0; sl < num_slices; ++sl) 
				{
					const uint32 first = st * (num_slices + 1) + sl;
					const uint32 second = first + num_slices + 1;

					// First triangle
					inline_mesh.m_indices.push_back(first);
					inline_mesh.m_indices.push_back(second);
					inline_mesh.m_indices.push_back(first + 1);

					// Second triangle
					inline_mesh.m_indices.push_back(second);
					inline_mesh.m_indices.push_back(second + 1);
					inline_mesh.m_indices.push_back(first + 1);
				}
			}
		}
		return inline_mesh;
	}

	static const mesh_data<vertex_data>& get_inline_mesh_triangle()
	{
		static mesh_data<vertex_data> inline_mesh{};
		if (inline_mesh.m_vertices.size() == 0u)
		{
			const static math::vectorf3 positions[3u]
			{
				{ 1.0f, 0.0f, 0.0f },
				{ -1.0f, 0.0f, 0.0f },
				{ 0.0f, 1.0f, 0.0f }
			};
			const static math::vectorf4 colours[3u]
			{
				{ 1.0f, 0.0f, 0.0f, 1.0f },
				{ 0.0f, 1.0f, 0.0f, 1.0f },
				{ 0.0f, 0.0f, 1.0f, 1.0f }
			};
			const static math::vectorf2 uvs[3u]
			{
				{ 1.0f, 1.0f },
				{ 0.0f, 1.0f },
				{ 0.0f, 0.0f }
			};
			const static math::vectorf3 normals[3u]
			{
				{ 0.0f, 0.0f, 1.0f },
				{ 0.0f, 0.0f, 1.0f },
				{ 0.0f, 0.0f, 1.0f }
			};

			inline_mesh.m_vertices.resize(3u);
			inline_mesh.m_indices.resize(3u);

			for (uint8 i = 0u; i < 3u; ++i)
			{
				inline_mesh.m_vertices[i] = {
					.m_position{positions[i]},
					.m_colour{colours[i]},
					.m_normal{normals[i]},
					.m_texcoords{uvs[i]} };
			}

			inline_mesh.m_indices[0] = 0u;
			inline_mesh.m_indices[1] = 1u;
			inline_mesh.m_indices[2] = 2u;
		}
		return inline_mesh;
	}

	static const mesh_data<vertex_data>& get_inline_mesh_quad()
	{
		static mesh_data<vertex_data> inline_mesh{};
		if (inline_mesh.m_vertices.size() == 0u)
		{
			inline_mesh = get_inline_mesh_triangle(); // todo: fix
		}
		return inline_mesh;
	}

	template <e_mesh _e>
	static const mesh_data<vertex_data>& get_inline_mesh()
	{
		if constexpr (_e == e_mesh::box) return get_inline_mesh_box();
		else if constexpr (_e == e_mesh::plane) return get_inline_mesh_plane();
		else if constexpr (_e == e_mesh::sphere) return get_inline_mesh_sphere();
		else if constexpr (_e == e_mesh::triangle) return get_inline_mesh_triangle();
		else if constexpr (_e == e_mesh::quad) return get_inline_mesh_quad();
		else
		{
			static_assert(false);
		}
	}
}