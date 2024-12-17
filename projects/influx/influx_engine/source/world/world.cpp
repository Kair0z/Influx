#include "engine_pch.h"
#include "world/world.h"

// influx::engine
#include "scene/scene.h"
#include "content/content_manager.h"

// influx::renderer
#include "influx_renderer/scene.h"

// influx::input
#include "influx_input.h"

namespace influx::engine
{
    world::world()
    {
        input::subscribe_keydown([this](input::e_key key) { m_deferred_keydowns.push(key); });
        input::subscribe_keyup([this](input::e_key key) { m_deferred_keyups.push(key); });
        input::subscribe_asciidown([this](char ascii) { m_deferred_ascii_downs.push(ascii); });
        input::subscribe_asciiup([this](char ascii) { m_deferred_ascii_ups.push(ascii); });
        input::subscribe_mousemove([this](const input::mouse_position& position) { m_deferred_mousemoves.push(position);  });
        input::subscribe_mousedown([this](input::e_mouse_button button, const input::mouse_position& position) { m_deferred_mousedowns.push({ button, position }); });
        input::subscribe_mouseup([this](input::e_mouse_button button, const input::mouse_position& position) { m_deferred_mouseups.push({ button, position }); });
    }

    world::~world()
    {

    }
    
    void world::update()
    {
        update_input_system();
        update_bounds_system();
        update_stream_system();
    }

    void world::build_renderscene(renderer::scene& scene, renderer::scene2D& scene2D, renderer::scene_debug& debugscene) const
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

        debugscene.m_camera = scene.m_camera;

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

        // make a cute gizmo :)
        debugscene.clear();
        // debugscene.add_line(math::float3{ 0.0f, 0.0f, 0.0f }, math::float3{ 1.0f, 0.0f, 0.0f }, { 1,0,0,1 });
        // debugscene.add_line(math::float3{ 0.0f, 0.0f, 0.0f }, math::float3{ 0.0f, 1.0f, 0.0f }, { 0,1,0,1 });
        // debugscene.add_line(math::float3{ 0.0f, 0.0f, 0.0f }, math::float3{ 0.0f, 0.0f, 1.0f }, { 0,0,1,1 });

        // build all meshes
        {
            auto view = m_registry.view<transform_component, mesh_component>();
            for (auto [entity, transform_comp, mesh_comp] : view.each())
            {
                math::transform3D transform = transform_comp.get_transform();
                if (mesh_comp.get_use_normalized_scale() && mesh_comp.m_mesh_boundsphere.m_radius > 0.0f)
                {
                    const float normalized_scale = 1.0f / mesh_comp.m_mesh_boundsphere.m_radius;
                    transform.set_scale(transform.get_scale() * normalized_scale);
                }
                transform.update_matrix();

                renderer::mesh_instance render_mesh{};
                render_mesh.m_name = mesh_comp.get_mesh_path();
                render_mesh.m_material_name = "";
                render_mesh.m_per_instance_colour = {};
                render_mesh.m_transform = transform.get_matrix();
                render_mesh.m_invert_normals = mesh_comp.get_invert_normals();
                scene.m_meshes.push_back(render_mesh);

                // transform gizmo ;)
                debugscene.add_line(transform.get_position(), transform.get_position() + transform.get_right(),     {1,0,0,1});
                debugscene.add_line(transform.get_position(), transform.get_position() + transform.get_up(),        {0,1,0,1});
                debugscene.add_line(transform.get_position(), transform.get_position() + transform.get_forward(),   {0,0,1,1});
            }
        }
    }

    entity world::create_entity()
    {
        return (entity)m_registry.create();
    }

    void world::flush()
    {

    }

    void world::update_input_system()
    {
        auto view = m_registry.view<input_component>();
        for (auto [entity, input] : view.each())
        {
            if (input.m_on_keydown)
            {
                m_deferred_keydowns.get_copy().read([&input](input::e_key keydown)
                    {
                        input.m_on_keydown(keydown);
                    });
            }

            if (input.m_on_keyup)
            {
                m_deferred_keyups.get_copy().read([&input](const auto& val)
                    {
                        input.m_on_keyup(val);
                    });
            }

            if (input.m_on_ascii_down)
            {
                m_deferred_ascii_downs.get_copy().read([&input](const auto& val)
                    {
                        input.m_on_ascii_down(val);
                    });
            }

            if (input.m_on_ascii_up)
            {
                m_deferred_ascii_ups.get_copy().read([&input](const auto& val)
                    {
                        input.m_on_ascii_up(val);
                    });
            }

            if (input.m_on_mouse_move)
            {
                m_deferred_mousemoves.get_copy().read([&input](const auto& val)
                    {
                        input.m_on_mouse_move(val);
                    });
            }

            if (input.m_on_mouse_down)
            {
                m_deferred_mousedowns.get_copy().read([&input](const auto& val)
                    {
                        input.m_on_mouse_down(val.first, val.second);
                    });
            }

            if (input.m_on_mouse_up)
            {
                m_deferred_mouseups.get_copy().read([&input](const auto& val)
                    {
                        input.m_on_mouse_up(val.first, val.second);
                    });
            }
        }

        m_deferred_keydowns.clear();
        m_deferred_keyups.clear();
        m_deferred_ascii_downs.clear();
        m_deferred_ascii_ups.clear();
        m_deferred_mousemoves.clear();
        m_deferred_mousedowns.clear();
        m_deferred_mouseups.clear();
    }

    void world::update_bounds_system()
    {

    }

    void world::update_stream_system()
    {
        content_manager* contman = get_engine()->get_content().get();

        {
            auto view = m_registry.view<sprite_component>();
            for (auto [entity, sprite] : view.each())
            {
                auto asset = contman->find<image_asset>(sprite.get_texture_path());
                if (asset.is_success() && asset->is_loaded())
                {
                    sprite.m_texture_dimensions = asset->m_resource.m_dimensions;
                }
            }
        }

        {
            auto view = m_registry.view<mesh_component>();
            for (auto [entity, mesh_comp] : view.each())
            {
                result<scene_asset const*> asset = contman->find<scene_asset>(mesh_comp.get_mesh_path());
                if (asset.is_success() && asset->is_loaded())
                {
                    const imp::mesh& mesh = asset->m_resource.get_main_mesh();
                    mesh_comp.m_mesh_boundbox = mesh.m_bounding_box;
                    mesh_comp.m_mesh_boundsphere = mesh.m_bounding_sphere;
                }
            }
        }
    }
}
