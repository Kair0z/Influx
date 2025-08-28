#pragma once

// influx::core
#include "core/basetypes.h"
#include "core/hash.h"
#include "core/string.h"
#include "core/math/colour.h"
#include "core/result.h"

#if INFLUX_DEBUG
#define RGNAME(name) influx::rendergraph::rgname(#name, influx::crc64(#name)) 
#define RGNAME_IDX(name, idx) influx::rendergraph::rgname(#name, influx::crc64(#name) + idx)
#else 
#define RGNAME(name) influx::rendergraph::rgname(influx::crc64(#name)) 
#define RGNAME_IDX(name, idx) influx::rendergraph::rgname(influx::crc64(#name) + idx)
#endif

// influx::graphics
#define INFLUX_RG_BACKEND_RHI		0
#define INFLUX_RG_BACKEND_GRAPHICS  1

// influx::graphics
#if INFLUX_RG_BACKEND_RHI
#include "influx_rhi.h"
namespace influx::rendergraph
{
	using rhi_device = rhi::device;
	using rhi_commandlist = rhi::commandlist;
	using rhi_resource = rhi::resource;
	using rhi_descheap = rhi::descheap;
	using rhi_descriptor = rhi::descriptor;
	using rhi_descheap_type = rhi::e_descriptor_heap_type;
	using rhi_bufferdesc = rhi::buffer_create_args;
	using rhi_texture2Ddesc = rhi::texture2D_create_args;
	using rhi_pixelformat = rhi::pixelformat;
	using rhi_resource_state = rhi::e_resource_state;
	using rhi_resource_bindflags = rhi::e_resource_bindflags;
	using rhi_store_op = rhi::e_store_op;
	using rhi_load_op = rhi::e_load_op;
	static constexpr uint32 k_num_descheap_types = rhi::k_num_descriptor_heap_types;
}
#endif
#if INFLUX_RG_BACKEND_GRAPHICS
#include "influx_graphics/device.h"
#include "influx_graphics/descriptors.h"
#include "influx_graphics/resource.h"
#include "influx_graphics/commandlist.h"
#include "influx_graphics/renderpass.h"
namespace influx::graphics
{
	class device;
	class commandlist;
	class resource;
	class descriptor_heap;
}

namespace influx::rendergraph
{
	using rhi_device = graphics::device;
	using rhi_commandlist = graphics::commandlist;
	using rhi_resource = graphics::resource;
	using rhi_descheap = graphics::descriptor_heap;
	using rhi_descriptor = graphics::descriptor_handle;
	using rhi_descriptor_id = graphics::descriptor_id;
	using rhi_descheap_type = graphics::e_descriptor_heap_type;
	using rhi_bufferdesc = graphics::buffer_desc;
	using rhi_texture2Ddesc = graphics::tex2D_desc;
	using rhi_pixelformat = graphics::e_format;
	using rhi_resource_state = graphics::e_resource_state;
	using rhi_resource_bindflags = graphics::e_bind_flags;
	using rhi_store_op = graphics::e_store_op;
	using rhi_load_op = graphics::e_load_op;
	static constexpr uint32 k_num_descheap_types = graphics::k_num_descriptor_heap_types;
}
#endif

namespace influx::rendergraph
{
	template <typename _t = char>
	using result = influx::result<_t, const char*>;

	/* 
		external descheaps
		these are the slots of CPU heaps the rendergraph system can tap into for its allocations.
	*/
	enum class e_ext_descheap_slot : uint8
	{
		rtv,
		dsv,
		resource,
		sampler,
		num
	};
	static constexpr uint32 k_num_ext_descheap_slots = static_cast<uint32>(e_ext_descheap_slot::num);

	/* configuration settings */
	struct global_config final
	{
		/* num frames to tick before we recycle an inactive resource for another */
		uint32 m_frames_until_resource_recycle = 64u;

		/* resource descriptor heap capacities */
		uint32 m_max_num_samplers = 8u;
		uint32 m_max_num_srvs = 64u;
		uint32 m_max_num_rtvs = 32;
		uint32 m_max_num_dsvs = 32;

		/* [optional] external descheaps to tap into */
		rhi_descheap* m_external_descheaps[k_num_ext_descheap_slots]{};
		void set_external_descheap(e_ext_descheap_slot slot, rhi_descheap& heap)
		{
			m_external_descheaps[static_cast<uint32>(slot)] = &heap;
		}
	};

	// -- textures & buffers
	struct texture_desc final
	{
		texture_desc() = default;
		texture_desc(uint32 w, uint32 h)
			: m_width{ w }, m_heigth{ h } {}

		inline bool is_recycle_match(const texture_desc& other) const
		{
			return m_width == other.m_width &&
				m_heigth == other.m_heigth &&
				m_depth == other.m_depth &&
				m_array_size == other.m_array_size &&
				m_num_mips == other.m_num_mips &&
				m_sample_count == other.m_sample_count &&
				m_format == other.m_format &&
				m_bindflags == other.m_bindflags &&
				m_allow_uav == other.m_allow_uav;
		}

		uint32 m_width = 1u;
		uint32 m_heigth = 1u;
		uint32 m_depth = 1u;
		uint32 m_array_size = 1u;
		uint32 m_num_mips = 1u;
		uint32 m_sample_count = 1u;
		rhi_pixelformat m_format;
		rhi_resource_state m_init_state = rhi_resource_state::common;
		rhi_resource_bindflags m_bindflags = rhi_resource_bindflags::none;
		bool m_allow_uav = false;
	};

	struct texture_view_desc final
	{
		/* is texture view expected in a pass */
		bool m_is_active = false;
		
		/* is texture view created already */
		bool m_is_created = false;

		uint32 m_first_slice = 0u;
		uint32 m_num_slices = uint32(-1);
		uint32 m_first_mip = 0u;
		uint32 m_num_mips = uint32(-1);

		// todo: flags
		// todo: channel mapping

		inline void clear()
		{
			m_is_created = false;
			m_is_active = false;
			m_first_slice = 0u;
			m_num_slices = uint32(-1);
			m_first_mip = 0u;
			m_num_mips = uint32(-1);
		}

		std::strong_ordering operator<=>(const texture_view_desc& other) const = default;
	};

	struct buffer_desc final
	{
		inline bool is_recycle_match(const buffer_desc& other) const
		{
			return m_bytesize == other.m_bytesize &&
				m_bytestride == other.m_bytestride &&
				m_flags == other.m_flags &&
				m_format == other.m_format &&
				m_bindflags == other.m_bindflags;
		}

		size_t m_bytesize;
		size_t m_bytestride;
		uint64 m_flags;
		rhi_pixelformat m_format;
		rhi_resource_state m_init_state = rhi_resource_state::common;
		rhi_resource_bindflags m_bindflags = rhi_resource_bindflags::none;
	};

	struct buffer_view_desc final
	{
		bool m_is_active = false;
		bool m_is_created = false;
		uint64 m_offset = 0u;
		uint64 m_size = uint64(-1);
		std::strong_ordering operator<=>(const buffer_view_desc& other) const = default;

		inline void clear()
		{
			m_is_created = false;
			m_is_active = false;
			m_offset = 0u;
			m_size = uint64(-1);
		}
	};

	// -- enums
	enum class rgresource_type : uint8
	{
		buffer,
		texture
	};

	enum class rgread_access : uint8
	{
		ps,
		non_ps,
		all_shader
	};

	// what the renderpass does to the rendertarget when ENTERING
	enum class e_rg_load : uint8
	{
		clear,			// clears the target on load
		discard,		// we won't read from whatever's in the target before, we will first write to it!
		preserve,		// our renderpass relies on the contents, they remain preserved
		no_access,		// must pair with store_no_access, means the renderpass does not use the target
		count
	};

	// what the renderpass does to the rendertarget when LEAVING
	enum class e_rg_store : uint8
	{
		resolve,		// resolve the target into another dest target
		discard,		// we won't read from this again until it gets written to again
		preserve,		// leaving the pass, we will read from this again!
		no_access,		// must pair with load_no_access, means the renderpass does not use the target
		count
	};

	enum class rgresource_mode : uint8
	{
		copy_src,
		copy_dst,
		ind_args,
		vertex,
		index,
		constant
	};

	enum class rgdescriptor_type : uint8
	{
		read_only,
		read_write,
		render_target,
		depth_target,
		count
	};

	enum class e_rgpass_type : uint8
	{
		graphics,
		compute,
		async_compute,
		count
	};

	enum class e_rgpass_flags : uint32
	{
		none = 0x00,
		force_no_cull = 0x01,
		allow_uav_write = 0x02
	};

	// -- rgname
#if 1
	using rgname = debug_name;

#else
	struct rgname final
	{
		static constexpr uint64 k_invalid_hash = uint64(-1);
#if INFLUX_DEBUG
		string m_namestr = "";
		char const* m_name;
#endif
		uint64 m_namehash;

		rgname(const string& name)
		{
			std::hash<string> hasher{};
			m_namehash = hasher(name);

#if INFLUX_DEBUG
			m_namestr = name;
			m_name = m_namestr.c_str();
#endif
		}

#if INFLUX_DEBUG
		rgname() : m_namehash{ k_invalid_hash }, m_name{ "none" }{}
		template<uint64 _n>
		constexpr explicit rgname(char const (&name)[_n], uint64 hash) : m_namehash{ hash }, m_name{ name }{}
		operator char const* () const { return m_name; }
#else
		rgname() : m_namehash{ k_invalid_hash }{}
		constexpr explicit rgname(uint64 hash) : m_namehash{ hash }{}
		operator char const* () const { return ""; }
#endif

		bool is_valid() const
		{
			return m_namehash != k_invalid_hash;
		}
	};
#endif

	// -- handles
	using rghandle = uint64;
	using rgpass_id = rghandle;
	inline constexpr static uint64 k_invalid_id = uint64(-1);

	struct rgresource_id
	{
		rgresource_id() : m_id{ k_invalid_id }  {}
		rgresource_id(const rgresource_id&) = default;
		rgresource_id(uint64 id) : m_id{ id } {}
		void set_invalid() { m_id = k_invalid_id; }
		bool is_valid() const { return m_id != k_invalid_id; }
		auto operator<=>(rgresource_id const&) const = default;

		uint64 m_id = k_invalid_id;
	};

	template <rgresource_type _t>
	struct trgresource_id : public rgresource_id
	{
		using rgresource_id::rgresource_id;
	};

	using rgbuffer_id = trgresource_id<rgresource_type::buffer>;
	using rgtexture_id = trgresource_id<rgresource_type::texture>;

	template <rgresource_mode _mode>
	struct rgtexturemode_id : public rgtexture_id
	{
	public:
		friend class rgbuilder;
		friend class rendergraph;

		rgtexturemode_id() = default;
		rgtexturemode_id(const rgtexture_id& id) : rgtexture_id(id) {}
	};
	template <rgresource_mode _mode>
	struct rgbuffermode_id : rgbuffer_id
	{
	public:
		friend class rgbuilder;
		friend class rendergraph;

		rgbuffermode_id() = default;
		rgbuffermode_id(const rgbuffer_id& id) : rgbuffer_id(id) {}
	};

	using rgtex_copysrc_id = rgtexturemode_id<rgresource_mode::copy_src>;
	using rgtex_copydst_id = rgtexturemode_id<rgresource_mode::copy_dst>;
	using rgbuf_copysrc_id = rgbuffermode_id<rgresource_mode::copy_src>;
	using rgbuf_copydst_id = rgbuffermode_id<rgresource_mode::copy_dst>;
	using rgbuf_indargs_id = rgbuffermode_id<rgresource_mode::ind_args>;
	using rgbuf_index_id = rgbuffermode_id<rgresource_mode::index>;
	using rgbuf_vertex_id = rgbuffermode_id<rgresource_mode::vertex>;
	using rgbuf_const_id = rgbuffermode_id<rgresource_mode::constant>;

	struct rgdescriptor_id
	{
		rgdescriptor_id() : m_id(k_invalid_id) {}
		rgdescriptor_id(uint64 view_id, rgresource_id handle)
			: m_id(k_invalid_id)
		{
			uint64 res_id = handle.m_id;
			m_id = (view_id << 32) | res_id;
		}

		uint64 get_descriptor_id() const 
		{ 
			return (m_id >> 32); 
		};

		uint64 get_resource_id() const
		{
			return (uint64)static_cast<uint32>(m_id);
		};

		rgresource_id operator*() const
		{
			return rgresource_id(get_resource_id());
		}

		void set_invalid() { m_id = k_invalid_id; }
		bool is_valid() const { return m_id != k_invalid_id; }
		auto operator<=>(const rgdescriptor_id&) const = default;

		uint64 m_id;
	};

	template <rgresource_type _t, rgdescriptor_type _d>
	struct trgdescriptor_id : rgdescriptor_id
	{
		using rgdescriptor_id::rgdescriptor_id;
		using rgdescriptor_id::operator*;

		uint64 get_resource_id() const
		{
			if constexpr (_t == rgresource_type::buffer) return rgbuffer_id(rgdescriptor_id::get_resource_id()).m_id;
			else if constexpr (_t == rgresource_type::texture) return rgtexture_id(rgdescriptor_id::get_resource_id()).m_id;
		}

		auto operator*() const
		{
			if constexpr (_t == rgresource_type::buffer) return rgbuffer_id(get_resource_id());
			else if constexpr (_t == rgresource_type::texture) return rgtexture_id(get_resource_id());
		}
	};

	using rgrendertarget_id			= trgdescriptor_id<rgresource_type::texture, rgdescriptor_type::render_target>;
	using rgdepthtarget_id			= trgdescriptor_id<rgresource_type::texture, rgdescriptor_type::depth_target>;
	using rgtexture_readonly_id		= trgdescriptor_id<rgresource_type::texture, rgdescriptor_type::read_only>;
	using rgtexture_readwrite_id	= trgdescriptor_id<rgresource_type::texture, rgdescriptor_type::read_write>;

	using rgbuffer_readonly_id		= trgdescriptor_id<rgresource_type::buffer, rgdescriptor_type::read_only>;
	using rgbuffer_readwrite_id		= trgdescriptor_id<rgresource_type::buffer, rgdescriptor_type::read_write>;

	struct rgaccess final
	{
		e_rg_load m_load;
		e_rg_store m_store;

		struct load_preserve_params final
		{
			// ...
		} m_load_preserve{};

		struct load_clear_params final
		{
			math::colour_rgba m_colour;
		} m_load_clear{};

		struct store_resolve_params final
		{
			rhi_pixelformat m_dest_format;
			rgtexture_id m_source_texture;
			rgtexture_id m_dest_texture;
			bool m_keep_source = false;
		} m_store_resolve{};

		struct store_preserve_params final
		{
			// ...
		} m_store_preserve;

		/* [load:preserve | store:resolve] */
		inline static rgaccess keep_and_copy(rgtexture_id src, rgtexture_id dst, const rhi_pixelformat& dest_format, bool keep_source = true)
		{
			static rgaccess access{};
			access.m_load = e_rg_load::preserve;
			access.m_store = e_rg_store::resolve;
			access.m_store_resolve.m_dest_texture = dst;
			access.m_store_resolve.m_source_texture = src;
			access.m_store_resolve.m_keep_source = keep_source;
			access.m_store_resolve.m_dest_format = dest_format;
			return access;
		}
		/* [load:preserve | store:preserve] */
		inline static rgaccess keep_and_keep()
		{
			static rgaccess access{};
			access.m_load = e_rg_load::preserve;
			access.m_store = e_rg_store::preserve;
			return access;
		}
		/* [load:clear | store:preserve] */
		inline static rgaccess clear_and_keep(const math::colour_rgba& clear_value) 
		{
			static rgaccess access{};
			access.m_load_clear.m_colour = clear_value;
			access.m_load = e_rg_load::clear;
			access.m_store = e_rg_store::preserve;
			return access;
		}
	};
}

// -- enum bit operators
ENABLE_ENUM_BIT_OPERATORS(influx::rendergraph::e_rgpass_flags);

// -- hashes
namespace std
{
	template <> struct hash<influx::rendergraph::rgtexture_id>
	{
		influx::uint64 operator()(const influx::rendergraph::rgtexture_id& h) const
		{
			return hash<decltype(h.m_id)>()(h.m_id);
		}
	};
	template <> struct hash<influx::rendergraph::rgbuffer_id>
	{
		influx::uint64 operator()(influx::rendergraph::rgbuffer_id const& h) const
		{
			return hash<decltype(h.m_id)>()(h.m_id);
		}
	};
	template <> struct hash<influx::rendergraph::rgtexture_readonly_id>
	{
		influx::uint64 operator()(influx::rendergraph::rgtexture_readonly_id const& h) const
		{
			return hash<decltype(h.m_id)>()(h.m_id);
		}
	};
	template <> struct hash<influx::rendergraph::rgtexture_readwrite_id>
	{
		influx::uint64 operator()(influx::rendergraph::rgtexture_readwrite_id const& h) const
		{
			return hash<decltype(h.m_id)>()(h.m_id);
		}
	};
	template <> struct hash<influx::rendergraph::rgrendertarget_id>
	{
		influx::uint64 operator()(influx::rendergraph::rgrendertarget_id const& h) const
		{
			return hash<decltype(h.m_id)>()(h.m_id);
		}
	};
	template <> struct hash<influx::rendergraph::rgdepthtarget_id>
	{
		influx::uint64 operator()(influx::rendergraph::rgdepthtarget_id const& h) const
		{
			return hash<decltype(h.m_id)>()(h.m_id);
		}
	};

	template <> struct hash<influx::rendergraph::rgbuffer_readonly_id>
	{
		influx::uint64 operator()(influx::rendergraph::rgbuffer_readonly_id const& h) const
		{
			return hash<decltype(h.m_id)>()(h.m_id);
		}
	};

	template <> struct hash<influx::rendergraph::rgbuffer_readwrite_id>
	{
		influx::uint64 operator()(influx::rendergraph::rgbuffer_readwrite_id const& h) const
		{
			return hash<decltype(h.m_id)>()(h.m_id);
		}
	};
}