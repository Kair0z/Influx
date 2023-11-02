
cbuffer view_constant_buffer : register(b0)
{
    float4x4 mat_wvp;
};

struct Vertex
{
    float3 position : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;

    row_major float4x4 inst_mat : INSTANCE_DATA;
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
    float4x4 transform = vertex.inst_mat;
    float4x4 wvp = mul(transform, mat_wvp);
    output.position =  mul(float4(vertex.position, 1.0f), wvp);
    output.color = vertex.color;
    return output;
}

[shader("pixel")]
float4 PSMain(PSInput input) : SV_TARGET
{
    return float4(0, 1, 0, 1);
}
