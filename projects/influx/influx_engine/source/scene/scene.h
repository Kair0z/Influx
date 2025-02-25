#pragma once

// influx::core
#include "core/container/vector.h"

// influx::engine
#include "component/component.h"

// influx::engine
#include "world/world.h"

namespace influx::engine
{
	class component;

	using entity_id = uint32;

	class scene final
	{
	private:
		friend class scene_manager;
		bool m_is_active = false;

		scene();
		~scene();

		void serialize(const string& path, bool is_loading);

	public:
		entity_id create_entity();
		void destroy_entity(entity_id);

		template<typename _c, typename... _args>
		_c& create_component(entt::entity, _args&&... args)
		{
			
		}

		template <typename _c>
		void destroy_component(entt::entity)
		{

		}

		template<typename _c>
		_c* get_component(entt::entity)
		{

		}

		template<typename _c>
		bool has_component(entt::entity)
		{
			
		}

		bool is_valid(entity_id) const;
		bool is_active(entity_id) const;
		bool is_visible(entity_id) const;

	private:
		struct entity_info final
		{
			uint32			m_components[k_num_component_types]{};
			entt::entity	m_entity;
		};
		vector<entity_info> m_entities{};
	};

	using scene_id = uint32;

	class scene_manager final
	{
		vector<scene> m_scenes{};

	public:	
		scene_manager();
		~scene_manager();

		bool is_active(scene_id) const;
		bool is_valid(scene_id) const;

		scene_id create_scene();
		void destroy_scene(scene_id id);
		scene_id import_scene(const string& path);
	};
}