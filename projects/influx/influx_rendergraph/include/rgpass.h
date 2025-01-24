#pragma once

// influx::core
#include "core/math/vector.h"
#include "core/container/map.h"
#include "core/function.h"

// influx::rendergraph
#include "rgcommon.h"
#include "rgpass_builder.h"

// influx::graphics
#include "influx_graphics/renderpass.h"
#include "influx_graphics/resource.h"
namespace influx::rendergraph
{
	class rgpass_builder;
	class rgpass_context;

	using rgpass_builder_clb = function<void(rgpass_builder&)>;
	using rgpass_process_clb = function<void(rgpass_context&)>;

	class rgpass;

	class rgpass final
	{
	public:
		uint32 get_num_reads() const;
		uint32 get_num_writes() const;

		INFLUX_RG_API void set_name(const rgname& name);

	protected:
		rgpass(
			const rgpass_builder_clb& builder_clb,
			const rgpass_process_clb& process_clb,
			e_rgpass_type type = e_rgpass_type::graphics,
			e_rgpass_flags flags = e_rgpass_flags::none);

	private:
		void build(rgpass_builder& builder);
		void execute(rgpass_context& ctx) const;

		bool is_culled() const;
		bool can_be_culled() const;
		bool allow_uav_writes() const;
		void set_id(rgpass_id id);
		e_rgpass_type get_type() const;
		bool depends_on(const rgpass& other) const;
		static bool has_dependency(const rgpass& a, const rgpass& b);

		bool is_graphics() const;
		bool is_compute() const;
		bool is_compute_any() const;
		bool is_async_compute() const;

		uint32 get_width() const;
		uint32 get_height() const;

		rgname m_name;
		rgpass_builder_clb m_builder_clb;
		rgpass_process_clb m_process_clb;

		e_rgpass_type m_type;
		e_rgpass_flags m_flags;
		rgpass_id m_id;
		uint32 m_width;
		uint32 m_height;
		uint32 m_refcount;
		bool m_is_culled;

		uset<rgtexture_id> m_texture_creates;
		uset<rgtexture_id> m_texture_reads;
		uset<rgtexture_id> m_texture_writes;
		uset<rgtexture_id> m_texture_destroys;
		umap<rgtexture_id, graphics::e_resource_state> m_texture_state_map;

		uset<rgbuffer_id> m_buffer_creates;
		uset<rgbuffer_id> m_buffer_reads;
		uset<rgbuffer_id> m_buffer_writes;
		uset<rgbuffer_id> m_buffer_destroys;
		umap<rgbuffer_id, graphics::e_resource_state> m_buffer_state_map;

		struct render_target final
		{
			rgtexture_id m_texture_id;
			rgaccess m_access;
		};
		vector<render_target> m_rtvs{};

		struct depth_stencil final
		{
			rgtexture_id m_texture_id;
			rgaccess m_depth_access;
			rgaccess m_stencil_access;
			bool m_depth_read_only;
			bool m_is_enabled = false;
		};
		depth_stencil m_dsv{};

		friend class rendergraph;
		friend class rglayer;
		friend class rgpass_builder;
	};

	class rglayer final
	{
	public:
		rglayer() = default;
		~rglayer() = default;

		inline void add_pass(const rgpass& pass)
		{
			m_passes.push_back(pass);
			m_texture_reads.insert(pass.m_texture_reads.begin(), pass.m_texture_reads.end());
			m_texture_writes.insert(pass.m_texture_writes.begin(), pass.m_texture_writes.end());
			m_buffer_reads.insert(pass.m_buffer_reads.begin(), pass.m_buffer_reads.end());
			m_buffer_writes.insert(pass.m_buffer_writes.begin(), pass.m_buffer_writes.end());
		}

		inline void reset()
		{
			m_passes.clear();
			m_texture_creates.clear();
			m_texture_reads.clear();
			m_texture_writes.clear();
			m_texture_destroys.clear();
			m_texture_to_state_map.clear();
			m_buffer_creates.clear();
			m_buffer_reads.clear();
			m_buffer_writes.clear();
			m_buffer_destroys.clear();
			m_buffer_to_state_map.clear();
		}

		vector<rgpass> m_passes;

		uset<rgtexture_id> m_texture_creates;
		uset<rgtexture_id> m_texture_reads;
		uset<rgtexture_id> m_texture_writes;
		uset<rgtexture_id> m_texture_destroys;
		umap<rgtexture_id, graphics::e_resource_state> m_texture_to_state_map;

		uset<rgbuffer_id> m_buffer_creates;
		uset<rgbuffer_id> m_buffer_reads;
		uset<rgbuffer_id> m_buffer_writes;
		uset<rgbuffer_id> m_buffer_destroys;
		umap<rgbuffer_id, graphics::e_resource_state> m_buffer_to_state_map;
	};
}