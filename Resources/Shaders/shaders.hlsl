
Texture2D<float4> g_texture         : register(t0);
SamplerState g_texture_sampler      : register(s0);

cbuffer view_constant_buffer : register(b0)
{
    column_major float4x4 mat_vp;
};

struct Vertex
{
    float3 position : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;

    column_major float4x4 instance_transform : INSTANCE_DATA;
    float4 instance_color : INSTANCE_COLOR;
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
    float4x4 wvp = vertex.instance_transform * mat_vp;
    output.position = mul(float4(vertex.position, 1.0f), wvp);
    output.color = vertex.instance_color;
    output.uv = vertex.uv;
    return output;
}

[shader("pixel")]
float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}
