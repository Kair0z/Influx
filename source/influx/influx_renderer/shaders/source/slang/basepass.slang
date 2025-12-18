// basepass.slang
module basepass;
import renderer;

// parameter blocks
struct draw_block
{
    ConstantBuffer<per_draw>        m_constants;
    StructuredBuffer<per_instance>  m_instance_buffer;

    Texture2D m_tex_albedo;
    SamplerState m_samp_albedo;
}
struct view_block
{
    ConstantBuffer<per_view> m_constants;
}
struct mat_block
{
    ConstantBuffer<per_material> m_constants;
}
ParameterBlock<view_block> g_view_block;
ParameterBlock<draw_block> g_draw_block;
ParameterBlock<mat_block> g_mat_block;

[shader("vertex")]
ps_input main_vs(vs_input input)
{
    return basepass_vs(
        input,
        0, // vtx_id
        0, // inst_id
        g_draw_block.m_constants,
        g_view_block.m_constants,
        g_mat_block.m_constants,
        g_draw_block.m_instance_buffer
    );
}

[shader("fragment")]
ps_output main_ps(ps_input input)
{
    return basepass_ps(
        input,
        g_draw_block.m_tex_albedo,
        g_draw_block.m_samp_albedo);
}