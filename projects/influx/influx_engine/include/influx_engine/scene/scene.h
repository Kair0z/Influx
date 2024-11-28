#pragma once

// influx::core
#include "core/container/vector.h"
#include "core/string.h"
#include "core/pointer.h"

#if 0
namespace influx::engine
{
	class entity;
	using entity_ref = ref_ptr<entity>;
}

namespace influx::engine
{
	class actor
	{
	public:
		actor(const entity_ref& entity)
			: m_entity{ entity } {}

		string get_name() const;
		void set_name(const string& name);

	private:
		entity_ref m_entity;
		string m_name;
	};

	class scene
	{
	public:
		scene() = default;

		INFLUX_ENGINE_API
		static scene* make_empty_scene();

		INFLUX_ENGINE_API
		static scene* make_default_scene();

		INFLUX_ENGINE_API
		static scene* load_from_file(const string& path);

		INFLUX_ENGINE_API
		static bool save_to_file(scene*, const string& path);

		INFLUX_ENGINE_API
		const vector<entity_ref> get_entities() const;

		INFLUX_ENGINE_API
		void add_actor(const string& name);

		INFLUX_ENGINE_API
		entity_ref find_actor(const string& name);

		INFLUX_ENGINE_API
		void remove_actor(const string&);

	private:
		vector<actor> m_actors{};
	};
}
#endif