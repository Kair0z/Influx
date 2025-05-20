#pragma once

// influx::core
#include "core/basetypes.h"
#include "core/math/vector.h"
#include "core/container/vector.h"
#include "core/string.h"

// influx::renderer
#include "influx_renderer/types.h"

// influx::graphics
#include "influx_graphics/device.h"
#include "influx_graphics/descriptors.h"
namespace influx::graphics
{
	class resource;
	class device;
	class descriptor_heap;
	class shader_resource_view;
}

namespace influx::renderer
{
	enum class e_texture_type : uint8
	{
		texture2D,
		texture3D,
		cubemap,
		count
	};

	/* input data structs */
#pragma region raw_data
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

	struct texture3D_data final
	{
		vector<pixel32> m_pixels{};
		uint32 m_width = 0u;
		uint32 m_height = 0u;

		uint32 get_width() const
		{
			return m_width;
		}
		uint32 get_height() const
		{
			return m_height;
		}
		uint32 get_depth() const
		{
			return static_cast<uint32>((m_pixels.size() / (get_width() * get_height())));
		}

		bool is_valid() const;
	};

	struct cubemap_data final
	{
		vector<pixel32> m_pixels{};
		uint32 m_width = 0u;
		uint32 m_height = 0u;

		uint32 get_width() const
		{
			return m_width;
		}
		uint32 get_height() const
		{
			return m_height;
		}
		uint32 get_depth() const
		{
			return 6u;
		}
	};
#pragma endregion

	/* texture descriptions */
#pragma region desc
	struct texture_desc final
	{
		texture_desc() = default;
		texture_desc(uint32 w, uint32 h)
			: m_width{ w }, m_heigth{ h } {}

		uint32 m_width = 1u;
		uint32 m_heigth = 1u;
		uint32 m_array_size = 1u;
		uint32 m_num_mips = 1u;
		uint32 m_sample_count = 1u;
	};
	struct texture3D_desc final
	{
		texture3D_desc() = default;
		texture3D_desc(uint32 w, uint32 h, uint32 d)
			: m_width{ w }, m_heigth{ h }, m_depth{ d } {
		}

		uint32 m_width = 1u;
		uint32 m_heigth = 1u;
		uint32 m_depth = 1u;
		uint32 m_array_size = 1u;
		uint32 m_num_mips = 1u;
		uint32 m_sample_count = 1u;
	};
	struct cubemap_desc final
	{
		cubemap_desc() = default;
		cubemap_desc(uint32 w, uint32 h)
			: m_width{ w }, m_heigth{ h } 
		{
		}

		uint32 m_width = 1u;
		uint32 m_heigth = 1u;
		uint32 m_depth = 6u;
		uint32 m_array_size = 1u;
		uint32 m_num_mips = 1u;
		uint32 m_sample_count = 1u;
	};

	inline graphics::tex2D_desc translate(const texture_desc& desc)
	{
		graphics::tex2D_desc result{};
		result.m_arraysize = 1u;
		result.m_dimensions = { desc.m_width, desc.m_heigth };
		result.m_format = graphics::e_format::rgba8;
		result.m_num_mips = 1u;
		result.m_sample_count = 1u;
		result.m_init_state = graphics::e_resource_state::all_srv;
		return result;
	}
	inline graphics::tex3D_desc translate(const texture3D_desc& desc)
	{
		graphics::tex3D_desc result{};
		result.m_arraysize = 1u;
		result.m_dimensions = { desc.m_width, desc.m_heigth, desc.m_depth };
		result.m_format = graphics::e_format::rgba8;
		result.m_num_mips = 1u;
		result.m_sample_count = 1u;
		return result;
	}
	inline graphics::cubemap_desc translate(const cubemap_desc& desc)
	{
		graphics::cubemap_desc result{};
		result.m_arraysize = 6u;
		result.m_dimensions = { desc.m_width, desc.m_heigth, 1u };
		result.m_format = graphics::e_format::rgba8;
		result.m_num_mips = 1u;
		result.m_sample_count = 1u;
		result.m_init_state = graphics::e_resource_state::all_srv;
		return result;
	}
#pragma endregion

	template <e_texture_type _t>
	class texture final : public imgui_texid_provider
	{
		using dim_type = std::tuple_element_t<static_cast<size_t>(_t), std::tuple<
			math::vectoru2,
			math::vectoru3,
			math::vectoru3>>;
		using desc_type = std::tuple_element_t<static_cast<size_t>(_t), std::tuple<
			texture_desc,
			texture3D_desc,
			cubemap_desc>>;
		using data_type = std::tuple_element_t<static_cast<size_t>(_t), std::tuple<
			texture_data,
			texture3D_data,
			cubemap_data>>;

	public:
		inline graphics::resource* get_resource() const
		{ return mp_resource; }

		inline graphics::descriptor_handle get_srv() const
		{ return m_srv; }

		inline dim_type get_dimensions() const
		{ return m_current_dimensions; }

		inline uint32 get_num_pixels() const
		{ return dim_type::get_summed(m_current_dimensions); }

		inline uint32 get_srv_heap_idx() const
		{ return 0u; }
		inline void* get_cpu_handle() const
		{ return nullptr; }

		inline void set_name(const debug_name& name)
		{ m_debug_name = name; }
		inline const debug_name& get_name() const
		{ return m_debug_name; }

	private:
		// constructs a texture from create_args, allocating new graphics resources
		inline explicit texture(graphics::device* device, const desc_type& args)
			: mp_device{ device }
			, m_args{ args }
		{
			mp_resource = device->create_resource(translate(args));
		}

		// re-allocates graphics resource
		inline void resize(const dim_type& new_dimensions)
		{
			if (new_dimensions != m_current_dimensions)
			{
				if (mp_resource) mp_resource->release(mp_device);

				// update size:
				m_current_dimensions = new_dimensions;

				// create new resource
				desc_type desc_cpy = m_args;
				desc_cpy.m_dimensions = new_dimensions;
				mp_resource = mp_device->create_resource(translate(desc_cpy));
			}
		}

		inline void upload(graphics::commandlist& commandlist, const data_type& data)
		{
			if (mp_upload == nullptr)
			{
				mp_upload = mp_device->create_upload_resource(mp_resource);
			}

			// map data to shared
			{
				const size_t data_bytesize = data.m_pixels.size() * sizeof(pixel32);
				graphics::map_args args{ .m_subres = 0u, .m_begin = 0u, .m_end = data_bytesize };
				mp_upload->map([&data, data_bytesize](void* target)
				{
					memcpy(target, data.m_pixels.data(), data_bytesize);
				}, args);
			}

			// copy shared -> GPU
			mp_upload->transition(&commandlist, graphics::e_resource_state::copy_src);
			mp_resource->transition(&commandlist, graphics::e_resource_state::copy_dst);
			graphics::copy_texture_args args{};
			commandlist.copy_texture(mp_upload, mp_resource, args);
		}

		graphics::resource* mp_resource;
		graphics::resource* mp_upload = nullptr;
		graphics::descriptor_handle m_srv;

		desc_type m_args;
		dim_type m_current_dimensions;
		graphics::device* mp_device;
		debug_name m_debug_name;

		// ~imgui_texid_provider begin
		virtual void* get_tex_descriptor() const override final { return m_srv; }
		virtual void* get_tex_resource() const override final { return mp_resource; };
		virtual debug_name get_rendergraph_id() const override final { return m_debug_name; };
		// ~imgui_texid_provider end
		
		// only backend can create textures
		friend class renderer_backend;
		friend class resource_manager;
	};

	using texture2D	= texture<e_texture_type::texture2D>;
	using texture3D = texture<e_texture_type::texture3D>;
	using cubemap	= texture<e_texture_type::cubemap>;
}