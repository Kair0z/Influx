#pragma once

// influx::core
#include "core/basetypes.h"

// influx::renderer
#include "influx_renderer/types.h"

// influx::graphics
#include "influx_graphics/descriptors.h"

// STL
#include <iostream>
#include <random>
#include <cstdint>

namespace influx::graphics
{
	class resource;
	class device;
	class swapchain;
	class descriptor_heap;
	class render_target_view;
	class depth_stencil_view;
}

namespace influx::renderer
{
	struct target_id final
	{
		explicit target_id()
		{
			// generate random uid
			std::random_device rd;
			std::mt19937 gen(rd());  // Mersenne Twister 32-bit
			std::uniform_int_distribution<uint32_t> dis;
			m_uid = dis(gen);
		}

		explicit target_id(const string& name)
			: m_uid{ (uint32)std::hash<string>()(name) }
			, m_name{ name }
		{

		}

		uint32 m_uid = 0u;
#if INFLUX_DEBUG 
		string m_name{};
#endif
	};

	inline bool operator==(const target_id& id0, const target_id& id1)
	{
		return id0.m_uid == id1.m_uid;
	}

	struct target_create_args final
	{
		target_create_args() = default;
		target_create_args(uint32 w, uint32 h)
			: m_width{ w }, m_heigth{ h } {}

		uint32 m_width = 1u;
		uint32 m_heigth = 1u;
		bool m_has_depth_stencil = true;
		bool m_has_colour = true;
	};

	// contains a texture resource, as well as a render target view and an optional depth stencil view
	// serving as a target for draw commands
	class target final : public imgui_texid_provider
	{
	public:
		graphics::resource* get_resource() const;
		graphics::resource* get_depth_resource() const;
		graphics::descriptor_handle get_rtv() const;
		graphics::descriptor_handle get_dsv() const;
		graphics::descriptor_handle get_srv() const;

		INFLUX_RENDER_API uint32 get_width() const;
		INFLUX_RENDER_API uint32 get_height() const;

		INFLUX_RENDER_API bool has_depth_stencil() const;
		INFLUX_RENDER_API bool is_depth_only() const;

		// re-allocates graphics resource, and recreates the rtv
		INFLUX_RENDER_API void resize(const math::vectoru2& new_dimensions);
		INFLUX_RENDER_API void resize(const target& target);

		INFLUX_RENDER_API void set_name(const debug_name& name);
		INFLUX_RENDER_API const debug_name& get_name() const;

		INFLUX_RENDER_API bool is_swapchain_target() const;

		INFLUX_RENDER_API string get_rendergraph_name() const;
		INFLUX_RENDER_API string get_depth_rendergraph_name() const;

		INFLUX_RENDER_API ~target();

		// ~imgui_texid_provider begin
		virtual void* get_tex_descriptor() const override final { return m_srv_cpu; }
		virtual void* get_tex_resource() const override final { return mp_resource; };
		// ~imgui_texid_provider end

	private:
		// constructs a target from create_args, allocating new graphics resources
		explicit target(
			graphics::device* device, const target_create_args& args);

		// constructs a target from existing swapchain resources
		explicit target(
			graphics::device* device,
			graphics::swapchain* swapchain,
			uint8 swapchain_index);

		// does not allocate a new descriptor handle, but recreates the view
		void recreate_rtv();
		void recreate_dsv();
		void recreate_srv();

		graphics::resource* mp_resource = nullptr;
		graphics::resource* mp_depth_resource = nullptr;

		graphics::descriptor_handle m_rtv_cpu;
		graphics::descriptor_handle m_dsv_cpu;
		graphics::descriptor_handle m_srv_cpu;
		bool m_is_swapchain_target = false;

		target_create_args m_args;
		math::vectoru2 m_current_dimensions;
		graphics::device* mp_device;

		target_id m_id;

		// only backend can create targets
		friend class renderer_backend;
	};
}
template <> struct std::hash<influx::renderer::target_id>
{
	influx::uint64 operator()(const influx::renderer::target_id& id) const
	{
		return std::hash<decltype(id.m_uid)>()(id.m_uid);
	}
};
