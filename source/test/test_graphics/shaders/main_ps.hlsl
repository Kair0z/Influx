struct vs_output
{
	float4 m_position : SV_POSITION;
	[[vk::location(0)]] float4 m_colour: COLOR0;
};
float4 main_ps(vs_output input) : SV_TARGET0
{
	return 0;
}