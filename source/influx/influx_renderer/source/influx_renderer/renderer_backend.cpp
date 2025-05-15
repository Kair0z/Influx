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

// influx::graphics
#include "influx_graphics.h"

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
            m_shadersource_directory = !args.m_shader_source_folder.empty() && file::is_directory(args.m_shader_source_folder)
                ? args.m_shader_source_folder : k_default_shadersource_directory;
            
            if (!file::is_directory(m_shadersource_directory))
                file::make_directory(m_shadersource_directory);
        }

        // create graphics objects
        {
            using namespace influx::graphics;
            mp_device = device::create(translate(args.m_api_type));

            queue_desc desc{};
            desc.m_type = e_queue_type::graphics;
            desc.m_priority = graphics::e_queue_priority::normal;
            mp_graphics_queue = mp_device->create_queue(desc);
            mp_commandlist = mp_device->create_graphics_commandlist();
            m_gpu_finished_fence = mp_device->create_fence(0u);
        }

        // create renderers & managers
        {
            mp_desc_manager = new descriptor_manager(mp_device);
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
        m_gpu_finished_fence->queue_signal(finished_value, mp_graphics_queue);
        m_gpu_finished_fence->wait_for_value(finished_value);
    }

    void renderer_backend::cleanup()
    {
        wait_gpu_finished();
        mp_device->cleanup();

        delete m_resource_manager;

        delete m_rendergraph;
        m_rendergraph = nullptr;

        delete mp_device;
        mp_device = nullptr;

        m_is_initialized = false;
    }

    void renderer_backend::start_frame()
    {
        m_rendergraph->reset_graph();

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
            m_rendergraph->execute(*mp_commandlist, *mp_device);
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

        ++m_frame_count;
    }

    result<target*> renderer_backend::create_target(const target_create_args& args)
    {
        using result_type = result<target*>;

        target* new_target = new target(mp_device, args);
        m_targets[new_target->m_id] = new_target;

        auto res = import_to_graph(*new_target);
        influx_assert(res.is_success());

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

    void renderer_backend::recreate_backbuffer_targets(swapchain& swapchain)
    {
        // delete old
        for (target*& target : swapchain.m_targets)
        {
            destroy_target(target);
        }
        swapchain.m_targets.clear();

        // create new
        const uint8 num_swapchain_buffers = swapchain.mp_swapchain->get_num_backbuffers();
        for (uint8 i = 0u; i < num_swapchain_buffers; ++i)
        {
            string target_name = "wintar_" + swapchain.m_windowtitle + "_" + to_string(i);
            target* new_target = new target(mp_device, swapchain.mp_swapchain, i);
            new_target->set_name(target_name);

            target_id new_id{ target_name };
            new_target->m_id = new_id;
            m_targets[new_id] = new_target;

            import_to_graph(*new_target);

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

    result<> renderer_backend::draw_scene(const scene& scene, const target& target)
    {
        using result_type = result<>;

        if (scene.is_empty())
        {
            return result_type::make_error("error: cannot draw an empty scene!");
        }

        auto res = import_to_graph(target);
        if (res.is_unex())
        {
            return result_type::make_error("error: failed importing target!");
        }

        mp_scene_renderer->render(*m_rendergraph, scene, target);
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
            builder.write_rendertarget(target.get_resource()->get_name().get(), access);
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

        pass->set_name(RGNAME("draw_imgui"));

        return true;
    }

    result<> renderer_backend::draw_imgui(const vector<ImDrawData const*>& draws, const vector<target const*>& targets)
    {
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

            const string name = string("draw_imgui_") + target.get_name().get();
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
        pass->set_name(RGNAME("copy"));
    }

    void renderer_backend::clear_target(const target& target, const clear_args& args)
    {
        import_to_graph(target);

        auto* pass = m_rendergraph->add_pass(rendergraph::e_rgpass_type::graphics,
        [&target, &args](rendergraph::rgpass_builder& builder)
        {
            rendergraph::rgaccess access{};
            access.m_load = rendergraph::e_rg_load::clear;
            access.m_store = rendergraph::e_rg_store::preserve;
            access.m_load_clear.m_colour = args.m_colour;
            builder.write_rendertarget(target.get_rendergraph_name(), access);
            builder.set_viewport(target.get_width(), target.get_height());
        },
        [](rendergraph::rgpass_context& context) {});

        pass->set_name(RGNAME("clear"));
    }

    void renderer_backend::present_all(const present_args& args)
    {
        mp_commandlist->start(mp_device);

        for (const auto& swapchain : m_swapchains)
        {
            auto res = swapchain.second.mp_swapchain->get_current_backbuffer_resource();
            influx_assert(res.is_success());
            graphics::resource* backbuffer = res.get();
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
        // make sure the swapchain has been created before
        influx_assert(m_swapchains.contains(&window));

        swapchain& swapchain = m_swapchains.at(&window);

        // run a commandlist to transition the backbuffer-resource to presentable
        mp_commandlist->start(mp_device);
        auto res = swapchain.mp_swapchain->get_current_backbuffer_resource();
        influx_assert(res.is_success());
        graphics::resource* backbuffer = res.get();
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
        auto& entry = m_resource_manager->load<e_resource_type::mesh>(title, new_copy, reload);

        // keep track in the rendergraph
        m_rendergraph->import_buffer("vb_" + title, entry.m_resource->m_vertexbuffer);
        m_rendergraph->import_buffer("ib_" + title, entry.m_resource->m_indexbuffer);

        // log(renderer::e_log::info, "loaded mesh");
    }

    // texture
    void renderer_backend::load(const string& title, const texture_data& data, bool reload)
    {
        auto& entry = m_resource_manager->load<e_resource_type::texture>(title, data, reload);

        // keep track in the rendergraph
        m_rendergraph->import_texture("texture_" + title, entry.m_resource->mp_resource);

        log(renderer::e_log::info, "loaded texture");
    }

    void renderer_backend::load(const string& title, const cubemap_data& data, bool reload)
    {
        auto& entry = m_resource_manager->load<e_resource_type::cubemap>(title, data, reload);

        m_rendergraph->import_texture("cubetex_" + title, entry.m_resource->mp_resource);

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
        mp_upload_manager->upload_texture(mp_graphics_queue, data, target_tex->get_resource());
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
            return m_resource_manager->get<e_resource_type::texture>(title).m_resource->get_cpu_handle();
        }
        else return nullptr;
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

    void* get_imgui_texture_id(const string& title)
    {
        return renderer_backend::get_instance().get_imgui_texture_id(title);
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
#pragma endregion
}
