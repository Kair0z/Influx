#pragma once

// influx::core
#include "core/container/vector.h"
#include "core/string.h"

namespace influx::engine
{
	class actor
	{
	public:
		actor(const string& name);

		void set_name(const string& name);

		const string& get_name() const;

		void add_component();

		void remove_component();

	private:
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
		const vector<actor>& get_all_actors() const;

		INFLUX_ENGINE_API
		void add_actor(const string& name);

		INFLUX_ENGINE_API
		actor* find_actor(const string& name);

		INFLUX_ENGINE_API
		void remove_actor(const string&);

	private:
		vector<actor> m_actors{};
	};
}