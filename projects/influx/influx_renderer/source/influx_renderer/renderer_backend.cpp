#include "renderer_pch.h"
#include "renderer_backend.h"

// influx::renderer
#include "influx_renderer/pipeline/pipeline_manager.h"
#include "influx_renderer/descriptor_manager.h"
#include "influx_renderer/upload_manager.h"
#include "influx_renderer/scene_renderer.h"

// influx::graphics
#include "influx_graphics.h"

namespace influx::renderer
{
    enum class e_frame_phase : uint8
    {
        scene = 0,
        scene2D = 1,
        imgui = 2,
        present = 3,
        count
    };

    uint64 get_signal_value(uint64 frame, e_frame_phase phase)
    {
        return (frame * (uint64)e_frame_phase::count) + (uint64)phase;
    }

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
        mp_device = device::create(translate(args.m_api_type));

        // create graphics command queue
        {
            queue_desc desc{};
            desc.m_type = e_queue_type::graphics;
            desc.m_priority = graphics::e_queue_priority::normal;
            mp_graphics_queue = mp_device->create_queue(desc);
        }

        // create commandlist & allocators for rendering:
        {
            mp_commandlist = mp_device->create_graphics_commandlist();
        }

        // create fence
        {
            mp_fence = mp_device->create_fence((uint64)-1);
            mp_copyfence = mp_device->create_fence(0u);
        }

        mp_desc_manager = new descriptor_manager(mp_device);
        mp_pipeline_manager = new pipeline_manager(mp_device);
        mp_upload_manager = new upload_manager(mp_device);
        mp_imgui = new imgui_manager(mp_device);
        mp_scene_renderer = new scene_renderer(this, mp_device, nullptr);

        get_default_texture();
        get_default_material();

        m_is_initialized = true;
    }

    bool renderer_backend::is_initialized() const
    {
        return m_is_initialized;
    }

    void renderer_backend::wait_gpu_finished() const
    {
        logwar("wait_gpu_finished");

        const uint64 finished_value = (uint64)-1;
        mp_fence->queue_signal(finished_value, mp_graphics_queue);
        mp_fence->wait_for_value(finished_value);
    }

    void renderer_backend::cleanup()
    {
        wait_gpu_finished();
        mp_device->cleanup();

        delete mp_device;
        mp_device = nullptr;

        m_is_initialized = false;
    }

    target* renderer_backend::create_target(const target_create_args& args)
    {
        return new target(mp_device, args);
    }

    target* renderer_backend::get_window_target(const platform::window& window)
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
                m_swapchain_targets.push_back(new target(mp_device, mp_swapchain, i));
                m_swapchain_targets[i]->set_name("window_target_" + to_string(i));
            }
        }

        acquire_swapchain_frame();
        const uint8 current_swapchain_index = mp_swapchain->get_current_backbuffer_index();

        if (mp_swapchain->needs_recreate(window))
        {
            wait_gpu_finished();

            mp_swapchain->resize(mp_device, window);

            const uint8 num_swapchain_buffers = mp_swapchain->get_num_backbuffers();
            for (uint8 i = 0u; i < num_swapchain_buffers; ++i)
            {
                delete m_swapchain_targets[i];
                m_swapchain_targets[i] = new target(mp_device, mp_swapchain, i);
                m_swapchain_targets[i]->set_name("window_target_" + to_string(i));
            }
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
            influx_scope("renderer_backend::draw_scene::record");

            graphics::resource* target_resource = target.get_resource();
            graphics::render_target_view* target_rtv = target.get_rtv();
            graphics::depth_stencil_view* target_dsv = target.get_dsv();

            mp_commandlist->start(mp_device, nullptr);
            {
                const uint32 target_width = target.get_width();
                const uint32 target_height = target.get_height();

                mp_commandlist->set(graphics::viewport{ 0.0f, 0.0f, (float)target_width, (float)target_height, 0.0f, 1.0f});
                mp_commandlist->set(graphics::rect{0u, 0u, target_width, target_height});

                target_resource->transition(mp_commandlist, graphics::e_resource_state::render_target);
                
                // clear targets
                mp_commandlist->set(target_rtv, target_dsv);
                mp_commandlist->clear_rtv(target_rtv, { 0.2, 0.2, 0.2, 1 });
                mp_commandlist->clear_dsv(target_dsv, 1.0f, 0u);
               
                // bind gpu descriptor heaps
                get_descriptor_manager()->start_commandlist(mp_commandlist);

                mp_scene_renderer->render(mp_commandlist, scene, target);
            }
            mp_commandlist->end();
        }

        uint64 signal_value = get_signal_value(m_frame_count, e_frame_phase::scene);
        {
            influx_scope("renderer_backend::draw_scene::submit");
            mp_graphics_queue->submit({ mp_commandlist });
            mp_graphics_queue->queue_signal(mp_fence, signal_value);
        }

        {
            influx_scope("renderer_backend::draw_scene::wait");
            mp_fence->wait_for_value(signal_value);
        }
    }

    void renderer_backend::draw_imgui(ImDrawData* draw_data, const target& target)
    {
        influx_scope("renderer_backend::draw_imgui");
        {
            influx_scope("renderer_backend::draw_imgui::record");
            mp_commandlist->start(mp_device, nullptr);

            graphics::render_target_view* target_rtv = target.get_rtv();
            mp_commandlist->set(target_rtv, nullptr);

            get_descriptor_manager()->start_commandlist(mp_commandlist);

            mp_imgui->render(mp_commandlist, draw_data, target);

            mp_commandlist->end();
        }

        uint64 signal_value = get_signal_value(m_frame_count, e_frame_phase::imgui);
        {
            influx_scope("renderer_backend::draw_imgui::submit");
            mp_graphics_queue->submit({ mp_commandlist });

            // queue a signal to the fence that the gpu work [frame_count] is done
            mp_graphics_queue->queue_signal(mp_fence, signal_value);

            // wait for signal
            wait_handle handle{};
            mp_fence->wait_for_value(signal_value, handle);
        }
    }

    void renderer_backend::draw_2D(const scene2D& scene, const target& target)
    {
        influx_scope("renderer_backend::draw_2D");
        {
            influx_scope("renderer_backend::draw2D::record");
            mp_commandlist->start(mp_device, nullptr);

            graphics::render_target_view* target_rtv = target.get_rtv();
            mp_commandlist->set(target_rtv, nullptr);

            get_descriptor_manager()->start_commandlist(mp_commandlist);

            mp_commandlist->end();
        }

        uint64 signal_value = get_signal_value(m_frame_count, e_frame_phase::scene2D);
        {
            influx_scope("renderer_backend::draw2D::submit");
            mp_graphics_queue->submit({ mp_commandlist });

            // queue a signal to the fence that the gpu work [frame_count] is done
            mp_graphics_queue->queue_signal(mp_fence, signal_value);

            // wait for signal
            wait_handle handle{};
            mp_fence->wait_for_value(signal_value, handle);
        }
    }

    void renderer_backend::copy_target(const target& source, const target& dest)
    {
        graphics::resource* source_resource = source.get_resource();
        graphics::resource* dest_resource = dest.get_resource();

        mp_commandlist->start(mp_device);

        source_resource->transition(mp_commandlist, graphics::e_resource_state::copy_source);
        dest_resource->transition(mp_commandlist, graphics::e_resource_state::copy_dest);

        mp_commandlist->copy_resource(source_resource, dest_resource);

        source_resource->revert_transition(mp_commandlist);
        dest_resource->revert_transition(mp_commandlist);

        mp_commandlist->end();
        mp_graphics_queue->submit({ mp_commandlist });

        // signal & wait for gpu to finish copying
        mp_graphics_queue->queue_signal(mp_copyfence, 1u);
        {
            influx_scope("renderer_backend::copy_target::wait");
            mp_copyfence->wait_for_value(1u);
        }
    }

    void renderer_backend::present_swapchain(const present_args& args)
    {
        influx_scope("renderer_backend::present");
        if (mp_swapchain)
        {
            // transition backbuffer into presenting state
            mp_commandlist->start(mp_device);
            graphics::resource* backbuffer = mp_swapchain->get_current_backbuffer_resource();
            backbuffer->transition(mp_commandlist, graphics::e_resource_state::present);
            mp_commandlist->end();

            const uint64 signal_value = get_signal_value(m_frame_count, e_frame_phase::present);
            mp_graphics_queue->submit({ mp_commandlist });
            mp_graphics_queue->queue_signal(mp_fence, signal_value);
            mp_fence->wait_for_value(signal_value);

            get_descriptor_manager()->end_frame();

            graphics::present_args p_args{};
            p_args.m_vsync = args.m_vsync;
            mp_swapchain->present(p_args);

            ++m_frame_count;
        }
    }

    descriptor_manager* renderer_backend::get_descriptor_manager()
    {
        return get_instance().mp_desc_manager;
    }

    upload_manager* renderer_backend::get_upload_manager()
    {
        return get_instance().mp_upload_manager;
    }

    pipeline_manager* renderer_backend::get_pipeline_manager()
    {
        return get_instance().mp_pipeline_manager;
    }


    // mesh
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

            #if _DEBUG
            m_index_buffers[title]->set_name("ib_" + title);
            m_vertex_buffers[title]->set_name("vb_" + title);
            #endif
        }
    }

    // texture
    void renderer_backend::load(const string& title, const texture_data& data)
    {
        texture_desc create_args{};
        create_args.m_width = 1024u;
        create_args.m_heigth = 1024u;
        texture* texture = create_texture(title, create_args);

        mp_upload_manager->upload_texture(mp_graphics_queue, data,
            texture->get_resource());
    }

    // shader
    void renderer_backend::load(const string& title, const shader_data& data)
    {
        umap<string, shader_data>* target_map = nullptr;
        switch (data.m_type)
        {
        case shader::e_shader_type::vs: target_map = &m_vertex_shaders;
            break;
        case shader::e_shader_type::ps: target_map = &m_pixel_shaders;
            break;
        }
        influx_assert_not_null(target_map);

        if (!target_map->contains(title))
        {
            (*target_map)[title] = data;
        }

        // build a pipeline off the first 2 shaders loaded
        // todo: this is ugly!!
        if (mp_pipeline_manager->get_num_pipelines() == 0u
            && !m_vertex_shaders.empty() && !m_pixel_shaders.empty())
        {
            mp_pipeline_manager->new_pipeline("pip_scene", 
                m_vertex_shaders.cbegin()->second, 
                m_pixel_shaders.cbegin()->second);

            // bind the shaders to the default material
            m_materials["none"].m_vertex_shader = m_vertex_shaders.cbegin()->first;
            m_materials["none"].m_pixel_shader = m_pixel_shaders.cbegin()->first;
        }
    }

    // material
    void renderer_backend::load(const string& title, const material& data)
    {
        if (m_materials.contains(title))
            return;

        m_materials[title] = data;
    }

    bool renderer_backend::has_mesh(const string& title) const
    {
        return m_vertex_buffers.contains(title);
    }

    bool renderer_backend::has_texture(const string& title) const
    {
        return m_textures.contains(title);
    }

    bool renderer_backend::has_shader(const string& title) const
    {
        return m_pixel_shaders.contains(title) || m_vertex_shaders.contains(title);
    }

    bool renderer_backend::has_material(const string& title) const
    {
        return m_materials.contains(title);
    }

    texture* renderer_backend::create_texture(const string& title, const texture_desc& args)
    {
        if (!m_textures.contains(title))
        {
            texture* new_texture = new texture(mp_device, args);
#if _DEBUG
            new_texture->set_name(title);
#endif
            m_textures[title] = new_texture;
        }

        return m_textures[title];
    }

    const umap<string, texture*>& renderer_backend::get_textures() const
    {
        return m_textures;
    }

    texture* renderer_backend::get_texture(const string& name)
    {
        if (m_textures.contains(name))
        {
            return m_textures[name];
        }

        return nullptr;
    }

    texture* renderer_backend::get_default_texture()
    {
        if (!m_textures.contains("none"))
        {
            texture_desc args{};
            args.m_width = 256u;
            args.m_heigth = 256u;
            m_textures["none"] = create_texture("none", args);

            texture_data dummy_data{};
            dummy_data.m_width = 256u;
            for (size_t i = 0u; i < 256u * 256u; ++i)
            {
                dummy_data.m_pixels.push_back(make_pixel32(255u, 255u, 255u, 255u));
            }

            mp_upload_manager->upload_texture(mp_graphics_queue, dummy_data, 
                m_textures["none"]->get_resource());
        }
        
        return m_textures["none"];
    }

    const umap<string, material> renderer_backend::get_materials() const
    {
        return m_materials;
    }

    material* renderer_backend::get_material(const string& name)
    {
        if (m_materials.contains(name))
            return &m_materials.at(name);

        return nullptr;
    }

    material* renderer_backend::get_default_material()
    {
        // setup default material
        m_materials["none"].m_basecolor = colour::k_red;
        m_materials["none"].m_tex_albedo = "none";
        m_materials["none"].m_tex_normal = "none";
        m_materials["none"].m_tex_roughness = "none";
        m_materials["none"].m_tex_special = "none";

        return &m_materials["none"];
    }

    void renderer_backend::upload_texture_data(texture* target_tex, const texture_data& data)
    {
        mp_upload_manager->upload_texture(mp_graphics_queue, data, target_tex->get_resource());
    }

    vector<string> renderer_backend::get_mesh_names() const
    {
        vector<string> out_names{};
        for (const auto& vertex_buffer : m_vertex_buffers)
        {
            out_names.push_back(vertex_buffer.first);
        }
        return out_names;
    }

    bool renderer_backend::get_mesh_buffers(const string& name, graphics::resource*& out_vertex_buffer, graphics::resource*& out_index_buffer)
    {
        influx_assert(m_vertex_buffers.contains(name) && m_index_buffers.contains(name));

        out_vertex_buffer = m_vertex_buffers[name];
        out_index_buffer = m_index_buffers[name];

        return true;
    }

    memory_info renderer_backend::get_memory_info() const
    {
        memory_info info{};

        graphics::memory_info graphics_info = mp_device->get_memory_info();
        info.m_gpu_budget = graphics_info.m_gpu_budget;
        info.m_gpu_usage = graphics_info.m_gpu_usage;

        return info;
    }

    void renderer_backend::validate_materials()
    {
        for (const auto& mat : m_materials)
        {
            const string& mat_name = mat.first;
            const material& material = mat.second;

            // check shaders
            const bool valid_pixelshader = m_pixel_shaders.contains(material.m_pixel_shader);
            const bool valid_vertexshader = m_vertex_shaders.contains(material.m_vertex_shader);

            const bool valid_pipeline = true;
            const bool valid_rootsignature = true;

            if (!valid_pixelshader)
            {
                influx_assert(false);
            }

            if (!valid_vertexshader)
            {
                influx_assert(false);
            }

            if (!valid_pipeline)
            {
                influx_assert(false);
            }

            if (!valid_rootsignature)
            {
                influx_assert(false);
            }
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

    target* create_target(const target_create_args& args)
    {
        return renderer_backend::get_instance().create_target(args);
    }

    target* get_window_target(const platform::window& window)
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

    void draw_imgui(ImDrawData* draw_data, const target& target)
    {
        renderer_backend::get_instance().draw_imgui(draw_data, target);
    }

    void draw_2D(const scene2D& scene, const target& target)
    {
        renderer_backend::get_instance().draw_2D(scene, target);
    }

    void load(const string& title, const mesh_data& data)
    {
        renderer_backend::get_instance().load(title, data);
    }

    void load(const string& title, const texture_data& data)
    {
        renderer_backend::get_instance().load(title, data);
    }

    void load(const string& title, const shader_data& data)
    {
        renderer_backend::get_instance().load(title, data);
    }

    void load(const string& title, const material& data)
    {
        renderer_backend::get_instance().load(title, data);
    }

    bool has_mesh(const string& title)
    {
        return renderer_backend::get_instance().has_mesh(title);
    }
    
    bool has_texture(const string& title)
    {
        return renderer_backend::get_instance().has_texture(title);
    }
    
    bool has_shader(const string& title)
    {
        return renderer_backend::get_instance().has_shader(title);
    }
    
    bool has_material(const string& title)
    {
        return renderer_backend::get_instance().has_material(title);
    }

    memory_info get_memory_info()
    {
        return renderer_backend::get_instance().get_memory_info();
    }
#pragma endregion

}
