#pragma once

// influx::core
#include "core/basetypes.h"

// influx::graphics
#include "influx_graphics/resource.h"

namespace influx::rendergraph
{
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

	struct buffer_desc final
	{
		size_t m_bytesize;
		size_t m_bytestride;
		graphics::e_resource_flags m_flags;
		graphics::e_resource_state m_init_state;
		graphics::e_format m_format;
	};

	class rgname
	{
#if INFLUX_DEBUG
	public:
		const char* get_name() const
		{
			return m_name.c_str();
		}

		void set_name(const string& name)
		{
			m_name = name;
		}

	private:
		string m_name;
#endif
	};

	using rghandle = uint64;
	using rgpass_id = rghandle;
	inline constexpr static uint64 k_invalid_id = uint64(-1);

	enum class rgresource_type : uint8
	{
		buffer,
		texture
	};

	struct rgresource_id
	{
		rgresource_id() : m_id{ k_invalid_id }  {}
		rgresource_id(const rgresource_id&) = default;
		rgresource_id(uint64 id) : m_id{ id } {}
		void set_invalid() { m_id = k_invalid_id; }
		bool is_valid() const { return m_id != k_invalid_id; }
		auto operator<=>(rgresource_id const&) const = default;

		uint64 m_id;
	};

	template <rgresource_type _t>
	struct trgresource_id : rgresource_id
	{
		using rgresource_id::rgresource_id;
	};
	using rgbuffer_id = trgresource_id<rgresource_type::buffer>;
	using rgtexture_id = trgresource_id<rgresource_type::texture>;

	enum class rgread_access : uint8
	{
		ps,
		non_ps,
		all_shader
	};

	enum class e_rg_load : uint8
	{
		clear,
		discard,
		preserve,
		no_access,
		count
	};

	enum class e_rg_store : uint8
	{
		resolve,
		discard,
		preserve,
		no_access,
		count
	};

	struct rgaccess final
	{
		e_rg_load m_load;
		e_rg_store m_store;
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

	template <rgresource_mode _mode>
	struct rgtexturemode_id : rgtexture_id
	{
	private:
		friend class rgbuilder;
		friend class rendergraph;

		rgtexturemode_id(const rgtexture_id& id) : rgtexture_id(id) {}
	};
	template <rgresource_mode _mode>
	struct rgbuffermode_id : rgbuffer_id
	{
	private:
		friend class rgbuilder;
		friend class rendergraph;

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

	enum class rgdescriptor_type : uint8
	{
		read_only,
		read_write,
		render_target,
		depth_target
	};

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
			if constexpr (_t == rgresource_type::buffer) return rgbuffer_id(rgdescriptor_id::get_resource_id());
			else if constexpr (_t == rgresource_type::texture) return rgtexture_id(rgdescriptor_id::get_resource_id());
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
}

// enum bit operators
ENABLE_ENUM_BIT_OPERATORS(influx::rendergraph::e_rgpass_flags);

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