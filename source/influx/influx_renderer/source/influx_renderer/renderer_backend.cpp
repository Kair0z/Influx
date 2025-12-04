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
#include "influx_renderer/renderer_imgui.h"
#include "influx_renderer/resources/resource_manager.h"
#include "influx_renderer/submitmanager.h"
#include "influx_renderer/renderjobs.h"

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
        }

        // create renderers & managers
        {
            m_job_manager = new job_manager();
            mp_desc_manager = new descriptor_manager(*mp_device);
            mp_pipeline_manager = new pipeline_manager(mp_device);
            mp_upload_manager = new upload_manager(mp_device);
            m_resource_manager = new resource_manager();
            m_submit_manager = new submit_manager(*mp_device);
            mp_imgui = new imgui_manager(mp_device);
            mp_scene_renderer = new scene_renderer();
            mp_quad_renderer = new quad_renderer();
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

    void renderer_backend::wait_until_gpu_idle() const
    {
        m_submit_manager->wait_until_gpu_idle();
    }

    void renderer_backend::cleanup()
    {
        wait_until_gpu_idle();

        m_submit_manager->shutdown(*mp_device);
        delete m_submit_manager; m_submit_manager = nullptr;
        delete m_job_manager; m_job_manager = nullptr;
        delete mp_desc_manager; mp_desc_manager = nullptr;
        delete mp_pipeline_manager; mp_pipeline_manager = nullptr;
        delete mp_upload_manager; mp_upload_manager = nullptr;
        delete mp_imgui; mp_imgui = nullptr;
        delete mp_scene_renderer; mp_scene_renderer = nullptr;
        delete mp_quad_renderer; mp_quad_renderer = nullptr;
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
        job_chain jobs{};

        // todo: one day, we'll be able to only upgrade the graph when the layout of the frame render changes
        // today is not that day...
        job_id reset_job = get_jobs().create_job([this]() {
            m_rendergraph->reset_graph();
        });

        jobs.append(reset_job);

        get_jobs().link_to_endframe(jobs);
    }

    void renderer_backend::end_frame()
    {
        // resolve all renderjobs
        get_jobs().endframe();

        // build the rendergraph
        {
            influx_scope("renderer::build_rendergraph");
            m_rendergraph->build();
        }

        // wait for last gpu frame 
        // (strict, ideally we let CPU get on with some of the work already)
        submit_manager& submanager = get_submit_manager();
        {
            influx_scope("renderer::wait_for_gpu_frame");
            submanager.wait_until_last_gpu_frame_finished();
        }

        // get the render commandlist
        gpu_submission& submission = submanager.get_submission(e_gpusubmit::render);
        graphics::commandlist& cmdlist = submission.get_commandlist();

        // reset & rebind the gpu heaps (of this frame)
        descriptor_manager* descman = get_descriptor_manager();
        descman->reset_gpu_heaps();
        descman->bind_gpu_heaps(cmdlist);

        // let the rendergraph fill up the render commandlist
        {
            influx_scope("renderer::rendergraph_execute");
            auto res = m_rendergraph->execute(cmdlist, *mp_device);
            if (res.is_fail())
                log(e_log::warning, "rendergraph execute failed!");
        }
        {
            influx_scope("submit_manager::submit_gpu_frame");
            submanager.submit_gpu_frame();
        }
    }

    void renderer_backend::present_all(const present_args& args)
    {
        // get (or create) the commandlist for this cpu frame
        submit_manager& subman = get_submit_manager();
        gpu_submission& submission = subman.get_submission(e_gpusubmit::pre_present);
        graphics::commandlist* cmdlist = submission.m_commandlist;
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

        subman.submit_pre_present();
        
        for (const auto& swapchain : m_swapchains)
        {
            graphics::present_args p_args{};
            p_args.m_vsync = args.m_vsync;
            swapchain.second.mp_swapchain->present(p_args);
        }
    }

    void renderer_backend::present(const platform::window& window, const present_args& args)
    {
        // make sure the swapchain has been created before
        influx_assert(m_swapchains.contains(&window));
        
        // get the backbuffer
        swapchain& swapchain = m_swapchains.at(&window);
        auto backbuffer_res = swapchain.mp_swapchain->get_current_backbuffer_resource();
        influx_assert(backbuffer_res.is_success());

        // transition the backbuffer to present state
        submit_manager& subman = get_submit_manager();
        gpu_submission& endframe_submission = subman.get_submission(e_gpusubmit::frame_end);
        graphics::commandlist* cmdlist = endframe_submission.m_commandlist;
        graphics::resource* backbuffer = backbuffer_res.get();
        backbuffer->transition(cmdlist, graphics::e_resource_state::present);

        // call a backbuffer flip
        if (swapchain.mp_swapchain)
        {
            graphics::present_args p_args{};
            p_args.m_vsync = args.m_vsync;
            swapchain.mp_swapchain->present(p_args);
        }
    }

    uint64 renderer_backend::query_gpu_frame()
    {
        return m_submit_manager->query_gpu_frame();
    }

    uint64 renderer_backend::get_cpu_frame() const
    {
        return m_submit_manager->get_cpu_frame();
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
        if (res.is_fail()) return result_type::make_error("failed importing to graph!");

        return new_target;
    }

    result<> renderer_backend::destroy_target(target*& target)
    {
        // remove resources from rendergraph book-keeping
        m_rendergraph->remove_imported_texture(target->get_name());
        if (target->has_depth_stencil())
        {
            m_rendergraph->remove_imported_texture(target->get_name_depth());
        }

        delete target;
        target = nullptr;

        return {};
    }

    result<> renderer_backend::import_to_graph(const target& target)
    {
        // import resources to rendergraph
        m_rendergraph->import_texture(target.get_name(),
            target.get_resource());

        if (target.has_depth_stencil())
        {
            m_rendergraph->import_texture(target.get_name_depth(),
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
            swapchain.mp_swapchain = mp_device->create_swapchain(&get_graphics_queue(), window, desc);
            recreate_backbuffer_finaltarget(swapchain);
        }

        // if need, recreate the swapchain
        if (swapchain.mp_swapchain->needs_recreate(window))
        {
            // wait for the last present to be finished
            submit_manager& subman = get_submit_manager();
            subman.wait_until_complete(subman.get_submission(e_gpusubmit::pre_present));

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
        if (res.is_fail())
            return result_type::make_error("error: failed importing target!");

        mp_scene_renderer->build(*m_rendergraph, scene, target);
        return {};
    }

    result<> renderer_backend::draw_imgui(ImDrawData const* draw_data, const target& target)
    {
        using result_type = result<>;
        import_to_graph(target);

        auto* pass = m_rendergraph->add_pass(rendergraph::e_rgpass_type::graphics,
        [this, &target, draw_data](rendergraph::rgpass_builder& builder)
        {
            mp_imgui->build_rendergraph(builder, target, *draw_data);
        },
        [this, draw_data, &target](rendergraph::rgpass_context& context)
        {
            influx_scope("renderer_backend::draw_imgui::record");
            mp_imgui->render(&context.get_commandlist(), *draw_data, target);
        });
        const string pass_name = string("draw_imgui_") + string(target.get_name());
        pass->set_name(pass_name);

        return {};
    }

    result<> renderer_backend::draw_imgui(const vector<ImDrawData const*>& draws, const vector<target const*>& targets)
    {
        using result_type = result<>;

        // ensure targets are imported
        for (const auto& target : targets)
        {
            if (target == nullptr)
                return result_type::make_error("one of the target resources is nullptr!");

            auto res = import_to_graph(*target);
            if (!res)
                return result_type::make_error("failed importing one of the target resources!");
        }

        // execute draws (each is a pass)
        for (uint32 i = 0u; i < draws.size(); ++i)
        {
            const target& target = *targets[i];
            const ImDrawData& drawdata = *draws[i];

            auto* pass = m_rendergraph->add_pass(rendergraph::e_rgpass_type::graphics,
            [this, &target, &drawdata](rendergraph::rgpass_builder& builder)
            {
                mp_imgui->build_rendergraph(builder, target, drawdata);
            },
            [this, &drawdata, &target](rendergraph::rgpass_context& context)
            {
                mp_imgui->render(&context.get_commandlist(), drawdata, target);
            });

            const string pass_name = string("draw_imgui_") + string(target.get_name());
            pass->set_name(pass_name);
        }

        return {};
    }

    result<> renderer_backend::draw_2D(const scene2D& scene, const target& target)
    {
        influx_scope("renderer_backend::draw2D::record");
        return {};
    }

    result<> renderer_backend::draw_postprocess(const scene_postprocess& scene, const target& target)
    {
        return {};
    }

    result<> renderer_backend::draw_world(const worldview& view, const target& target)
    {
        using result_type = result<>;
        if (view.m_world == nullptr)
            return result_type::make_warning({}, "warning: view.m_world is nullptr");

        auto res = import_to_graph(target);
        if (res.is_fail())
            return result_type::make_error("error: failed importing target to graph!");

        //mp_scene_renderer->build(*m_rendergraph, view, target);
        return {};
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
            builder.read_copysrc_texture(source.get_name()).get();
            builder.write_copydst_texture(dest.get_name()).get();
            builder.set_viewport(dest.get_width(), dest.get_height());
        },
        [&source, &dest](rendergraph::rgpass_context& context)
        {
            graphics::resource* src_resource = context.get_copysrc_texture(source.get_name()).get().m_resource;
            graphics::resource* dst_resource = context.get_copydst_texture(dest.get_name()).get().m_resource;
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
            builder.write_rendertarget(target.get_name(),
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
        return get_instance().m_submit_manager->get_graphics_queue();
    }

    graphics::device& renderer_backend::get_device()
    {
        return *get_instance().mp_device;
    }

    job_manager& renderer_backend::get_jobs()
    {
        return *get_instance().m_job_manager;
    }

    submit_manager& renderer_backend::get_submit_manager()
    {
        return *get_instance().m_submit_manager;
    }

    // mesh
    void renderer_backend::load(const mesh_id& id, const mesh_data<vertex_data>& data, bool reload)
    {
        if (m_resource_manager == nullptr)
            return;

        if (!has_mesh(id) || reload)
        {
            // we store mesh data as detail::base_mesh_data
            mesh_data<vertex_data>* copy = new mesh_data<vertex_data>(data);
            auto& entry = m_resource_manager->load<e_resource_type::mesh>(id, copy, reload);

            // m_rendergraph->import_buffer("vb_" + title, entry.m_resource->m_vertexbuffer);
            // m_rendergraph->import_buffer("ib_" + title, entry.m_resource->m_indexbuffer);
        }
        log(renderer::e_log::info, "loaded mesh");
    }

    // texture
    void renderer_backend::load(const tex_id& id, const texture_data& data, bool reload)
    {
        auto& entry = m_resource_manager->load<e_resource_type::texture>(id, data, reload);

        // keep track in the rendergraph
        // m_rendergraph->import_texture("texture_" + title, entry.m_resource->mp_resource);

        log(renderer::e_log::info, "loaded texture");
    }

    void renderer_backend::load(const cubemap_id& id, const cubemap_data& data, bool reload)
    {
        auto& entry = m_resource_manager->load<e_resource_type::cubemap>(id, data, reload);
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
    void renderer_backend::load(const mat_id& id, const material& data, bool reload)
    {
        // nothing here
    }

    bool renderer_backend::has_mesh(const mesh_id& id) const
    {
        return m_resource_manager->contains<e_resource_type::mesh>(id);
    }

    bool renderer_backend::has_texture(const tex_id& id) const
    {
        return m_resource_manager->contains<e_resource_type::texture>(id);
    }

    bool renderer_backend::has_cubemap(const cubemap_id& id) const
    {
        return m_resource_manager->contains<e_resource_type::cubemap>(id);
    }

    bool renderer_backend::has_shader(const shader::shader_signature& signature) const
    {
        return m_resource_manager->contains<e_resource_type::shader>(signature);
    }

    bool renderer_backend::has_material(const mat_id& id) const
    {
        return true;
    }

    string renderer_backend::get_last_executed_rendergraph_dump()
    {
        if (m_rendergraph)
            return m_rendergraph->make_dump();

        return "";
    }

    string renderer_backend::get_mesh_name(const mesh_id id) const
    {
        return "";
        // return m_resource_manager->get_debugname(id).get_string();
    }

    time::point renderer_backend::get_time_loaded_shader(const shader::shader_signature& signature) const
    {
        return m_resource_manager->get_time_loaded<e_resource_type::shader>(signature);
    }
    time::point renderer_backend::get_time_loaded_texture(const tex_id& id) const
    {
        return m_resource_manager->get_time_loaded<e_resource_type::texture>(id);
    }
    time::point renderer_backend::get_time_loaded_cubemap(const cubemap_id& id) const
    {
        return m_resource_manager->get_time_loaded<e_resource_type::cubemap>(id);
    }
    time::point renderer_backend::get_time_loaded_mesh(const mesh_id& id) const
    {
        return m_resource_manager->get_time_loaded<e_resource_type::mesh>(id);
    }

    void renderer_backend::set_settings(const render_settings& settings)
    {
        m_settings = settings;
    }

    const render_settings& renderer_backend::get_settings() const
    {
        return m_settings;
    }

    texture2D& renderer_backend::get_default_texture()
    {
        const tex_id tex_none = get_internal_texture_id(e_texture::none);
        return *m_resource_manager->get<e_resource_type::texture>(tex_none).m_resource;
    }

    void renderer_backend::upload_texture_data(texture2D* target_tex, const texture_data& data)
    {
        mp_upload_manager->upload_texture(&get_graphics_queue(), data, target_tex->get_resource().get());
    }

    vector<string> renderer_backend::get_mesh_names() const
    {
        vector<string> names{};
        for (const debug_name& name : m_resource_manager->get_all_debugnames<e_resource_type::mesh>())
        {
            names.push_back( name.get_string() );
        }
        return names;
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

    void wait_until_gpu_idle()
    {
        renderer_backend::get_instance().wait_until_gpu_idle();
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

    result<> draw_world(const worldview& view, const target& target)
    {
        return renderer_backend::get_instance().draw_world(view, target);
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

    void load(const mesh_id& id, const mesh_data<vertex_data>& data, bool reload)
    {
        renderer_backend::get_instance().load(id, data, reload);
    }

    void load(const tex_id& id, const texture_data& data, bool reload)
    {
        renderer_backend::get_instance().load(id, data, reload);
    }

    void load(const cubemap_id& id, const cubemap_data& data, bool reload)
    {
        renderer_backend::get_instance().load(id, data, reload);
    }

    void load(const shader::shader_signature& signature, const shader_data& data, bool reload)
    {
        renderer_backend::get_instance().load(signature, data, reload);
    }

    void load(const mat_id& id, const material& data, bool reload)
    {
        renderer_backend::get_instance().load(id, data, reload);
    }

    time::point get_time_loaded_shader(const shader::shader_signature& signature)
    {
        return renderer_backend::get_instance().get_time_loaded_shader(signature);
    }
    time::point get_time_loaded_texture(const tex_id& id)
    {
        return renderer_backend::get_instance().get_time_loaded_texture(id);
    }
    time::point get_time_loaded_cubemap(const cubemap_id& id)
    {
        return renderer_backend::get_instance().get_time_loaded_cubemap(id);
    }
    time::point get_time_loaded_mesh(const mesh_id& id)
    {
        return renderer_backend::get_instance().get_time_loaded_mesh(id);
    }

    bool has_mesh(const mesh_id& id)
    {
        return renderer_backend::get_instance().has_mesh(id);
    }
    
    bool has_texture(const tex_id& id)
    {
        return renderer_backend::get_instance().has_texture(id);
    }
    
    bool has_cubemap(const cubemap_id& id)
    {
        return renderer_backend::get_instance().has_cubemap(id);
    }

    bool has_shader(const shader::shader_signature& signature)
    {
        return renderer_backend::get_instance().has_shader(signature);
    }
    
    bool has_material(const mat_id& id)
    {
        return renderer_backend::get_instance().has_material(id);
    }

    mesh_id get_mesh_id(e_mesh mesh)
    {
        return get_internal_mesh_id(mesh);
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
