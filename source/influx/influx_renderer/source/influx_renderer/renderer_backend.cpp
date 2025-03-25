#include "renderer_pch.h"
#include "renderer_backend.h"

// influx::renderer
#include "influx_renderer/shader_manager.h"
#include "influx_renderer/pipeline/pipeline_manager.h"
#include "influx_renderer/descriptor_manager.h"
#include "influx_renderer/upload_manager.h"
#include "influx_renderer/scene_renderer.h"
#include "influx_renderer/debug_renderer.h"
#include "influx_renderer/quad_renderer.h"
#include "influx_renderer/shadertoy/shadertoy_renderer.h"
#include "influx_renderer/resources/resource_manager.h"

// influx::rendergraph
#include "rendergraph.h"

// influx::graphics
#include "influx_graphics.h"

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

    void renderer_backend::log(e_log type, const char* message)
    {
        log_function user_func = get_instance().m_init_args.m_log_func;
        if (user_func)
        {
            user_func(type, message);
        }
    }

    void renderer_backend::initialize(const init_args& args)
    {
        influx_scope("renderer_backend::initialize");
        m_init_args = args;

        {
            using namespace influx::graphics;
            mp_device = device::create(translate(args.m_api_type));

            queue_desc desc{};
            desc.m_type = e_queue_type::graphics;
            desc.m_priority = graphics::e_queue_priority::normal;
            mp_graphics_queue = mp_device->create_queue(desc);
            mp_commandlist = mp_device->create_graphics_commandlist();
            mp_fence = mp_device->create_fence((uint64)-1);
            mp_copyfence = mp_device->create_fence(0u);
        }

        mp_desc_manager         = new descriptor_manager(mp_device);
        mp_pipeline_manager     = new pipeline_manager(mp_device);
        mp_upload_manager       = new upload_manager(mp_device);
        m_resource_manager      = new resource_manager();

        mp_imgui                = new imgui_manager(mp_device);
        mp_scene_renderer       = new scene_renderer();
        mp_debug_renderer       = new debug_renderer();
        mp_quad_renderer        = new quad_renderer();
        mp_shadertoy_renderer   = new shadertoy_renderer();
        mp_shader_manager       = new shader_manager();

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

        delete m_resource_manager;

        delete mp_device;
        mp_device = nullptr;

        m_is_initialized = false;
    }

    void renderer_backend::start_frame()
    {
        m_rendergraph = new rendergraph::rendergraph(mp_device);

        // start the commandlist
        mp_commandlist->start(mp_device, nullptr);
        mp_commandlist->set_name("frame");

        // bind gpu heaps
        get_descriptor_manager()->start_commandlist(mp_commandlist);
    }

    void renderer_backend::end_frame()
    {
        {
            influx_scope("renderer::rendergraph_build");
            m_rendergraph->build();
        }
        {
            influx_scope("renderer::rendergraph_execute");
            m_rendergraph->execute(mp_commandlist);
        }

        mp_commandlist->end();

        influx_scope("renderer_backend::end_frame");
        {
            influx_scope("renderer_backend::end_frame::submit");
            mp_commandlist->submit(mp_graphics_queue);
        }

        {
            influx_scope("renderer_backend::end_frame::descriptor_deallocate");
            get_descriptor_manager()->end_frame();
        }
        
        {
            influx_scope("renderer_backend::end_frame::wait_for_gpu");
            mp_commandlist->wait_for_completion();
        }

        delete m_rendergraph;
        m_rendergraph = nullptr;

        ++m_frame_count;
    }

    target* renderer_backend::create_target(const target_create_args& args)
    {
        return new target(mp_device, args);
    }

    void renderer_backend::recreate_backbuffer_targets(swapchain& swapchain)
    {
        // delete old
        for (target*& target : swapchain.m_targets)
        {
            if (target != nullptr)
            {
                delete target;
                target = nullptr;
            }
        }
        swapchain.m_targets.clear();

        // create new
        const uint8 num_swapchain_buffers = swapchain.mp_swapchain->get_num_backbuffers();
        for (uint8 i = 0u; i < num_swapchain_buffers; ++i)
        {
            target* new_target = new target(mp_device, swapchain.mp_swapchain, i);
            new_target->set_name("window_target_" + swapchain.m_windowtitle + "_" + to_string(i));
            swapchain.m_targets.push_back(new_target);
        }
    }

    target* renderer_backend::get_current_window_target(swapchain& swapchain)
    {
        if (swapchain.mp_swapchain != nullptr)
        {
            const uint8 current_swapchain_index = swapchain.mp_swapchain->get_current_backbuffer_index();
            return swapchain.m_targets[current_swapchain_index];
        }

        return nullptr;
    }

    target* renderer_backend::get_window_target(const platform::window& window)
    {
        if (!window.is_valid())
        {
            log(e_log::error, "renderer_backend::get_window_target(window) >> window is not valid!");
            return nullptr;
        }
        
        swapchain& swapchain = m_swapchains[&window];
        swapchain.m_windowtitle = window.get_title() + to_string(reinterpret_cast<uint64>(&window));

        // create the swapchain for the first time
        if (swapchain.mp_swapchain == nullptr)
        {
            graphics::swapchain_desc desc{};
            desc.m_num_buffers = get_num_buffers(k_buffering);
            desc.m_format; //  todo
            desc.m_dimensions = window.get_dimensions(platform::window::e_space::client);
            swapchain.mp_swapchain = mp_device->create_swapchain(mp_graphics_queue, window, desc);
            recreate_backbuffer_targets(swapchain);
        }

        // if need, recreate the swapchain
        if (swapchain.mp_swapchain->needs_recreate(window))
        {
            mp_commandlist->wait_for_completion();
            wait_gpu_finished();

            swapchain.mp_swapchain->resize(mp_device, window);

            recreate_backbuffer_targets(swapchain);
        }

        acquire_swapchain_frame(swapchain);
        
        return get_current_window_target(swapchain);
    }

    void renderer_backend::acquire_swapchain_frame(swapchain& swapchain)
    {
        influx_scope("renderer_backend::acquire_swapchain_frame");
        swapchain.mp_swapchain->acquire_backbuffer();
    }

    result<bool> renderer_backend::draw_scene(const scene& scene, const target& target)
    {
        // scene render
        mp_scene_renderer->render(*m_rendergraph, scene, target);

        // debug render
        {
            static string color_name{}; color_name = target.get_resource()->get_name().get();
            m_rendergraph->import_texture(color_name, target.get_resource());

            auto* pass = m_rendergraph->add_pass(rendergraph::e_rgpass_type::graphics,
                [&target](rendergraph::rgpass_builder& builder)
                {
                    rendergraph::rgaccess access{};
                    access.m_load = rendergraph::e_rg_load::preserve;
                    access.m_store = rendergraph::e_rg_store::preserve;
                    builder.write_rendertarget(color_name, access);

                    builder.set_viewport(target.get_width(), target.get_height());
                },
                [this, &scene, &target](rendergraph::rgpass_context& context)
                {
                    influx_scope("renderer_backend::draw_debug::record");
                    graphics::commandlist& commandlist = context.get_commandlist();

                    mp_debug_renderer->render(&commandlist, scene, target);
                });

            pass->set_name(RGNAME("draw_debug"));
        }

        return true;
    }

    result<bool> renderer_backend::draw_imgui(ImDrawData const* draw_data, const target& target)
    {
        static string color_name{}; color_name = target.get_resource()->get_name().get();
        m_rendergraph->import_texture(color_name, target.get_resource());

#if 0
        const math::colour_rgba colour = math::colour_rgba{ (float)target.get_width() / 1000, 0.0f, 0.0f, 1.0f };
        m_rendergraph->add_clear_pass(target.get_resource(), { .m_colour = colour });
#else
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
                influx_scope("renderer_backend::draw_imgui::record");
                mp_imgui->render(&context.get_commandlist(), *draw_data, target);
            });

        pass->set_name(RGNAME("draw_imgui"));
#endif

        return true;
    }

    result<bool> renderer_backend::draw_imgui(const vector<ImDrawData const*>& draws, const vector<target const*>& targets)
    {
        static string color_name{}; 

        for (uint32 i = 0u; i < draws.size(); ++i)
        {
            const target& target = *targets[i];
            const ImDrawData& draw = *draws[i];

#if 0
            // clear this imgui
            if (i == 1u)
            {
                const math::colour_rgba colour = math::colour_rgba{ (float)target.get_width() / 1000, 0.0f, 0.0f, 1.0f };
                m_rendergraph->add_clear_pass(target.get_resource(), { .m_colour = colour });
                return;
            }
#endif

            // import the texture
            color_name = target.get_resource()->get_name().get();
            m_rendergraph->import_texture(color_name, target.get_resource());

            auto* pass = m_rendergraph->add_pass(rendergraph::e_rgpass_type::graphics,
            [&target](rendergraph::rgpass_builder& builder)
            {
                rendergraph::rgaccess access{};
                access.m_load = rendergraph::e_rg_load::preserve;
                access.m_store = rendergraph::e_rg_store::preserve;
                builder.write_rendertarget(target.get_resource()->get_name().get(), access);
                builder.set_viewport(target.get_width(), target.get_height());
            },
            [this, &draw, &target](rendergraph::rgpass_context& context)
            {
                influx_scope("renderer_backend::draw_imgui::record");
                mp_imgui->render(&context.get_commandlist(), draw, target);
            });

            const string name = string("draw_imgui_") + target.get_name().get();
            const rendergraph::rgname rgname{ name };
            pass->set_name(rgname);
        }

        return true;
    }

    result<bool> renderer_backend::draw_2D(const scene2D& scene, const target& target)
    {
        influx_scope("renderer_backend::draw2D::record");
        return true;
    }

    result<bool> renderer_backend::draw_shadertoy(const scene_shadertoy& scene, const target& target)
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
        return true;
    }

    result<bool> renderer_backend::draw_postprocess(const scene_postprocess& scene, const target& target)
    {
        return true;
    }

    result<bool> renderer_backend::can_draw_postprocess() const
    {
        return true;
    }

    result<bool> renderer_backend::can_draw_imgui() const
    {
        return true;
    }

    result<bool> renderer_backend::can_draw_scene() const
    {
        return true;
    }

    result<bool> renderer_backend::can_draw_2D() const
    {
        return true;
    }

    result<bool> renderer_backend::can_draw_debug() const
    {
        if (mp_debug_renderer == nullptr) return "no debug renderer";
        if (!mp_debug_renderer->can_build_pipeline())
        {
            return "debug renderer : missing shaders";
        }

        return true;
    }

    void renderer_backend::copy_target(const target& source, const target& dest)
    {
        const bool keep_source = true;
        m_rendergraph->add_copypass(source.get_resource(), dest.get_resource(), keep_source);
    }

    void renderer_backend::clear_target(const target& target, const clear_args& args)
    {
        influx_scope("renderer_backend::clear_target::record");
        m_rendergraph->add_clear_pass(target.get_resource(), { args.m_colour });
    }

    void renderer_backend::present_all(const present_args& args)
    {
        mp_commandlist->start(mp_device);

        for (const auto& swapchain : m_swapchains)
        {
            graphics::resource* backbuffer = swapchain.second.mp_swapchain->get_current_backbuffer_resource();
            backbuffer->transition(mp_commandlist, graphics::e_resource_state::present);
        }

        mp_commandlist->end();
        mp_commandlist->submit(mp_graphics_queue);

        for (const auto& swapchain : m_swapchains)
        {
            graphics::present_args p_args{};
            p_args.m_vsync = args.m_vsync;
            swapchain.second.mp_swapchain->present(p_args);
        }
    }

    void renderer_backend::present(const platform::window& window, const present_args& args)
    {
        swapchain& swapchain = m_swapchains.at(&window);

        // run a commandlist to transition the backbuffer-resource to presentable
        mp_commandlist->start(mp_device);
        graphics::resource* backbuffer = swapchain.mp_swapchain->get_current_backbuffer_resource();
        backbuffer->transition(mp_commandlist, graphics::e_resource_state::present);
        mp_commandlist->end();
        mp_commandlist->submit(mp_graphics_queue);

        if (swapchain.mp_swapchain)
        {
            graphics::present_args p_args{};
            p_args.m_vsync = args.m_vsync;
            swapchain.mp_swapchain->present(p_args);
        }
    }

    shader_manager& renderer_backend::get_shader_manager()
    {
        return *get_instance().mp_shader_manager;
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

    resource_manager& renderer_backend::get_resource_manager()
    {
        return *get_instance().m_resource_manager;
    }

    graphics::queue& renderer_backend::get_graphics_queue()
    {
        return *get_instance().mp_graphics_queue;
    }

    graphics::device& renderer_backend::get_device()
    {
        return *get_instance().mp_device;
    }

    // mesh
    void renderer_backend::load(const string& title, const mesh_data<vertex_data>& data, bool reload)
    {
        mesh_data<vertex_data>* new_copy = new mesh_data<vertex_data>(data);
        m_resource_manager->load<e_resource_type::mesh>(title, new_copy, reload);
    }

    // texture
    void renderer_backend::load(const string& title, const texture_data& data, bool reload)
    {
        m_resource_manager->load<e_resource_type::texture>(title, data, reload);
    }

    void renderer_backend::load(const string& title, const cubemap_data& data, bool reload)
    {
        m_resource_manager->load<e_resource_type::cubemap>(title, data, reload);
    }

    // shader
    void renderer_backend::load(const shader::shader_signature& signature, const shader_data& data, bool reload)
    {
        m_resource_manager->load<e_resource_type::shader>(signature, data, reload);

        get_shader_manager().load(signature, data, reload);
    }

    // material
    void renderer_backend::load(const string& title, const material& data, bool reload)
    {
    }

    bool renderer_backend::has_mesh(const string& title) const
    {
        return m_resource_manager->contains<e_resource_type::mesh>(title);
    }

    bool renderer_backend::has_texture(const string& title) const
    {
        return m_resource_manager->contains<e_resource_type::texture>(title);
    }

    bool renderer_backend::has_texturecube(const string& title) const
    {
        return m_resource_manager->contains<e_resource_type::cubemap>(title);
    }

    bool renderer_backend::has_shader(const shader::shader_signature& signature) const
    {
        return get_shader_manager().has_shader(signature);
    }

    bool renderer_backend::has_material(const string& title) const
    {
        return true;
    }

    time::point renderer_backend::get_time_loaded_shader(const shader::shader_signature& signature) const
    {
        return m_resource_manager->get_time_loaded<e_resource_type::shader>(signature);
    }
    time::point renderer_backend::get_time_loaded_texture(const string& title) const
    {
        return m_resource_manager->get_time_loaded<e_resource_type::texture>(title);
    }
    time::point renderer_backend::get_time_loaded_texturecube(const string& title) const
    {
        return m_resource_manager->get_time_loaded<e_resource_type::cubemap>(title);
    }
    time::point renderer_backend::get_time_loaded_mesh(const string& title) const
    {
        return m_resource_manager->get_time_loaded<e_resource_type::mesh>(title);
    }

    void renderer_backend::set_settings(const render_settings& settings)
    {
        m_settings = settings;
    }

    const render_settings& renderer_backend::get_settings() const
    {
        return m_settings;
    }

    texture2D* renderer_backend::find_texture(const string& name)
    {
        return m_resource_manager->get<e_resource_type::texture>(name).m_resource;
    }

    cubemap* renderer_backend::find_texturecube(const string& name)
    {
        return m_resource_manager->get<e_resource_type::cubemap>(name).m_resource;
    }

    texture2D& renderer_backend::get_default_texture()
    {
        return *m_resource_manager->get<e_resource_type::texture>("none").m_resource;
    }

    void renderer_backend::upload_texture_data(texture2D* target_tex, const texture_data& data)
    {
        mp_upload_manager->upload_texture(mp_graphics_queue, data, target_tex->get_resource());
    }

    vector<string> renderer_backend::get_mesh_names() const
    {
        return m_resource_manager->get_signatures<e_resource_type::mesh>();
    }

    bool renderer_backend::get_mesh_buffers(const string& name, graphics::resource*& out_vertex_buffer, graphics::resource*& out_index_buffer)
    {
        const mesh_buffers* buffers = m_resource_manager->get<e_resource_type::mesh>(name).m_resource;
        influx_assert(buffers != nullptr);

        out_vertex_buffer = buffers->m_vertexbuffer;
        out_index_buffer = buffers->m_indexbuffer;
        return true;
    }

    bool renderer_backend::get_mesh_buffers(const mesh_id& id, graphics::resource*& out_vertex_buffer, graphics::resource*& out_index_buffer)
    {
        return false;
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

    void* renderer_backend::get_imgui_texture_id(const string& title)
    {
        if (m_resource_manager->contains<e_resource_type::texture>(title))
        {
            return m_resource_manager->get<e_resource_type::texture>(title).m_resource;
        }
        else return nullptr;
    }

    bool renderer_backend::allow_bindless()
    {
        return INFLUX_RENDER_BINDLESS;
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

    void end_frame()
    {
        renderer_backend::get_instance().end_frame();
    }

    void copy_target(const target& source, const target& dest)
    {
        renderer_backend::get_instance().copy_target(source, dest);
    }

    void clear_target(const target& target, const clear_args& args)
    {
        renderer_backend::get_instance().clear_target(target, args);
    }

    void present_all(const present_args& args)
    {
        renderer_backend::get_instance().present_all(args);
    }

    void present(const platform::window& window, const present_args& args)
    {
        renderer_backend::get_instance().present(window, args);
    }

    void wait_gpu_finished()
    {
        renderer_backend::get_instance().wait_gpu_finished();
    }

    result<bool> draw_scene(const scene& scene, const target& target)
    {
        return renderer_backend::get_instance().draw_scene(scene, target);
    }

    result<bool> draw_imgui(ImDrawData const* draw_data, const target& target)
    {
        return renderer_backend::get_instance().draw_imgui(draw_data, target);
    }

    result<bool> draw_imgui(const vector<ImDrawData const*>& draws, const vector<target const*>& targets)
    {
        return renderer_backend::get_instance().draw_imgui(draws, targets);
    }

    result<bool> draw_2D(const scene2D& scene, const target& target)
    {
        return renderer_backend::get_instance().draw_2D(scene, target);
    }

    result<bool> draw_shadertoy(const scene_shadertoy& scene, const target& target)
    {
        return renderer_backend::get_instance().draw_shadertoy(scene, target);
    }

    result<bool> draw_postprocess(const scene_postprocess& scene, const target& target)
    {
        return renderer_backend::get_instance().draw_postprocess(scene, target);
    }

    result<bool> can_draw_postprocess()
    {
        return renderer_backend::get_instance().can_draw_postprocess();
    }
    result<bool> can_draw_imgui()
    {
        return renderer_backend::get_instance().can_draw_imgui();
    }
    result<bool> can_draw_scene()
    {
        return renderer_backend::get_instance().can_draw_scene();
    }
    result<bool> can_draw_2D()
    {
        return renderer_backend::get_instance().can_draw_2D();
    }
    result<bool> can_draw_debug()
    {
        return renderer_backend::get_instance().can_draw_debug();
    }

    void load(const string& title, const mesh_data<vertex_data>& data, bool reload)
    {
        renderer_backend::get_instance().load(title, data, reload);
    }

    void load(const string& title, const texture_data& data, bool reload)
    {
        renderer_backend::get_instance().load(title, data, reload);
    }

    void load(const string& title, const cubemap_data& data, bool reload)
    {
        renderer_backend::get_instance().load(title, data, reload);
    }

    void load(const shader::shader_signature& signature, const shader_data& data, bool reload)
    {
        renderer_backend::get_instance().load(signature, data, reload);
    }

    void load(const string& title, const material& data, bool reload)
    {
        renderer_backend::get_instance().load(title, data, reload);
    }

    time::point get_time_loaded_shader(const shader::shader_signature& signature)
    {
        return renderer_backend::get_instance().get_time_loaded_shader(signature);
    }
    time::point get_time_loaded_texture(const string& title)
    {
        return renderer_backend::get_instance().get_time_loaded_texture(title);
    }
    time::point get_time_loaded_texturecube(const string& title)
    {
        return renderer_backend::get_instance().get_time_loaded_texturecube(title);
    }
    time::point get_time_loaded_mesh(const string& title)
    {
        return renderer_backend::get_instance().get_time_loaded_mesh(title);
    }

    bool has_mesh(const string& title)
    {
        return renderer_backend::get_instance().has_mesh(title);
    }
    
    bool has_texture(const string& title)
    {
        return renderer_backend::get_instance().has_texture(title);
    }
    
    bool has_texturecube(const string& title)
    {
        return renderer_backend::get_instance().has_texturecube(title);
    }

    bool has_shader(const shader::shader_signature& signature)
    {
        return renderer_backend::get_instance().has_shader(signature);
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
