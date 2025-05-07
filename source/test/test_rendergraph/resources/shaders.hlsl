
struct vs_input
{
    float3 m_position : SV_POSITION;
};

struct ps_input
{
    float4 m_position : SV_POSITION;
};

[shader("vertex")]
ps_input main_vs(vs_input input)
{
    ps_input result;
    result.m_position = float4(input.m_position, 1.0f);
    return result;
}

[shader("pixel")]
float4 main_ps() : SV_TARGET0
{
    return float4(0,0,1,1);
}

[shader("compute")]
[numthreads(8,1,1)]
void main_cs()
{
    // ...
}