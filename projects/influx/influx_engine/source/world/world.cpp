#include "engine_pch.h"
#include "world/world.h"

// influx::engine
#include "scene/scene.h"
#include "content/content_manager.h"

// influx::renderer
#include "influx_renderer/scene.h"

// entt
#include "entt/entt.hpp"

entt::registry m_registry;

namespace influx::engine
{
    world::world()
    {
        
    }

    world::~world()
    {

    }

    void world::build_renderscene(renderer::scene& scene, renderer::scene2D& scene2D) const
    {
        // general scene
        scene.m_camera.m_far_plane = 1000.0f;
        scene.m_camera.m_near_plane = 0.001f;
        
        // build the camera
        {
            auto view = m_registry.view<const transform_component, camera_component>();
            for (auto [entity, transform_comp, camera_comp] : view.each())
            {
                math::transform3D transform = transform_comp.get_transform();

                scene.m_camera.m_fov = camera_comp.get_fov();
                scene.m_camera.m_transform = transform;
                scene.m_camera.m_transform.update_matrix();
            }
        }

        // 2D scene
        // build all sprites
        {
            auto view = m_registry.view<const transform_component, sprite_component>();
            for (auto [entity, transform_comp, sprite] : view.each())
            {
                math::transform3D transform = transform_comp.get_transform();
                math::vectorf2 position2D = { transform.get_position().x, transform.get_position().y };

                renderer::sprite2D render_sprite{};
                render_sprite.m_texture = sprite.get_texture_path();
                render_sprite.m_rectangle = math::rectf::square_rect(1.0f);
                render_sprite.m_scale_to_view = true;
                render_sprite.m_transform.set_position(position2D);
                scene2D.m_sprites.push_back(render_sprite);
            }
        }

        // 3D scene
        const auto& scenes = get_engine()->get_content()->get_scenes();
        if (scenes.empty())
        {
            logonce(e_log_category::warning, "world::build_renderscene > no scene content to render!");
            return;
        }

        // build all meshes
        // use a callback
        {
            auto view = m_registry.view<const transform_component, mesh_component>();
            for (auto [entity, transform_comp, mesh_comp] : view.each())
            {
                math::transform3D transform = transform_comp.get_transform();

                renderer::mesh_instance render_mesh{};
                render_mesh.m_name = mesh_comp.get_mesh_path();
                render_mesh.m_material_name = "";
                render_mesh.m_per_instance_colour = {};
                render_mesh.m_transform = transform.get_matrix();
                scene.m_meshes.push_back(render_mesh);
            }
        }
    }

    void world::flush()
    {

    }
}
