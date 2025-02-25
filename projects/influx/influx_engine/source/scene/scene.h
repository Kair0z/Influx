#pragma once

// influx::core
#include "core/container/vector.h"

namespace influx::engine
{
	class scene final
	{
	private:
		friend class scene_manager;
		bool m_is_active = false;

		scene();
		~scene();
		void serialize(const string& path, bool is_loading);

	public:
		void create_entity();
		void destroy_entity();

		void create_component(entity&);
		void destroy_component(entity&);

		component* get_component(const entity&) const;
		bool has_component(const entity&) const;
	};

	class scene_manager final
	{
		vector<scene> m_scenes{};

	public:
		scene_manager();
		~scene_manager();

		void create_scene();
		void destroy_scene();
	};
}