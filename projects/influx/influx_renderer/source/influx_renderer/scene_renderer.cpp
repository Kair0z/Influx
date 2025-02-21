#include "renderer_pch.h"
#include "scene_renderer.h"

// influx::core
#include "core/math/vector.h"

// influx::renderer
#include "influx_renderer/target.h"
#include "influx_renderer/pipeline/pipeline.h"
#include "influx_renderer/pipeline/pipeline_manager.h"
#include "influx_renderer/renderer_backend.h"
#include "influx_renderer/descriptor_manager.h"
#include "influx_renderer/renderer_common.h"

// influx::graphics
#include "influx_graphics/commandlist.h"
#include "influx_graphics/resource.h"
#include "influx_graphics/device.h"

// influx::rendergraph
#include "rendergraph.h"

namespace influx::renderer
{
    static constexpr uint32 k_num_gbuffers = 3u;
    static constexpr graphics::e_format k_gbuffer_formats[k_num_gbuffers]
    {
        graphics::e_format::rgba_u32,
        graphics::e_format::u32,
        graphics::e_format::u32,
    };

    static graphics_pipeline_signature& get_scene_basepass_pipeline_signature()
    {
        static graphics_pipeline_signature signature{};
        static bool once = true;
        if (once)
        {
            signature.m_is_bindless = true;
            signature.set_shader_id(graphics_pipeline::e_shader_slot::vs, "basepass::main_vs");
            signature.set_shader_id(graphics_pipeline::e_shader_slot::ps, "basepass::main_ps");

            signature.m_primitive_type = graphics::e_primitive_topology_type::triangle;
            signature.m_cullmode = graphics::e_cull_mode::nocull;
            signature.m_fillmode = graphics::e_fill_mode::solid;

            signature.m_forced_samplecount = 0u;
            signature.m_sample_mask = (uint32)-1;
            signature.m_sample_count = 1u;
            signature.m_front_ccw = false;
            signature.m_depthclip = false;
            signature.m_multisample = false;
            signature.m_antialiased_line = false;
            signature.m_conservative_raster = false;
            signature.m_depthbias = 0;
            signature.m_depthbias_clamp = 0.0f;
            signature.m_slope_depthbias = 0.0f;

            signature.m_depth_enable = true;
            signature.m_stencil_enable = false;
            signature.m_depth_comparison = graphics::e_comparison_func::less;
            signature.m_depth_format = graphics::e_format::d32;

            for (uint32 i = 0u; i < 8u; ++i)
            {
                if (i < k_num_gbuffers)
                {
                    signature.m_rtv_actives[i] = true;
                    signature.m_rtv_formats[i] = k_gbuffer_formats[i];
                    signature.m_blend_writemasks[i] = 15u;
                }
                else
                {
                    signature.m_rtv_actives[i] = false;
                }
            }

            once = false;
        }
        
        return signature;
    }

    static compute_pipeline_signature& get_scene_resolve_pipeline_signature()
    {
        static compute_pipeline_signature signature{};
        signature.m_is_bindless = true;
        signature.set_shader_id(compute_pipeline::e_shader_slot::cs, "resolvepass::main_cs");
        return signature;
    };

    scene_renderer::scene_renderer()
    {
        renderer_backend& backend = renderer_backend::get_instance();
        graphics::device& device = backend.get_device();
        static descriptor_manager& descriptor_manager = *backend.get_descriptor_manager();

        m_instance_data = new frontend::per_instance[k_max_num_instances]{};

        // instance buffer
        {
            graphics::heap_desc heap_desc{};
            heap_desc.m_type = graphics::e_heap_type::shared;

            graphics::buffer_desc desc{};
            desc.m_bytesize = k_max_num_instances * sizeof(frontend::per_instance);
            desc.m_bytestride = sizeof(frontend::per_instance);
            desc.m_init_state = graphics::e_resource_state::gen_read;
            mp_instancebuffer = device.create_resource(desc, heap_desc);
            mp_instancebuffer->set_name({ "scene_instance_buffer" });

            // create srv
            m_instance_buffer_srv = descriptor_manager.create_buffer_srv(mp_instancebuffer);
        }

        // lightbuffers
        {
            constexpr static uint64 buffer_strides[k_num_light_types]
            {
                sizeof(frontend::per_pointlight),
                sizeof(frontend::per_spotlight),
                sizeof(frontend::per_dirlight)
            };

            for (uint32 i = 0u; i < k_num_light_types; ++i)
            {
                graphics::heap_desc heap_desc{};
                heap_desc.m_type = graphics::e_heap_type::shared;

                graphics::buffer_desc desc{};
                const influx::scene::e_light_type type = static_cast<influx::scene::e_light_type>(i);
                desc.m_bytestride = buffer_strides[i];
                desc.m_bytesize = k_max_num_lights * desc.m_bytestride;

                mp_lightbuffers[i] = device.create_resource(desc, heap_desc);
                mp_lightbuffers[i]->set_name("lightbuffer_" + to_string(i));

                m_lightbuffer_srvs[i] = descriptor_manager.create_buffer_srv(mp_lightbuffers[i]);
            }
        }

        m_sampler_view = descriptor_manager.create_sampler();
    }

    scene_renderer::~scene_renderer()
    {
        delete[] m_instance_data;
    }

#pragma region batch
    // 1 draw-call == 1 batch
    class batch final
    {
    public:
        string m_mesh_name;
        string m_material_name;
        vector<frontend::per_instance> m_instances;
        graphics::resource* m_vertexbuffer;
        graphics::resource* m_indexbuffer;
        uint64 m_base_instance{};
    };
#pragma endregion

    inline frontend::per_instance translate(const mesh_instance& mesh)
    {
        frontend::per_instance instance_data{};
        instance_data.m_transform = mesh.m_transform;
        instance_data.m_colour = mesh.m_per_instance_colour;
        return instance_data;
    }

    vector<batch> scene_renderer::create_batches(const scene& scene, graphics::commandlist* commandlist)
    {
        renderer_backend& backend = renderer_backend::get_instance();
        umap<texture*, uint32> tex_to_idx{};

        // stage all srv/uav/const descriptors on the bindless heap
        // in bindless, MUST happen before setting descriptor (mp_pipeline->set_state)
        {
            vector<graphics::descriptor_handle> all_srvs{};
            all_srvs.push_back(m_instance_buffer_srv);

            uint32 texture_count = 0u;
            for (const mesh_instance& instance : scene.m_meshes)
            {
                string diffuse_name = "";
                if (instance.m_material != nullptr)
                {
                    const material& material = *instance.m_material;
                    diffuse_name = material.get_texture_diffuse_name();
                }
                
                texture* diffuse_texture = backend.find_texture(diffuse_name);
                if (diffuse_texture != nullptr && !tex_to_idx.contains(diffuse_texture))
                {
                    // add to list of unique srvs
                    all_srvs.push_back(diffuse_texture->get_cpu_handle());

                    // keep the idx of the srv
                    tex_to_idx[diffuse_texture] = texture_count;
                    texture_count++;
                }
            }

            graphics::descriptor_range gpu_range = backend.get_descriptor_manager()->stage(all_srvs);
            graphics::descriptor_range gpu_range_samp = backend.get_descriptor_manager()->stage_sampler(m_sampler_view);
            // mp_pipeline->set_resource_table(commandlist, "all_descriptors", gpu_range);
            backend.get_descriptor_manager()->start_commandlist(commandlist);
        }

        using mesh_to_instance_map = umap<string, vector<frontend::per_instance>>;
        mesh_to_instance_map meshname_to_instances{};

        for (const mesh_instance& instance : scene.m_meshes)
        {
            graphics::resource* indexbuffer = nullptr;
            graphics::resource* vertexbuffer = nullptr;

            frontend::per_instance gpu_data = translate(instance);

            string diffuse_name = "";
            if (instance.m_material != nullptr)
            {
                const material& material = *instance.m_material;
                diffuse_name = material.get_texture_diffuse_name();
            }

            gpu_data.set_albedo_index( tex_to_idx[backend.find_texture(diffuse_name)] );
            meshname_to_instances[instance.m_name].push_back(gpu_data);
        }

        vector<batch> batches{};
        uint64 offset = 0u;
        for (const auto& pair : meshname_to_instances)
        {
            graphics::resource* vertexbuffer = nullptr;
            graphics::resource* indexbuffer = nullptr;
            if (backend.get_mesh_buffers(pair.first, vertexbuffer, indexbuffer))
            {
                batches.push_back({});
                batch& batch = batches.back();

                batch.m_indexbuffer = indexbuffer;
                batch.m_vertexbuffer = vertexbuffer;
                batch.m_instances = pair.second;
                batch.m_base_instance = offset;

                offset += batch.m_instances.size();
            }
        }
        return batches;
    }

    void scene_renderer::update_instance_buffer(const vector<batch>& batches)
    {
        mp_instancebuffer->map([&batches](void* dest)
        {
            frontend::per_instance* data = reinterpret_cast<frontend::per_instance*>(dest);
            for (const batch& batch : batches)
            {
                for (size_t i = 0u; i < batch.m_instances.size(); ++i)
                {
                    data[batch.m_base_instance + i] = batch.m_instances[i];
                }
            }
        });
    }

    void scene_renderer::update_lightbuffers(const scene& scene)
    {
        // map lightbuffers
        const uint32 num_lights = scene.get_num_lights();
        for (uint32 i = 0u; i < k_num_light_types; ++i)
        {
            const auto current_type = static_cast<influx::scene::e_light_type>(i);

            mp_lightbuffers[i]->map([num_lights, current_type, i, &scene](void* dest)
            {
                uint32 index = 0u;
                for (uint32 l = 0; l < num_lights; ++l)
                {
                    if (scene.m_lights[l].m_light.get_type() != current_type) continue;
                    
                    switch (current_type)
                    {
                    case influx::scene::e_light_type::directional:
                    {
                        frontend::per_dirlight* data = reinterpret_cast<frontend::per_dirlight*>(dest);
                        data[index].m_colour = scene.m_lights[l].m_light.get_colour();
                        break;
                    }

                    case influx::scene::e_light_type::point:
                    {
                        frontend::per_pointlight* data = reinterpret_cast<frontend::per_pointlight*>(dest);
                        data[index].m_attenuation = scene.m_lights[l].m_light.get_attenuation();
                        data[index].m_colour      = scene.m_lights[l].m_light.get_colour();
                        data[index].m_position    = scene.m_lights[l].m_world_position;
                        break;
                    }

                    case influx::scene::e_light_type::spot:
                    {
                        frontend::per_spotlight* data = reinterpret_cast<frontend::per_spotlight*>(dest);
                        data[index].m_position = scene.m_lights[l].m_world_position;
                        break;
                    }
                    }

                    ++index;
                }
            });
        }
    }

    void scene_renderer::apply_pipeline_settings(const target& target)
    {
        const render_settings& settings = renderer_backend::get_instance().get_settings();
        graphics_pipeline_signature& signature = get_scene_basepass_pipeline_signature();
        
        signature.m_depth_enable = target.has_depth_stencil();
        signature.m_fillmode = settings.m_wireframe ? graphics::e_fill_mode::wireframe : graphics::e_fill_mode::solid;
        switch (settings.m_cullmode)
        {
        case render_settings::cullmode::back:  signature.m_cullmode = graphics::e_cull_mode::back; break;
        case render_settings::cullmode::front: signature.m_cullmode = graphics::e_cull_mode::front; break;
        case render_settings::cullmode::none:  signature.m_cullmode = graphics::e_cull_mode::nocull; break;
        }
    }

    void scene_renderer::build_basepass(rendergraph::rgpass_builder& builder, const target& target)
    {
        static string color_name{}; color_name = target.get_resource()->get_name().get();
        static string depth_name{}; depth_name = color_name + "_depth";

        // declare gbuffer rendertargets
        rendergraph::texture_desc gbuffer_desc{};
        gbuffer_desc.m_width = target.get_width();
        gbuffer_desc.m_heigth = target.get_height();
        gbuffer_desc.m_format = graphics::e_format::rgba_u32;
        builder.declare_texture(RGNAME("gbuffer_a"), gbuffer_desc);
        gbuffer_desc.m_format = graphics::e_format::u32;
        builder.declare_texture(RGNAME("gbuffer_b"), gbuffer_desc);
        builder.declare_texture(RGNAME("gbuffer_c"), gbuffer_desc);

        rendergraph::rgaccess access{};
        access.m_load = rendergraph::e_rg_load::clear;
        access.m_store = rendergraph::e_rg_store::preserve;
        builder.write_rendertarget(RGNAME("gbuffer_a"), access);
        builder.write_rendertarget(RGNAME("gbuffer_b"), access);
        builder.write_rendertarget(RGNAME("gbuffer_c"), access);

        if (target.has_depth_stencil())
        {
            access.m_load = rendergraph::e_rg_load::clear;
            builder.write_depthtarget(depth_name, access);
        }
        builder.set_viewport(target.get_width(), target.get_height());
    }

    void scene_renderer::execute_basepass(rendergraph::rgpass_context& context, const target& target, const scene& scene)
    {
        renderer_backend& backend = renderer_backend::get_instance();
        pipeline_manager& pipeline_man = *backend.get_pipeline_manager();

        apply_pipeline_settings(target);
        graphics_pipeline& pipeline = pipeline_man.get_or_create_pipeline( get_scene_basepass_pipeline_signature() );
        if (scene.is_empty())
        {
            return;
        }

        // hot-reload our shaders if necessary:
        pipeline.rebuild(backend.get_device());

        influx_scope("renderer_backend::draw_scene::record");
        logonce(e_log_category::warning, "influx::renderer::scene_renderer: first scene render!");

        graphics::commandlist& commandlist = context.get_commandlist();

        // setup batches & instance buffer
        vector<batch> batches = create_batches(scene, &commandlist);
        update_instance_buffer(batches);

        // set generic pipeline state (pipeline, rootsignature, primitive topo, ...)
        pipeline.set_state(commandlist);
        commandlist.set(graphics::e_primitive_topology::trilist);

        // per-scene
        m_gpu_perscene.m_time.x = scene.m_delta_seconds;
        m_gpu_perscene.m_time.y = scene.m_seconds;

        // per view constants
        // update viewprojection matrix
        {
            const camera& camera = scene.m_camera;
            math::transform3D transform = camera.m_transform;
            transform.update_matrix();

            const float ar = (float)target.get_width() / target.get_height();
            m_gpu_perview.m_viewprojection = make_viewprojection(transform.get_matrix(), ar, camera.m_fov, camera.m_near_plane, camera.m_far_plane);
            pipeline.set_constants<frontend::per_scene>(commandlist, "g_perscene", m_gpu_perscene);
            pipeline.set_constants<frontend::per_view>(commandlist, "g_perview", m_gpu_perview);
        }

        for (const batch& batch : batches)
        {
            // per draw constants
            m_gpu_perdraw.m_base_instance = static_cast<uint32>(batch.m_base_instance);
            m_gpu_permaterial.m_colour = {};
            pipeline.set_constants(commandlist, "g_permaterial", m_gpu_permaterial);
            pipeline.set_constants(commandlist, "g_perdraw", m_gpu_perdraw);

            // bind index buffer
            graphics::resource* index_buffer = batch.m_indexbuffer;
            const uint32 num_indices = (uint32)index_buffer->get_bytesize() / (uint32)index_buffer->get_bytestride();
            commandlist.set_indexbuffer(index_buffer);
            commandlist.set_vertexbuffer(batch.m_vertexbuffer);
            influx_assert(batch.m_vertexbuffer->get_bytestride() == 48u);
            const uint32 num_instances = (uint32)batch.m_instances.size();
            commandlist.draw_indexed(
            {
                .m_num_indexes_per_instance = num_indices,
                .m_num_instances = num_instances,
                .m_start_index = 0u,
                .m_start_vertex = 0u,
                .m_start_instance = m_gpu_perdraw.m_base_instance
            });
        }
    }

    static rendergraph::rgtexture_readonly_id gbuffer_reads[k_num_gbuffers]{};
    static rendergraph::rgtexture_readwrite_id resolve_write{};

    static bool g_use_proxy_pass = false;
    static rendergraph::rgname g_proxy_name = RGNAME("uav_proxy");

    void scene_renderer::build_resolvepass(rendergraph::rgpass_builder& builder, const target& target, const scene& scene)
    {
        rendergraph::rgname gbuffernames[k_num_gbuffers]
        {
            RGNAME("gbuffer_a"),
            RGNAME("gbuffer_b"),
            RGNAME("gbuffer_c")
        };

        // read gbuffers and write into target buffer as result
        static string color_name{}; color_name = target.get_resource()->get_name().get();
        for (uint32 i = 0; i < k_num_gbuffers; ++i)
        {
            gbuffer_reads[i] = builder.read_texture(gbuffernames[i]);
        }

        builder.set_viewport(target.get_width(), target.get_height());

        if (g_use_proxy_pass)
        {
            // write to proxy
            resolve_write = builder.write_texture(g_proxy_name);
        }
        else
        {
            // write to target
            resolve_write = builder.write_texture(target.get_name().get());
        }
    }

    void scene_renderer::execute_resolvepass(rendergraph::rgpass_context& context, const target& target, const scene& scene)
    {
        renderer_backend& backend = renderer_backend::get_instance();
        pipeline_manager& pipeline_man = *backend.get_pipeline_manager();
        descriptor_manager& descriptor_man = *backend.get_descriptor_manager();
        compute_pipeline& pipeline = pipeline_man.get_or_create_pipeline(get_scene_resolve_pipeline_signature());
        graphics::commandlist& commandlist = context.get_commandlist();

        // hot-reload our shaders if necessary:
        pipeline.rebuild(backend.get_device());

        // update buffers for deferred lights
        update_lightbuffers(scene);

        // build resolve args
        struct resolve_args final
        {
            int texture_indices[4u];
            int buffer_indices[4u];
            math::float4 screen_size;
            math::float4 camera_position;
            math::matrix4x4f inv_viewprojection;
            int num_lights[4u];
        } args{};

        for (uint32 i = 0u; i < k_num_light_types; ++i)
        {
            args.num_lights[i] = scene.get_num_lights(static_cast<influx::scene::e_light_type>(i));
        }

        args.screen_size = math::float4(target.get_width(), target.get_height(), 1.0f / target.get_width(), 1.0f / target.get_height());
        const camera& camera = scene.m_camera;
        math::transform3D transform = camera.m_transform;
        transform.update_matrix();
        const float ar = (float)target.get_width() / target.get_height();
        args.inv_viewprojection = make_viewprojection(transform.get_matrix(), ar, camera.m_fov, camera.m_near_plane, camera.m_far_plane).inverted();

        args.camera_position = transform.get_position().get_xyz();
        args.camera_position.w = 1.0f;

        // stage the descriptors onto the gpu heap
        graphics::descriptor_range gpu_range = descriptor_man.stage({
                context.get_read_texture(gbuffer_reads[0]),
                context.get_read_texture(gbuffer_reads[1]),
                context.get_read_texture(gbuffer_reads[2]),
                context.get_write_texture(resolve_write),
                m_lightbuffer_srvs[0],
                m_lightbuffer_srvs[1],
                m_lightbuffer_srvs[2]
            });

        pipeline.set_state(commandlist);

        // set the descriptorheap bindless indices
        args.texture_indices[0] = gpu_range.m_start_idx;
        args.texture_indices[1] = gpu_range.m_start_idx + 1u;
        args.texture_indices[2] = gpu_range.m_start_idx + 2u;
        args.texture_indices[3] = gpu_range.m_start_idx + 3u;
        args.buffer_indices[0] = gpu_range.m_start_idx + 4u;
        args.buffer_indices[1] = gpu_range.m_start_idx + 5u;
        args.buffer_indices[2] = gpu_range.m_start_idx + 6u;

        pipeline.set_constants<resolve_args>(commandlist, "g_resolve_args", args);

        const uint32 num_groups_x = target.get_width() / 8u;
        const uint32 num_groups_y = target.get_height() / 8u;
        commandlist.dispatch({ {num_groups_x, num_groups_y, 1u} });
    }

    void scene_renderer::render(rendergraph::rendergraph& graph, const scene& scene, const target& target)
    {
        renderer_backend& backend = renderer_backend::get_instance();

        static string color_name{}; color_name = target.get_resource()->get_name().get();
        static string depth_name{}; depth_name = color_name + "_depth";

        graph.import_texture(color_name, target.get_resource());
        if (target.has_depth_stencil()) graph.import_texture(depth_name, target.get_depth_resource());
        else
        {
            logonce(e_log_category::warning, "influx_renderer::scene_renderer::render >> rendering scene without depth because specified target has no depth!");
        }

        // deferred gbuffer basepass
        auto* basepass = graph.add_pass(rendergraph::e_rgpass_type::graphics,
            [this, &target](rendergraph::rgpass_builder& builder)
            {
                build_basepass(builder, target);
            },
            [this, &scene, &target](rendergraph::rgpass_context& context)
            {
                execute_basepass(context, target, scene);
            });
        basepass->set_name(RGNAME("basepass"));
       
        // if we're directly writing to the swapchain, we need to write to a intermediate that allows for uav writes
        g_use_proxy_pass = target.is_swapchain_target() || !target.get_resource()->allows_uav();
        if (g_use_proxy_pass)
        {
            static rendergraph::rgtex_copysrc_id src_tex_id{};
            static rendergraph::rgtex_copydst_id dst_tex_id{};
            auto* proxypass = graph.add_pass(rendergraph::e_rgpass_type::compute,
                [&target](rendergraph::rgpass_builder& builder)
                {
                    rendergraph::texture_desc proxy_desc{};
                    proxy_desc.m_allow_uav = true;
                    proxy_desc.m_array_size = 1u;
                    proxy_desc.m_bindflags = graphics::e_bind_flags::uav;
                    proxy_desc.m_depth = 1u;
                    proxy_desc.m_format = target.get_resource()->get_format();
                    proxy_desc.m_width = target.get_width();
                    proxy_desc.m_heigth = target.get_height();
                    proxy_desc.m_num_mips = 1u;
                    proxy_desc.m_sample_count = 1u;
                    builder.declare_texture(g_proxy_name, proxy_desc);

                    src_tex_id = builder.read_copysrc_texture(target.get_name().get());
                    dst_tex_id = builder.write_copydst_texture(g_proxy_name);
                    builder.set_viewport(target.get_width(), target.get_height());
                },
                [](rendergraph::rgpass_context& context)
                {
                    graphics::resource* src_resource = context.get_copysrc_resource(src_tex_id);
                    graphics::resource* dst_resource = context.get_copydst_resource(dst_tex_id);
                    context.get_commandlist().copy_resource(src_resource, dst_resource);
                });
            proxypass->set_name(RGNAME("proxypass_a"));
        }

        // resolve gbuffer to the target
        auto* resolvepass = graph.add_pass(rendergraph::e_rgpass_type::compute,
            [this, &target, &scene](rendergraph::rgpass_builder& builder)
            {
                build_resolvepass(builder, target, scene);
            },
            [this, &target, &scene](rendergraph::rgpass_context& ctx)
            {
                execute_resolvepass(ctx, target, scene);
            });
        resolvepass->set_name(RGNAME("resolvepass"));

        if (g_use_proxy_pass)
        {
            static rendergraph::rgtex_copysrc_id src_tex_id{};
            static rendergraph::rgtex_copydst_id dst_tex_id{};
            auto* proxypass = graph.add_pass(rendergraph::e_rgpass_type::compute,
                [&target](rendergraph::rgpass_builder& builder)
                {
                    src_tex_id = builder.read_copysrc_texture(g_proxy_name);
                    dst_tex_id = builder.write_copydst_texture(target.get_name().get());
                    builder.set_viewport(target.get_width(), target.get_height());
                },
                [](rendergraph::rgpass_context& context)
                {
                    graphics::resource* src_resource = context.get_copysrc_resource(src_tex_id);
                    graphics::resource* dst_resource = context.get_copydst_resource(dst_tex_id);
                    context.get_commandlist().copy_resource(src_resource, dst_resource);
                });
            proxypass->set_name(RGNAME("proxypass_b"));
        }
    }
}