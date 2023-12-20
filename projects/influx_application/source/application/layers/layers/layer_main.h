#pragma once
#include "application/layers/layer_base.h"
#include <iostream>

namespace influx::application
{
	class layer_main final : public layer_base
	{
	private:
		layer_main(const layer_base_args& args)
			: layer_base(args) {}

		virtual void on_enable() override
		{
			std::cout << get_name() << " | " << "enabled! \n";
		}

		virtual void on_event(layer_event*) override
		{
			std::cout << get_name() << " | " << "event! \n";
		}

		virtual void on_tick() override
		{
			std::cout << get_name() << " | " << "tick! \n";
		}

		virtual void on_disable() override
		{
			std::cout << get_name() << " | " << "disable! \n";
		}

		friend class layer_stack;
	};
}


