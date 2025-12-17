// basepass.slang
module basepass;
import renderer;

ConstantBuffer<per_draw> g_per_draw;
ConstantBuffer<per_view> g_per_view;
ConstantBuffer<per_material> g_per_material;
StructuredBuffer<per_instance> g_instance_buffer;

[shader("vertex")]
ps_input main_vs(vs_input input)
{
    return basepass_vs(
        input,
        0, // vtx_id
        0, // inst_id
        g_per_draw,
        g_per_view,
        g_per_material,
        g_instance_buffer
    );
}

[shader("fragment")]
ps_output main_ps(ps_input input)
{
    return basepass_ps(
        input,
        {},
        {});
}