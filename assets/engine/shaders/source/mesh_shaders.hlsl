// source:
// https://github.com/chaoticbob/GraphicsExperiments/blob/main/assets/projects/111_mesh_shader_meshlets/shaders.hlsl

// mesh output -> pixelshader
struct mesh_output
{
    float4 position : SV_POSITION;
    float3 color    : COLOR;
};

// root constants
struct constants
{
    float4x4 mat_vp;
    float3 light_direction;
};
ConstantBuffer<constants> g_constants : register(b0);

// hard-coded-cubes
#define NUM_CUBE_VERTICES           8
#define NUM_TRIANGLES_PER_CUBE      12
#define NUM_CUBE_INDICES            NUM_TRIANGLES_PER_CUBE * 3

static const float3 k_cube_verts[NUM_CUBE_VERTICES] = {
        float3(0,0,0), float3(1,0,0), float3(1,1,0), float3(0,1,0),
        float3(0,0,1), float3(1,0,1), float3(1,1,1), float3(0,1,1)
};
float3 get_cube_vertex(int index)
{
    return k_cube_verts[index % NUM_CUBE_VERTICES];
}

static const uint3 k_cube_tris[NUM_TRIANGLES_PER_CUBE] = {
    {0,1,2}, {0,2,3}, {1,5,6}, {1,6,2},
    {5,4,7}, {5,7,6}, {4,0,3}, {4,3,7},
    {3,2,6}, {3,6,7}, {1,0,4}, {1,4,5}
};
uint3 get_cube_triangle(uint index)
{
    return k_cube_tris[index % NUM_TRIANGLES_PER_CUBE];
}

// hardware limits:
#define MAX_NUM_TRIANGLES_PER_GROUP     128
#define MAX_NUM_VERTICES_PER_GROUP      256
#define MAX_NUM_CUBES_PER_GROUP         10  // MAX_NUM_TRIANGLES_PER_GROUP / NUM_TRIANGLES_PER_CUBE

// settings:
#define GRID_DIMENSIONS             5
#define TOTAL_NUM_CUBES             GRID_DIMENSIONS * GRID_DIMENSIONS * GRID_DIMENSIONS
#define TOTAL_NUM_TRIANGLES         TOTAL_NUM_CUBES * NUM_TRIANGLES_PER_CUBE

uint3 linear_to_grid(uint index)
{
    uint3 grid_position;
    grid_position.z = index % GRID_DIMENSIONS;
    grid_position.y = (index / GRID_DIMENSIONS) % GRID_DIMENSIONS;
    grid_position.x = index / (GRID_DIMENSIONS * GRID_DIMENSIONS);
    return grid_position;
}
float3 get_grid_position(int x, int y, int z)
{
    float padding = 1.5f;
    return float3(x, y, z) * padding;
}

// 1 thread -> 1 cube -> 12 triangles && 8 vertices
// 1 group -> 10 threads -> 10 cubes (max hardware limit)
[outputtopology("triangle")]
[numthreads(MAX_NUM_CUBES_PER_GROUP, 1, 1)]
[shader("mesh")]
void main_ms(
    uint3 gid : SV_GroupID,
    uint3 gtid : SV_GroupThreadID,
    out indices  uint3      triangles[MAX_NUM_TRIANGLES_PER_GROUP],     // max 128 per group
    out vertices mesh_output vertices[MAX_NUM_VERTICES_PER_GROUP])      // max 256 per group
{
    // cube_index_group -> index of the cube in this group
    // cube_index_global -> index of the cube in total
    uint cube_index_group = gtid.x;
    uint cube_index_global = (gid.x * MAX_NUM_CUBES_PER_GROUP) + gtid.x;

    // avoid over-writing
    if (cube_index_global   >= TOTAL_NUM_CUBES) return;
    if (cube_index_group    >= MAX_NUM_CUBES_PER_GROUP) return;

    // transform the global index to a grid position
    uint3 grid_index        = linear_to_grid(cube_index_global);
    float3 grid_position    = get_grid_position(grid_index.x, grid_index.y, grid_index.z);

    // base vertex/triangle of this cube
    uint base_vertex        = cube_index_group * NUM_CUBE_VERTICES;
    uint base_triangle      = cube_index_group * NUM_TRIANGLES_PER_CUBE;

    // Must be called before writing the geometry output
    SetMeshOutputCounts(MAX_NUM_CUBES_PER_GROUP * NUM_CUBE_VERTICES, MAX_NUM_CUBES_PER_GROUP * NUM_TRIANGLES_PER_CUBE);

    // each thread outputs NUM_CUBE_VERTICES (8)
    for (uint i = 0; i < NUM_CUBE_VERTICES; ++i)
    {
        // transform to camera
        float4 position = float4(grid_position.xyz + get_cube_vertex(i).xyz, 1.0f);
        position = mul(g_constants.mat_vp, float4(position.xyz, 1.0f));

        // todo: fix this!
        float other = -position.z;
        position.z = -position.w;
        position.w = other;

        // color vertices
        vertices[base_vertex + i].position = position;
        vertices[base_vertex + i].color.xyz = float3(0, 0, 0);
        vertices[base_vertex + i].color[i % 3] = 1.0f;
    }

    // each thread outputs NUM_TRIANGLES_PER_CUBE (12)
    for (uint t = 0; t < NUM_TRIANGLES_PER_CUBE; ++t)
    {
        uint triangle_idx = base_triangle + t;
        uint3 cube_triangle = get_cube_triangle(triangle_idx);
        cube_triangle.x += base_vertex;
        cube_triangle.y += base_vertex;
        cube_triangle.z += base_vertex;
        triangles[triangle_idx] = cube_triangle;
    }
}

[shader("pixel")]
float4 main_ps(mesh_output input) : SV_TARGET
{
    return float4(input.color, 1);
}