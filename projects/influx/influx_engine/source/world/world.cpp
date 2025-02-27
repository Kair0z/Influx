#include "engine_pch.h"
#include "world/world.h"

// influx::engine
#include "scene/scene.h"
#include "content/content_manager.h"
#include "editor/editor_manager.h"
#include "input/input_manager.h"
#include "editor/editor_manager.h"

// influx::renderer
#include "influx_renderer/scene.h"

namespace influx::engine
{
    class world_ui final : public editor_window
    {
    public:
        virtual void on_run() override
        {
            world& world = get_engine()->get_world();
            set_name("engine:world");

            bool tabbar = ImGui::BeginTabBar("world");

            if (ImGui::BeginTabItem("entities") && tabbar)
            {
                uint32 index = 0u;
                world.visit_entities([&index](const entt::entity& entity)
                {
                    const string tag = "entity" + to_string(index++);
                    if (ImGui::TreeNode(tag.c_str()))
                    {
                        ImGui::TreePop();
                    }
                });
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("transforms") && tabbar)
            {
                uint32 index = 0u;
                world.visit_components<transform_component>([&index](const transform_component& transform)
                {
                    const string tag = "transform" + to_string(index++);
                    if (ImGui::TreeNode(tag.c_str()))
                    {
                        ImGui::TreePop();
                    }
                });

                ImGui::EndTabItem();
            }

            if (tabbar) ImGui::EndTabBar();
        }
    };


    world::world()
    {
        editor::editor_manager::static_window<world_ui>("world");
    }

    world::~world()
    {
    }
    
    void world::update()
    {
        update_transform_system();
        update_input_system();
        update_bounds_system();
        update_stream_system();
        update_rigidbody_system();
    }

    static math::ray* g_lastray = nullptr;

    void world::build_renderscene(renderer::scene& scene, renderer::scene2D& scene2D, renderer::scene_debug& debugscene) const
    {
        const bool is_editor = get_engine()->is_editor();
        const float delta_time = get_engine()->get_time().get_delta_seconds();

        debugscene.clear();
        
        // choose camera
        {
            influx_scope("build_camera");
            for (auto [entity, transform_comp, camera_comp] 
                : m_registry.view<const transform_component, camera_component>().each())
            {
                math::transform3D transform = transform_comp.get_transform();
                scene.m_camera.m_fov = camera_comp.get_fov();
                scene.m_camera.m_far_plane = camera_comp.get_farplane();
                scene.m_camera.m_near_plane = camera_comp.get_nearplane();
                scene.m_camera.m_transform = transform;
                scene.m_camera.m_transform.update_matrix();
            }
            debugscene.m_camera = scene.m_camera;
        }

        // render sprites
        {
            influx_scope("build_sprites");
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

        // render meshes
        {
            influx_scope("build_meshes");
            auto view = m_registry.view<transform_component, mesh_component>();
            for (auto [entity, transform_comp, mesh_comp] : view.each())
            {
                math::transform3D transform = transform_comp.get_transform();

                // normalize scale to bounding sphere
                if (mesh_comp.get_use_normalized_scale() && mesh_comp.m_mesh_boundsphere.m_radius > 0.0f)
                {
                    transform.set_scale(1.0f / mesh_comp.m_mesh_boundsphere.m_radius);
                    transform.update_matrix();
                }
                
                // setup mesh
                renderer::mesh_instance render_mesh{};
                render_mesh.m_name = mesh_comp.get_mesh_name();
                render_mesh.m_per_instance_colour = {};
                render_mesh.m_transform = transform.get_matrix();

                scene.m_meshes.push_back(render_mesh);
            }
        }

        // lights
        {
            influx_scope("build_lights");
            auto view = m_registry.view<transform_component, light_component>();
            for (auto [entity, transform_comp, light_comp] : view.each())
            {
                math::transform3D transform = transform_comp.get_transform();
                transform.update_matrix();

                renderer::light render_light{};
                render_light.m_light = light_comp.get_light();
                render_light.m_world_position = transform.get_position();
                render_light.m_world_forward = transform.get_forward();
                scene.m_lights.push_back(render_light);
            }
        }

        // editor render
        if (is_editor)
        {
            influx_scope("build_gizmos");
            debugscene.add_gizmo_transform(math::transform3D::identity());

            // grid render
            {
                const uint32 num_lines = 30u;
                const math::colour_rgba line_colour = { 0.2f, 0.2f, 0.2f };
                const float line_distance = 1.0f;
                const float line_length = num_lines * line_distance;
                const float half_offset = line_length * 0.5f;
                for (uint32 z = 0u; z < num_lines; ++z)
                {
                    const math::float3 basepos = math::float3{ -half_offset, 0.0f, -half_offset + (z * line_distance) };
                    const math::float3 endpos = basepos + math::float3{ half_offset * 2, 0, 0 };
                    if (z != num_lines / 2)
                    {
                        debugscene.add_line(basepos, endpos, line_colour);
                    }
                }
                for (uint32 x = 0u; x < num_lines; ++x)
                {
                    const math::float3 basepos = math::float3{ -half_offset + (x * line_distance), 0.0f, -half_offset };
                    const math::float3 endpos = basepos + math::float3{ 0, 0, half_offset * 2 };
                    if (x != num_lines / 2)
                    {
                        debugscene.add_line(basepos, endpos, line_colour);
                    }
                }
                
                const math::float3 origin = math::float3{ 0, 0, 0 };
                debugscene.add_line(origin, math::float3{ half_offset, 0, 0 }, colour::k_red);
                debugscene.add_line(origin, math::float3{ 0, half_offset, 0 }, colour::k_green);
                debugscene.add_line(origin, math::float3{ 0, 0, half_offset }, colour::k_blue);
            }

            // ray
            if (g_lastray)
            {
                // debugscene.add_line(g_lastray->get_origin(), g_lastray->get_origin() + g_lastray->get_direction() * 100.0f, colour::k_white);
            }

            // transform gizmos
            for (auto [entity, transform_comp] : m_registry.view<transform_component>().each())
            {
                if (m_registry.try_get<camera_component>(entity))
                {
                    // cameras shouldn't here
                    continue;
                }

                debugscene.add_gizmo_transform(transform_comp.get_transform());
            }

            // bounds boxes
            for (auto [entity, transform_comp, mesh_comp] : m_registry.view<transform_component, mesh_component>().each())
            {
                math::transform3D transform_copy = transform_comp.get_transform();
                const math::boxf transformed_bounds = mesh_comp.m_mesh_boundbox.get_transformed3D(transform_copy.get_matrix());
                debugscene.add_box(transformed_bounds, { 1,0,0,1 });
            }
        }
    }

    entt::entity world::create_entity()
    {
        entt::entity result = m_registry.create();
        return result;
    }

    void world::destroy_entity(entt::entity e)
    {
        m_registry.destroy(e);
    }

    bool world::trace(const math::ray& ray, trace_result& out_result, e_collision_layer layer)
    {
        out_result.m_entity = nullptr;

        struct hit_result final
        {
            entt::entity* m_entity;
            float m_hit_distance;
        };
        vector<hit_result> hit_results{};

        bool hit_any = false;
        for (auto [entity, transform_comp, mesh_comp] : m_registry.view<
            transform_component, mesh_component>().each())
        {
            math::transform3D transform = transform_comp.get_transform();
            transform.set_scale(1.0f / mesh_comp.m_mesh_boundsphere.m_radius);
            transform.update_matrix();

            math::boxf bounds = mesh_comp.m_mesh_boundbox;
            bounds = bounds.get_transformed3D(transform.get_matrix());

            float out_distance{};
            if (bounds.trace(ray, out_distance))
            {
                hit_result new_result{};
                new_result.m_entity = &entity;
                new_result.m_hit_distance = out_distance;
                hit_results.push_back(new_result);

                if (g_lastray == nullptr)
                    g_lastray = new math::ray();

                (*g_lastray) = ray;
                // transform_comp.move({ 0.0f, 1.0f, 0.0f });
            }
        }

        // find the closest one
        if (hit_results.size() > 0u)
        {
            std::sort(hit_results.begin(), hit_results.end(), [](const hit_result& a, const hit_result& b)
            {
                return a.m_hit_distance < b.m_hit_distance;
            });

            out_result.m_entity = hit_results[0u].m_entity;
        }

        return hit_results.size() != 0u;
    }

    bool world::is_valid(entt::entity e) const
    {
        return m_registry.valid(e);
    }

    void world::clear()
    {
        m_registry.clear();
    }

    void world::load_project(const influx::files::projectfile& proj)
    {
        clear();

        for (const files::entityfile& entityfile : proj.m_entities)
        {
            auto new_entity = create_entity();

            for (const files::componentfile& compfile : entityfile.m_components)
            {
                auto new_component = create_component<transform_component>(new_entity);
            }
        }
    }

    void world::save_project(influx::files::projectfile& proj)
    {
        proj.clear();

        for (auto entity : m_registry.view<entt::entity>())
        {
            proj.m_entities.push_back({});
        }
    }

    math::matrix4x4f world::get_main_projection_matrix() const
    {
        math::matrix4x4f projection_matrix{};
        for (auto [entity, transform_comp, camera_comp]
            : m_registry.view<const transform_component, camera_component>().each())
        {
            projection_matrix = camera_comp.get_projection();
        }
        return projection_matrix;
    }

    math::matrix4x4f world::get_main_viewmatrix() const
    {
        math::matrix4x4f view_matrix = {};
        for (auto [entity, transform_comp, camera_comp]
            : m_registry.view<const transform_component, camera_component>().each())
        {
            math::transform3D transform = transform_comp.get_transform();
            view_matrix = transform.get_matrix().inverted();
        }
        return view_matrix;
    }

    math::float3 world::get_main_cameraposition() const
    {
        math::float3 position{};
        for (auto [entity, transform_comp, camera_comp]
            : m_registry.view<const transform_component, camera_component>().each())
        {
            position = transform_comp.get_position();
        }
        return position;
    }

    void world::update_transform_system()
    {
        influx_scope("transform_system");
        auto view = m_registry.view<transform_component>();
        for (auto [entity, transform] : view.each())
        {
            transform.update_matrix();
        }
    }

    void world::update_input_system()
    {
        influx_scope("input_system");
        auto view = m_registry.view<input_component>();
        static input_manager& inputman = get_engine()->get_input();

        input::for_each_ascii([&view](char ascii)
        {
            const buttonstate& state = inputman.get_keystate(ascii);
            bool is_new = state.m_num_frames == 0u;
            if (state.m_is_down && is_new)
            {
                for (auto [entity, input] : view.each())
                {
                    if (input.m_on_ascii_down) input.m_on_ascii_down(ascii);
                }
            }
            if (!state.m_is_down && is_new)
            {
                for (auto [entity, input] : view.each())
                {
                    if (input.m_on_ascii_up) input.m_on_ascii_up(ascii);
                }
            }
        });
        input::for_each_key([&view](input::e_key key)
        {
            const buttonstate& state = inputman.get_keystate(key);
            bool is_new = state.m_num_frames == 0u;
            if (state.m_is_down && is_new)
            {
                for (auto [entity, input] : view.each())
                {
                    if (input.m_on_keydown) input.m_on_keydown(key);
                }
            }
            if (!state.m_is_down && is_new)
            {
                for (auto [entity, input] : view.each())
                {
                    if (input.m_on_keyup) input.m_on_keyup(key);
                }
            }
        });

        // mouse
        input::mouse_position position{};
        position.m_client = inputman.get_mouse_position_client();
        position.m_screen = inputman.get_mouse_position_screen();

        if (inputman.get_mouse_delta().sqr_magnitude() > 0.0f)
        {
            for (auto [entity, input] : view.each())
            {
                if (input.m_on_mouse_move) input.m_on_mouse_move(position);
            }
        }
        
        input::for_each_mousebutton([&view, &position](input::e_mouse_button button)
        {
             const buttonstate& state = inputman.get_mousebutton_state(button);
             bool is_new = state.m_num_frames == 0u;
             if (state.m_is_down && is_new)
             {
                 for (auto [entity, input] : view.each())
                 {
                     if (input.m_on_mouse_down) input.m_on_mouse_down(button, position);
                 }
             }
             if (!state.m_is_down && is_new)
             {
                 for (auto [entity, input] : view.each())
                 {
                     if (input.m_on_mouse_up) input.m_on_mouse_up(button, position);
                 }
             }
        });
    }

    void world::update_bounds_system()
    {
        influx_scope("bounds_system");
        auto view = m_registry.view<transform_component, mesh_component, collider_component>();
        for (auto [entity, transform_comp, mesh_comp, collider_comp] : view.each())
        {
            const math::boxf& transformed_box = mesh_comp.m_mesh_boundbox.get_transformed3D(transform_comp.get_matrix());
            collider_comp.grow(transformed_box);
        }
    }

    void world::update_stream_system()
    {
        influx_scope("stream_system");
        content_manager& contman = get_engine()->get_content();

        // stream in image asset info -> sprite components
        {
            auto view = m_registry.view<sprite_component>();
            for (auto [entity, sprite] : view.each())
            {
                auto asset = contman.find<image_asset>(sprite.get_texture_path());
                if (asset && asset->is_loaded())
                {
                    sprite.m_texture_dimensions = asset->m_resource.m_dimensions;
                }
            }
        }

        // stream in scene asset info -> mesh components
        {
            auto view = m_registry.view<mesh_component>();
            for (auto [entity, mesh_comp] : view.each())
            {
                // deduce the scene name & mesh index from the name
                const string& mesh_name = mesh_comp.get_mesh_name();
                const vector<string>& parts = str::split(mesh_name, "_");
                const string scene_name = parts.size() > 0u ? parts[0u] : "";
                const string index_str = parts.size() > 1u ? parts[1u] : "";
                const uint32 mesh_idx = !index_str.empty() ? std::stoi(index_str) : 0u;

                result<scene_asset const*> asset = contman.find<scene_asset>(scene_name);
                if (asset && asset->is_loaded())
                {
                    // update bounding box / sphere
                    const imp::mesh_data& mesh = asset->m_resource.get_mesh(mesh_idx);
                    mesh_comp.m_mesh_boundbox = mesh.m_bounding_box;
                    mesh_comp.m_mesh_boundsphere = mesh.m_bounding_sphere;
                }
            }
        }
    }

    void world::update_rigidbody_system()
    {
        influx_scope("rigidbody_system");
        const float delta_time = get_engine()->get_time().get_delta_seconds();
        {
            auto view = m_registry.view<transform_component, rigidbody_component>();
            for (auto [entity, transform_comp, body_comp] : view.each())
            {
                const float max_speed = body_comp.get_max_speed();
                const math::float3 old_velocity = body_comp.get_velocity();
                const math::float3 acceleration = body_comp.get_acceleration();
                math::float3 new_velocity = old_velocity + acceleration * delta_time;

                // clamp max speed
                new_velocity.clamp_length(max_speed);

                // perform drag
                const float drag = body_comp.get_drag();
                if (drag > 0.0f)
                {
                    const float inv_drag = 1.0f - drag;
                    if (max_speed > 0.0f)
                    {
                        // dynamic drag, based on max speed use higher drag when slowing down
                        const float speed_sqr = new_velocity.sqr_magnitude();
                        const float speed_fraction = speed_sqr / (max_speed * max_speed);
                        const float damp_factor = (1.0f - speed_fraction) * inv_drag;
                        new_velocity *= damp_factor;
                    }
                    else
                    {
                        new_velocity *= inv_drag;
                    }
                }
                
                // finally, update velocity
                body_comp.set_velocity(new_velocity);

                // move
                const math::float3 old_position = transform_comp.get_position();
                const math::float3 new_position = old_position + new_velocity * delta_time;
                transform_comp.set_position(new_position);
                transform_comp.update_matrix();
            }
        }
    }
}
