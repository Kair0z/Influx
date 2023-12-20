#include "app_pch.h"
#include "layer_stack.h"

#include "application/layers/layer_base.h"

namespace influx::application
{
	void layer_stack::set_enabled(const string& string, bool new_enabled)
	{
		layer_base* found = find_layer(string);
		if (found != nullptr)
		{
			found->set_enabled(new_enabled);
		}
	}

	void layer_stack::pop(const string& name)
	{
		auto found = std::find_if(mp_layers.begin(), mp_layers.end(), [name](const layer_base* layer)
		{
			return layer->get_name() == name;
		});

		if (found != mp_layers.end())
		{
			pop_at(found);
		}
	}

	void layer_stack::pop()
	{
		if (mp_layers.empty())
		{
			return;
		}

		pop_at(mp_layers.end() - 1);
	}

	void layer_stack::clear()
	{
		while (!mp_layers.empty())
		{
			pop();
		}
	}

	void layer_stack::process_events()
	{
		for (auto it = mp_layers.rbegin(); it != mp_layers.rend(); ++it)
		{
			(*it)->process_events_if_enabled();
		}
	}

	void layer_stack::tick()
	{
		for (layer_base* layer : mp_layers)
		{
			layer->tick_if_enabled();
		}
	}

	void layer_stack::queue_event(layer_event* e)
	{
		influx_assert_not_null(e);

		for (layer_base* layer : mp_layers)
		{
			layer->queue_event(e);
		}
	}

	layer_stack::~layer_stack()
	{
		clear();
	}

	layer_base* layer_stack::find_layer(const std::string& name) const
	{
		auto found = std::find_if(mp_layers.cbegin(), mp_layers.cend(), [name](const layer_base* layer)
		{
			return layer->get_name() == name;
		});

		return found != mp_layers.cend() ? *found : nullptr;
	}

	bool layer_stack::pop_at(vector<layer_base*>::iterator it)
	{
		if (it == mp_layers.end())
		{
			return false;
		}

		(*it)->set_enabled(false);
		influx_delete(*it);
		mp_layers.erase(it);
		return true;
	}
}

