#include "engine_pch.h"
#include "world/world.h"

// influx::engine
#include "scene/scene.h"
#include "content/content_manager.h"
#include "editor/editor_manager.h"
#include "input/input_manager.h"
#include "editor/editor_manager.h"
#include "rendering/render_manager.h"

// influx::renderer
#include "influx_renderer/scene.h"

namespace influx::engine
{
    class world_ui final : public editor::editor_window
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
        update_stream_system();
        update_rigidbody_system();
    }

    void world::build_renderscene(renderer::scene& scene, renderer::scene2D& scene2D) const
    {
        const float delta_time = get_engine()->get_time().get_delta_seconds();

        // choose camera
        {
            influx_scope("build_camera");
            float priority = 0.0f;
            for (auto [entity, transform_comp, camera_comp] 
                : m_registry.view<const transform_component, camera_component>().each())
            {
                if (camera_comp.get_priority() > priority)
                {
                    renderer::camera render_camera{};
                    math::transform3D transform = transform_comp.get_transform();
                    render_camera.m_camera = camera_comp.get_camera();

                    scene.set_camera(render_camera, transform.get_matrix());

                    priority = camera_comp.get_priority();
                }
            }
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
                scene.add_mesh(mesh_comp.get_mesh_name(), transform.get_matrix());
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
                scene.add_light(light_comp.get_light(), transform.get_matrix());
            }
        }

        // editor render
        const bool is_editor = get_engine()->is_editor();
        if (is_editor)
        {
            influx_scope("build_gizmos");
            scene.add_gizmo_transform(math::transform3D::identity());

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
                        scene.add_line(basepos, endpos, line_colour);
                    }
                }
                for (uint32 x = 0u; x < num_lines; ++x)
                {
                    const math::float3 basepos = math::float3{ -half_offset + (x * line_distance), 0.0f, -half_offset };
                    const math::float3 endpos = basepos + math::float3{ 0, 0, half_offset * 2 };
                    if (x != num_lines / 2)
                    {
                        scene.add_line(basepos, endpos, line_colour);
                    }
                }
                
                const math::float3 origin = math::float3{ 0, 0, 0 };
                scene.add_line(origin, math::float3{ half_offset, 0, 0 }, colour::k_red);
                scene.add_line(origin, math::float3{ 0, half_offset, 0 }, colour::k_green);
                scene.add_line(origin, math::float3{ 0, 0, half_offset }, colour::k_blue);
            }

            // transform gizmos
            for (auto [entity, transform_comp] : m_registry.view<transform_component>().each())
            {
                if (m_registry.try_get<camera_component>(entity))
                {
                    // cameras shouldn't here
                    continue;
                }

                scene.add_gizmo_transform(transform_comp.get_transform());
            }

            // bounds boxes
            for (auto [entity, transform_comp, mesh_comp] : m_registry.view<transform_component, mesh_component>().each())
            {
                const math::boxf transformed_bounds = mesh_comp.m_mesh_boundbox.get_transformed3D(transform_comp.get_matrix());
                scene.add_line_box(transformed_bounds, { 1,0,0,1 });
            }
        }
    }

    void world::build_renderviews() const
    {
        render_manager& renderman = get_engine()->get_renderer();
        for (uint32 i = 0u; i < render_manager::k_num_render_views; ++i)
        {
            render_manager::e_render_view view_enum = static_cast<render_manager::e_render_view>(i);
            render_view& render_view = renderman.get_renderview(view_enum);

            renderer::scene& scene = render_view.get_scene();
            renderer::scene2D& scene2D = render_view.get_scene2D();
            scene = {}; scene2D = {}; // reset the scenes

            const e_view_visibility_flags view_flags = static_cast<e_view_visibility_flags>(1u << i);

            // gather camera
            {
                influx_scope("gather_camera");
                float priority = -1.0f;
                for (auto [entity, transform_comp, camera_comp]
                : m_registry.view<const transform_component, camera_component>().each())
                {
                    if (camera_comp.get_priority() > priority)
                    {
                        renderer::camera render_camera{};
                        math::transform3D transform = transform_comp.get_transform();
                        render_camera.m_camera = camera_comp.get_camera();
                        scene.set_camera(render_camera, transform.get_matrix());
                        priority = camera_comp.get_priority();
                    }
                }
            }

            // gather meshes
            {
                influx_scope("gather_meshes");
                auto view = m_registry.view<const render_component, const transform_component, const mesh_component>();
                for (auto [entity, render_comp, transform_comp, mesh_comp] : view.each())
                {
                    if (render_comp.is_in_view(view_flags) == false) continue;

                    math::transform3D transform = transform_comp.get_transform();

                    // normalize scale to bounding sphere
                    if (mesh_comp.get_use_normalized_scale() && mesh_comp.m_mesh_boundsphere.m_radius > 0.0f)
                    {
                        transform.set_scale(1.0f / mesh_comp.m_mesh_boundsphere.m_radius);
                        transform.update_matrix();
                    }

                    // setup mesh
                    renderer::mesh_instance render_mesh{};
                    scene.add_mesh(mesh_comp.get_mesh_name(), transform.get_matrix());
                }
            }
            
            // gather lights
            {
                influx_scope("gather_lights");
                auto view = m_registry.view<const render_component, const transform_component, const light_component>();
                for (auto [entity, render_comp, transform_comp, light_comp] : view.each())
                {
                    if (render_comp.is_in_view(view_flags) == false) continue;

                    math::transform3D transform = transform_comp.get_transform();
                    transform.update_matrix();
                    scene.add_light(light_comp.get_light(), transform.get_matrix());
                }
            }

            // gather sprites
            {
                influx_scope("gather_sprites");
                auto view = m_registry.view<const render_component, const transform_component, sprite_component>();
                for (auto [entity, render_comp, transform_comp, sprite] : view.each())
                {
                    if (render_comp.is_in_view(view_flags)) continue;

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

            // gather gizmos
            {
                influx_scope("gather_gizmos");
                scene.add_gizmo_transform(math::transform3D::identity());

                // grid render
                {
                    const uint32 num_lines = 30u;
                    const math::colour_rgba line_colour = { 0.2f, 0.2f, 0.2f, 1.0f };
                    const float line_distance = 1.0f;
                    const float line_length = num_lines * line_distance;
                    const float half_offset = line_length * 0.5f;
                    for (uint32 z = 0u; z < num_lines; ++z)
                    {
                        const math::float3 basepos = math::float3{ -half_offset, 0.0f, -half_offset + (z * line_distance) };
                        const math::float3 endpos = basepos + math::float3{ half_offset * 2, 0, 0 };
                        if (z != num_lines / 2)
                        {
                            scene.add_line(basepos, endpos, line_colour);
                        }
                    }
                    for (uint32 x = 0u; x < num_lines; ++x)
                    {
                        const math::float3 basepos = math::float3{ -half_offset + (x * line_distance), 0.0f, -half_offset };
                        const math::float3 endpos = basepos + math::float3{ 0, 0, half_offset * 2 };
                        if (x != num_lines / 2)
                        {
                            scene.add_line(basepos, endpos, line_colour);
                        }
                    }

                    const math::float3 origin = math::float3{ 0, 0, 0 };
                    scene.add_line(origin, math::float3{ half_offset, 0, 0 }, colour::k_red);
                    scene.add_line(origin, math::float3{ 0, half_offset, 0 }, colour::k_green);
                    scene.add_line(origin, math::float3{ 0, 0, half_offset }, colour::k_blue);
                }

                // transform gizmos
                for (auto [entity, transform_comp] : m_registry.view<transform_component>().each())
                {
                    if (m_registry.try_get<camera_component>(entity))
                    {
                        // cameras shouldn't here
                        continue;
                    }

                    scene.add_gizmo_transform(transform_comp.get_transform());
                }

                // bounds boxes
                for (auto [entity, transform_comp, mesh_comp] : m_registry.view<transform_component, mesh_component>().each())
                {
                    const math::boxf transformed_bounds = mesh_comp.m_mesh_boundbox.get_transformed3D(transform_comp.get_matrix());
                    scene.add_line_box(transformed_bounds, { 1,0,0,1 });
                }
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

    world::trace_result world::trace(const math::ray& ray, e_collision_layer layer) const
    {
        trace_result result{};
        result.m_is_hit = false;
        result.m_entity = nullptr;

        struct hit_result final
        {
            entt::entity* m_entity = nullptr;
            float m_hit_distance   = -1.0f;
        };
        vector<hit_result> hit_results{};
        bool hit_any = false;
        
        for (auto [entity, transform_comp, mesh_comp] : m_registry.view<transform_component, mesh_component>().each())
        {
            math::boxf bounds = mesh_comp.m_mesh_boundbox.get_transformed3D(transform_comp.get_matrix());

            float out_distance{};
            if (bounds.trace(ray, out_distance))
            {
                hit_result new_result
                {
                    .m_entity = &entity,
                    .m_hit_distance = out_distance
                };
                hit_results.push_back(new_result);
            }
        }

        if (hit_any)
        {
            if (hit_results.size() > 1u)
            {
                std::sort(hit_results.begin(), hit_results.end(),
                    [](const hit_result& a, const hit_result& b)
                    {
                        return a.m_hit_distance < b.m_hit_distance;
                    });
            }
            result.m_is_hit = true;
            result.m_entity = hit_results[0u].m_entity;
        }

        return result;
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

    result<cptr<camera_component>> world::get_main_scene_camera() const
    {
        using result_type = result<cptr<camera_component>>;

        for (auto [entity, transform_comp, camera_comp]
            : m_registry.view<const transform_component, const camera_component>().each())
        {
            return &camera_comp;
        }

        return result_type::make_error("error: no camera in scene!");
    }

    result<cptr<transform_component>> world::get_main_scene_camera_transform() const
    {
        using result_type = result<cptr<transform_component>>;

        for (auto [entity, transform_comp, camera_comp]
            : m_registry.view<const transform_component, const camera_component>().each())
        {
            return &transform_comp;
        }

        return result_type::make_error("error: no camera in scene!");
    }
    
    result<math::ray> world::make_main_scene_viewray(const math::float2& uv) const
    {
        using result_type = result<math::ray>;

        auto res_cam = get_main_scene_camera();
        auto res_transform = get_main_scene_camera_transform();
        if (res_cam.is_success() && res_transform.is_success())
        {
            const camera_component& camera = *res_cam.get();
            const transform_component& transform = *res_transform.get();
            return make_viewray(transform, camera, uv);
        }
        else
        {
            return result_type::make_error("error: failed fetching main camera from scene!");
        }
    }

    result<world::trace_result> world::trace_main_scene(const math::float2& uv) const
    {
        using result_type = result<world::trace_result>;

        auto res = make_main_scene_viewray(uv);
        if (res.is_success())
        {
            return trace(res.get());
        }
        else 
            return result_type::make_error("error: failed making main scene viewray!");
    }

    math::ray world::make_viewray(const transform_component& transform, const camera_component& camera, const math::vectorf2& uv)
    {
        // get camera matrices
        const math::matrix4x4f projection = camera.get_projection();
        const math::matrix4x4f view = transform.get_matrix().inverted();
        const math::float3 camera_pos = transform.get_position();

        // convert pixel to ndc space
        math::float3 mouse_ndc =
        {
            (2.0f * uv.x) - 1.0f,
            1.0f - (2.0f * uv.y),
            -1.0f
        };

        mouse_ndc.x = -mouse_ndc.x;
        mouse_ndc.y = -mouse_ndc.y;

        // unproject ndc -> view
        const math::float3 raypos_ndc = math::float4(mouse_ndc.x, mouse_ndc.y, mouse_ndc.z);
        math::float3 raypos_view = projection.inverted() * raypos_ndc;
        raypos_view.z = -1.0f;

        // unview view -> world
        math::float4 raypoint_world = view.inverted() * raypos_view;

        // make the ray
        math::ray ray_from_eye{};
        ray_from_eye.m_direction = -(raypoint_world.get_xyz() - camera_pos).normalized();
        ray_from_eye.m_origin = camera_pos;
        ray_from_eye.m_min = 0.0f;
        ray_from_eye.m_max = FLT_MAX;
        return ray_from_eye;
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
            if (state.is_firstframe_down())
            {
                for (auto [entity, input] : view.each())
                {
                    if (input.m_on_ascii_down) input.m_on_ascii_down(ascii);
                }
            }
            if (state.is_firstframe_up())
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
            if (state.is_firstframe_down())
            {
                for (auto [entity, input] : view.each())
                {
                    if (input.m_on_keydown) input.m_on_keydown(key);
                }
            }
            if (state.is_firstframe_up())
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
             if (state.is_firstframe_down())
             {
                 for (auto [entity, input] : view.each())
                 {
                     if (input.m_on_mouse_down) input.m_on_mouse_down(button, position);
                 }
             }
             if (state.is_firstframe_up())
             {
                 for (auto [entity, input] : view.each())
                 {
                     if (input.m_on_mouse_up) input.m_on_mouse_up(button, position);
                 }
             }
        });
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
            auto view = m_registry.view<transform_component, movement_component>();
            for (auto [entity, transform_comp, move_comp] : view.each())
            {
                const float max_speed           = move_comp.get_max_speed();
                const math::float3 old_velocity = move_comp.get_velocity();
                const math::float3 acceleration = move_comp.get_acceleration();
                const float drag                = move_comp.get_drag();

                // calculate new velocity
                math::float3 new_velocity = old_velocity + acceleration * delta_time;

                // clamp max velocity
                new_velocity.clamp_length(max_speed);

                // perform drag
                if (drag > 0.0f)
                {
                    const float one_minus_drag = 1.0f - drag;
                    if (max_speed > 0.0f)
                    {
                        // dynamic drag, based on max speed use higher drag when slowing down
                        const float speed_sqr       = new_velocity.sqr_magnitude();
                        const float speed_fraction  = speed_sqr / (max_speed * max_speed);
                        const float damp_factor     = (1.0f - speed_fraction) * one_minus_drag;
                        new_velocity *= damp_factor;
                    }
                    else
                    {
                        new_velocity *= one_minus_drag;
                    }
                }
                
                // finally set result velocity
                move_comp.set_velocity(new_velocity);

                // do the move
                const math::float3 old_position = transform_comp.get_position();
                const math::float3 new_position = old_position + new_velocity * delta_time;
                transform_comp.set_position(new_position);
                transform_comp.update_matrix();
            }
        }
    }
}
