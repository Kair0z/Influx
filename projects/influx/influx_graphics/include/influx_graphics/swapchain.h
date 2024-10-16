#pragma once
#include "influx_graphics/base.h"
#include "influx_graphics/common.h"

#include "core/math/vector.h"
#include "core/container/vector.h"

// influx::platform
#include "influx_platform/window.h"

namespace influx::graphics
{
	class resource;
	class render_target_view;
	class device;
	class descriptor_heap;
	class queue;

	struct swapchain_desc final
	{
		e_format m_format = e_format::rgba8;
		math::vectoru2 m_dimensions{};
		uint8 m_num_buffers = 2u;
	};

	struct present_args final
	{
		bool m_vsync = false;
	};

	struct swapchain_dependencies final
	{
		swapchain_dependencies(device* device, queue* queue)
			: mp_device{ device }, mp_queue{ queue } {}

		device* mp_device = nullptr;
		queue* mp_queue = nullptr;
	};

	class swapchain : public base
	{
	public:
		INFLUX_GFX_API virtual void present(const present_args& args) = 0;

		// acquires the next available backbuffer (and returns the index)
		INFLUX_GFX_API virtual uint8 acquire_backbuffer() = 0;

		INFLUX_GFX_API void resize(const math::vectoru2& new_dimensions);
		INFLUX_GFX_API void resize(const platform::window& window);

		INFLUX_GFX_API resource* get_backbuffer_resource(uint8 at_index) const;

		INFLUX_GFX_API resource* get_current_backbuffer_resource() const;

		INFLUX_GFX_API uint8 get_num_backbuffers() const;

		INFLUX_GFX_API const swapchain_desc& get_desc() const;

		// checks the window handle to find wether a recreate of resources is necessary
		INFLUX_GFX_API bool needs_recreate(const platform::window& window) const;

		INFLUX_GFX_API uint8 get_current_backbuffer_index() const;

		INFLUX_GFX_API const math::vectoru2& get_dimensions() const;

		INFLUX_GFX_API e_format get_format() const;

		INFLUX_GFX_API void destroy_resources();

	protected:
		swapchain(const swapchain_desc& desc, const swapchain_dependencies& dependencies);
		virtual ~swapchain();

		void update_backbuffer_index(uint8 new_index);

		device* get_parent_device();

		vector<resource*> mp_buffer_resources;
	private:
		swapchain_desc m_desc{};
		math::vectoru2 m_current_dimensions{};
		uint32 m_current_backbuffer_index = 0u;

		// dependencies
		device* mp_parent_device;
		queue* mp_queue;

		virtual vector<resource*> create_resources() = 0;
	};
}