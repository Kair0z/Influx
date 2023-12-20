#pragma once

#include "core/container/vector.h"
#include "core/container/map.h"
#include "core/string.h"

namespace influx::application
{
	class layer_base;
	struct layer_event;
	class layer_stack final
	{
	public:
		template <class _layer_t, class _args>
		void push(_args&& args)
		{
			mp_layers.push_back(new _layer_t(args));
		}

		void set_enabled(const string& string, bool new_enabled);
		void pop(const string& string);
		void pop();
		void clear();
		void process_events();
		void tick();

		void queue_event(layer_event* e);

		virtual ~layer_stack();
	private:
		vector<layer_base*> mp_layers{};

		layer_base* find_layer(const std::string& name) const;
		bool pop_at(vector<layer_base*>::iterator it);
	};
}


