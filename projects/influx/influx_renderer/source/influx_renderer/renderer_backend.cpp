#include "renderer_pch.h"
#include "renderer_backend.h"

// influx::renderer
#include "influx_renderer/pipeline/pipeline_manager.h"
#include "influx_renderer/descriptor_manager.h"
#include "influx_renderer/upload_manager.h"
#include "influx_renderer/scene_renderer.h"
#include "influx_renderer/debug_renderer.h"
#include "influx_renderer/quad_renderer.h"
#include "influx_renderer/shadertoy/shadertoy_renderer.h"

// influx::rendergraph
#include "rendergraph.h"

// influx::graphics
#include "influx_graphics.h"

namespace influx::renderer
{
    enum class e_frame_phase : uint8
    {
        scene = 0,
        scene2D = 1,
        debug = 2,
        imgui = 3,
        present = 4,
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

        using namespace influx::graphics;
        mp_device = device::create(translate(args.m_api_type));

        queue_desc desc{};
        desc.m_type = e_queue_type::graphics;
        desc.m_priority = graphics::e_queue_priority::normal;
        mp_graphics_queue = mp_device->create_queue(desc);

        mp_commandlist = mp_device->create_graphics_commandlist();
        mp_fence = mp_device->create_fence((uint64)-1);
        mp_copyfence = mp_device->create_fence(0u);

        mp_desc_manager = new descriptor_manager(mp_device);
        mp_pipeline_manager = new pipeline_manager(mp_device);
        mp_upload_manager = new upload_manager(mp_device);
        mp_imgui = new imgui_manager(mp_device);
        mp_scene_renderer = new scene_renderer(this, mp_device, nullptr);
        mp_debug_renderer = new debug_renderer(this, mp_device, nullptr);
        mp_quad_renderer = new quad_renderer();
        mp_shadertoy_renderer = new shadertoy_renderer();

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

    void renderer_backend::start_frame()
    {
        m_rendergraph = new rendergraph::rendergraph(mp_device);

        auto* window_target = get_current_window_target();
        m_rendergraph->import_texture(window_target->get_name().get(), window_target->get_resource());

        mp_commandlist->start(mp_device, nullptr);
        mp_commandlist->set_name("frame");
        get_descriptor_manager()->start_commandlist(mp_commandlist);
    }

    void renderer_backend::end_frame()
    {
        influx_scope("renderer_backend::end_frame");

        m_rendergraph->build();
        m_rendergraph->execute(mp_commandlist);

        // transition backbuffer to present state
        graphics::resource* backbuffer = mp_swapchain->get_current_backbuffer_resource();
        backbuffer->transition(mp_commandlist, graphics::e_resource_state::present);

        mp_commandlist->end();

        {
            influx_scope("renderer_backend::end_frame::submit");
            mp_commandlist->submit(mp_graphics_queue);
        }

        {
            influx_scope("renderer_backend::descriptor_deallocate");
            get_descriptor_manager()->end_frame();
        }
        
        {
            mp_commandlist->wait_for_completion();
        }

        present_swapchain({ .m_vsync = false });

        delete m_rendergraph;
        m_rendergraph = nullptr;

        ++m_frame_count;
    }

    target* renderer_backend::create_target(const target_create_args& args)
    {
        return new target(mp_device, args);
    }

    void renderer_backend::recreate_backbuffer_targets()
    {
        // delete old
        for (target*& target : m_swapchain_targets)
        {
            if (target != nullptr)
            {
                delete target;
                target = nullptr;
            }
        }
        m_swapchain_targets.clear();

        // create new
        const uint8 num_swapchain_buffers = mp_swapchain->get_num_backbuffers();
        for (uint8 i = 0u; i < num_swapchain_buffers; ++i)
        {
            target* new_target = new target(mp_device, mp_swapchain, i);
            new_target->set_name("window_target_" + to_string(i));
            m_swapchain_targets.push_back(new_target);
        }
    }

    target* renderer_backend::get_current_window_target()
    {
        if (mp_swapchain != nullptr)
        {
            const uint8 current_swapchain_index = mp_swapchain->get_current_backbuffer_index();
            return m_swapchain_targets[current_swapchain_index];
        }

        return nullptr;
    }

    target* renderer_backend::get_window_target(const platform::window& window)
    {
        // stall
        wait_gpu_finished();

        // create the swapchain for the first time
        if (mp_swapchain == nullptr)
        {
            graphics::swapchain_desc desc{};
            desc.m_num_buffers = get_num_buffers(k_buffering);
            desc.m_format; //  todo
            desc.m_dimensions; // todo
            mp_swapchain = mp_device->create_swapchain(mp_graphics_queue, window, desc);

            recreate_backbuffer_targets();
        }

        // if need, recreate the swapchain
        if (mp_swapchain->needs_recreate(window))
        {
            mp_swapchain->resize(mp_device, window);

            recreate_backbuffer_targets();
        }

        // acquire the frame
        acquire_swapchain_frame();
        
        return get_current_window_target();
    }

    void renderer_backend::acquire_swapchain_frame()
    {
        influx_scope("renderer_backend::acquire_swapchain_frame");
        mp_swapchain->acquire_backbuffer();
    }

    void renderer_backend::draw_scene(const scene& scene, const target& target)
    {
        static string color_name{}; color_name = target.get_resource()->get_name().get();
        static string depth_name{}; depth_name = color_name + "_depth";
        m_rendergraph->import_texture(color_name, target.get_resource());
        m_rendergraph->import_texture(depth_name, target.get_depth_resource());

        auto* pass = m_rendergraph->add_pass(rendergraph::e_rgpass_type::graphics,
            [&target](rendergraph::rgpass_builder& builder)
            {
                rendergraph::rgaccess access{};
                access.m_load = rendergraph::e_rg_load::clear;
                access.m_store = rendergraph::e_rg_store::preserve;
                builder.write_rendertarget(color_name, access);
                builder.write_depthtarget(depth_name, access);

                builder.set_viewport(target.get_width(), target.get_height());
            },
            [this, &scene, &target](rendergraph::rgpass_context& context)
            {
                influx_scope("renderer_backend::draw_scene::record");
                graphics::commandlist& commandlist = context.get_commandlist();

                mp_scene_renderer->render(&commandlist, scene, target);

                // post processing
                mp_quad_renderer->render_quad(&commandlist, target);
            });

        pass->set_name(RGNAME("render_scene"));
    }

    void renderer_backend::draw_imgui(ImDrawData* draw_data, const target& target)
    {
        influx_scope("renderer_backend::draw_imgui::record");

        auto* pass = m_rendergraph->add_pass(rendergraph::e_rgpass_type::graphics,
            [&target](rendergraph::rgpass_builder& builder)
            {
                rendergraph::rgaccess access{};
                access.m_load = rendergraph::e_rg_load::preserve;
                access.m_store = rendergraph::e_rg_store::preserve;
                builder.write_rendertarget(target.get_resource()->get_name().get(), access);

                builder.set_viewport(target.get_width(), target.get_height());
            },
            [this, draw_data, &target](rendergraph::rgpass_context& context)
            {
                mp_imgui->render(&context.get_commandlist(), draw_data, target);
            });

        pass->set_name(RGNAME("draw_imgui"));
    }

    void renderer_backend::draw_2D(const scene2D& scene, const target& target)
    {
        influx_scope("renderer_backend::draw2D::record");

    }

    void renderer_backend::draw_debug(const scene_debug& scene, const target& target)
    {
        influx_scope("renderer_backend::draw_debug::record");

        m_rendergraph->add_pass(rendergraph::e_rgpass_type::graphics,
            [](rendergraph::rgpass_builder& builder)
            {
                rendergraph::rgaccess access{};
                access.m_load = rendergraph::e_rg_load::preserve;
                access.m_store = rendergraph::e_rg_store::preserve;
                builder.write_rendertarget(RGNAME("scene_target"), access);
                builder.read_depthtarget(RGNAME("scene_depth"), access);
            },
            [this, &scene, &target](rendergraph::rgpass_context& context)
            {
                graphics::commandlist& commandlist = context.get_commandlist();
                mp_debug_renderer->render(&commandlist, scene, target);
            });
    }

    void renderer_backend::draw_shadertoy(const scene_shadertoy& scene, const target& target)
    {
        graphics::resource* target_resource = target.get_resource();
        graphics::descriptor_handle target_rtv = target.get_rtv();
        graphics::descriptor_handle target_dsv = target.get_dsv();

        influx_scope("renderer_backend::draw_shadertoy::record");
        const uint32 target_width = target.get_width();
        const uint32 target_height = target.get_height();

        target_resource->transition(mp_commandlist, graphics::e_resource_state::render_target);

        // bind gpu descriptor heaps
        get_descriptor_manager()->start_commandlist(mp_commandlist);

        mp_commandlist->set(graphics::viewport{ 0.0f, 0.0f, (float)target_width, (float)target_height, 0.0f, 1.0f });
        mp_commandlist->set(graphics::rect{ 0u, 0u, target_width, target_height });
        mp_commandlist->set_rtv(target_rtv, target_dsv);

        mp_shadertoy_renderer->render(mp_commandlist, scene, target);
    }

    void renderer_backend::copy_target(const target& source, const target& dest)
    {
        const bool keep_source = true;
        m_rendergraph->add_copypass(source.get_resource(), dest.get_resource(), keep_source);
    }

    void renderer_backend::clear_target(const target& target, const clear_args& args)
    {
        influx_scope("renderer_backend::clear_target::record");
        m_rendergraph->add_clear_pass(target.get_resource());
    }

    void renderer_backend::present_swapchain(const present_args& args)
    {
        influx_scope("renderer_backend::present");
        if (mp_swapchain)
        {
            graphics::present_args p_args{};
            p_args.m_vsync = args.m_vsync;
            mp_swapchain->present(p_args);
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
    void renderer_backend::load(const string& title, const mesh_data& data, bool reload)
    {
        create_vertexbuffer<vertex_data>(title, data.m_vertices, reload);
        create_indexbuffer(title, data.m_indices, reload);
    }

    // texture
    void renderer_backend::load(const string& title, const texture_data& data, bool reload)
    {
        texture_desc create_args{};
        create_args.m_width = data.get_width();
        create_args.m_heigth = data.get_height();
        texture* texture = create_texture(title, create_args);

        mp_upload_manager->upload_texture(mp_graphics_queue, data, texture->get_resource());
    }

    // shader
    void renderer_backend::load(const string& title, const shader_data& data, bool reload)
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

        if (!target_map->contains(title) || reload)
        {
            (*target_map)[title] = data;
        }
    }

    // material
    void renderer_backend::load(const string& title, const material& data, bool reload)
    {
        if (!m_materials.contains(title) || reload)
        {
            m_materials[title] = data;
        }
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

    void renderer_backend::set_settings(const render_settings& settings)
    {
        m_settings = settings;
    }

    const render_settings& renderer_backend::get_settings() const
    {
        return m_settings;
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

    texture* renderer_backend::find_texture(const string& name)
    {
        if (m_textures.contains(name))
        {
            return m_textures[name];
        }

        return &get_default_texture();
    }

    texture& renderer_backend::get_default_texture()
    {
        static texture* default_texture = nullptr;
        if (!default_texture)
        {
            texture_desc args{};
            args.m_width = 256u;
            args.m_heigth = 256u;
            default_texture = create_texture("none", args);

            texture_data dummy_data{};
            dummy_data.m_width = 256u;
            for (size_t i = 0u; i < 256u * 256u; ++i)
            {
                dummy_data.m_pixels.push_back(make_pixel32(255u, 255u, 255u, 255u));
            }

            mp_upload_manager->upload_texture(mp_graphics_queue, dummy_data,
                default_texture->get_resource());

            influx_assert(default_texture != nullptr);
        }
        
        m_textures["none"] = default_texture;
        return *default_texture;
    }

    const umap<string, material> renderer_backend::get_materials() const
    {
        return m_materials;
    }

    material* renderer_backend::get_material(const string& name)
    {
        if (m_materials.contains(name))
            return &m_materials.at(name);

        return &get_default_material();
    }

    material& renderer_backend::get_default_material()
    {
        static material k_default{};
        k_default.set_basecolour(colour::k_white);
        return k_default;
    }

    void renderer_backend::upload_texture_data(texture* target_tex, const texture_data& data)
    {
        mp_upload_manager->upload_texture(mp_graphics_queue, data, target_tex->get_resource());
    }

    vector<string> renderer_backend::get_mesh_names() const
    {
        vector<string> out_names{};
        out_names.reserve(m_vertex_buffers.size());

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

    pipeline_info renderer_backend::get_pipeline_info() const
    {
        pipeline_info info{};
        info.m_num_pipelines = mp_pipeline_manager->get_num_pipelines();
        return info;
    }

    umap<string, shader_data>& renderer_backend::get_vertex_shaders()
    {
        return m_vertex_shaders;
    }

    umap<string, shader_data>& renderer_backend::get_pixel_shaders()
    {
        return m_pixel_shaders;
    }

    void* renderer_backend::get_imgui_texture_id(const string& title)
    {
        if (has_texture(title))
        {
            return m_textures.at(title);
        }
        else
        {
            return nullptr;
        }
    }

    graphics::resource* renderer_backend::create_indexbuffer(const string& title, const vector<index>& data, bool reload)
    {
        if (!m_index_buffers.contains(title) || reload)
        {
            // create index / vertex buffer on the shared heap (so cpu can write to it)
            graphics::heap_desc heap_desc{};
            heap_desc.m_type = graphics::e_heap_type::shared;

            // set default resource state to read
            graphics::buffer_desc desc{};
            desc.m_init_state = graphics::e_resource_state::gen_read;

            // create index buffer resource
            desc.m_bytesize = data.size() * sizeof(index);
            desc.m_bytestride = sizeof(index);
            desc.m_format = graphics::e_format::u32;
            m_index_buffers[title] = mp_device->create_resource(desc, heap_desc);
            m_index_buffers[title]->map([&data](void* target)
            {
                 memcpy(target, data.data(), data.size() * sizeof(index));
            });

            m_index_buffers[title]->set_name("ib_" + title);
        }

        return m_index_buffers[title];
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

    // creates / switches to the appropriate target representation of our window backbuffer
    target* get_window_target(const platform::window& window)
    {
        return renderer_backend::get_instance().get_window_target(window);
    }

    void start_frame()
    {
        renderer_backend::get_instance().start_frame();
    }

    void draw_scene(const scene& scene, const target& target)
    {
        renderer_backend::get_instance().draw_scene(scene, target);
    }

    void copy_target(const target& source, const target& dest)
    {
        renderer_backend::get_instance().copy_target(source, dest);
    }

    void clear_target(const target& target, const clear_args& args)
    {
        renderer_backend::get_instance().clear_target(target, args);
    }

    void present_swapchain(const present_args& args)
    {
        renderer_backend::get_instance().present_swapchain(args);
    }

    void end_frame()
    {
        renderer_backend::get_instance().end_frame();
    }

    void wait_gpu_finished()
    {
        renderer_backend::get_instance().wait_gpu_finished();
    }

    void draw_imgui(ImDrawData* draw_data, const target& target)
    {
        renderer_backend::get_instance().draw_imgui(draw_data, target);
    }

    void draw_2D(const scene2D& scene, const target& target)
    {
        renderer_backend::get_instance().draw_2D(scene, target);
    }

    void draw_debug(const scene_debug& scene, const target& target)
    {
        renderer_backend::get_instance().draw_debug(scene, target);
    }

    void draw_shadertoy(const scene_shadertoy& scene, const target& target)
    {
        renderer_backend::get_instance().draw_shadertoy(scene, target);
    }

    void load(const string& title, const mesh_data& data, bool reload)
    {
        renderer_backend::get_instance().load(title, data, reload);
    }

    void load(const string& title, const texture_data& data, bool reload)
    {
        renderer_backend::get_instance().load(title, data, reload);
    }

    void load(const string& title, const shader_data& data, bool reload)
    {
        renderer_backend::get_instance().load(title, data, reload);
    }

    void load(const string& title, const material& data, bool reload)
    {
        renderer_backend::get_instance().load(title, data, reload);
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

    void set_settings(const render_settings& settings)
    {
        renderer_backend::get_instance().set_settings(settings);
    }

    render_settings get_settings()
    {
        return renderer_backend::get_instance().get_settings();
    }

    void* get_imgui_texture_id(const string& title)
    {
        return renderer_backend::get_instance().get_imgui_texture_id(title);
    }

    memory_info get_memory_info()
    {
        return renderer_backend::get_instance().get_memory_info();
    }

    pipeline_info get_pipeline_info()
    {
        return renderer_backend::get_instance().get_pipeline_info();
    }
#pragma endregion
}
