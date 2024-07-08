#include "renderer_pch.h"
#include "renderer_backend.h"

// graphics includes
#include "influx_graphics.h"
#include "influx_renderer/descriptor_manager.h"
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
        }

        // create descriptor manager
        {
            mp_desc_manager = new descriptor_manager(mp_device);
        }

        // create textures
        {
            texture_create_args args{};
            args.m_width = 512u;
            args.m_heigth = 512u;
            for (size_t i = 0u; i < 128u; ++i)
            {
                m_textures.push_back(new texture(mp_device, mp_desc_manager->get_input_heap(), args));
            }
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

    void renderer_backend::draw_scene(const scene& scene, const target& target)
    {
        influx_scope("renderer_backend::draw_scene");
        {
            influx_scope("renderer_backend::draw_scene::record_commands");

            graphics::resource* target_resource = target.get_resource();
            graphics::render_target_view* target_rtv = target.get_rtv();

            mp_commandlist->start(mp_allocators[0u], mp_pipeline);
            {
                const uint32 target_width = target.get_width();
                const uint32 target_height = target.get_height();
                mp_commandlist->set(graphics::viewport{ (float)target_width, (float)target_height, 0.0f, 1.0f});
                mp_commandlist->set(graphics::rect{0u, 0u, target_width, target_height});

                target_resource->transition(mp_commandlist, graphics::e_resource_state::render_target);
                
                mp_commandlist->set(target_rtv);
                mp_commandlist->clear_rtv(target_rtv, { 1, 0, 0, 1 });
               
                draw_meshes(scene);

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

    void renderer_backend::draw_meshes(const scene& scene)
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
        mp_commandlist->set(mp_desc_manager->get_input_heap());
        mp_commandlist->set(mp_desc_manager->get_samp_heap());

        for (size_t i = 0u; i < scene.m_meshes.size(); ++i)
        {
            const string& mesh_name = scene.m_meshes[i].m_name;
            influx_assert(m_vertex_buffers.contains(mesh_name));

            graphics::resource* vertex_buffer = m_vertex_buffers[mesh_name];
            const uint32 num_vertices = vertex_buffer->get_bytesize() / vertex_buffer->get_bytestride();

            // mp_commandlist->set_indexbuffer(m_index_buffers[mesh_name]);
            mp_commandlist->set_vertexbuffer(vertex_buffer);
            mp_commandlist->draw_instanced({ num_vertices, 1u, 0u, 0u });
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
        // todo...
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

    target* get_window_target(const platform::window_handle& window)
    {
        return renderer_backend::get_instance().get_window_target(window);
    }

    void acquire_swapchain_frame()
    {
        renderer_backend::get_instance().acquire_swapchain_frame();
    }

    void draw_scene(const scene& scene, const target& target)
    {
        renderer_backend::get_instance().draw_scene(scene, target);
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
