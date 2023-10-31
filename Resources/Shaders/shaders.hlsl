
cbuffer view_constant_buffer : register(b0)
{
    float4x4 mat_wvp;
};

struct Vertex
{
    float3 position : SV_POSITION;
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
    output.position = mul(vertex.position, mat_wvp);
    output.color = vertex.color;
    return output;
}

[shader("pixel")]
float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}
