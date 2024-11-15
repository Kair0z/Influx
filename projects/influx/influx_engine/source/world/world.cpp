#include "engine_pch.h"
#include "world.h"

// influx::engine
#include "influx_engine/scene/scene.h"
#include "content/content_manager.h"

// influx::renderer
#include "influx_renderer/scene.h"

namespace influx::engine
{
    world::world()
    {
        foreach_pool([](entity_pool& pool)
        {
            pool.reset();
        });
    }

    world::~world()
    {

    }

    void world::load_scene(scene* scene, bool set_main)
    {
        m_scenes.push_back(scene);

        if (set_main)
        {
            m_mainscene_idx = uint32(m_scenes.size() - 1u);
        }
    }

    void world::unload_scene(scene*)
    {

    }

    scene* world::get_main_scene()
    {
        return m_scenes[m_mainscene_idx];
    }

    ref_ptr<entity> world::find_entity(entity_id id)
    {
        entity_capsule* capsule = find_capsule(id);
        if (capsule)
        {
            return capsule->m_entity;
        }

        return nullptr;
    }

    uint32 g_idtracker = 0u;
    ref_ptr<entity> world::new_entity()
    {
        entity_capsule new_capsule{};
        const uint32 pool_idx = 0u;
        new_capsule.m_entity = m_entity_pools[pool_idx].allocate();
        new_capsule.m_entity->set_id(g_idtracker++);
        new_capsule.m_pool_idx = pool_idx;
        m_entities.push_back(new_capsule);
        return m_entities.back().m_entity;
    }

    bool world::remove_entity(entity_id id)
    {
        entity_capsule* capsule = find_capsule(id);
        if (capsule)
        {
            // remove reference
            capsule->m_entity.release();
            return true;
        }

        return false;
    }

    void world::remove_all()
    {
        for (entity_capsule& capsule : m_entities)
        {
            capsule.m_entity.release();
        }
    }

    void world::build_renderscene(renderer::scene& scene) const
    {
        scene.m_camera.m_far_plane = 1000.0f;
        scene.m_camera.m_near_plane = 0.001f;
        scene.m_camera.m_fov = 90.0f;
        scene.m_camera.m_transform = math::transform3D::identity();

        const auto& scenes = get_engine()->get_content()->get_scenes();
        if (scenes.empty())
        {
            logonce(e_log_category::warning, "world::build_renderscene > no scene content to render!");
            return;
        }

        const string mesh_name = "sphere";

        scene.m_meshes.clear();
        for (uint32 i = 0u; i < 10u; ++i)
        {
            renderer::mesh_instance mesh{};
            mesh.m_name = mesh_name;
            mesh.m_transform = math::matrix4x4f::identity();
            mesh.m_material_name = "";
            mesh.m_per_instance_colour;
            scene.m_meshes.push_back(mesh);
        }
    }

    // deletes unreferenced entities
    void world::flush()
    {
        m_entities.remove_if([this](const entity_capsule& capsule)
        {
            // the only reference remaining is the one held in the list m_entities
            // nobody else references it anymore, so let's get rid of it
            if (capsule.m_entity && capsule.m_entity.get_refcount() == 0u)
            {
                m_entity_pools[capsule.m_pool_idx].free_lockless(capsule.m_entity.get_ptr());
                return true;
            }
            
            return false;
        });
    }

    world::entity_capsule* world::find_capsule(entity_id id)
    {
        // find capsule with id
        const auto& found = std::find_if(m_entities.begin(), m_entities.end(), [id](entity_capsule& capsule)
        {
            return capsule.m_entity && capsule.m_entity->get_id() == id;
        });

        if (found != m_entities.cend())
        {
            return &(*found);
        }

        return nullptr;
    }

    entity_id entity::get_id() const
    {
        return m_id;
    }

    void entity::set_id(entity_id id)
    {
        m_id = id;
    }
}