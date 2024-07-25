#include "renderer_pch.h"
#include "renderer_backend.h"

// graphics includes
#include "influx_graphics.h"
#include "influx_renderer/descriptor_manager.h"
#include "influx_renderer/upload_manager.h"
#include "influx_graphics/pipeline.h"
#include "influx_graphics/rootsignature.h"

#include "influx_renderer/systems/imgui_system.h"

namespace influx::renderer
{
#pragma region translation
    constexpr static e_render_api translate(graphics::e_api_type type)
    {
        switch (type)
        {
        case graphics::e_api_type::dx12:        return e_render_api::dx12;
        case graphics::e_api_type::vulkan:      return e_render_api::vulkan;
        default:
        case graphics::e_api_type::unsupported: return e_render_api::unsupported;
        }
    }

    constexpr static graphics::e_api_type translate(e_render_api type)
    {
        switch (type)
        {
        case e_render_api::dx12:        return graphics::e_api_type::dx12;
        case e_render_api::vulkan:      return graphics::e_api_type::vulkan;
        default:
        case e_render_api::unsupported: return graphics::e_api_type::unsupported;
        }
    }
#pragma endregion

    void renderer_backend::initialize(const init_args& args)
    {
        influx_scope("renderer_backend::initialize");

        // create the graphics device
        using namespace influx::graphics;

        const e_api_type translated_type = translate(args.m_api_type);
        mp_device = device::create(translated_type);

        // create graphics command queue
        {
            // graphics queue
            command_queue_desc desc{};
            desc.m_type = e_command_queue_type::graphics;
            desc.m_priority = 0;
            mp_graphics_queue = mp_device->create_command_queue(desc);
        }

        // create commandlist & allocators for rendering:
        {
            for (size_t i = 0u; i < get_num_buffers(k_buffering); ++i)
            {
                mp_allocators.push_back(mp_device->create_graphics_allocator());
            }

            mp_commandlist = mp_device->create_graphics_command_list(mp_allocators[0u]);
        }

        // create fence
        {
            mp_fence = mp_device->create_fence((uint64)-1);
            mp_copyfence = mp_device->create_fence(0u);
        }

        // create descriptor manager
        {
            mp_desc_manager = new descriptor_manager(mp_device);
        }

        // create textures
        {
            texture_create_args args{};
            args.m_width = 1024u;
            args.m_heigth = 1024u;
            for (size_t i = 0u; i < 128u; ++i)
            {
                m_textures.push_back(new texture(mp_device, mp_desc_manager->get_srv_heap(), args));
            }
        }

        // create upload manager
        {
            mp_upload_manager = new upload_manager(mp_device);
        }

        create_render_systems();

        m_is_initialized = true;
    }

    bool renderer_backend::is_initialized() const
    {
        return m_is_initialized;
    }

    void renderer_backend::cleanup()
    {
        // delete the device
        delete mp_device;
        mp_device = nullptr;

        m_is_initialized = false;
    }

    target* renderer_backend::create_target(const target_create_args& args)
    {
        return new target(mp_device, mp_desc_manager->get_rtv_heap(), args);
    }

    depth_stencil* renderer_backend::create_depth_stencil(const depth_stencil_create_args& args)
    {
        return new depth_stencil(mp_device, mp_desc_manager->get_dsv_heap(), args);
    }

    target* renderer_backend::get_window_target(const platform::window_handle& window)
    {
        // create the swapchain for the first time
        if (mp_swapchain == nullptr)
        {
            graphics::swapchain_desc desc{};
            desc.m_num_buffers = get_num_buffers(k_buffering);
            desc.m_format; //  todo
            desc.m_dimensions; // todo
            mp_swapchain = mp_device->create_swapchain(mp_graphics_queue, window, desc);

            const uint8 num_swapchain_buffers = mp_swapchain->get_num_backbuffers();
            for (uint8 i = 0u; i < num_swapchain_buffers; ++i)
            {
                m_swapchain_targets.push_back(new target(mp_device, mp_swapchain, i,
                    mp_desc_manager->get_rtv_heap()));

#if _DEBUG
                m_swapchain_targets[i]->set_name("window_target_" + to_string(i));
#endif
            }
        }

        // acquire the frame
        acquire_swapchain_frame();
        const uint8 current_swapchain_index 
            = mp_swapchain->get_current_backbuffer_index();

        // resize the swapchain resources if necessary
        if (mp_swapchain->needs_recreate(window))
        {
            mp_swapchain->resize(window); // resizes the underlying resources
            m_swapchain_targets[current_swapchain_index]->recreate_rtv(); // only recreates rtv
        }
        
        // return the current swapchain target
        return m_swapchain_targets[current_swapchain_index];
    }

    void renderer_backend::acquire_swapchain_frame()
    {
        influx_scope("renderer_backend::acquire_swapchain_frame");
        mp_swapchain->acquire_backbuffer();
    }

    void renderer_backend::draw_scene(const scene& scene, const target& target, const depth_stencil& depth_stencil)
    {
        influx_scope("renderer_backend::draw_scene");
        {
            influx_scope("renderer_backend::draw_scene::record_commands");

            graphics::resource* target_resource = target.get_resource();
            graphics::render_target_view* target_rtv = target.get_rtv();

            graphics::depth_stencil_view* dsv = depth_stencil.get_dsv();

            mp_commandlist->start(mp_allocators[0u], mp_pipeline);
            {
                const uint32 target_width = target.get_width();
                const uint32 target_height = target.get_height();
                mp_commandlist->set(graphics::viewport{ 0.0f, 0.0f, (float)target_width, (float)target_height, 0.0f, 1.0f});
                mp_commandlist->set(graphics::rect{0u, 0u, target_width, target_height});

                target_resource->transition(mp_commandlist, graphics::e_resource_state::render_target);
                
                mp_commandlist->set(target_rtv, dsv);
                mp_commandlist->clear_rtv(target_rtv, { 0.2, 0.2, 0.2, 1 });
                mp_commandlist->clear_dsv(dsv, 1.0f, 0u);
               
                draw_meshes(scene, target);

                target_resource->transition(mp_commandlist, graphics::e_resource_state::present);
            }
            mp_commandlist->end();
        }

        {
            influx_scope("renderer_backend::draw_scene::submit_commands");
            mp_graphics_queue->submit_commandlists({ mp_commandlist });

            // queue a signal to the fence that the gpu work [frame_count] is done
            mp_graphics_queue->queue_signal(mp_fence, m_frame_count);
        }

        {
            influx_scope("renderer_backend::draw_scene::wait_for_commands");

            // wait for the signal
            wait_handle handle{};
            mp_fence->wait_for_value(m_frame_count, handle);
        }

        ++m_frame_count;
    }

    void renderer_backend::draw_meshes(const scene& scene, const target& target)
    {
        // try to create the pipeline
        const bool we_have_a_pipeline = create_pipeline_if_possible();
        if (!we_have_a_pipeline)
        {
            return;
        }

        mp_commandlist->set(graphics::e_primitive_topology::trilist);
        mp_commandlist->set(mp_rootsig);
        mp_commandlist->set(mp_pipeline);

        // set descriptor heaps
        mp_commandlist->set(mp_desc_manager->get_samp_heap());
        mp_commandlist->set(mp_desc_manager->get_srv_heap());

        // setup constants
        const camera& camera = scene.m_camera;
        const math::matrix4x4f mat_view = camera.m_transform.inverted();
        const math::matrix4x4f mat_proj = math::matrix4x4f::make_projection_RH(camera.m_fov, (float)target.get_width() / target.get_height(), camera.m_near_plane, camera.m_far_plane);

        uint32 texture_idx = m_frame_count % 1u;
        mp_commandlist->set(m_textures[0]->get_irv(), 0u);
        
        mp_commandlist->set_constants(2u, 1u, &texture_idx);

        // draw meshes
        for (const auto& vertex_buffer : m_vertex_buffers)
        {
            const string& mesh_name = vertex_buffer.first;

            vector<gpu_instance_data> instances{};
            instances.reserve(scene.m_meshes.size());
            for (const mesh_instance& instance : scene.m_meshes)
            {
                if (instance.m_name == mesh_name)
                {
                    gpu_instance_data instance_data{};
                    instance_data.m_transform = instance.m_transform;
                    instance_data.m_colour = instance.m_per_instance_colour;
                    instances.push_back(instance_data);
                }
            }

            if (instances.size() > 0u)
            {
                struct vs_consants final
                {
                    math::matrix4x4f m_mvp;
                } constants;

                constants.m_mvp = instances[0u].m_transform * mat_view * mat_proj;
                mp_commandlist->set_constants(1u, sizeof(constants) / sizeof(uint32), &constants);

                graphics::resource* vertex_buffer = m_vertex_buffers[mesh_name];
                graphics::resource* index_buffer = m_index_buffers[mesh_name];
                const uint32 num_vertices = (uint32)vertex_buffer->get_bytesize() / (uint32)vertex_buffer->get_bytestride();
                const uint32 num_indices = (uint32)index_buffer->get_bytesize() / (uint32)index_buffer->get_bytestride();

                mp_commandlist->set_indexbuffer(m_index_buffers[mesh_name]);
                mp_commandlist->set_vertexbuffer(vertex_buffer);
                mp_commandlist->draw_indexed(
                {
                    .m_num_indexes_per_instance = num_indices,
                    .m_num_instances = (uint32)instances.size(),
                    .m_start_index = 0u,
                    .m_start_vertex = 0,
                    .m_start_instance = 0u
                });
            }
        }
    }

    bool renderer_backend::create_pipeline_if_possible()
    {
        if (!m_vertex_shaders.empty() 
            && !m_pixel_shaders.empty()
            && mp_pipeline == nullptr)
        {
            {
                graphics::rootsignature_desc desc{};
                mp_rootsig = mp_device->create_rootsignature(desc);
            }

            {
                graphics::pipeline_desc desc{};
                desc.m_vs = m_vertex_shaders.cbegin()->second.m_bytecode;
                desc.m_ps = m_pixel_shaders.cbegin()->second.m_bytecode;
                desc.m_depth_stencil.m_depth_enable = true;
                desc.m_depth_stencil.m_depth_func = graphics::e_comparison_func::less;
                desc.m_depth_stencil.m_stencil_enable = false;
                desc.m_rasterizer.m_cullmode = graphics::e_cull_mode::nocull;
                desc.m_format_dsv = graphics::e_format::d32;
                mp_pipeline = mp_device->create_pipeline(mp_rootsig, desc);
            }
        }

        return mp_pipeline != nullptr;
    }

    void renderer_backend::copy_target(const target& source, const target& dest)
    {
        graphics::resource* source_resource = source.get_resource();
        graphics::resource* dest_resource = dest.get_resource();

        mp_copy_commandlist->start(mp_copy_allocator);

        source_resource->transition(mp_copy_commandlist, graphics::e_resource_state::copy_source);
        dest_resource->transition(mp_copy_commandlist, graphics::e_resource_state::copy_dest);

        mp_copy_commandlist->copy_resource(source_resource, dest_resource);

        source_resource->revert_transition(mp_copy_commandlist);
        dest_resource->revert_transition(mp_copy_commandlist);

        mp_copy_commandlist->end();

        mp_copy_queue->submit_commandlists({ mp_copy_commandlist });
    }

    void renderer_backend::present_swapchain(const present_args& args)
    {
        graphics::present_args p_args{};
        p_args.m_vsync = args.m_vsync;
        mp_swapchain->present(p_args);
    }

    descriptor_manager* renderer_backend::get_descriptor_manager()
    {
        return get_instance().mp_desc_manager;
    }

    upload_manager* renderer_backend::get_upload_manager()
    {
        return get_instance().mp_upload_manager;
    }

    void renderer_backend::create_render_systems()
    {
        create_render_system<imgui_system>(mp_device);
    }

    void renderer_backend::load(const string& title, const mesh_data& data)
    {
        if (!m_vertex_buffers.contains(title))
        {
            // create index / vertex buffer on the shared heap (so cpu can write to it)
            graphics::heap_desc heap_desc{};
            heap_desc.m_type = graphics::e_heap_type::shared;

            // set default resource state to read
            graphics::buffer_desc desc{};
            desc.m_init_state = graphics::e_resource_state::read;

            // create vertex buffer resource
            desc.m_bytesize = data.m_vertices.size() * sizeof(vertex_data);
            desc.m_bytestride = sizeof(vertex_data);
            m_vertex_buffers[title] = mp_device->create_resource(desc, heap_desc);
            m_vertex_buffers[title]->map([&data](void* target)
            {
                memcpy(target, 
                    data.m_vertices.data(), 
                    data.m_vertices.size() * sizeof(vertex_data));
            });

            // create instance buffer resource
            desc.m_bytesize = k_max_instances * sizeof(gpu_instance_data);
            desc.m_bytestride = sizeof(gpu_instance_data);
            m_instance_buffers[title] = mp_device->create_resource(desc, heap_desc);

            // create index buffer resource
            desc.m_bytesize = data.m_indices.size() * sizeof(index);
            desc.m_bytestride = sizeof(index);
            desc.m_format = graphics::e_format::u32;
            m_index_buffers[title] = mp_device->create_resource(desc, heap_desc);
            m_index_buffers[title]->map([&data](void* target)
            {
                 memcpy(target,
                     data.m_indices.data(),
                     data.m_indices.size() * sizeof(index));
            });
        }
    }

    void renderer_backend::load(const string& title, const texture_data& data)
    {
        static uint32 num_textures = 0u;
        mp_upload_manager->upload_texture(
            mp_graphics_queue, 
            data, 
            m_textures[num_textures++]->get_resource());
    }

    void renderer_backend::load(const string& title, const material_data& data)
    {
        // todo...
    }

    void renderer_backend::load(const string& title, const shader_data& data)
    {
        map<string, shader_data>* target_map = nullptr;
        switch (data.m_type)
        {
        case e_shader_type::vs: target_map = &m_vertex_shaders;
            break;
        case e_shader_type::ps: target_map = &m_pixel_shaders;
            break;
        }
        influx_assert_not_null(target_map);

        if (!target_map->contains(title))
        {
            (*target_map)[title] = data;
        }
    }

#pragma region frontend_api
    void initialize(const init_args& args)
    {
        renderer_backend::get_instance().initialize(args);
    }

    bool is_initialized()
    {
        return renderer_backend::get_instance().is_initialized();
    }

    void cleanup()
    {
        renderer_backend::get_instance().cleanup();
    }

    // create a target to render to
    target* create_target(const target_create_args& args)
    {
        return renderer_backend::get_instance().create_target(args);
    }

    depth_stencil* create_depth_stencil(const depth_stencil_create_args& args)
    {
        return renderer_backend::get_instance().create_depth_stencil(args);
    }

    target* get_window_target(const platform::window_handle& window)
    {
        return renderer_backend::get_instance().get_window_target(window);
    }

    void acquire_swapchain_frame()
    {
        renderer_backend::get_instance().acquire_swapchain_frame();
    }

    void draw_scene(const scene& scene, const target& target, const depth_stencil& depth_stencil)
    {
        renderer_backend::get_instance().draw_scene(scene, target, depth_stencil);
    }

    void copy_target(const target& source, const target& dest)
    {
        renderer_backend::get_instance().copy_target(source, dest);
    }

    void present_swapchain(const present_args& args)
    {
        renderer_backend::get_instance().present_swapchain(args);
    }

    void load(const string& title, const mesh_data& data)
    {
        renderer_backend::get_instance().load(title, data);
    }

    void load(const string& title, const texture_data& data)
    {
        renderer_backend::get_instance().load(title, data);
    }

    void load(const string& title, const material_data& data)
    {
        renderer_backend::get_instance().load(title, data);
    }

    void load(const string& title, const shader_data& data)
    {
        renderer_backend::get_instance().load(title, data);
    }
#pragma endregion

}
