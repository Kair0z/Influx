struct vs_input
{
    [[vk::location(0)]] float3 m_position : POSITION0;
	[[vk::location(1)]] float4 m_colour : COLOR0;
};
struct vs_output
{
	float4 m_position : SV_POSITION;
	[[vk::location(0)]] float4 m_colour: COLOR0;
};
vs_output main_vs(vs_input input, , uint vert_index : SV_VertexID)
{
    vs_output output;
    output.m_position = float4(input.m_position, 1.0);
    output.m_colour   = input.m_colour;
    return output;
}