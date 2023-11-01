
cbuffer view_constant_buffer : register(b0)
{
    float4x4 mat_wvp;
};

cbuffer draw_constant_buffer : register(b1)
{
    float4x4 mat_transform;
};

struct Vertex
{
    float3 position : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float3 normal : NORMAL;
    float2 uv : UV;
};

[shader("vertex")]
PSInput VSMain(Vertex vertex)
{
    PSInput output;
    float4x4 wvp = mul(mat_wvp, mat_transform);
    output.position =  mul(float4(vertex.position, 1.0f), wvp);
    output.color = vertex.color;
    return output;
}

[shader("pixel")]
float4 PSMain(PSInput input) : SV_TARGET
{
    return float4(0, 1, 0, 1);
}
