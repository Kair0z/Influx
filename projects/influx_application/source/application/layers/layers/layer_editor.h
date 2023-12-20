#pragma once

#include "application/layers/layer_base.h"

namespace influx::application
{
	class layer_editor final : public layer_base
	{
	private:
		layer_editor(const layer_base_args& args)
			: layer_base(args) {}

		virtual void on_enable() override;
		virtual void on_event(layer_event*) override;
		virtual void on_tick() override;
		virtual void on_disable() override;
	};
}