#pragma once

#include "core/container/ringbuffer.h"
#include "core/string.h"
#include <thread>

namespace influx::application
{
	struct layer_base_args
	{
		layer_base_args() = default;
		layer_base_args(const string& name, bool ded_thread)
			: m_name{ name }, m_dedicated_thread{ ded_thread }{}

		string m_name{};
		bool m_dedicated_thread = true;
	};

	struct layer_event
	{

	};

	class layer_base
	{
	public:
		virtual ~layer_base();

		void set_enabled(bool new_enabled);
		void queue_event(layer_event* e);
		void tick_if_enabled();
		void process_events_if_enabled();

		enum class e_state
		{
			started,
			enabled,
			disabled,
			ended,
			max
		};

		const string& get_name() const;
		bool is_dedicated_thread() const;
		const layer_base_args& get_base_args() const;

	protected:
		layer_base(const layer_base_args& args);

	private:
		virtual void on_enable() = 0;
		virtual void on_event(layer_event*) = 0;
		virtual void on_tick() = 0;
		virtual void on_disable() = 0;

		layer_base_args m_base_args{};
		e_state m_state = e_state::started;
		std::thread m_thread_obj{};
		ringbuffer<layer_event*, 4096u> m_event_queue{};

		void defer_state_change(e_state new_state);
		ringbuffer<e_state, 16u> m_deferred_state_changes{};

		void set_enabled_st(bool new_enabled);
		void tick_if_enabled_st();
		void process_events_if_enabled_st();
	};

	class layer_null final : public layer_base
	{
	private:
		layer_null(const layer_base_args& args)
			: layer_base(args) {}

		virtual void on_enable() override
		{

		}

		virtual void on_event(layer_event*) override
		{

		}

		virtual void on_tick() override
		{

		}

		virtual void on_disable() override
		{

		}
	};
}


