#pragma once

// influx::graphics
#include "influx_graphics/base.h"
#include "influx_graphics/common.h"

// influx::core
#include "core/math/vector.h"
#include "core/container/vector.h"
#include "core/pointer.h"

// influx::platform
#include "influx_platform/window.h"

namespace influx::graphics
{
	class device;
	class resource;
	class render_target_view;
	class descriptor_heap;
	class queue;

	struct swapchain_desc final
	{
		static swapchain_desc default_double()
		{
			static swapchain_desc result{};
			result.m_format = e_format::rgba8;
			result.m_num_buffers = 2u;
			return result;
		}

		static swapchain_desc default_tripple()
		{
			static swapchain_desc result{};
			result.m_format = e_format::rgba8;
			result.m_num_buffers = 3u;
			return result;
		}

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
		/* flips the backbuffer into the window visible buffer  */
		INFLUX_GFX_API virtual result<> present(const present_args& args) = 0;

		/* acquires the next available backbuffer(and returns the index) */
		INFLUX_GFX_API virtual result<> acquire_backbuffer() = 0;

		/* recreates resources according to the new dimensions of the window */
		INFLUX_GFX_API result<> resize(device*, const math::vectoru2& new_dimensions);
		INFLUX_GFX_API result<> resize(device*, const platform::window& window);

		/* */
		INFLUX_GFX_API result<resource*> get_backbuffer_resource(uint8 at_index) const;

		/* */
		INFLUX_GFX_API result<resource*> get_current_backbuffer_resource();

		/* */
		INFLUX_GFX_API uint8 get_num_backbuffers() const;

		INFLUX_GFX_API const swapchain_desc& get_desc() const;

		// checks the window handle to find wether a recreate of resources is necessary
		INFLUX_GFX_API bool needs_recreate(const platform::window& window) const;

		/* */
		INFLUX_GFX_API virtual result<uint8> get_current_backbuffer_index() = 0;
		INFLUX_GFX_API uint8 get_current_backbuffer_index() const;

		INFLUX_GFX_API const math::vectoru2& get_dimensions() const;

		INFLUX_GFX_API e_format get_format() const;

	protected:
		swapchain(
			const swapchain_desc& desc, 
			const swapchain_dependencies& dependencies);

		virtual ~swapchain();

		device* get_parent_device();

		vector<resource*>& get_resources();
		uint32 m_current_backbuffer_index = 0u;

		swapchain_desc m_desc{};
		math::vectoru2 m_current_dimensions{};
		vector<resource*> mp_resources;

		// dependencies
		device* mp_parent_device;
		queue* mp_queue;

		/* (re)creates graphics api resources using the passed device */
		virtual result<vector<resource*>> create_resources(device*) = 0;

		/* */
		virtual result<> resize_impl(const math::vectoru2& old_dim, const math::vectoru2& new_dim) = 0;

		/* destroys graphics api resources using the passed device */
		virtual result<> destroy_resources(device*) = 0;
	};
}