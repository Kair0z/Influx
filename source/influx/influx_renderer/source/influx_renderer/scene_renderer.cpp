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
#include "influx_renderer/resources/resource_manager.h"

// influx::graphics
#include "influx_graphics/commandlist.h"
#include "influx_graphics/resource.h"
#include "influx_graphics/device.h"

// influx::rendergraph
#include "rendergraph.h"

// influx::shader
#include "influx_shader.h"

namespace influx::renderer
{
    static constexpr uint32 k_num_gbuffers = 3u;
    static constexpr graphics::e_format k_gbuffer_formats[k_num_gbuffers]
    {
        graphics::e_format::rgba_u32,
        graphics::e_format::u32,
        graphics::e_format::u32,
    };

#pragma region shaders
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

    static graphics_pipeline_signature& get_debug_pipeline_signature()
    {
        static graphics_pipeline_signature signature{};
        {
            signature.m_shader_identifiers[(uint8)graphics_pipeline::e_shader_slot::vs] = "debug_shaders::main_vs";
            signature.m_shader_identifiers[(uint8)graphics_pipeline::e_shader_slot::ps] = "debug_shaders::main_ps";

            signature.m_primitive_type = graphics::e_primitive_topology_type::line;
            signature.m_cullmode = graphics::e_cull_mode::nocull;
            signature.m_fillmode = graphics::e_fill_mode::wireframe;
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

            signature.m_depth_enable = false;
            signature.m_stencil_enable = false;
            signature.m_depth_comparison = graphics::e_comparison_func::less;
            signature.m_depth_format = graphics::e_format::d32;

            signature.m_rtv_actives[0] = true;
            signature.m_rtv_formats[0] = graphics::e_format::rgba8;

            signature.m_blend_actives[0] = false;
        }
        return signature;
    }
#pragma endregion

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
                const influx::e_light_type type = static_cast<influx::e_light_type>(i);
                desc.m_bytestride = buffer_strides[i];
                desc.m_bytesize = k_max_num_lights * desc.m_bytestride;

                mp_lightbuffers[i] = device.create_resource(desc, heap_desc);
                mp_lightbuffers[i]->set_name("lightbuffer_" + to_string(i));

                m_lightbuffer_srvs[i] = descriptor_manager.create_buffer_srv(mp_lightbuffers[i]);
            }
        }

        // LINE RENDER
        m_line_instance_data.clear();
        m_line_instance_data.reserve(k_max_lines);

        // line-render: create instance buffer & srv
        {
            graphics::heap_desc heap_desc{};
            heap_desc.m_type = graphics::e_heap_type::shared;
            graphics::buffer_desc desc{};
            desc.m_bytesize = k_max_lines * sizeof(frontend::line_gpu_instance_data);
            desc.m_bytestride = sizeof(frontend::line_gpu_instance_data);
            desc.m_init_state = graphics::e_resource_state::gen_read;
            m_line_instance_buffer = device.create_resource(desc, heap_desc);
            m_line_instance_buffer->set_name({ "line_instance_buffer" });
            m_line_instance_buffer_srv = backend.get_descriptor_manager()->create_buffer_srv(m_line_instance_buffer);
        }

        // line-render: create 2-element vertexbuffer
        {
            vector<frontend::line_vertex> vertices = {
                {.m_position{0.0f, 0.0f, 0.0f}, .m_colour{1,1,1,1}},
                {.m_position{1.0f, 0.0f, 0.0f}, .m_colour{1,1,1,1}} };

            graphics::heap_desc heap_desc{};
            heap_desc.m_type = graphics::e_heap_type::shared;
            graphics::buffer_desc desc{};
            desc.m_init_state = graphics::e_resource_state::gen_read;
            desc.m_bytesize = vertices.size() * sizeof(frontend::line_vertex);
            desc.m_bytestride = sizeof(frontend::line_vertex);
            m_line_vertex_buffer = device.create_resource(desc, heap_desc);
            m_line_vertex_buffer->map([&vertices](void* target)
            {
                memcpy(target, vertices.data(), vertices.size() * sizeof(frontend::line_vertex));
            });
        }

        m_sampler_view = descriptor_manager.create_sampler();
        m_skybox_sampler = descriptor_manager.create_sampler();
    }

    scene_renderer::~scene_renderer()
    {
        delete[] m_instance_data;
    }

    void scene_renderer::load_shaders()
    {
        renderer_backend& backend = renderer_backend::get_instance();
        resource_manager& resourceman = backend.get_resource_manager();

        string base_dir = backend.get_shadersource_directory(e_shadersource_directory::base);
        string src_dir = backend.get_shadersource_directory(e_shadersource_directory::source);
        string inc_dir = backend.get_shadersource_directory(e_shadersource_directory::include);

        string basepass_sourcefile_path = src_dir + "/basepass.hlsl";
        string resolvepass_sourcefile_path = src_dir + "/resolvepass.hlsl";
        string debugpass_sourcefile_path = src_dir + "/debug_shaders.hlsl";

        // parse the shader files for their necessary shaders
        shader::compile_args compile_args{};
        compile_args.m_include_folder = base_dir;
        compile_args.m_signature.m_target = shader::e_shader_target::_6_6;;
        compile_args.m_reflection = true;
        compile_args.m_defines = {};
        compile_args.m_compile_debug = false;
        compile_args.m_pbd = false;

        using parse_result = shader::result<vector<shader::parse_output>>;

        // 1. parse each shader from file
        parse_result basepass_parse = shader::parse_shaders_in_file(basepass_sourcefile_path);
        parse_result resolvepass_parse = shader::parse_shaders_in_file(resolvepass_sourcefile_path);
        parse_result debugpass_parse = shader::parse_shaders_in_file(debugpass_sourcefile_path);
        const bool all_shaders_parsed = !(basepass_parse.is_unex() || resolvepass_parse.is_unex() || debugpass_parse.is_unex());
        influx_assert(all_shaders_parsed);

        // 2. assert no missing shaders
        auto has_all_shaders = 
        [](const vector<shader::parse_output>& parsed_shaders, shader::e_shader_type_flags flags) -> bool
        {
            // check for each type that is flagged
            for (uint32 i = 0u; i < shader::k_num_shadertypes; ++i)
            {
                shader::e_shader_type current_type = static_cast<shader::e_shader_type>(i);
                if (has_flag(flags, shader::get_shader_flag(current_type)))
                {
                    const bool has_shader = vector_helpers::contains(parsed_shaders,
                    [&current_type](const shader::parse_output& shader)
                    {
                        return shader.get_shader_type() == current_type;
                    });

                    if (!has_shader) return false;
                }
            }
            return true;
        };

        // assert required shaders are present
        vector<shader::parse_output> basepass_parsed_file = basepass_parse.get();
        influx_assert(has_all_shaders(basepass_parsed_file,
            shader::e_shader_type_flags::vs | shader::e_shader_type_flags::ps));
        vector<shader::parse_output> resolvepass_parsed_file = resolvepass_parse.get();
        influx_assert(has_all_shaders(resolvepass_parsed_file,
            shader::e_shader_type_flags::cs));
        const auto& debugpass_parsed_file = debugpass_parse.get();
        influx_assert(has_all_shaders(debugpass_parsed_file,
            shader::e_shader_type_flags::vs | shader::e_shader_type_flags::ps));

        // compile all shaders
        auto compile_shaders =
        [&resourceman](const vector<shader::parse_output>& parsed_shaders,
            const string& filepath, const shader::compile_args& master_args)
        {
            for (const shader::parse_output& shader_parse : parsed_shaders)
            {
                shader::compile_args local_args = master_args;
                local_args.m_signature = shader_parse.m_signature;
                local_args.m_signature.m_target = master_args.m_signature.m_target;

                // compile
                auto compile_result = shader::compile_shader_in_file(filepath, local_args);
                influx_assert(compile_result.is_success());

                // load into resource_manager
                shader::compile_output compile_output = compile_result.get();
                influx_assert(compile_output.m_success);
                resourceman.load<e_resource_type::shader>(compile_output.m_signature, shader_data::translate(compile_output), true);
            }
        };
        compile_shaders(basepass_parsed_file, basepass_sourcefile_path, compile_args);
        compile_shaders(resolvepass_parsed_file, resolvepass_sourcefile_path, compile_args);
        compile_shaders(debugpass_parsed_file, debugpass_sourcefile_path, compile_args);
    }

#pragma region batching
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

    inline frontend::per_instance translate(const mesh_instance& mesh, const scene& scene)
    {
        frontend::per_instance instance_data{};
        instance_data.m_transform = scene.get_transform( mesh.m_transform_id );
        instance_data.m_colour = mesh.m_per_instance_colour;
        return instance_data;
    }

    vector<batch> scene_renderer::create_batches(const scene& scene, graphics::commandlist* commandlist)
    {
        renderer_backend& backend = renderer_backend::get_instance();
        umap<texture2D*, uint32> tex_to_idx{};

        // stage all srv/uav/const descriptors on the bindless heap
        // in bindless, MUST happen before setting descriptor (mp_pipeline->set_state)
        {
            vector<graphics::descriptor_handle> all_srvs{};
            all_srvs.push_back(m_instance_buffer_srv);

            uint32 texture_count = 0u;
            for (const mesh_instance& instance : scene.get_meshes())
            {
                string diffuse_name = ""; // todo: fix
                texture2D* diffuse_texture = backend.find_texture(diffuse_name);
                if (diffuse_texture != nullptr && !tex_to_idx.contains(diffuse_texture))
                {
                    // add to list of unique srvs
                    all_srvs.push_back(diffuse_texture->get_srv());

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

        using mesh_to_instance_map = umap<mesh_id, vector<frontend::per_instance>>;
        mesh_to_instance_map mesh_to_instances{};

        for (const mesh_instance& instance : scene.get_meshes())
        {
            graphics::resource* indexbuffer = nullptr;
            graphics::resource* vertexbuffer = nullptr;

            frontend::per_instance gpu_data = translate(instance, scene);

            string diffuse_name = "";
            gpu_data.set_albedo_index( tex_to_idx[backend.find_texture(diffuse_name)] );
            mesh_to_instances[instance.m_mesh_id].push_back(gpu_data);
        }

        vector<batch> batches{};
        uint64 offset = 0u;
        for (const auto& pair : mesh_to_instances)
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

    void scene_renderer::update_line_instance_buffer(const scene& scene)
    {
        // load up the instance data
        m_line_instance_data.clear();
        for (const line& line : scene.get_lines())
        {
            frontend::line_gpu_instance_data instance_data{};
            instance_data.m_colour = line.m_colour;
            instance_data.m_start_wp = line.m_points[0u];
            instance_data.m_end_wp = line.m_points[1u];
            m_line_instance_data.push_back(instance_data);
        }

        // map the cpu data to the shared gpu resource
        m_line_instance_buffer->map([this](void* dest)
        {
            frontend::line_gpu_instance_data* data = reinterpret_cast<frontend::line_gpu_instance_data*>(dest);
            for (uint64 i = 0u; i < m_line_instance_data.size(); ++i)
            {
                data[i] = m_line_instance_data[i];
            }
        });
    }

    void scene_renderer::update_lightbuffers(const scene& scene)
    {
        // map lightbuffers
        const uint32 num_lights = scene.get_num_lights();
        for (uint32 i = 0u; i < k_num_light_types; ++i)
        {
            const auto current_type = static_cast<influx::e_light_type>(i);

            mp_lightbuffers[i]->map([num_lights, current_type, i, &scene](void* dest)
            {
                uint32 index = 0u;
                for (uint32 l = 0; l < num_lights; ++l)
                {
                    const light& light = scene.get_lights()[l];
                    if (light.m_light.get_type() != current_type) continue;
                    
                    const math::matrix4x4f light_transform = scene.get_transform(light.m_transform_id);
                    switch (current_type)
                    {
                    case influx::e_light_type::directional:
                    {
                        frontend::per_dirlight* data = reinterpret_cast<frontend::per_dirlight*>(dest);
                        data[index].m_colour = light.m_light.get_colour();
                        break;
                    }

                    case influx::e_light_type::point:
                    {
                        frontend::per_pointlight* data = reinterpret_cast<frontend::per_pointlight*>(dest);
                        data[index].m_attenuation = light.m_light.get_attenuation();
                        data[index].m_colour = light.m_light.get_colour();
                        data[index].m_position = light_transform.get_translation();
                        break;
                    }

                    case influx::e_light_type::spot:
                    {
                        frontend::per_spotlight* data = reinterpret_cast<frontend::per_spotlight*>(dest);
                        data[index].m_position = light_transform.get_translation();
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
            builder.write_depthtarget(target.get_depth_rendergraph_name(), access);
        }
        builder.set_viewport(target.get_width(), target.get_height());
    }

    void scene_renderer::execute_basepass(rendergraph::rgpass_context& context, const target& target, const scene& scene)
    {
        if (scene.is_empty())
        {
            return;
        }

        influx_scope("renderer_backend::draw_scene::record");
        logonce(e_log_category::warning, "influx::renderer::scene_renderer: first scene render!");

        renderer_backend& backend = renderer_backend::get_instance();
        pipeline_manager& pipeline_man = *backend.get_pipeline_manager();

        apply_pipeline_settings(target);
        graphics_pipeline& pipeline = pipeline_man.get_or_create_pipeline(get_scene_basepass_pipeline_signature());
        
        // hot-reload our shaders if necessary:
        static bool once = true;
        if (once)
        {
            pipeline.rebuild(backend.get_device());
            influx_assert(pipeline.is_valid());
            once = false;
        }
        
        // skybox
        if (mp_skybox == nullptr)
        {
            if (cubemap* cube = backend.find_texturecube("graycloud"))
            {
                mp_skybox = cube->get_resource();
            }
        }
 
        graphics::commandlist& commandlist = context.get_commandlist();

        // setup batches & instance buffer
        vector<batch> batches = create_batches(scene, &commandlist);
        update_instance_buffer(batches);

        // set generic pipeline state (pipeline, rootsignature, primitive topo, ...)
        pipeline.set_state(commandlist);
        commandlist.set(graphics::e_primitive_topology::trilist);

        // constants
        m_gpu_perscene.m_time.x = scene.m_delta_seconds;
        m_gpu_perscene.m_time.y = scene.m_seconds;
        m_gpu_perview.m_viewprojection = scene.get_view_matrices().m_viewprojection;
        pipeline.set_constants<frontend::per_scene>(commandlist, "g_perscene", m_gpu_perscene);
        pipeline.set_constants<frontend::per_view>(commandlist, "g_perview", m_gpu_perview);

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
            // write directly to target
            resolve_write = builder.write_texture(target.get_rendergraph_name());
        }
    }

    void scene_renderer::execute_resolvepass(rendergraph::rgpass_context& context, const target& target, const scene& scene)
    {
        renderer_backend& backend = renderer_backend::get_instance();
        pipeline_manager& pipeline_man = *backend.get_pipeline_manager();
        descriptor_manager& descriptor_man = *backend.get_descriptor_manager();
        compute_pipeline& pipeline = pipeline_man.get_or_create_pipeline(get_scene_resolve_pipeline_signature());
        graphics::commandlist& commandlist = context.get_commandlist();
        resource_manager& resourceman = backend.get_resource_manager();

        // hot-reload our shaders if necessary:
        pipeline.rebuild(backend.get_device());
        pipeline.set_state(commandlist);

        // build resolve args
        struct resolve_args final
        {
            int texture_indices[4u];
            int buffer_indices[4u];
            int skybox_indices[4u];
            math::float4 screen_size;
            math::float4 camera_position;
            math::matrix4x4f inv_viewprojection;
            math::matrix4x4f inv_projection;
            int num_lights[4u];
        } args{};
        args.screen_size = math::float4(target.get_width(), target.get_height(), 1.0f / target.get_width(), 1.0f / target.get_height());

        // update buffers for deferred lights
        {
            update_lightbuffers(scene);
            for (uint32 i = 0u; i < k_num_light_types; ++i)
            {
                args.num_lights[i] = scene.get_num_lights(static_cast<influx::e_light_type>(i));
            }
        }

        args.inv_viewprojection = scene.get_view_matrices().m_inv_viewprojection;
        args.inv_projection = scene.get_view_matrices().m_inv_projection;
        args.camera_position = scene.get_camera_transform().get().get_row(3);
        args.camera_position.w = 1.0f;

        // stage the descriptors onto the gpu heap
        {
            graphics::descriptor_range gpu_range = descriptor_man.stage({
                context.get_read_texture(gbuffer_reads[0]).get(),
                context.get_read_texture(gbuffer_reads[1]).get(),
                context.get_read_texture(gbuffer_reads[2]).get(),
                context.get_write_texture(resolve_write).get(),
                resourceman.get<e_resource_type::cubemap>("graycloud").m_resource->get_srv(),
                m_lightbuffer_srvs[0],
                m_lightbuffer_srvs[1],
                m_lightbuffer_srvs[2]
                });
            // set the descriptorheap bindless indices
            args.texture_indices[0] = gpu_range.m_start_idx;
            args.texture_indices[1] = gpu_range.m_start_idx + 1u;
            args.texture_indices[2] = gpu_range.m_start_idx + 2u;
            args.texture_indices[3] = gpu_range.m_start_idx + 3u;
            args.skybox_indices[0] = gpu_range.m_start_idx + 4u;
            args.buffer_indices[0] = gpu_range.m_start_idx + 5u;
            args.buffer_indices[1] = gpu_range.m_start_idx + 6u;
            args.buffer_indices[2] = gpu_range.m_start_idx + 7u;
            descriptor_man.stage_sampler(m_skybox_sampler);
        }
       
        pipeline.set_constants<resolve_args>(commandlist, "g_resolve_args", args);

        const uint32 num_groups_x = target.get_width() / 8u;
        const uint32 num_groups_y = target.get_height() / 8u;
        commandlist.dispatch({ {num_groups_x, num_groups_y, 1u} });
    }

    void scene_renderer::render(rendergraph::rendergraph& graph, const scene& scene, const target& target)
    {
        if (scene.is_empty())
        {
            return;
        }

        renderer_backend& backend = renderer_backend::get_instance();
        backend.import_to_graph(target);

        // | BASEPASS
        // | renders a couple of deferred gbuffers
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
       
        // | PROXY PASS (in case target isn't UAV compatible)
        // if we're directly writing to the swapchain, we need to write to a intermediate that allows for uav writes
        // the proxy pass copies the existing target contents into proxy (UAV) target
        const bool target_is_uav_compatible = !target.is_swapchain_target() && target.get_resource()->allows_uav();
        g_use_proxy_pass = target_is_uav_compatible == false;
        if (g_use_proxy_pass)
        {
            // copy target -> proxy
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

                src_tex_id = builder.read_copysrc_texture(target.get_rendergraph_name());
                dst_tex_id = builder.write_copydst_texture(g_proxy_name);

                builder.set_viewport(target.get_width(), target.get_height());
            },
            [](rendergraph::rgpass_context& context)
            {
                graphics::resource* src_resource = context.get_copysrc_resource(src_tex_id).get();
                graphics::resource* dst_resource = context.get_copydst_resource(dst_tex_id).get();
                context.get_commandlist().copy_resource(src_resource, dst_resource);
            });
            proxypass->set_name(RGNAME("proxypass_a"));
        }

        // | RESOLVE PASS
        // | compute shader that resolves lighting into a final scene colour UAV
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

        // | PROXY PASS
        // | proxy -> target
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
                graphics::resource* src_resource = context.get_copysrc_resource(src_tex_id).get();
                graphics::resource* dst_resource = context.get_copydst_resource(dst_tex_id).get();
                context.get_commandlist().copy_resource(src_resource, dst_resource);
            });
            proxypass->set_name(RGNAME("proxypass_b"));
        }

        // | POST PROCESSING PASS
        // ...
        
        // | LINE RENDER PASS (DEBUG)
        if (scene.has_debug_primitives() && scene.is_debug_render_enabled())
        {
            auto* pass = graph.add_pass(rendergraph::e_rgpass_type::graphics,
            [&target](rendergraph::rgpass_builder& builder)
            {
                rendergraph::rgaccess access{};
                access.m_load = rendergraph::e_rg_load::preserve;
                access.m_store = rendergraph::e_rg_store::preserve;
                builder.write_rendertarget(target.get_rendergraph_name(), access);
                builder.set_viewport(target.get_width(), target.get_height());
            },
            [this, &scene, &target](rendergraph::rgpass_context& context)
            {
                influx_scope("renderer_backend::draw_debug::record");
                graphics::commandlist& commandlist = context.get_commandlist();
                
                // get the pipeline
                renderer_backend& backend = renderer_backend::get_instance();
                pipeline_manager& pipelineman = *backend.get_pipeline_manager();
                graphics_pipeline& pipeline = pipelineman.get_or_create_pipeline(get_debug_pipeline_signature());
                descriptor_manager& descriptorman = *backend.get_descriptor_manager();

                // hot-reload our shaders if necessary:
                pipeline.rebuild(backend.get_device());
                influx_assert(pipeline.is_valid());

                logonce(e_log_category::warning, "influx::renderer::debug_renderer: first debug render!");

                // update viewprojection matrix
                m_gpu_perview.m_viewprojection = scene.get_view_matrices().m_viewprojection;

                pipeline.set_state(commandlist);
                commandlist.set(graphics::e_primitive_topology::linelist);
                pipeline.set_constants<frontend::per_view>(commandlist, "g_perview", m_gpu_perview);
                commandlist.set_vertexbuffer(m_line_vertex_buffer);

                update_line_instance_buffer(scene);

                // stage the instance buffer and set as resource table
                const graphics::descriptor_range gpu_range = descriptorman.stage(m_instance_buffer_srv);
                pipeline.set_resource_table(commandlist, "g_instancebuffer", gpu_range);

                const uint32 num_instances = (uint32)m_line_instance_data.size();
                commandlist.draw_instanced(
                {
                    .m_num_vertices_per_instance = 2u,
                    .m_num_instances = num_instances,
                    .m_start_vertex = 0u,
                    .m_start_instance = 0u
                });
            });

            pass->set_name(RGNAME("draw_debug"));
        }
    }
}