#pragma once

// influx::renderer
#include "influx_renderer/common.h"

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
		static constexpr uint32 k_depth = 6u;

		uint32 get_width() const
		{
			return m_width;
		}
		uint32 get_height() const
		{
			return m_height;
		}
	};
#pragma endregion

	/* texture descriptions */
#pragma region desc
	struct common_texture_desc final
	{
		graphics::e_format m_texelformat = graphics::e_format::rgba8;
		uint32 m_array_size = 1u;
		uint32 m_num_mips = 1u;
		uint32 m_sample_count = 1u;
		graphics::e_bind_flags m_bindflags{};
		bool m_allow_uav = false;
	};
	struct texture_desc final
	{
		texture_desc() = default;
		texture_desc(uint32 w, uint32 h)
			: m_width{ w }, m_heigth{ h } {}

		uint32 m_width = 1u;
		uint32 m_heigth = 1u;
		common_texture_desc m_common{};
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
		common_texture_desc m_common{};
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
		common_texture_desc m_common{};
	};

	// graphics:: translations
	inline graphics::tex2D_desc translate(const texture_desc& desc)
	{
		graphics::tex2D_desc result{};
		result.m_arraysize = desc.m_common.m_array_size;
		result.m_allow_uav = desc.m_common.m_allow_uav;
		result.m_bindflags = desc.m_common.m_bindflags;
		result.m_format = desc.m_common.m_texelformat;
		result.m_num_mips = desc.m_common.m_num_mips;
		result.m_sample_count = desc.m_common.m_sample_count;

		result.m_dimensions = { desc.m_width, desc.m_heigth };
		result.m_init_state = graphics::e_resource_state::all_srv;
		return result;
	}
	inline graphics::tex3D_desc translate(const texture3D_desc& desc)
	{
		graphics::tex3D_desc result{};
		result.m_arraysize = desc.m_common.m_array_size;
		result.m_allow_uav = desc.m_common.m_allow_uav;
		result.m_bindflags = desc.m_common.m_bindflags;
		result.m_format = desc.m_common.m_texelformat;
		result.m_num_mips = desc.m_common.m_num_mips;
		result.m_sample_count = desc.m_common.m_sample_count;

		result.m_dimensions = { desc.m_width, desc.m_heigth, desc.m_depth };
		result.m_init_state = graphics::e_resource_state::all_srv;
		return result;
	}
	inline graphics::cubemap_desc translate(const cubemap_desc& desc)
	{
		graphics::cubemap_desc result{};
		result.m_allow_uav = desc.m_common.m_allow_uav;
		result.m_bindflags = desc.m_common.m_bindflags;
		result.m_format = desc.m_common.m_texelformat;
		result.m_num_mips = desc.m_common.m_num_mips;
		result.m_sample_count = desc.m_common.m_sample_count;

		result.m_dimensions = { desc.m_width, desc.m_heigth };
		result.m_init_state = graphics::e_resource_state::all_srv;
		return result;
	}
#pragma endregion

	template <e_texture_type _t>
	class texture final : public imgui_texid_provider
	{
		// dim_type represents size type for the number of pixels
		using dim_type = std::tuple_element_t<static_cast<size_t>(_t), std::tuple<
			math::uint2,		// e_texture_type::texture2D
			math::uint3,		// e_texture_type::texture3D
			math::uint2>>;		// e_texture_type::cubemap

		// desc_type is the struct type that describes the texture (size, nummips etc.)
		using desc_type = std::tuple_element_t<static_cast<size_t>(_t), std::tuple<
			texture_desc,		// e_texture_type::texture2D
			texture3D_desc,		// e_texture_type::texture3D
			cubemap_desc>>;		// e_texture_type::cubemap

		// data_type is the struct type that contains the raw data as input for a texture
		using data_type = std::tuple_element_t<static_cast<size_t>(_t), std::tuple<
			texture_data,		// e_texture_type::texture2D
			texture3D_data,		// e_texture_type::texture3D
			cubemap_data>>;		// e_texture_type::cubemap

		graphics::resource*			m_resource = nullptr;
		graphics::resource*			m_upload = nullptr;

		graphics::descriptor_handle m_rtv = nullptr;
		graphics::descriptor_handle m_dsv = nullptr;
		graphics::descriptor_handle m_srv = nullptr;

		desc_type m_desc{};
		dim_type m_current_dimensions{};
		debug_name m_debug_name{};

	public:
		inline result<graphics::resource*> get_resource() const
		{ return m_resource; }

		inline result<graphics::descriptor_handle> get_srv() const
		{ return m_srv; }

		inline result<graphics::descriptor_handle> get_rtv() const
		{ return m_rtv; }

		inline result<graphics::descriptor_handle> get_dsv() const
		{ return m_dsv; }

		inline dim_type get_dimensions() const
		{ return m_current_dimensions; }

		inline uint32 get_num_pixels() const
		{ return dim_type::get_summed(m_current_dimensions); }

		inline uint32 get_array_size() const
		{ return m_desc.m_array_size; }

		inline void set_name(const debug_name& name)
		{ m_debug_name = name; }

		inline const debug_name& get_name() const
		{ return m_debug_name; }

	private:
		inline explicit texture(graphics::device& device, const desc_type& desc) : m_desc{ desc }
		{
			m_resource = device.create_resource(translate(desc));
		}

		// re-allocates graphics resource
		inline void resize(graphics::device& device, const dim_type& new_dimensions)
		{
			if (new_dimensions != m_current_dimensions)
			{
				if (m_resource)
				{
					device.release(m_resource);
				}

				// make a copy of our current desc, with altered dimensions
				desc_type desc_cpy = m_desc;
				desc_cpy.m_dimensions = new_dimensions;
				m_resource = device.create_resource(translate(desc_cpy));

				m_current_dimensions = new_dimensions;
			}
		}

		// uploads the data to an upload-heap resource, then copies that resource to the GPU resource
		inline result<> upload(graphics::device& device, graphics::commandlist& commandlist, const data_type& data)
		{
			if (m_upload == nullptr)
			{
				m_upload = device.create_upload_resource(m_resource);
				if (m_upload == nullptr)
				{
					return result<>::make_error("failed creating upload heap for resource");
				}
			}

			// map data to upload resource
			{
				const size_t data_bytesize = data.m_pixels.size() * sizeof(pixel32);
				graphics::map_args args{ .m_subres = 0u, .m_begin = 0u, .m_end = data_bytesize };
				m_upload->map([&data, data_bytesize](void* target)
				{
					memcpy(target, data.m_pixels.data(), data_bytesize);
				}, args);
			}

			// copy shared -> GPU
			m_upload->transition(&commandlist, graphics::e_resource_state::copy_src);
			m_resource->transition(&commandlist, graphics::e_resource_state::copy_dst);
			
			graphics::copy_texture_args args{};
			if (!commandlist.copy_texture(m_upload, m_resource, args))
			{
				return result<>::make_error("upload -> resource commandlist copy_texture failed");
			}

			return {};
		}

		// ~imgui_texid_provider begin
		virtual void* get_tex_descriptor() const override final { return m_srv; }
		virtual void* get_tex_resource() const override final { return m_resource; };
		virtual debug_name get_rendergraph_id() const override final { return m_debug_name; };
		// ~imgui_texid_provider end
		
		// only backend can create textures
		friend class renderer_backend;
		friend class resource_manager;
	};

	using texture2D	= texture<e_texture_type::texture2D>;
	using texture3D = texture<e_texture_type::texture3D>;
	using cubemap	= texture<e_texture_type::cubemap>;

	/* internal textures */
	enum class e_texture : uint8
	{
		none,
		num
	};
	static constexpr uint8 k_num_internal_textures = static_cast<uint32>(e_texture::num);
	static const char* k_internal_texture_names[k_num_internal_textures] =
	{
		"none"
	};
	static mesh_id get_internal_texture_id(const e_texture& mesh)
	{
		return static_cast<mesh_id>(mesh);
	}
	inline constexpr const char* get_internal_texture_name(const e_texture& tex)
	{
		return k_internal_texture_names[static_cast<uint32>(tex)];
	}
	static bool is_internal_texture(const tex_id id)
	{
		return static_cast<uint32>(id) < k_num_internal_textures;
	}

}