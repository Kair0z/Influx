#ifndef _INFLUX_COMMON_HF
#define _INFLUX_COMMON_HF

#define INFLUX_DEFAULT_ROOTSIGNATURE \
	"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \

    // Root constants
	"RootConstants(num32BitConstants=12, b999), " \
	
    // Descriptor tables
	"DescriptorTable( " \
		"CBV(b3, numDescriptors = 11, flags = DATA_STATIC_WHILE_SET_AT_EXECUTE)," \
		"SRV(t0, numDescriptors = 16, flags = DESCRIPTORS_VOLATILE | DATA_STATIC_WHILE_SET_AT_EXECUTE)," \
		"UAV(u0, numDescriptors = 16, flags = DESCRIPTORS_VOLATILE | DATA_STATIC_WHILE_SET_AT_EXECUTE)" \
	")," \

    // Static samplers
	"StaticSampler(s100, addressU = TEXTURE_ADDRESS_CLAMP, addressV = TEXTURE_ADDRESS_CLAMP, addressW = TEXTURE_ADDRESS_CLAMP, filter = FILTER_MIN_MAG_MIP_LINEAR),"

#endif // _INFLUX_COMMON_HF

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

    uint sampler_slotidx;
};
// constant buffers
ConstantBuffer<vs_constants> _perframe_vs : register(b0);
ConstantBuffer<ps_constants> _perframe_ps : register(b0);

// root descriptors

// root samplers
SamplerState g_sampler : register(s0);

// descriptor tables
Texture2D g_textures[4] : register(t0);