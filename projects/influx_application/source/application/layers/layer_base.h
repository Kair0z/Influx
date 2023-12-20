#pragma once

#include "application/konstants.h"
#include "core/container/ringbuffer.h"
#include "core/string.h"
#include <thread>

namespace influx::application
{
	struct layer_base_args
	{
		layer_base_args() = default;
		layer_base_args(const string& name)
			: m_name{ name } {}

		string m_name{};
	};

	struct layer_event
	{

	};

	class layer_base
	{
	public:
		void set_enabled(bool new_enabled);
		void queue_event(layer_event* e);
		void tick_if_enabled();
		void process_events_if_enabled();

		enum class e_state : uint8
		{
			created,
			started,
			enabled,
			disabled,
			ended,
			max
		};

		const string& get_name() const;
		const layer_base_args& get_base_args() const;
		virtual ~layer_base();

	protected:
		layer_base(const layer_base_args& args);

	private:
		virtual void on_enable() = 0;
		virtual void on_event(layer_event*) = 0;
		virtual void on_tick() = 0;
		virtual void on_disable() = 0;

		layer_base_args m_base_args{};
		e_state m_state = e_state::created;
		ringbuffer<layer_event*, k_event_queue_capacity> m_event_queue{};
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


