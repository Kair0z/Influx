#include "influx_file.h"
#include <cassert>

int main()
{
	using namespace influx;

	files::scenefile file{};
	
	for (uint32 i = 0u; i < 10u; ++i)
	{
		file.add_actor(i, "actor:" + std::to_string(i));
	}

	for (uint32 i = 0u; i < file.get_num_actors(); ++i)
	{
		assert(file.m_actor_ids[i] == i);
	}
	
	file.save("D:/data/stash/scene.flx");
}