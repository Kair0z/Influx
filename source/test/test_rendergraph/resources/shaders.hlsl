struct vs_input
{
    float3 m_position   : SV_POSITION;
    float2 m_uv         : TEXCOORD0;
};

struct ps_input
{
    float4 m_position   : SV_POSITION;
    float2 m_uv         : TEXCOORD0;
};

[shader("vertex")]
ps_input main_vs(vs_input input)
{
    ps_input result;
    result.m_position   = float4(input.m_position, 1.0f);
    result.m_uv         = input.m_uv;
    return result;
}

[shader("pixel")]
float4 main_ps(ps_input input) : SV_TARGET0
{
    return float4(input.m_uv.x, input.m_uv.y,0,1);
}

[shader("compute")]
[numthreads(8,1,1)]
void main_cs()
{
    // ...
}