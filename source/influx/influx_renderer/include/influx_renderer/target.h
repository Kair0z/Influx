#pragma once

// influx::core
#include "core/basetypes.h"
#include "core/string.h"
// influx::renderer
#include "influx_renderer/common.h"
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
	struct target_create_args final
	{
		debug_name m_name{};

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
		inline static const char* k_depth_name_postfix = "_depth";

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
		INFLUX_RENDER_API const debug_name& get_name_depth() const;
		INFLUX_RENDER_API static debug_name get_name_depth(const debug_name& base);

		INFLUX_RENDER_API ~target();

		// ~imgui_texid_provider begin
		virtual void* get_tex_descriptor() const override final { return m_srv_cpu; }
		virtual void* get_tex_resource() const override final { return mp_resource; };
		virtual debug_name get_rendergraph_id() const override final { return get_name(); };
		// ~imgui_texid_provider end

	private:
		// constructs a target from create_args, allocating new graphics resources
		explicit target(
			graphics::device* device, const target_create_args& args);

		// does not allocate a new descriptor handle, but recreates the view
		void recreate_rtv();
		void recreate_dsv();
		void recreate_srv();

		graphics::resource* mp_resource = nullptr;
		graphics::resource* mp_depth_resource = nullptr;

		graphics::descriptor_handle m_rtv_cpu;
		graphics::descriptor_handle m_dsv_cpu;
		graphics::descriptor_handle m_srv_cpu;

		target_create_args m_createargs;
		math::vectoru2 m_current_dimensions;
		math::vectoru2 m_prev_dimensions;
		graphics::device* mp_device;
		debug_name m_depth_name;

		// only backend can create targets
		friend class renderer_backend;
	};
	
	static string get_target_pass_name(const char* pass_name, const target& target)
	{
		return pass_name + ("_" + target.get_name().get_string());
	}
}
