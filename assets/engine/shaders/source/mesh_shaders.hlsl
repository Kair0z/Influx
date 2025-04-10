#include "include/common.hlsli"

// source:
// https://github.com/chaoticbob/GraphicsExperiments/blob/main/assets/projects/111_mesh_shader_meshlets/shaders.hlsl

struct mesh_output
{
    float4 position : SV_POSITION;
    float3 color    : COLOR;
};

[outputtopology("triangle")]
[numthreads(128, 1, 1)]
void main_ms(
    out indices  uint3      triangles[128],
    out vertices mesh_output vertices[64])
{
    // Must be called before writing the geometry output
    SetMeshOutputCounts(3, 1); // 3 vertices, 1 primitive

    triangles[0] = uint3(0, 1, 2);

    vertices[0].position = float4(-0.5, 0.5, 0.0, 1.0);
    vertices[0].color = float3(1.0, 0.0, 0.0);

    vertices[1].position = float4(0.5, 0.5, 0.0, 1.0);
    vertices[1].color = float3(0.0, 1.0, 0.0);

    vertices[2].position = float4(0.0, -0.5, 0.0, 1.0);
    vertices[2].color = float3(0.0, 0.0, 1.0);
}

float4 main_ps(mesh_output input) : SV_TARGET
{
    return float4(input.color, 1);
}