#pragma region

// influx::core
#include "core/container/pool.h"
#include "core/container/list.h"
#include "core/container/vector.h"
#include "core/pointer.h"

namespace influx::engine
{
	class scene;
}

namespace influx::engine
{
	using entity_id = uint32;
	class entity
	{
	public:
		entity_id get_id() const;
		void set_id(entity_id);

	private:
		entity_id m_id;
	};

	class world final
	{
		static constexpr uint64 k_max_entity_memory = 1024 * 4u; // 4KB
		static constexpr uint32 k_max_num_entities = k_max_entity_memory / sizeof(entity);
		static constexpr uint32 k_num_entity_pools = 4u;
		using entity_pool = pool<entity, k_max_num_entities / k_num_entity_pools>;

		struct entity_capsule final
		{
			ref_ptr<entity> m_entity;
			uint32 m_pool_idx;
		};

	public:
		world();

		virtual ~world();

		// loads all entities belonging to the scene into the world
		void load_scene(scene*, bool set_main = true);
		
		// unloads all entities belonging to this scene from the world
		void unload_scene(scene*);

		// currently active / last added scene
		scene* get_main_scene();

		ref_ptr<entity> find_entity(entity_id);

		ref_ptr<entity> new_entity();

		bool remove_entity(entity_id);
		
		void remove_all();

		// deletes unreferenced entities
		void flush();

	private:
		entity_pool m_entity_pools[k_num_entity_pools]{};
		list<entity_capsule> m_entities{};
		vector<scene*> m_scenes{};
		uint32 m_mainscene_idx = 0u;

		template <typename _func>
		void foreach_pool(_func&& func)
		{
			for (uint32 i = 0u; i < k_num_entity_pools; ++i)
				func(m_entity_pools[i]);
		}

		entity_capsule* find_capsule(entity_id id);
	};
}