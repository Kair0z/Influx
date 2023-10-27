

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
    PSInput result;
    result.position = float4(vertex.position.xyz, 1.0f);
    result.color = vertex.color;
    return result;
}

[shader("pixel")]
float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}
