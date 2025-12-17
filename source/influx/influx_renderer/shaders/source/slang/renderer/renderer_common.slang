// renderer_common.slang
implementing renderer;

// typealias uint4 = vector<uint, 4>;

void foo() {

}

// quad rendering
float4 make_quad_ndc_pos(float2 uv)
{
    float2 ndc_pos = (uv * 2.0f) - float2(1, 1);
    return float4(ndc_pos.xy, 0.0f, 1.0f);
} 

// misc
float3 hash(uint3 x)
{
    //https://www.shadertoy.com/view/4lXyWN

    //multiply large prime value first.
    uint k = 1103515245U;
    x *= k;
    //mix x, y, z values.
    //Without shift operator, x, y and z value become same value.
    x = ((x >> 2u) ^ (x.yzx >> 1u) ^ x.zxy) * k;

    return (float3(x) * (1.0 / float(0xffffffffU)));
}

// snap the very low normal values (packing could otherwise mess them up)
float3 snap_normal(float3 normal)
{
    float epsilon = 5.96e-4;
    if (abs(normal.x) < epsilon) normal.x = 0;
    if (abs(normal.y) < epsilon) normal.y = 0;
    if (abs(normal.z) < epsilon) normal.z = 0;
    return normalize(normal);
}

// lights
struct pointlight_data
{
    float4 position;
    float4 colour;
    float4 attenuation;
};
struct spotlight_data
{
    float4 position;
};
struct dirlight_data
{
    float4 colour;
};

// packing
// f3 -> uint (10b prec)
uint f3_uint(float3 f3)
{
    uint r = (uint)(f3.x * 1023.0); // 10-bit max value is 1024
    uint g = (uint)(f3.y * 1023.0); // 10-bit max value is 1024
    uint b = (uint)(f3.z * 1023.0); // 10-bit max value is 1024
    return (r) | (g << 10) | (b << 20); // Pack into a single uint
}
float3 uint_f3(uint packed)
{
    const float scale = 1.0f / 1023.0;
    uint r = (packed      ) & 0x000000003FF;
    uint g = (packed >> 10) & 0x000000003FF;
    uint b = (packed >> 20) & 0x000000003FF;
    return float3(r, g, b) * scale;
}

// 3 x f3 -> uint4 => 96/128bit
uint4 f3_u4(float3 f0, float3 f1, float3 f2, uint extra)
{
    return uint4( f3_uint(f0), f3_uint(f1), f3_uint(f2), extra );
}

void u4_f3s(uint4 value, out float3 f0, out float3 f1, out float3 f2, out float3 extra)
{
    f0 = uint_f3(value.x);
    f1 = uint_f3(value.y);
    f2 = uint_f3(value.z);
    extra = uint_f3(value.w);
}

float2 pack_normal(float3 normal)
{
    float2 result;
    result.xy = normalize(normal.xy) * sqrt(normal.z * 0.5f + 0.5f);
    return float2(normal.x, normal.y);
}

float3 unpack_normal(float2 packed)
{
    float3 result;
    result.x = packed.x;
    result.y = packed.y;
    result.z = sqrt(1 - packed.x * packed.x - packed.y * packed.y);
    return result;
}

uint f2_u(float2 value)
{
    uint x = (uint) (value.x * 65535.0) & 0xFFFF;
    uint y = (uint) (value.y * 65535.0) & 0xFFFF;
    return (y << 16) | x;
}

float2 u_f2(uint packed)
{
    uint x = (packed            & 0xFFFF);
    uint y = ((packed >> 16)    & 0xFFFF);
    return float2(x, y) / 65535.0;
}