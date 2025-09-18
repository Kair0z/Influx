#pragma once

// influx::core
#include "core/function.h"
#include "core/basetypes.h"
#include "core/math/rect.h"

// influx::engine
#include "editor_common.h"

namespace influx::engine::editor
{
	class editor_window
	{
	public:
		// any property that is 'lockable'
		// can be set to ignore all factors except the C++ locked value
		template <typename _t>
		struct lockable final
		{
			void lock(const _t& value)
			{
				m_force_value = value;
			}

			void release()
			{
				m_force_value.reset();
			}

			bool is_locked() const
			{
				return m_force_value.has_value();
			}

			const _t& get() const
			{
				return m_force_value.value();
			}

			option<_t> m_force_value;
		};

	private:
		string m_title;
		e_group_flags m_flags = e_group_flags::none;
		lockable<math::float2> m_position_locked;
		lockable<math::float2> m_size_locked;
		bool m_is_visible = true;

		math::float2 m_last_position;
		math::float2 m_last_size;
		
	public:
		using callback = std::function<void()>;
		void run(const callback& clb);

		void set_name(const string& name);
		string get_name() const;

		// coordinates are normalized!
		math::rectf	get_rect() const;
		math::float2 get_position() const;
		math::float2 get_size() const;

		void toggle();
		void set_visible(bool new_visible);
		bool is_visible() const;

		void set_position(const math::float2& new_position);
		void set_size(const math::float2& new_size);

		virtual void on_run() {}
		virtual void on_prerun() {}

	private:
		// imgui can manage these properties, but we can force their values
		lockable<math::float2>& get_size_prop();
		lockable<math::float2>& get_position_prop();
	};
}