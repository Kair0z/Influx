struct vs_input
{
    float2 uv : TEXCOORD;
};

struct ps_input
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD;
};

[shader("vertex")]
ps_input main_vs(vs_input input)
{
    ps_input output;

    float2 ndc_pos = (input.uv * 2.0f) - float2(1, 1);
    output.position = float4(ndc_pos.xy, 0.0f, 1.0f);
    output.uv = input.uv;

    return output;
}

[shader("pixel")]
float4 main_ps(ps_input input) : SV_TARGET
{
    const float distance = length(input.uv - float2(0.5, 0.5));
    const float vignette = lerp(0.0f, 1.0f, pow(distance,2));
    return float4(1.0f, 0.0f, 0.0f, vignette);
}