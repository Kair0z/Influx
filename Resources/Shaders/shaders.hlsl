struct vs_input
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
};

struct ps_input
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

struct vs_constants
{
    float4x4 mat_mvp;
};

struct ps_constants
{
    uint texture_index;
};


ConstantBuffer<vs_constants> _perframe_vs : register(b0);

[shader("vertex")]
ps_input VSMain ( vs_input input )
{
    ps_input output = (ps_input)0;
    output.position = mul ( _perframe_vs.mat_mvp, float4 ( input.position, 1.0f ) );
    output.texcoord = input.texcoord;
    return output;
}

ConstantBuffer<ps_constants> _perframe_ps : register(b0);
Texture2D _texture[128] : register(t0);
SamplerState _sampler : register(s0);

[shader("pixel")]
float4 PSMain ( ps_input input ) : SV_TARGET
{
    return float4(0.0f, 1.0f, 0.0f, 1.0f);
    // return _texture[_perframe_ps.texture_index].Sample(_sampler, input.texcoord);
}