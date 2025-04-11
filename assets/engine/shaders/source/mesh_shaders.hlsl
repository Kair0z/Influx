// source:
// https://github.com/chaoticbob/GraphicsExperiments/blob/main/assets/projects/111_mesh_shader_meshlets/shaders.hlsl

struct mesh_output
{
    float4 position : SV_POSITION;
    float3 color    : COLOR;
};

[outputtopology("triangle")]
[numthreads(128, 1, 1)]
[shader("mesh")]
void main_ms(
    uint tid : SV_GroupThreadID,
    out indices  uint3      triangles[64],  // max 128 per group
    out vertices mesh_output vertices[192]) // max 256 per group
{
    float2 g_window_size = float2(640.0, 480.0);
    uint g_num_triangles = 64;

    if (tid >= g_num_triangles) return;

    // Must be called before writing the geometry output
    SetMeshOutputCounts(g_num_triangles * 3, g_num_triangles); // 3 vertices, 1 primitive

    // Grid dimensions (change for aspect ratio)
    uint grid_w = 16;
    uint grid_h = g_num_triangles / grid_w;
    float2 cell_size = float2(2.0 / grid_w, 2.0 / grid_h);
    uint x = tid % grid_w;
    uint y = tid / grid_w;

    float2 origin = float2(-1.0 + x * cell_size.x, -1.0 + y * cell_size.y);

    // Simple upright triangle in cell
    float2 p0 = origin + float2(0.1, 0.2) * cell_size;
    float2 p1 = origin + float2(0.9, 0.2) * cell_size;
    float2 p2 = origin + float2(0.5, 0.8) * cell_size;

    uint vtx_base = tid * 3;
    triangles[tid] = uint3(vtx_base, vtx_base + 1, vtx_base + 2);

    vertices[vtx_base + 0].position = float4(p0, 0.0, 1.0);
    vertices[vtx_base + 1].position = float4(p1, 0.0, 1.0);
    vertices[vtx_base + 2].position = float4(p2, 0.0, 1.0);

    float3 base_color = float3(
        float(x) / grid_w,
        float(y) / grid_h,
        1.0 - float(x + y) / (grid_w + grid_h)
        );

    vertices[vtx_base + 0].color = base_color;
    vertices[vtx_base + 1].color = base_color;
    vertices[vtx_base + 2].color = base_color;
}

[shader("pixel")]
float4 main_ps(mesh_output input) : SV_TARGET
{
    return float4(input.color, 1);
}