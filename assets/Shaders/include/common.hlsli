#ifndef _INFLUX_COMMON_HF
#define _INFLUX_COMMON_HF

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

#endif