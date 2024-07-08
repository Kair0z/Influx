#pragma once
#include "core/basetypes.h"
#include "core/math/vector.h"
#include "core/container/vector.h"

namespace influx::graphics
{
	class resource;
	class device;
	class descriptor_heap;
	class input_resource_view;
}

namespace influx::renderer
{
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

	struct texture_create_args final
	{
		texture_create_args() = default;
		texture_create_args(uint32 w, uint32 h)
			: m_width{ w }, m_heigth{ h } {}

		uint32 m_width = 1u;
		uint32 m_heigth = 1u;
	};

	// contains a texture resource, as well as a input resource view (irv)
	class texture
	{
	public:
		graphics::resource* get_resource() const;
		graphics::input_resource_view* get_irv() const;

		uint32 get_width() const;
		uint32 get_height() const;

#if _DEBUG
		void set_name(const string& name);
		const string& get_name() const;
#endif

	private:
		// constructs a target from create_args, allocating new graphics resources
		explicit texture(graphics::device* device, graphics::descriptor_heap* irv_heap, const texture_create_args& args);
		explicit texture(graphics::device* device, graphics::resource* resource, graphics::input_resource_view* irv);

		// re-allocates graphics resource, and recreates the irv
		void resize(const math::vectoru2& new_dimensions);

		graphics::resource* mp_resource;
		graphics::input_resource_view* mp_irv;
		void* m_descriptor_handle;

		target_create_args m_args;
		math::vectoru2 m_current_dimensions;
		graphics::device* mp_device;

#if _DEBUG
		string m_debug_name;
#endif

		// only backend can create textures
		friend class renderer_backend;
	};
}