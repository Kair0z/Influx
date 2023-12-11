#pragma once

#ifndef __INFLUX_RENDERER_H_
#define __INFLUX_RENDERER_H_

#if _DLL
	#define INFLUX_RENDER_API __declspec(dllexport)
#else
	#define INFLUX_RENDER_API __declspec(dllimport)
#endif

#include "Core/basetypes.h"
#include "Core/Function.h"
#include "Core/Platform/Platform.h"
#include "Core/Container/Vector.h"
#include "Core/String.h"

#include "Core/Math/Vector.h"
#include "Core/Math/Matrix.h"

namespace influx::renderer
{
	enum class INFLUX_RENDER_API e_buffering
	{
		dubble = 2,
		tripple = 3,
		max
	};

#pragma region konstants
	constexpr static bool	k_useWarp = true;
	constexpr static uint8	k_max_srvs = 64u;
	constexpr static e_buffering k_default_buffering = e_buffering::dubble;
	constexpr static uint8 k_max_swapchain_buffers = static_cast<uint8>(e_buffering::max) - 1u;
	constexpr static uint8 k_default_num_swapchain_buffers = static_cast<uint8>(k_default_buffering);
	constexpr static uint8 k_max_num_frames_in_flight = k_default_num_swapchain_buffers;
	constexpr static uint32 k_max_stat_frames = 2048u;
#pragma endregion

	enum class e_render_api
	{
		dx12,

		unsupported, // everything below is unsupported!
		vulkan,
		rhi,
		max
	};

	struct frame_stats final
	{
		float m_ms_acquire = 0.0f; // percentage of total spent on acquire
		float m_ms_build = 0.0f; // time spent on generating the cmdlist
		float m_ms_frame = 0.0f; // time spend on a frame

		frame_stats& operator+=(const frame_stats& stats)
		{
			m_ms_acquire += stats.m_ms_acquire;
			m_ms_build += stats.m_ms_build;
			m_ms_frame += stats.m_ms_frame;
			return *this;
		}

		frame_stats& operator/=(float val)
		{
			m_ms_acquire /= val;
			m_ms_build /= val;
			m_ms_frame /= val;
			return *this;
		}

		static frame_stats average(const vector<frame_stats>& stats)
		{
			frame_stats result{};
			uint64 num = stats.size();
			for (uint64 i = 0u; i < num; ++i)
			{
				result += stats[i];
			}

			result /= (float)num;
			return result;
		}
	};

	struct init_args final
	{
		e_render_api m_api_type = e_render_api::dx12;
		string m_resource_dir = "";
	};

	struct present_args final
	{
		bool m_vsync = false;
	};
	
	struct render_args final
	{
		math::vectorf4 m_clear_colour = {};
	};

#pragma region scene proxy
	struct camera_proxy final
	{
		float m_fov = 90.0f;
		float m_near_plane = 0.0001f;
		float m_far_plane = 100.0f;

		math::vectorf3 m_position = {};
		math::vectorf3 m_forward = -math::vectorf3::forward();
		math::matrix4x4f m_transform = math::matrix4x4f::identity();

		inline void look_at(const math::vectorf3& at)
		{
			m_forward = (at - m_position).normalized();
		}
	};

	struct mesh_proxy final
	{
		string m_name = "";
		string m_material_name = "";
		math::vectorf4 m_per_instance_colour = {};
		math::matrix4x4f m_transform = math::matrix4x4f::identity();
	};

	struct scene_proxy final
	{
		vector<mesh_proxy> m_meshes = {};
		vector<camera_proxy> m_cameras = {};
		// lightproxy ...
	};

	using imgui_proxy = function<void(void* ctx)>;

#pragma endregion

	using index = uint32;

	struct vertex_data final
	{
		math::vectorf3 m_position{};
		math::vectorf4 m_colour{};
		math::vectorf3 m_normal{};
		math::vectorf2 m_texcoords{};
	};

	struct mesh_data final
	{
		vector<vertex_data> m_vertices{};
		vector<index> m_indices{};

		bool is_valid() const;
	};

	struct texture_data final
	{
		vector<math::vectorf4> m_pixels{};
		uint32 m_width = 0u;

		uint32 get_width() const
		{
			return m_width;
		}

		uint32 get_height() const
		{
			return static_cast<uint32>(m_pixels.size()) / get_width();
		}

		bool is_valid() const;
	};

	struct material_data final
	{
		math::vectorf4 m_albedo{};
	};

	INFLUX_RENDER_API void initialize(const init_args& args);

	INFLUX_RENDER_API bool is_initialized();
	INFLUX_RENDER_API bool is_initialized_imgui();
	INFLUX_RENDER_API void cleanup();

	INFLUX_RENDER_API void render_to_window(
		const scene_proxy* scene_proxy, 
		platform::window_handle window, 
		const imgui_proxy* imgui_proxy = nullptr,
		const render_args& render_args = {}, 
		const present_args& present = {});

	INFLUX_RENDER_API void load(const string& title, const mesh_data& data);
	INFLUX_RENDER_API void load(const string& title, const texture_data& data);
	INFLUX_RENDER_API void load(const string& title, const material_data& data);
	INFLUX_RENDER_API const mesh_data* find_mesh_data(const string& title); 
	INFLUX_RENDER_API vector<const mesh_data*> get_all_mesh_datas();

	INFLUX_RENDER_API void* get_backend_device();

	INFLUX_RENDER_API vector<frame_stats> get_frame_stats(const uint32 over_num_frames = 1u);
}

#endif