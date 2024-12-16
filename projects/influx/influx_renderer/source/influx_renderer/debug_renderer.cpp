#include "renderer_pch.h"
#include "debug_renderer.h"

// influx::graphics
#include "influx_graphics/resource.h"
#include "influx_graphics/device.h"

// influx::renderer
#include "influx_renderer/renderer_backend.h"
#include "influx_renderer/descriptor_manager.h"
#include "influx_renderer/pipeline/pipeline_manager.h"
#include "influx_renderer/pipeline/pipeline.h"

namespace influx::renderer
{
    struct debug_renderer::gpu_instance_data final
    {
        math::vectorf3	m_start_wp;
        math::vectorf3	m_end_wp;
        math::colour_rgba m_colour;
    };

    struct debug_renderer::gpu_perview final
    {
        math::matrix4x4f m_vp;
    };

    debug_renderer::debug_renderer(renderer_backend* backend, graphics::device* device, pipeline* pipeline)
        : mp_pipeline{pipeline}
        , mp_backend{backend}
        , mp_device{device}
    {
        m_instance_data.clear();
        m_instance_data.reserve(k_max_instances);
        
        // create instance buffer & srv
        {
            graphics::heap_desc heap_desc{};
            heap_desc.m_type = graphics::e_heap_type::shared;
            graphics::buffer_desc desc{};
            desc.m_bytesize = k_max_instances * sizeof(gpu_instance_data);
            desc.m_bytestride = sizeof(gpu_instance_data);
            desc.m_init_state = graphics::e_resource_state::read;
            mp_instancebuffer = device->create_resource(desc, heap_desc);
            mp_instancebuffer->set_name({ "debug_instance_buffer" });
            mp_instance_buffer_srv = backend->get_descriptor_manager()->create_buffer_srv(mp_instancebuffer);
        }
        
        // create 2-element vertexbuffer (mini)
        {
            vector<math::float3> vertices = { {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f} };

            graphics::heap_desc heap_desc{};
            heap_desc.m_type = graphics::e_heap_type::shared;
            graphics::buffer_desc desc{};
            desc.m_init_state = graphics::e_resource_state::read;

            desc.m_bytesize = vertices.size() * sizeof(math::float3);
            desc.m_bytestride = sizeof(math::float3);
            mp_vertexbuffer = mp_device->create_resource(desc, heap_desc);
            mp_vertexbuffer->map([&vertices](void* target)
            {
                memcpy(target,
                vertices.data(),
                vertices.size() * sizeof(math::float3));
            });
        }

        m_gpu_perview = new gpu_perview();
    }

    debug_renderer::~debug_renderer()
    {
        delete m_gpu_perview;
    }

    void debug_renderer::render(graphics::commandlist* commandlist, const scene_debug& scene, const target& target)
    {
        mp_pipeline = mp_backend->get_pipeline_manager()->get_or_create_pipeline("pip_debug",
            pipeline_key{.m_vs_name{"debug_shaders_vs"}, .m_ps_name{"debug_shaders_ps"}});

        if (mp_pipeline == nullptr)
        {
            logonce(e_log_category::warning, "influx::renderer::debug_renderer: no debug pipeline!");
            return;
        }

        logonce(e_log_category::warning, "influx::renderer::debug_renderer: first debug render!");

        update_instance_buffer(scene);

        // set generic pipeline state (pipeline, rootsignature, primitive topo, ...)
        mp_pipeline->set_state(commandlist);

        // update viewprojection matrix
        {
            const camera& camera = scene.m_camera;
            math::transform3D transform = camera.m_transform;
            transform.update_matrix();

            auto copy = transform.get_matrix();
            math::matrix4x4f::invert(copy); // <-- this is probably wrong
            copy.set_collumn(2u, -copy.get_collumn(2u));

            const math::matrix4x4f mat_view = copy;
            const math::matrix4x4f mat_proj = math::matrix4x4f::make_projection_RH(camera.m_fov, (float)target.get_width() / target.get_height(), camera.m_near_plane, camera.m_far_plane);
            m_gpu_perview->m_vp = mat_view * mat_proj;
        }
        
        // set per-view constant buffer
        mp_pipeline->set_constants<gpu_perview>(commandlist, "g_perview", *m_gpu_perview);

        // set vertexbuffer
        {
            commandlist->set_vertexbuffer(mp_vertexbuffer);
        }

        // stage the instance buffer
        {
            graphics::descriptor_range gpu_range =
                mp_backend->get_descriptor_manager()->stage(mp_instance_buffer_srv->get_cpu_handle());

            // set the resource table
            mp_pipeline->set_resource_table(commandlist, "g_instancebuffer", gpu_range);
        }

        const uint32 num_instances = (uint32)m_instance_data.size();
        commandlist->draw_instanced(
        {
            .m_num_vertices_per_instance = 2u,
            .m_num_instances = num_instances,
            .m_start_vertex = 0u,
            .m_start_instance = 0u
        });
    }

    void debug_renderer::update_instance_buffer(const scene_debug& scene)
    {
        m_instance_data.clear();

        for (const scene_debug::line& line : scene.m_lines)
        {
            gpu_instance_data instance_data{};
            instance_data.m_colour = line.m_colour;
            instance_data.m_start_wp = line.m_points[0u];
            instance_data.m_end_wp = line.m_points[1u];
            m_instance_data.push_back(instance_data);
        }

        mp_instancebuffer->map([this](void* dest)
        {
            gpu_instance_data* data = reinterpret_cast<gpu_instance_data*>(dest);
            for (uint64 i = 0u; i < m_instance_data.size(); ++i)
            {
                data[i] = m_instance_data[i];
            }
        });
    }
}