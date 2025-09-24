struct vsinput
{
    float3 m_position : POSITION0;   // position semantic
    float4 m_colour   : COLOR0;      // color semantic
};
struct vsoutput
{
    float4 m_position : SV_Position; // required for vertex shader
    float4 m_colour   : COLOR0;      // color output
};
vsoutput main_vs(vsinput input)
{
    vsoutput output;
    output.m_position = float4(input.m_position, 1.0);
    output.m_colour   = input.m_colour;
    return output;
}