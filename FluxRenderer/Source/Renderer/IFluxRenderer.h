#pragma once

#include "Core/Math/Vector.h"
#include "Core/Scene/Mesh.h"
#include "Core/Scene/Camera.h"
#include "Core/Container/Vector.h"
#include "Core/Platform/Platform.h"

namespace influx
{
	class base_flux_renderer
	{
	protected:
		base_flux_renderer() = default;

	public:
		enum class e_swapchain_buffering : uint8
		{
			single = 1u,
			dubble = 2u,
			tripple = 3u,
			max
		};

		struct material_data final
		{
			material_data() = default;
			material_data(const vector<byte>& vs, const vector<byte>& ps)
				: m_vertexShader{ vs }, m_pixelShader{ ps } {}

			vector<byte> m_vertexShader;
			vector<byte> m_pixelShader;
		};

		struct vertex final
		{
			math::vectorf3 m_position{};
		};

		using index = uint32;

		struct mesh_data final
		{
			vector<vertex> m_vertices{};
		};

		virtual void record_commands(platform::window_handle windowHandle) = 0;

		virtual void present_to_window(platform::window_handle windowHandle) = 0;

		void set_material(const material_data& material)
		{
			m_material = material;
		}

		void add_mesh(const mesh_data& mesh)
		{
			m_meshes.push_back(mesh);
		}
		
		void set_camera_transform(const math::vectorf3& position, const math::vectorf3& forward)
		{
			m_cameraPosition = position;
			m_cameraForward = forward;
		}

		void set_camera_data(const scene::camera& cameraData)
		{
			m_camera_data = cameraData;
		}

		const vector<mesh_data>& GetMeshes() const
		{
			return m_meshes;
		}

		const scene::camera& GetCameraData() const
		{
			return m_camera_data;
		}

		const math::vectorf3& get_camera_position() const
		{
			return m_camera_position;
		}

		const math::vectorf3& get_camera_forward() const
		{
			return m_camera_forward;
		}

		const material_data& GetMaterial() const
		{
			return m_material;
		}

		virtual ~base_flux_renderer() = default;

	private:
		scene::camera m_camera_data;
		math::vectorf3 m_camera_position;
		math::vectorf3 m_camera_forward;
		material_data m_material;

		uint64 m_num_vertices;
		uint64 m_num_indices;

		vector<mesh_data> m_meshes;

	protected:
		constexpr static e_swapchain_buffering k_swapchain_buffering = e_swapchain_buffering::tripple;

		constexpr static uint64 get_vertex_size() { return sizeof(vertex); }
		constexpr static uint64 get_index_size() { return sizeof(index); }

		const uint64 get_vertex_buffer_size() const
		{
			return m_num_vertices * get_vertex_size();
		}

		const uint64 get_index_buffer_size() const
		{
			return m_num_indices * get_index_size();
		}

		constexpr static uint8 get_num_swapchain_buffers() { return static_cast<uint8>(k_swapchain_buffering); }
	};
}


