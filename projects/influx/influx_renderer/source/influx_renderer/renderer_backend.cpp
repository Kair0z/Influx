#include "renderer_pch.h"
#include "renderer_backend.h"

// graphics includes
#include "influx_graphics.h"
#include "influx_renderer/descriptor_manager.h"

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
            for (size_t i = 0u; i < k_num_inflight_max; ++i)
            {
                mp_allocators.push_back(mp_device->create_graphics_allocator());
            }

            mp_commandlist = mp_device->create_graphics_command_list(mp_allocators[0u]);
        }

        // create fence
        {
            mp_fence = mp_device->create_fence();
        }

        // create descriptor manager
        {
            mp_desc_manager = new descriptor_manager(mp_device);
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
        if (mp_swapchain == nullptr)
        {
            // create swapchain and targets:
            graphics::swapchain_desc desc{};
            desc.m_num_buffers = k_num_backbuffers;
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

        const uint8 current_swapchain_index = mp_swapchain->get_current_backbuffer_index();

        // resize if necessary
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

            mp_commandlist->start(mp_allocators[0u], nullptr);
            {
                target_resource->transition(mp_commandlist, graphics::e_resource_state::render_target);

                mp_commandlist->clear_rtv(target_rtv, { 1, 0, 0, 1 });

                target_resource->transition(mp_commandlist, graphics::e_resource_state::present);
            }
            mp_commandlist->end();
        }

        {
            influx_scope("renderer_backend::draw_scene::submit_commands");
            mp_graphics_queue->submit_commandlists({ mp_commandlist });

            // add signal command to queue
            mp_fence->queue_signal(m_frame_count, mp_graphics_queue);
        }

        {
            influx_scope("renderer_backend::draw_scene::wait_for_commands");

            // wait for command queue to signal our frame value
            wait_handle handle{};
            mp_fence->wait_for_value(m_frame_count, handle);
        }

        ++m_frame_count;
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
        // todo...
    }

    void renderer_backend::load(const string& title, const texture_data& data)
    {
        // todo...
    }

    void renderer_backend::load(const string& title, const material_data& data)
    {
        // todo...
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
#pragma endregion

}
