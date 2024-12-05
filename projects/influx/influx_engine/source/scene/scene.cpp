#include "engine_pch.h"
#include "scene/scene.h"

#if 0
// influx::engine
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
		file_scene file{};
		file.load(path);

		scene* new_scene = new scene();
		const uint32 num_entities = file.get_num_actors();
		for (uint32 i = 0u; i < num_entities; ++i)
		{
			new_scene->add_actor("actor");
		}

		return new_scene;
	}

	bool scene::save_to_file(scene* scene, const string& path)
	{
		influx_assert(scene != nullptr);

		file_scene new_file{};
		const uint32 num_entities = new_file.get_num_actors();
		for (uint32 i = 0u; i < num_entities; ++i)
		{
			const actor& actor = scene->m_actors[i];
			new_file.add_actor(i, actor.get_name());
		}

		new_file.save(path);
		return true;
	}

	void scene::add_actor(const string& name)
	{
		
	}

	string actor::get_name() const
	{
		return m_name;
	}

	void actor::set_name(const string& name)
	{
		m_name = name;
	}
}
#endif