#include "engine_pch.h"
#include "influx_engine/scene/scene.h"

#include "file/engine_files.h"

namespace influx::engine
{
	scene* scene::load_from_file(const string& path)
	{
		scene* new_file = new scene();
		return new_file;
	}

	const vector<actor>& scene::get_all_actors() const
	{
		return m_actors;
	}
}