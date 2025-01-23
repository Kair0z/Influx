#pragma once

// influx::core
#include "core/basetypes.h"
#include "core/math/vector.h"
#include "core/container/vector.h"

namespace influx::graphics
{
	class resource;
	class device;
	class descriptor_heap;
	class shader_resource_view;
}

namespace influx::renderer
{
	struct texture_data final
	{
		vector<pixel32> m_pixels{};
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

	struct texture_desc final
	{
		texture_desc() = default;
		texture_desc(uint32 w, uint32 h)
			: m_width{ w }, m_heigth{ h } {}

		uint32 m_width = 1u;
		uint32 m_heigth = 1u;
		uint32 m_depth = 1u;
		uint32 m_array_size = 1u;
		uint32 m_num_mips = 1u;
		uint32 m_sample_count = 1u;
	};

	// contains a texture resource, as well as a shader resource view (srv)
	class texture
	{
	public:
		graphics::resource* get_resource() const;
		graphics::descriptor_handle get_srv() const;

		uint32 get_width() const;
		uint32 get_height() const;
		uint32 get_num_pixels() const;
		uint32 get_srv_heap_idx() const;

		// useful for ImTextureID (imgui)
		void* get_cpu_handle() const;

#if _DEBUG
		void set_name(const string& name);
		const string& get_name() const;
#endif

	private:
		// constructs a texture from create_args, allocating new graphics resources
		explicit texture(graphics::device* device, const texture_desc& args);

		// re-allocates graphics resource
		void resize(const math::vectoru2& new_dimensions);

		graphics::resource* mp_resource;
		graphics::descriptor_handle m_srv;

		texture_desc m_args;
		math::vectoru2 m_current_dimensions;
		graphics::device* mp_device;

#if _DEBUG
		string m_debug_name;
#endif

		// only backend can create textures
		friend class renderer_backend;
	};
}