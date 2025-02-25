#include "engine_pch.h"
#include "scene/scene.h"

namespace influx::engine
{
	uint32 get_component_index(e_component type)
	{
		return static_cast<uint32>(type);
	}

	scene::scene()
	{
	}

	scene::~scene()
	{
	}

	void scene::serialize(const string& path, bool is_loading)
	{
	}

	entity_id scene::create_entity()
	{
		static world& world = get_engine()->get_world();

		entity_info info{};
		info.m_entity = world.create_entity();
		m_entities.push_back(info);
		return static_cast<uint32>(m_entities.size() - 1);
	}

	void scene::destroy_entity(entity_id handle)
	{
		if (is_valid(handle) == false) return;

		static world& world = get_engine()->get_world();
		world.destroy_entity(m_entities[handle].m_entity);
	}
	void scene::create_component(entity_id handle, e_component comp)
	{
		if (is_valid(handle) == false) return;

		m_entities[handle].m_components[get_component_index(comp)] = 1u;
	}
	void scene::destroy_component(entity_id handle, e_component comp)
	{
		if (is_valid(handle) == false) return;
		m_entities[handle].m_components[get_component_index(comp)] = 0u;
	}
	component* scene::get_component(entity_id handle, e_component comp) const
	{
		if (is_valid(handle) == false) return nullptr;

		if (has_component(handle, comp))
		{
			
		}
		else
		{
			return nullptr;
		}
	}
	bool scene::has_component(entity_id handle, e_component comp) const
	{
		if (is_valid(handle) == false) return false;
		return m_entities[handle].m_components[get_component_index(comp)] > 0u;
	}
	bool scene::is_valid(entity_id handle) const
	{
		return handle < m_entities.size();
	}

#pragma region scene manager
	scene_manager::scene_manager()
	{

	}

	scene_manager::~scene_manager()
	{

	}

	bool scene_manager::is_active(scene_id id) const
	{
		if (is_valid(id) == false) return false;
	}
	bool scene_manager::is_valid(scene_id id) const
	{
		return id < m_scenes.size();
	}
	scene_id scene_manager::create_scene()
	{
		scene new_scene{};
		m_scenes.push_back(new_scene);
		return static_cast<uint32>(m_scenes.size() - 1u);
	}
	void scene_manager::destroy_scene(scene_id id)
	{
		if (is_valid(id))
		{
			m_scenes[id] = m_scenes.back();
			m_scenes.pop_back();
		}
	}
	scene_id scene_manager::import_scene(const string& path)
	{
		scene new_scene{};

		m_scenes.push_back(new_scene);
		return static_cast<uint32>(m_scenes.size() - 1u);
	}
#pragma endregion
}