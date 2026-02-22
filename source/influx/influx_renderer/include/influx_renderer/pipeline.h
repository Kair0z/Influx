#pragma once
#include "influx_renderer/common.h"
#include "influx_renderer/texture.h"
#include "influx_renderer/buffer.h"

#include "core/container/vectormap.h"

namespace influx::renderer
{
	using pipeline_id = uint32;
	
	class pipeline final
	{
	public:
		pipeline();

		using pass_id = uint32;
		using texture_id = uint32;
		using buffer_id = uint32;

		enum class resource_usage
		{
			texture_write,
			texture_read,
			buffer_write,
			buffer_read,
			num
		};

		class pass_desc final
		{
			friend class pipeline;
			vector<texture_id> m_texture_writes;
			vector<texture_id> m_texture_reads;
			vector<buffer_id> m_buffer_writes;
			vector<buffer_id> m_buffer_reads;

		public:
			const vector<texture_id>& get_texture_reads() const { return m_texture_reads; }
			const vector<texture_id>& get_texture_writes() const { return m_texture_writes; }
			const vector<buffer_id>& get_buffer_reads() const { return m_buffer_reads; }
			const vector<buffer_id>& get_buffer_writes() const { return m_buffer_writes; }
		};

		INFLUX_RENDER_API static pass_id make_pass_id(const char* name);
		INFLUX_RENDER_API static texture_id make_texture_id(const char* name);
		INFLUX_RENDER_API static buffer_id make_buffer_id(const char* name);

	public:
		INFLUX_RENDER_API result<> parse(const string& filepath);
		INFLUX_RENDER_API result<texture_id> register_texture(const texture_id& id, const texture_desc& desc);
		INFLUX_RENDER_API result<buffer_id> register_buffer(const buffer_id& id, const buffer_desc& desc);
		INFLUX_RENDER_API result<pass_id> register_pass(const pass_id& id, const pass_desc& desc);

		INFLUX_RENDER_API result<cptr<texture_desc>> get_texture(const texture_id& id) const;
		INFLUX_RENDER_API result<cptr<buffer_desc>> get_buffer(const buffer_id& id) const;
		INFLUX_RENDER_API result<cptr<pass_desc>> get_pass(const pass_id& id) const;

		INFLUX_RENDER_API bool has_texture(const texture_id& id) const;
		INFLUX_RENDER_API bool has_buffer(const buffer_id& id) const;
		INFLUX_RENDER_API bool has_pass(const pass_id& id) const;

		INFLUX_RENDER_API const vector<pass_desc>& get_passes() const;

	private:
		vectormap<texture_id, texture_desc> m_textures;
		vectormap<buffer_id, buffer_desc> m_buffers;
		vectormap<pass_id, pass_desc> m_passes;
	};
}