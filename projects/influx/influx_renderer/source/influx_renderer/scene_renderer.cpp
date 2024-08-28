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
#include "influx_graphics/device.h"

namespace influx::renderer
{
    scene_renderer::scene_renderer(renderer_backend* backend, graphics::device* device, pipeline* pipeline)
        : mp_pipeline{ pipeline }
        , mp_backend{ backend }
        , mp_device{ device }
    {
        const uint32 num_descriptors = 4u;

        graphics::descriptor_heap::create_args args{};
        args.m_capacity = num_descriptors;
        args.m_shader_visible = true;
        args.m_type = graphics::e_descriptor_heap_type::srv;
        mp_srv_heap_gpu = device->create_descriptor_heap(args);

        // allocate 4 gpu descriptors
        m_srv_gpu_range = mp_srv_heap_gpu->allocate_range_gpu(num_descriptors);
    }

    void scene_renderer::render(graphics::command_list* commandlist, const scene& scene, const target& target)
    {
        // try to fish the scene pipeline from the manager...
        mp_pipeline = mp_backend->get_pipeline_manager()->get_scene_pipeline();
        if (mp_pipeline == nullptr)
        {
            return;
        }

        // set generic pipeline state (pipeline, rootsignature, primitive topo, ...)
        mp_pipeline->set_state(commandlist);

        // update constants
        m_ps_constants.m_albedo_slotidx = 0u;
        m_ps_constants.m_normals_slotidx = 1u;
        m_ps_constants.m_other_slotidx = 2u;

        // invert camera :) (engine is right handed, but d3d12 is left handed)
        const camera& camera = scene.m_camera;
        math::transform3D transform = camera.m_transform;
        transform.set_position_z(-transform.get_position().z);
        transform.update_matrix();

        const math::matrix4x4f mat_view = transform.get_matrix().inverted();
        const math::matrix4x4f mat_proj = math::matrix4x4f::make_projection_RH(camera.m_fov, (float)target.get_width() / target.get_height(), camera.m_near_plane, camera.m_far_plane);

        const texture& a_texture = *mp_backend->get_textures()[0u];
        mp_pipeline->set_texture(commandlist, "_textures", a_texture);
        mp_pipeline->set_constants(commandlist, "_perframe_ps", m_ps_constants);

        // copy cpu srv range into the gpu shader visible descriptor heap
        mp_device->copy_descriptors({}, m_srv_gpu_range, graphics::e_descriptor_heap_type::srv);
        commandlist->set(mp_srv_heap_gpu);

        graphics::resource* index_buffer;
        graphics::resource* vertex_buffer;

        // draw all meshes
        for (const string& mesh_name : mp_backend->get_mesh_names())
        {
            influx_assert(mp_backend->get_mesh_buffers(mesh_name, vertex_buffer, index_buffer));

            // gather instances per material
            umap<string, vector<gpu_instance_data>> instances_per_material{};
            for (const mesh_instance& instance : scene.m_meshes)
            {
                if (instance.m_name == mesh_name)
                {
                    // convert to gpu_instance_data
                    gpu_instance_data instance_data{};
                    instance_data.m_transform = instance.m_transform;
                    instance_data.m_colour = instance.m_per_instance_colour;
                    instances_per_material[instance.m_material_name].push_back(instance_data);
                }
            }

            // render instances per material
            for (const auto& pair : instances_per_material)
            {
                const string& material_name = pair.first;
                const vector<gpu_instance_data>& instances = pair.second;

                // find the material
                const material* material = mp_backend->get_material(material_name);

                if (instances.size() > 0u && material != nullptr)
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
}