#include "engine_pch.h"
#include "influx_engine/scene/scene.h"

#include "file/engine_files.h"

namespace influx::engine
{
	scene* scene::make_empty_scene()
	{
		return new scene();
	}

	scene* scene::make_default_scene()
	{
		return make_empty_scene();
	}

	scene* scene::load_from_file(const string& path)
	{
		file_scene new_file{};
		new_file.load(path);

		scene* new_scene = new scene();

		// file -> scene
		const uint64 num_actors = new_file.m_actor_ids.size();
		for (uint64 i = 0u; i < num_actors; ++i)
		{
			new_scene->add_actor("actor");
		}

		return new_scene;
	}

	bool scene::save_to_file(scene* scene, const string& path)
	{
		influx_assert(scene != nullptr);

		file_scene new_file{};
		const uint64 num_actors = scene->m_actors.size();
		for (uint64 i = 0u; i < num_actors; ++i)
		{
			const actor& actor = scene->m_actors[i];
			new_file.m_actor_ids.push_back(uint32(i));
			new_file.m_actor_names.push_back(actor.get_name());
		}

		// save to path
		new_file.save(path);
		return true;
	}

	const vector<actor>& scene::get_all_actors() const
	{
		return m_actors;
	}

	void scene::add_actor(const string& name)
	{
		m_actors.push_back(actor(name));
	}

	actor::actor(const string& name)
		: m_name{ name }
	{
		
	}

	const string& actor::get_name() const
	{
		return m_name;
	}
}