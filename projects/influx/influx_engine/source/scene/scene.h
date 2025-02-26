#pragma once

// influx::engine
#include "world/world.h"

namespace influx::engine
{
	class scene final
	{
	public:
		using entity_id = uint32;
		~scene();

	private:
		friend class scene_manager;
		scene();

		bool m_is_active = false;
		struct entity_info final
		{
			uint32			m_components[k_num_component_types]{};
			entt::entity	m_entity;
		};
		vector<entity_info> m_entities{};
		
		void serialize(const string& path, bool is_loading);

	public:
		entity_id create_entity();
		void destroy_entity(entity_id);

		template <typename _c, typename... _args>
		inline _c& create_component(entity_id id, _args&&... args)
		{
			influx_assert(is_valid(id));

			static world& world = get_engine()->get_world();
			return world.create_component<_c>(m_entities[id].m_entity, args...);
		}

		template <typename _c>
		inline void destroy_component(entity_id id)
		{
			if (is_valid(id) == false) return;

			static world& world = get_engine()->get_world();
			return world.destroy_component<_c>(m_entities[id].m_entity);
		}

		template<typename _c>
		inline _c* get_component(entity_id id)
		{
			if (is_valid(id) == false) return nullptr;

			static world& world = get_engine()->get_world();
			return world.get_component<_c>(m_entities[id].m_entity);
		}

		template<typename _c>
		inline bool has_component(entity_id id)
		{
			if (is_valid(id) == false) return false;

			static world& world = get_engine()->get_world();
			return world.has_component<_c>(m_entities[id].m_entity);
		}

		bool is_valid(entity_id) const;
		bool is_active(entity_id) const;
		bool is_visible(entity_id) const;

		bool is_active() const;
	};

	class scene_manager final
	{
	public:
		using scene_id = uint32;
		
	private:
		vector<scene> m_scenes{};
		uint32 m_current_index = 0u;

	public:	
		scene_manager();
		~scene_manager();

		scene& get_current_scene();

		bool is_active(scene_id) const;
		bool is_valid(scene_id) const;

		scene_id	create_scene(bool make_current = true);
		void		destroy_scene(scene_id id);
		scene_id	import_scene(const string& path, bool make_current = true);
	};
}