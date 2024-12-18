#pragma once

// influx::core
#include "core/function.h"
#include "core/basetypes.h"

namespace influx::engine
{
	class editor_window
	{
	public:
		template <typename _t>
		struct optional_property
		{
			void force(const _t& value)
			{
				m_force_value = value;
			}

			void release()
			{
				m_force_value.reset();
			}

			bool is_forced() const
			{
				return m_force_value.has_value();
			}

			const _t& get() const
			{
				return m_force_value.value();
			}

			option<_t> m_force_value;
		};

	public:
		using callback = std::function<void()>;
		void run(const callback& clb);

		void set_name(const string& name);
		const string& get_name() const;

		// coordinates are in in pixels
		math::rectf get_rect() const;
		const math::float2& get_position() const;
		const math::float2& get_size() const;

		void toggle();
		void set_visible(bool new_visible);
		bool is_visible() const;

		void set_position(const math::float2& new_position);
		void set_size(const math::float2& new_size);

		virtual void on_run() {}

	private:
		string m_title;
		optional_property<math::float2> m_force_position;
		optional_property<math::float2> m_force_size;
		bool m_is_visible = true;

		math::float2 m_last_position;
		math::float2 m_last_size;

		// imgui can manage these properties, but we can force their values
		optional_property<math::float2>& get_size_prop();
		optional_property<math::float2>& get_position_prop();
	};
}