#include "engine_pch.h"
#include "scene/scene.h"

namespace influx::engine
{
	inline static uint32 get_component_index(e_component type)
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

	scene::entity_id scene::create_entity()
	{
		static world& world = get_engine()->get_world();

		entity_info info{};
		info.m_entity = world.create_entity();

		// create scene component
		scene_component& scene_comp = world.create_component<scene_component>(info.m_entity);
		scene_comp.set_scene_active(m_is_active);

		m_entities.push_back(info);
		return static_cast<uint32>(m_entities.size() - 1);
	}

	void scene::destroy_entity(entity_id handle)
	{
		if (is_valid(handle) == false) return;

		static world& world = get_engine()->get_world();
		world.destroy_entity(m_entities[handle].m_entity);
	}
	
	bool scene::is_valid(entity_id handle) const
	{
		return handle < m_entities.size();
	}

	bool scene::is_active(entity_id) const
	{
		return true;
	}

	void scene::set_active(bool new_active)
	{
		if (new_active == is_active()) return;

		static world& world = get_engine()->get_world();
		for (const entity_info& ent : m_entities)
		{
			world.get_component<scene_component>(ent.m_entity)->set_scene_active(new_active);
		}
	}

	bool scene::is_active() const
	{
		return m_is_active;
	}

#pragma region scene manager
	scene_manager::scene_manager()
	{

	}

	scene_manager::~scene_manager()
	{

	}

	scene& scene_manager::get_current_scene()
	{
		return m_scenes[m_current_index];
	}

	bool scene_manager::is_active(scene_id id) const
	{
		if (is_valid(id) == false) return false;
		
		return m_scenes[id].is_active();
	}

	bool scene_manager::is_valid(scene_id id) const
	{
		return id < m_scenes.size();
	}

	scene_manager::scene_id scene_manager::create_scene(bool make_current)
	{
		scene new_scene{};
		m_scenes.push_back(new_scene);

		uint32 new_index = static_cast<uint32>(m_scenes.size() - 1u);
		if (make_current) m_current_index = new_index;
		return new_index;
	}

	void scene_manager::destroy_scene(scene_id id)
	{
		if (is_valid(id))
		{
			m_scenes[id] = m_scenes.back();
			m_scenes.pop_back();
		}
	}

	scene_manager::scene_id scene_manager::import_scene(const string& path, bool make_current)
	{
		scene new_scene{};

		m_scenes.push_back(new_scene);

		uint32 new_index = static_cast<uint32>(m_scenes.size() - 1u);
		if (make_current) m_current_index = new_index;
		return new_index;
	}
#pragma endregion
}