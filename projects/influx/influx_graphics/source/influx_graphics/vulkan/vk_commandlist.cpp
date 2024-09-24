#include "graphics_pch.h"

#include "influx_graphics/vulkan/vk_commandlist.h"
#include "influx_graphics/vulkan/vk_conversion.h"

#include "vk_headers.h"

namespace influx::graphics
{
    vk_commandlist::vk_commandlist(const vk::CommandBuffer& vkbuffer)
        : m_vk_commandbuffer{vkbuffer}
    {
        mp_native = &m_vk_commandbuffer;
    }

    void vk_commandlist::start(command_allocator* allocator, pipeline* init_state)
    {

    }

    void vk_commandlist::renderpass_begin(const renderpass_args& args)
    {
    }

    void vk_commandlist::renderpass_end()
    {
    }

    void vk_commandlist::draw_instanced(const draw_instanced_args& args)
    {
    }

    void vk_commandlist::draw_indexed(const draw_indexed_args& args)
    {
    }

    void vk_commandlist::set_constants(uint32 param_index, uint32 num_dwords, void* source_data)
    {
    }

    void vk_commandlist::set_indexbuffer(resource* index_buffer)
    {
    }

    void vk_commandlist::set_vertexbuffer(resource* vertex_buffer)
    {
    }

    void vk_commandlist::clear_rtv(render_target_view* view, const math::vectorf4& clear_value)
    {

    }

    void vk_commandlist::clear_dsv(depth_stencil_view* view, float clear_depth, uint32 clear_stencil)
    {
    }

    void vk_commandlist::transition_resource(resource* resource, e_resource_state before, e_resource_state after)
    {

    }

    void vk_commandlist::copy_resource(resource* source, resource* dest)
    {

    }

    void vk_commandlist::copy_texture(resource* src, resource* dest, const copy_texture_args&)
    {
    }

    void vk_commandlist::copy_buffer(resource* src, resource* dest, uint32 bytesize, const copy_buffer_args&)
    {
    }

    void vk_commandlist::set(descriptor_heap* heap)
    {
    }

    void vk_commandlist::set(render_target_view* rtv, depth_stencil_view* dsv)
    {
    }

    void vk_commandlist::set(shader_resource_view* srv, uint32 param_idx)
    {
    }

    void vk_commandlist::set(const descriptor_range& gpu_range, uint32 param_idx)
    {
    }

    void vk_commandlist::set(rootsignature* rootsig)
    {
    }

    void vk_commandlist::set(pipeline* pipeline)
    {
    }

    void vk_commandlist::set(const viewport& viewport)
    {
    }

    void vk_commandlist::set(const rect& rect)
    {
    }

    void vk_commandlist::set(e_primitive_topology topo)
    {
    }

    void vk_commandlist::end()
    {

    }
}