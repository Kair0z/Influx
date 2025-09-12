#include "influx_app.h"
#include "renderer_pch.h"
#include "renderer_backend.h"

// influx::core
#include "core/file.h"

// influx::renderer
#include "influx_renderer/pipeline/pipeline_manager.h"
#include "influx_renderer/descriptor_manager.h"
#include "influx_renderer/upload_manager.h"
#include "influx_renderer/scene_renderer.h"
#include "influx_renderer/quad_renderer.h"
#include "influx_renderer/shadertoy/shadertoy_renderer.h"
#include "influx_renderer/resources/resource_manager.h"

// influx::rendergraph
#include "rendergraph.h"

namespace influx::renderer
{
#pragma region translation
    shader_data shader_data::translate(const shader::compile_output& compile_output)
    {
        shader_data result{};
        result.m_bytecode = compile_output.m_bytecode;
        result.m_reflection = compile_output.m_reflection;
        result.m_type = compile_output.m_signature.m_type;
        return result;
    }

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

    renderer_backend::renderer_backend()
    {
    }

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

        // setup the shader source directory
        {
            static const string k_default_shadersource_directory = "./shaderslol/";
            m_shadersource_directory = !args.m_shader_source_folder.empty() && path::is_directory(args.m_shader_source_folder)
                ? args.m_shader_source_folder : k_default_shadersource_directory;
            
            if (!path::is_directory(m_shadersource_directory))
                path::create_directory(m_shadersource_directory);
        }

        // create graphics objects
        {
            using namespace influx::graphics;
            mp_device = device::create(translate(args.m_api_type));

            queue_desc desc{};
            desc.m_type = e_queue_type::graphics;
            desc.m_priority = graphics::e_queue_priority::normal;
            m_mainqueue = mp_device->create_queue(desc);
            m_gpu_finished_fence = mp_device->create_fence(0u);
            m_frame_fence = mp_device->create_fence(0u);
        }

        // create renderers & managers
        {
            mp_desc_manager = new descriptor_manager(*mp_device);
            mp_pipeline_manager = new pipeline_manager(mp_device);
            mp_upload_manager = new upload_manager(mp_device);
            m_resource_manager = new resource_manager();
            mp_imgui = new imgui_manager(mp_device);
            mp_scene_renderer = new scene_renderer();
            mp_quad_renderer = new quad_renderer();
            mp_shadertoy_renderer = new shadertoy_renderer();
            m_rendergraph = new rendergraph::rendergraph({}, *mp_device);
        }
        
        // load internal shaders
        mp_scene_renderer->load_shaders();

        m_is_initialized = true;
    }

    bool renderer_backend::is_initialized() const
    {
        return m_is_initialized;
    }

    void renderer_backend::wait_gpu_finished() const
    {
        const uint64 finished_value = (uint64)-1;
        m_gpu_finished_fence->queue_signal(finished_value, m_mainqueue);
        m_gpu_finished_fence->wait_for_value(finished_value);
    }

    void renderer_backend::cleanup()
    {
        wait_gpu_finished();

        delete mp_desc_manager; mp_desc_manager = nullptr;
        delete mp_pipeline_manager; mp_pipeline_manager = nullptr;
        delete mp_upload_manager; mp_upload_manager = nullptr;
        delete mp_imgui; mp_imgui = nullptr;
        delete mp_scene_renderer; mp_scene_renderer = nullptr;
        delete mp_quad_renderer; mp_quad_renderer = nullptr;
        delete mp_shadertoy_renderer; mp_shadertoy_renderer = nullptr;
        delete m_resource_manager; m_resource_manager = nullptr;

        delete m_rendergraph;
        m_rendergraph = nullptr;

        mp_device->cleanup();
        delete mp_device;
        mp_device = nullptr;

        m_is_initialized = false;
    }

    void renderer_backend::start_frame()
    {
        // todo: one day, we'll be able to only upgrade the graph when the layout of the frame render changes
        // today is not that day...
        m_rendergraph->reset_graph();
    }

    void renderer_backend::end_frame()
    {
        // build the rendergraph
        {
            influx_scope("renderer::build_rendergraph");
            m_rendergraph->build();
        }

        // wait for gpu frame
        {
            influx_scope("renderer::wait_for_gpu_frame");

            // as long as GPU is k_num_inflight_max behind...
            uint64 gpu_frame = query_gpu_frame();
            while (m_cpu_frame - gpu_frame > k_num_inflight_max)
            {
                // take the commandlist at the current GPU frame, and wait for it to complete
                const auto gpu_commandlist = m_frame_cmdlist.get_gpu();
                if (gpu_commandlist) gpu_commandlist->wait_for_completion();
                gpu_frame = query_gpu_frame();
            }
        }
        
        // get (or create) the commandlist for this cpu frame
        graphics::commandlist*& cmdlist = m_frame_cmdlist.get_cpu();
        if (cmdlist == nullptr) cmdlist = mp_device->create_graphics_commandlist();

        cmdlist->start(mp_device, nullptr);
        cmdlist->set_name("frame");
        
        // reset & rebind the gpu heaps (of this frame)
        descriptor_manager* descman = get_descriptor_manager();
        descman->reset_gpu_heaps();
        descman->bind_gpu_heaps(*cmdlist);

        // execute the rendergraph
        {
            influx_scope("renderer::rendergraph_execute");
            auto res = m_rendergraph->execute(*cmdlist, *mp_device);
            if (res.is_unex())
            {
                log(e_log::warning, "rendergraph execute failed!");
            }
        }
        cmdlist->end();

        {
            influx_scope("renderer_backend::submit");
            cmdlist->submit(m_mainqueue).get();
        }
    }

    void renderer_backend::present_all(const present_args& args)
    {
        // get (or create) the commandlist for this cpu frame
        graphics::commandlist*& cmdlist = m_present_cmdlist.get_cpu();
        if (cmdlist == nullptr) cmdlist = mp_device->create_graphics_commandlist();

        cmdlist->start(mp_device, nullptr);
        cmdlist->set_name("present");
        for (const auto& swapchain : m_swapchains)
        {
            // each swapchain has a final target proxy (uav writable)
            target* finaltarget = swapchain.second.m_finaltarget_proxy;
            graphics::resource* finaltarget_resource = finaltarget->get_resource();

            // copy the finaltarget -> current backbuffer:
            auto res = swapchain.second.mp_swapchain->get_current_backbuffer_resource();
            influx_assert(res.is_success());
            graphics::resource* backbuffer = res.get();

            // transition & copy finaltargetproxy into the backbuffer
            finaltarget_resource->transition(cmdlist, graphics::e_resource_state::copy_src);
            backbuffer->transition(cmdlist, graphics::e_resource_state::copy_dst);
            cmdlist->copy_resource(finaltarget_resource, backbuffer);

            // transition to present the backbuffer
            backbuffer->transition(cmdlist, graphics::e_resource_state::present);
        }
        cmdlist->end();
        cmdlist->submit(m_mainqueue);
        
        for (const auto& swapchain : m_swapchains)
        {
            graphics::present_args p_args{};
            p_args.m_vsync = args.m_vsync;
            swapchain.second.mp_swapchain->present(p_args);
        }

        m_frame_fence->queue_signal(m_cpu_frame + 1u, m_mainqueue);
        ++m_cpu_frame;
    }

    void renderer_backend::present(const platform::window& window, const present_args& args)
    {
        // make sure the swapchain has been created before
        influx_assert(m_swapchains.contains(&window));

        graphics::commandlist* cmdlist = mp_device->create_graphics_commandlist();
        cmdlist->start(mp_device, nullptr);
        cmdlist->set_name("present");

        swapchain& swapchain = m_swapchains.at(&window);

        // run a commandlist to transition the backbuffer-resource to presentable
        auto res = swapchain.mp_swapchain->get_current_backbuffer_resource();
        influx_assert(res.is_success());
        graphics::resource* backbuffer = res.get();
        backbuffer->transition(cmdlist, graphics::e_resource_state::present);
        cmdlist->end();
        cmdlist->submit(m_mainqueue);

        if (swapchain.mp_swapchain)
        {
            graphics::present_args p_args{};
            p_args.m_vsync = args.m_vsync;
            swapchain.mp_swapchain->present(p_args);
        }
    }

    uint64 renderer_backend::query_gpu_frame()
    {
        m_gpu_frame = m_frame_fence->query_value();
        return m_gpu_frame;
    }

    uint64 renderer_backend::get_cpu_frame() const
    {
        return m_cpu_frame;
    }

    result<target*> renderer_backend::create_target(const target_create_args& args)
    {
        using result_type = result<target*>;

        target_create_args create_args_copy = args;
        if (create_args_copy.m_name.is_valid() == false)
        {
            create_args_copy.m_name = to_string(m_targets.size());
        }

        target* new_target = new target(mp_device, create_args_copy);
        m_targets[new_target->get_name()] = new_target;

        auto res = import_to_graph(*new_target);
        if (res.is_unex()) return result_type::make_error("failed importing to graph!");

        return new_target;
    }

    result<> renderer_backend::destroy_target(target*& target)
    {
        // remove resources from rendergraph book-keeping
        m_rendergraph->remove_imported_texture(target->get_rendergraph_name());
        if (target->has_depth_stencil())
        {
            m_rendergraph->remove_imported_texture(target->get_depth_rendergraph_name());
        }

        delete target;
        target = nullptr;

        return {};
    }

    result<> renderer_backend::import_to_graph(const target& target)
    {
        // import resources to rendergraph
        m_rendergraph->import_texture(target.get_rendergraph_name(),
            target.get_resource());

        if (target.has_depth_stencil())
        {
            m_rendergraph->import_texture(target.get_depth_rendergraph_name(),
                target.get_depth_resource());
        }

        return {};
    }

    void renderer_backend::recreate_backbuffer_finaltarget(swapchain& swapchain)
    {
        if (swapchain.m_finaltarget_proxy != nullptr)
            destroy_target(swapchain.m_finaltarget_proxy).get();

        string name = "tex_finaltarget_" + swapchain.m_windowtitle;
        renderer::target_create_args args{};
        args.m_has_colour = true;
        args.m_has_depth_stencil = true;
        args.m_width = swapchain.mp_swapchain->get_dimensions().x;
        args.m_heigth = swapchain.mp_swapchain->get_dimensions().y;
        args.m_name = name;

        target* new_target = new target(mp_device, args);
        new_target->set_name(name);
        // add to backend book keeping
        m_targets[name] = new_target;
        import_to_graph(*new_target);
        swapchain.m_finaltarget_proxy = new_target;
    }

    target* renderer_backend::get_or_create_window_target(const platform::window& window)
    {
        if (!window.is_valid())
        {
            log(e_log::error, "renderer_backend::get_window_target(window) >> window is not valid!");
            return nullptr;
        }
        
        swapchain& swapchain = m_swapchains[&window];
        swapchain.m_windowtitle = window.get_title(); /*  +to_string(reinterpret_cast<uint64>(&window)); */

        // create the swapchain for the first time
        if (swapchain.mp_swapchain == nullptr)
        {
            graphics::swapchain_desc desc{};
            desc.m_num_buffers = get_num_buffers(k_buffering);
            desc.m_format; //  todo
            desc.m_dimensions = window.get_dimensions(platform::window::e_space::client);
            swapchain.mp_swapchain = mp_device->create_swapchain(m_mainqueue, window, desc);
            recreate_backbuffer_finaltarget(swapchain);
        }

        // if need, recreate the swapchain
        if (swapchain.mp_swapchain->needs_recreate(window))
        {
            // WAIT
            while (m_present_cmdlist.is_inflight()) {};

            swapchain.mp_swapchain->resize(mp_device, window);
            recreate_backbuffer_finaltarget(swapchain);
        }
        acquire_swapchain_frame(swapchain);
        return swapchain.m_finaltarget_proxy;
    }

    void renderer_backend::acquire_swapchain_frame(swapchain& swapchain)
    {
        influx_scope("renderer_backend::acquire_swapchain_frame");
        swapchain.mp_swapchain->acquire_backbuffer();
    }

    result<> renderer_backend::draw_scene(const scene& scene, const target& target)
    {
        using result_type = result<>;
        if (scene.is_empty())
            return result_type::make_warning({}, "warning: cannot draw an empty scene!");

        auto res = import_to_graph(target);
        if (res.is_unex())
            return result_type::make_error("error: failed importing target!");

        mp_scene_renderer->build(*m_rendergraph, scene, target);
        return {};
    }

    result<> renderer_backend::draw_imgui(ImDrawData const* draw_data, const target& target)
    {
        import_to_graph(target);

        // ensure dependency textures are imported!
        auto texture_dependencies = imgui_manager::get_texture_dependencies(draw_data);
        for (const auto& texture : texture_dependencies)
        {
            m_rendergraph->import_texture(
                texture->get_rendergraph_id(),
                (graphics::resource*)texture->get_tex_resource());
        }

        auto* pass = m_rendergraph->add_pass(rendergraph::e_rgpass_type::graphics,
        [&target, texture_dependencies](rendergraph::rgpass_builder& builder)
        {
            rendergraph::rgaccess access{};
            access.m_load = rendergraph::e_rg_load::preserve;
            access.m_store = rendergraph::e_rg_store::preserve;
            builder.write_rendertarget(target.get_resource()->get_name(), access);
            builder.set_viewport(target.get_width(), target.get_height());

            for (const auto& texture : texture_dependencies)
            {
                builder.read_texture(texture->get_rendergraph_id());
            }
        },
        [this, draw_data, &target](rendergraph::rgpass_context& context)
        {
            influx_scope("renderer_backend::draw_imgui::record");
            mp_imgui->render(&context.get_commandlist(), *draw_data, target);
        });

        pass->set_name("draw_imgui");

        return true;
    }

    result<> renderer_backend::draw_imgui(const vector<ImDrawData const*>& draws, const vector<target const*>& targets)
    {
        static uint32 frame = 0u;
        if (frame == 3u)
        {
            static int a; a++;
        }
        ++frame;

        // ensure targets are imported
        for (const auto& target : targets)
        {
            import_to_graph(*target);
        }

        // execute draws
        for (uint32 i = 0u; i < draws.size(); ++i)
        {
            const target& target = *targets[i];
            const ImDrawData& draw = *draws[i];

            // ensure dependency textures are imported!
            auto texture_dependencies = imgui_manager::get_texture_dependencies(&draw);
            for (const auto& texture : texture_dependencies)
            {
                m_rendergraph->import_texture(
                    texture->get_rendergraph_id(),
                    (graphics::resource*)texture->get_tex_resource());
            }

            auto* pass = m_rendergraph->add_pass(rendergraph::e_rgpass_type::graphics,
            [&target, texture_dependencies](rendergraph::rgpass_builder& builder)
            {
                // register write
                rendergraph::rgaccess access{};
                access.m_load = rendergraph::e_rg_load::preserve;
                access.m_store = rendergraph::e_rg_store::preserve;
                builder.write_rendertarget(target.get_rendergraph_name(), access);

                // register reads
                for (const auto& texture : texture_dependencies)
                {
                    builder.read_texture(texture->get_rendergraph_id());
                }

                builder.set_viewport(target.get_width(), target.get_height());
            },
            [this, &draw, &target](rendergraph::rgpass_context& context)
            {
                influx_scope("renderer_backend::draw_imgui::record");
                mp_imgui->render(&context.get_commandlist(), draw, target);
            });

            const string name = string("draw_imgui_") + string(target.get_name());
            const rendergraph::rgname rgname{ name };
            pass->set_name(rgname);
        }

        return true;
    }

    result<> renderer_backend::draw_2D(const scene2D& scene, const target& target)
    {
        influx_scope("renderer_backend::draw2D::record");
        return true;
    }

    result<> renderer_backend::draw_postprocess(const scene_postprocess& scene, const target& target)
    {
        return true;
    }

    bool renderer_backend::can_draw_postprocess() const
    {
        return true;
    }

    bool renderer_backend::can_draw_imgui() const
    {
        return true;
    }

    bool renderer_backend::can_draw_scene() const
    {
        return true;
    }

    bool renderer_backend::can_draw_2D() const
    {
        return true;
    }

    bool renderer_backend::can_draw_debug() const
    {
        return true;
    }

    void renderer_backend::copy_target(const target& source, const target& dest)
    {
        import_to_graph(source);
        import_to_graph(dest);

        const bool keep_source = true;
        
        auto* pass = m_rendergraph->add_pass(rendergraph::e_rgpass_type::compute,
        [&source, &dest, keep_source](rendergraph::rgpass_builder& builder)
        {
            builder.read_copysrc_texture(source.get_rendergraph_name()).get();
            builder.write_copydst_texture(dest.get_rendergraph_name()).get();
            builder.set_viewport(dest.get_width(), dest.get_height());
        },
        [&source, &dest](rendergraph::rgpass_context& context)
        {
            graphics::resource* src_resource = context.get_copysrc_texture(source.get_rendergraph_name()).get().m_resource;
            graphics::resource* dst_resource = context.get_copydst_texture(dest.get_rendergraph_name()).get().m_resource;
            context.get_commandlist().copy_resource(src_resource, dst_resource);
        });
        pass->set_name("copy");
    }

    void renderer_backend::clear_target(const target& target, const clear_args& args)
    {
        import_to_graph(target);

        auto* pass = m_rendergraph->add_pass(rendergraph::e_rgpass_type::graphics,
        [&target, &args](rendergraph::rgpass_builder& builder)
        {
            builder.write_rendertarget(target.get_rendergraph_name(), 
                rendergraph::rgaccess::clear_and_keep(args.m_colour));

            builder.set_viewport(target.get_width(), target.get_height());
        },
        [](rendergraph::rgpass_context& context) {});
        pass->set_name(get_target_pass_name("clear", target));
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
        return *get_instance().m_mainqueue;
    }

    graphics::device& renderer_backend::get_device()
    {
        return *get_instance().mp_device;
    }

    // mesh
    void renderer_backend::load(const string& title, const mesh_data<vertex_data>& data, bool reload)
    {
        if (m_resource_manager == nullptr)
            return;

        if (!has_mesh(title) || reload)
        {
            // we store mesh data as detail::base_mesh_data
            mesh_data<vertex_data>* copy = new mesh_data<vertex_data>(data);
            auto& entry = m_resource_manager->load<e_resource_type::mesh>(title, copy, reload);

            // keep track in the rendergraph
            // m_rendergraph->import_buffer("vb_" + title, entry.m_resource->m_vertexbuffer);
            // m_rendergraph->import_buffer("ib_" + title, entry.m_resource->m_indexbuffer);
        }
        // log(renderer::e_log::info, "loaded mesh");
    }

    // texture
    void renderer_backend::load(const string& title, const texture_data& data, bool reload)
    {
        auto& entry = m_resource_manager->load<e_resource_type::texture>(title, data, reload);

        // keep track in the rendergraph
        // m_rendergraph->import_texture("texture_" + title, entry.m_resource->mp_resource);

        log(renderer::e_log::info, "loaded texture");
    }

    void renderer_backend::load(const string& title, const cubemap_data& data, bool reload)
    {
        auto& entry = m_resource_manager->load<e_resource_type::cubemap>(title, data, reload);

        // m_rendergraph->import_texture("cubetex_" + title, entry.m_resource->mp_resource);

        log(renderer::e_log::info, "loaded cubemap");
    }

    // shader
    void renderer_backend::load(const shader::shader_signature& signature, const shader_data& data, bool reload)
    {
        m_resource_manager->load<e_resource_type::shader>(signature, data, reload);

        log(renderer::e_log::info, "loaded shader");
    }

    // material
    void renderer_backend::load(const string& title, const material& data, bool reload)
    {
        // nothing here
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
        return m_resource_manager->contains<e_resource_type::shader>(signature);
    }

    bool renderer_backend::has_material(const string& title) const
    {
        return true;
    }

    string renderer_backend::get_last_executed_rendergraph_dump()
    {
        if (m_rendergraph)
            return m_rendergraph->make_dump();

        return "";
    }

    mesh_id renderer_backend::get_mesh_id(e_mesh mesh) const
    {
        return get_internal_mesh_name(mesh);
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
        mp_upload_manager->upload_texture(m_mainqueue, data, target_tex->get_resource().get());
    }

    vector<string> renderer_backend::get_mesh_names() const
    {
        return m_resource_manager->get_signatures<e_resource_type::mesh>();
    }

    bool renderer_backend::get_mesh_buffers(const mesh_id& id, graphics::resource*& out_vertex_buffer, graphics::resource*& out_index_buffer)
    {
        const mesh_buffers* buffers = m_resource_manager->get<e_resource_type::mesh>(id).m_resource;
        influx_assert(buffers != nullptr);

        out_vertex_buffer = buffers->m_vertexbuffer;
        out_index_buffer = buffers->m_indexbuffer;
        return true;
    }

    memory_info renderer_backend::get_memory_info() const
    {
        memory_info info{};

        graphics::memory_info graphics_info = mp_device->get_memory_info().get();
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

    rendergraph_info renderer_backend::get_rendergraph_info() const
    {
        rendergraph_info info{};
        for (const auto& texture : m_rendergraph->get_textures())
        {
            info.m_textures.push_back({
                .m_name = texture.m_name
                });
        }
        for (const auto& buffer : m_rendergraph->get_buffers())
        {
            info.m_buffers.push_back({
                .m_name = buffer.m_name
                });
        }
        return info;
    }

    bool renderer_backend::allow_bindless()
    {
        return INFLUX_RENDER_BINDLESS;
    }

    string renderer_backend::get_shadersource_directory(e_shadersource_directory _enum) const
    {
        switch (_enum)
        {
        case e_shadersource_directory::base: return m_shadersource_directory;
        case e_shadersource_directory::include: return m_shadersource_directory + "/include/";
        case e_shadersource_directory::source: return m_shadersource_directory + "/source/";
        }

        return m_shadersource_directory;
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
        return renderer_backend::get_instance().create_target(args).get();
    }

    // creates / switches to the appropriate target representation of our window backbuffer
    target* get_or_create_window_target(const platform::window& window)
    {
        return renderer_backend::get_instance().get_or_create_window_target(window);
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

    result<> draw_scene(const scene& scene, const target& target)
    {
        return renderer_backend::get_instance().draw_scene(scene, target);
    }

    result<> draw_imgui(ImDrawData const* draw_data, const target& target)
    {
        return renderer_backend::get_instance().draw_imgui(draw_data, target);
    }

    result<> draw_imgui(const vector<ImDrawData const*>& draws, const vector<target const*>& targets)
    {
        return renderer_backend::get_instance().draw_imgui(draws, targets);
    }

    result<> draw_2D(const scene2D& scene, const target& target)
    {
        return renderer_backend::get_instance().draw_2D(scene, target);
    }

    result<> draw_postprocess(const scene_postprocess& scene, const target& target)
    {
        return renderer_backend::get_instance().draw_postprocess(scene, target);
    }

    bool can_draw_postprocess()
    {
        return renderer_backend::get_instance().can_draw_postprocess();
    }
    bool can_draw_imgui()
    {
        return renderer_backend::get_instance().can_draw_imgui();
    }
    bool can_draw_scene()
    {
        return renderer_backend::get_instance().can_draw_scene();
    }
    bool can_draw_2D()
    {
        return renderer_backend::get_instance().can_draw_2D();
    }
    bool can_draw_debug()
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

    mesh_id get_mesh_id(e_mesh mesh)
    {
        return renderer_backend::get_instance().get_mesh_id(mesh);
    }

    void set_settings(const render_settings& settings)
    {
        renderer_backend::get_instance().set_settings(settings);
    }

    render_settings get_settings()
    {
        return renderer_backend::get_instance().get_settings();
    }

    string get_last_rendergraph_dump()
    {
        return renderer_backend::get_instance().get_last_executed_rendergraph_dump();
    }

    memory_info get_memory_info()
    {
        return renderer_backend::get_instance().get_memory_info();
    }

    pipeline_info get_pipeline_info()
    {
        return renderer_backend::get_instance().get_pipeline_info();
    }
    rendergraph_info get_rendergraph_info()
    {
        return renderer_backend::get_instance().get_rendergraph_info();
    }
#pragma endregion
}
