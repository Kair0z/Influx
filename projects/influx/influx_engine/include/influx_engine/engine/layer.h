#pragma once

// influx::engine
#include "influx_engine/engine/engine.h" // ctx_update

namespace influx::engine
{
	class world;
	struct ctx_update;
	class gameobject;

	class layer
	{
	public:
		// layers
		void add_child(layer*);

		// inheritable
		virtual void on_start() {}
		virtual void on_update() {}
		virtual void on_exit() {}

		// dont touch this
		void update()
		{
			on_update();

			for (uint64 i = 0u; i < m_children.size(); ++i)
			{
				layer* child = m_children[i];
				child->update();
			}
		}

	private:
		layer* m_parent;
		vector<layer*> m_children;
	};
}