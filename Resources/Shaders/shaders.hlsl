struct vs_input
{
    float3 position : POSITION;
    float4 colour : COLOR;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
};

struct ps_input
{
    float4 position : SV_POSITION;
    float3 worldPos : WORLD_POS;
    nointerpolation float4 colour : COLOR;
    float3 normal : NORMAL;
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

float3 hash( uint3 x )
{  
    //https://www.shadertoy.com/view/4lXyWN
    
    //multiply large prime value first.
    uint k = 1103515245U;
    x*= k;
    //mix x, y, z values.
    //Without shift operator, x, y and z value become same value.
    x = ((x>>2u)^(x.yzx>>1u)^x.zxy)*k;
 
    return (float3(x)*(1.0/float(0xffffffffU)));
}

ConstantBuffer<vs_constants> _perframe_vs : register(b0);

[shader("vertex")]
ps_input VSMain ( vs_input input, uint vID : SV_VertexID )
{
    ps_input output = (ps_input)0;
    output.position = mul ( _perframe_vs.mat_mvp, float4 ( input.position, 1.0f ) );
    output.worldPos = input.position;
    output.texcoord = input.texcoord;
    output.colour = float4(hash(uint3(vID,vID+7,vID+41)), 1.0f);
    output.normal = input.normal;
    return output;
}

ConstantBuffer<ps_constants> _perframe_ps : register(b0);
Texture2D _texture[128] : register(t0);
SamplerState _sampler : register(s0);


[shader("pixel")]
float4 PSMain ( ps_input input ) : SV_TARGET
{
    float3 dx = ddx_fine(input.worldPos);
    float3 dy = ddy_fine(input.worldPos);
    float3 n = normalize(cross(dx, dy));

    float3 cameraPos = float3(0,0,400);
    float3 lightDir = float3(-0.5,-0.5,-0.5);

    float ambient = 0.1f;
    float diffuse = max(ambient,dot(n, normalize(lightDir)));

    //float3 viewDir = normalize(input.worldPos - cameraPos);

    return diffuse * input.colour; //_texture[_perframe_ps.texture_index].Sample(_sampler, input.texcoord);
}