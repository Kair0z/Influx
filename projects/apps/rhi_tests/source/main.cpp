
#include "influx_file.h"
#include <iostream>

void load()
{
	influx::files::projectfile project{};
	project.load("C:/Users/Arne/Desktop/folder/project.flx");
	printf(project.m_name.c_str());
}

void save()
{
	influx::files::projectfile project{};

	for (int i = 0; i < 1; ++i)
	{
		project.m_entities.push_back({});
		auto& entity = project.m_entities.back();
		entity.m_name = "entity_" + std::to_string(i);
		for (int c = 0; c < 4; ++c)
		{
			entity.m_components.push_back({});
			auto& comp = entity.m_components.back();
			comp.m_name = "comp_" + std::to_string(c);
		}
	}
	
	project.m_name = "noks";
	
	project.save("C:/Users/Arne/Desktop/folder/project.flx");
}

int main()
{
	save();
	load();
}