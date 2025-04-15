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

static const float3 k_cube_verts[8] = {
        float3(0,0,0), float3(1,0,0), float3(1,1,0), float3(0,1,0),
        float3(0,0,1), float3(1,0,1), float3(1,1,1), float3(0,1,1)
};

static const uint3 k_cube_tris[12] = {
    {0,1,2}, {0,2,3}, {1,5,6}, {1,6,2},
    {5,4,7}, {5,7,6}, {4,0,3}, {4,3,7},
    {3,2,6}, {3,6,7}, {1,0,4}, {1,4,5}
};

#define NUM_CUBE_VERTICES           8
#define NUM_CUBE_INDICES            36
#define NUM_TRIANGLES_PER_CUBE      12 // NUM_CUBE_INDICES / 3
float3 get_cube_vertex(int index)
{
    return k_cube_verts[index];
}
uint3 get_cube_triangle(int index)
{
    return k_cube_tris[index];
}
uint get_cube_index(int index)
{
    uint3 tri = get_cube_triangle(index / 3);
    return tri[index % 3];
}

#define MAX_NUM_TRIANGLES_PER_GROUP     128
#define MAX_NUM_VERTICES_PER_GROUP      256
#define MAX_NUM_CUBES_PER_GROUP         10  // MAX_NUM_TRIANGLES_PER_GROUP / NUM_TRIANGLES_PER_CUBE

#define GRID_DIMENSIONS             5       // 5x5x5
#define TOTAL_NUM_CUBES             125     // 5x5x5
#define TOTAL_NUM_TRIANGLES         1500    // TOTAL_NUM_CUBES x NUM_TRIANGLES_PER_CUBE
#define NUM_THREADS                 128

float3 get_grid_position(int x, int y, int z)
{
    float offset = 10.0f / GRID_DIMENSIONS;
    return float3(x * offset, y * offset, z * offset);
}

// 1 thread -> 1 cube -> 12 triangles && 8 vertices
// 1 group -> 10 threads -> 10 cubes (max hardware limit)
[outputtopology("triangle")]
[numthreads(MAX_NUM_CUBES_PER_GROUP, 1, 1)]
[shader("mesh")]
void main_ms(
    uint gid : SV_GroupID,
    uint gtid : SV_GroupThreadID,
    out indices  uint3      triangles[MAX_NUM_TRIANGLES_PER_GROUP],     // max 128 per group
    out vertices mesh_output vertices[MAX_NUM_VERTICES_PER_GROUP])      // max 256 per group
{
    float2 k_window_size = float2(640.0, 480.0);
    
    uint cube_index_group = gtid.x;
    uint cube_index_global = (gid.x * MAX_NUM_CUBES_PER_GROUP) + gtid.x;
    if (cube_index_global >= TOTAL_NUM_CUBES) return;

    // get the position in the grid
    uint cube_y = cube_index_global / (GRID_DIMENSIONS * GRID_DIMENSIONS);
    uint cube_z = (cube_index_global / GRID_DIMENSIONS) % GRID_DIMENSIONS;
    uint cube_x = cube_index_global % GRID_DIMENSIONS;
    float3 grid_position = get_grid_position(cube_x, cube_y, cube_z);

    uint base_vertex        = cube_index_group * NUM_CUBE_VERTICES;
    uint base_triangle      = cube_index_group * NUM_TRIANGLES_PER_CUBE;

    // Must be called before writing the geometry output
    SetMeshOutputCounts(MAX_NUM_CUBES_PER_GROUP * NUM_CUBE_VERTICES, MAX_NUM_CUBES_PER_GROUP * NUM_TRIANGLES_PER_CUBE);

    // each thread outputs NUM_CUBE_VERTICES (12)
    for (uint i = 0; i < NUM_CUBE_VERTICES; ++i)
    {
        float4 position = float4(grid_position.xyz + get_cube_vertex(i), 1.0f);
        position = mul(g_constants.mat_vp, float4(position.xyz, 1.0f));

        position.z = -position.z;
        position.w = -position.w;

        vertices[base_vertex + i].position = position;
        vertices[base_vertex + i].color = float3(1, 1, 1);
    }

    // each thread outputs NUM_TRIANGLES_PER_CUBE (12)
    for (uint t = 0; t < NUM_TRIANGLES_PER_CUBE; ++t)
    {
        uint triangle_index = base_triangle + t;
        triangles[triangle_index] = get_cube_triangle(triangle_index);
    }
}

[shader("pixel")]
float4 main_ps(mesh_output input) : SV_TARGET
{
    return float4(input.color, 1);
}