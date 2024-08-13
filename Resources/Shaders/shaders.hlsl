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
    uint albedo_slotidx;
	uint normals_slotidx;
	uint other_slotidx;
};

ConstantBuffer<vs_constants> _perframe_vs : register(b0);
ConstantBuffer<ps_constants> _perframe_ps : register(b0);
Texture2D _textures[128] : register(t0);
SamplerState _sampler : register(s0);

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


[shader("pixel")]
float4 PSMain ( ps_input input ) : SV_TARGET
{
    float3 lightDir = float3(-0.5,-0.5,-0.5);

    float ambient = 0.2f;
    float diffuse = max(ambient,dot(normalize(input.normal), normalize(lightDir)));

    float4 albedo = _textures[_perframe_ps.albedo_slotidx].Sample(_sampler, input.texcoord).rgba;
    float3 normal = _textures[_perframe_ps.normals_slotidx].Sample(_sampler, input.texcoord).rgb;
    float3 other = _textures[_perframe_ps.other_slotidx].Sample(_sampler, input.texcoord).rgb;

    return diffuse * albedo;
}