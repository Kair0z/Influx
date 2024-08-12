#include "renderer_pch.h"
#include "scene_renderer.h"

#include "influx_renderer/target.h"
#include "influx_renderer/pipeline.h"
#include "influx_renderer/renderer_backend.h"
#include "influx_renderer/descriptor_manager.h"
#include "influx_renderer/pipeline_manager.h"

#include "influx_graphics/commandlist.h"
#include "influx_graphics/resource.h"
#include "influx_graphics/resource_views.h"

namespace influx::renderer
{
    scene_renderer::scene_renderer(renderer_backend* backend, graphics::device* device, pipeline* pipeline)
        : mp_pipeline{ pipeline }
        , mp_backend{ backend }
    {
    }

    void scene_renderer::render(graphics::command_list* commandlist, const scene& scene, const target& target)
    {
        // try to fish the scene pipeline from the manager...
        mp_pipeline = mp_backend->get_pipeline_manager()->get_scene_pipeline();
        if (mp_pipeline == nullptr)
        {
            return;
        }

        // bind descriptor heaps
        commandlist->set(mp_backend->get_descriptor_manager()->get_samp_heap());
        commandlist->set(mp_backend->get_descriptor_manager()->get_srv_heap());

        // set generic pipeline state (pipeline, rootsignature, primitive topo, ...)
        mp_pipeline->set_state(commandlist);

        // update constants
        m_ps_constants.m_texture_idx = 0u;

        // invert camera :) (engine is right handed, but d3d12 is left handed)
        const camera& camera = scene.m_camera;
        math::transform3D transform = camera.m_transform;
        transform.set_position_z(-transform.get_position().z);
        transform.update_matrix();

        const math::matrix4x4f mat_view = transform.get_matrix().inverted();
        const math::matrix4x4f mat_proj = math::matrix4x4f::make_projection_RH(camera.m_fov, (float)target.get_width() / target.get_height(), camera.m_near_plane, camera.m_far_plane);

        const texture& a_texture = *mp_backend->get_textures()[0u];
        mp_pipeline->set_texture(commandlist, "_texture", a_texture);
        mp_pipeline->set_constants(commandlist, "_perframe_ps", m_ps_constants);

        graphics::resource* index_buffer;
        graphics::resource* vertex_buffer;

        // draw all meshes
        for (const string& mesh_name : mp_backend->get_mesh_names())
        {
            influx_assert(mp_backend->get_mesh_buffers(mesh_name, vertex_buffer, index_buffer));

            // gather instances
            vector<gpu_instance_data> instances{};
            instances.reserve(scene.m_meshes.size());
            for (const mesh_instance& instance : scene.m_meshes)
            {
                if (instance.m_name == mesh_name)
                {
                    gpu_instance_data instance_data{};
                    instance_data.m_transform = instance.m_transform;
                    instance_data.m_colour = instance.m_per_instance_colour;
                    instances.push_back(instance_data);
                }
            }

            // render instances
            if (instances.size() > 0u)
            {
                m_vs_constants.m_mvp = instances[0u].m_transform * mat_view * mat_proj;
                mp_pipeline->set_constants(commandlist, "_perframe_vs", m_vs_constants);

                const uint32 num_vertices = (uint32)vertex_buffer->get_bytesize() / (uint32)vertex_buffer->get_bytestride();
                const uint32 num_indices = (uint32)index_buffer->get_bytesize() / (uint32)index_buffer->get_bytestride();

                commandlist->set_indexbuffer(index_buffer);
                commandlist->set_vertexbuffer(vertex_buffer);
                commandlist->draw_indexed(
                {
                    .m_num_indexes_per_instance = num_indices,
                    .m_num_instances = (uint32)instances.size(),
                    .m_start_index = 0u,
                    .m_start_vertex = 0,
                    .m_start_instance = 0u
                });
            }
        }
    }
}