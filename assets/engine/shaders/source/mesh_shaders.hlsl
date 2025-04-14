// source:
// https://github.com/chaoticbob/GraphicsExperiments/blob/main/assets/projects/111_mesh_shader_meshlets/shaders.hlsl

struct mesh_output
{
    float4 position : SV_POSITION;
    float3 color    : COLOR;
};

struct constants
{
    float4x4 mat_vp;
    float3 light_direction;
};
ConstantBuffer<constants> g_constants : register(b0);


// var indexData = new short[]
// {
//     0, 1, 2, // Side 0
//         2, 1, 3,
//         4, 0, 6, // Side 1
//         6, 0, 2,
//         7, 5, 6, // Side 2
//         6, 5, 4,
//         3, 1, 7, // Side 3 
//         7, 1, 5,
//         4, 5, 0, // Side 4 
//         0, 5, 1,
//         3, 7, 2, // Side 5 
//         2, 7, 6
// };

#define NUM_CUBE_VERTICES           8
#define NUM_CUBE_INDICES            36
#define NUM_TRIANGLES_PER_CUBE      12 // NUM_CUBE_INDICES / 3
float3 get_cube_vertex(int index)
{
    if (index == 0) return float3(-1,+1,-1);
    if (index == 1) return float3(+1,+1,-1);
    if (index == 2) return float3(-1,-1,-1);
    if (index == 3) return float3(1,-1,-1);
    if (index == 4) return float3(-1,1,1);
    if (index == 5) return float3(1,1,1);
    if (index == 6) return float3(-1,-1,1);
    if (index == 7) return float3(1,-1,1);
    return float3(0, 0, 0);
}
uint get_cube_index(int index)
{
    if      (index < (3 ))  return (index % 3);
    else if (index < (6 ))  return (3 + index % 3);
    else if (index < (9 ))  return (6 + index % 3);
    else if (index < (12))  return (9 + index % 3);
    else if (index < (15))  return (12 + index % 3);
    else if (index < (18))  return (15 + index % 3);
    else if (index < (21))  return (18 + index % 3);
    else if (index < (24))  return (21 + index % 3);
    else if (index < (27))  return (24 + index % 3);
    else if (index < (30))  return (27 + index % 3);
    else if (index < (33))  return (30 + index % 3);
    else if (index < (36))  return (33 + index % 3);

    return 0;
}

#define MAX_NUM_TRIANGLES_PER_GROUP     128
#define MAX_NUM_VERTICES_PER_GROUP      256
#define MAX_NUM_CUBES_PER_GROUP         10  // MAX_NUM_TRIANGLES_PER_GROUP / NUM_TRIANGLES_PER_CUBE

#define GRID_DIMENSIONS             5       // 5x5x5
#define TOTAL_NUM_CUBES             125     // 5x5x5
#define TOTAL_NUM_TRIANGLES         1500    // TOTAL_NUM_CUBES x NUM_TRIANGLES_PER_CUBE

float3 get_grid_position(int x, int y, int z)
{
    float offset = 1.0f / GRID_DIMENSIONS;
    return float3(x * offset, y * offset, z * offset);
}

[outputtopology("triangle")]
[numthreads(128, 1, 1)]
[shader("mesh")]
void main_ms(
    uint gid : SV_GroupID,
    uint tid : SV_GroupThreadID,
    out indices  uint3      triangles[64],  // max 128 per group
    out vertices mesh_output vertices[192]) // max 256 per group
{
    float2 g_window_size = float2(640.0, 480.0);
    if (tid >= TOTAL_NUM_TRIANGLES) return;

    // Must be called before writing the geometry output
    SetMeshOutputCounts(TOTAL_NUM_TRIANGLES * 3, TOTAL_NUM_TRIANGLES); // 3 vertices, 1 primitive

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

    float4 group_offset = float4(0.01f, 0.0f, 0.0f, 0.0f) * gid;
    vertices[vtx_base + 0].position = float4(p0, 0.0, 1.0) + group_offset;
    vertices[vtx_base + 1].position = float4(p1, 0.0, 1.0) + group_offset;
    vertices[vtx_base + 2].position = float4(p2, 0.0, 1.0) + group_offset;

    // apply viewprojection
    vertices[vtx_base + 0] = mul(g_constants.mat_vp, float4(vertices[vtx_base + 0].position.xyz, 1.0f));
    vertices[vtx_base + 1] = mul(g_constants.mat_vp, float4(vertices[vtx_base + 1].position.xyz, 1.0f));
    vertices[vtx_base + 2] = mul(g_constants.mat_vp, float4(vertices[vtx_base + 2].position.xyz, 1.0f));

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