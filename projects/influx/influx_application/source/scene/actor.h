#pragma once
#include "object.h"

namespace influx::scene
{
	class actor final : public object
	{
	public:
		actor(actor_id id);

		void tick();
		
		template <typename _tcomp> 
		_tcomp* get_component();

		template <typename _tcomp>
		bool has_component();

		template <typename _tcomp>
		void add_component();

		template <typename _tcomp>
		void remove_component();

		actor_id get_actor_id() const;

	private:
		actor_id m_id = k_invalid_id;
	};
}