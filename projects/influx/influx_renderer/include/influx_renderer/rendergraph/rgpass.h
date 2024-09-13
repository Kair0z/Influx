#pragma once

#include "rgcommon.h"

#include "influx_graphics/renderpass.h"

namespace influx::renderer
{
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

	class rgpass_base
	{
		friend class rendergraph;
		friend class rglayer;

	protected:
		rgpass_base(e_rgpass_type type, e_rgpass_flags flags);

	private:
		virtual void setup() = 0;
		virtual void execute() = 0;

		bool is_culled() const;
		bool allow_uav_writes() const;
		void set_id(rgpass_id id);
		e_rgpass_type get_type() const;
		static bool has_dependency(rgpass_base* a, rgpass_base* b);
		graphics::renderpass_args make_renderpass_args(class rendergraph& rg) const;

		uint32 get_width() const;
		uint32 get_height() const;

	private:
		e_rgpass_type m_type;
		e_rgpass_flags m_flags;
		bool m_is_culled;
		rgpass_id m_id;
		uint32 m_width;
		uint32 m_height;

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
		};
		depth_stencil m_dsv{};
	};

	template <typename _passdata>
	class trgpass final : public rgpass_base {};

	template<>
	class trgpass<void> final : public rgpass_base
	{
	public:
		using setup_func = function<void()>;
		using execute_func = function<void()>;

	private:
		trgpass(setup_func&& setup, execute_func&& exe,
			e_rgpass_type type = e_rgpass_type::graphics,
			e_rgpass_flags flags = e_rgpass_flags::none)
			: rgpass_base(type, flags)
			, m_setup{ std::move(setup) }
			, m_execute{ std::move(exe) }
		{

		}

		virtual void setup() override
		{
			m_setup();
		}

		virtual void execute() override
		{
			m_execute();
		}

	private:
		setup_func m_setup;
		execute_func m_execute;
	};
	
	template <typename _passdata>
	using rgpass = trgpass<_passdata>;
}