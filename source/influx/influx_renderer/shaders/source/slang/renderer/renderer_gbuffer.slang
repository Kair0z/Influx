// renderer_gbuffer.slang
implementing renderer;

struct gbuffer
{
    uint4 albedo_emmisive_pbr   : SV_TARGET0; // rgb8 x 4 = 96/128bit
    uint normal                 : SV_TARGET1; // rg8  x 1 = 16/32bit
    uint depth_stencil          : SV_TARGET2; // d24s8    = 32/32bit
    
    [mutating]
    void set_normal(float3 norm) {
        float2 packed_normal = pack_normal(norm);
        normal = f2_u(packed_normal);
    }
    
    float3 get_normal() {
        return normalize(unpack_normal(u_f2(normal)));
    }
    
    [mutating]
    void set_albedo(float3 albedo) {
        albedo_emmisive_pbr.x = f3_uint(albedo);
    }

    float3 get_albedo() {
        uint packed_albedo = albedo_emmisive_pbr.x;
        return uint_f3(packed_albedo);
    }

    [mutating]
    void set_emmisive(float3 emmisive) {
        albedo_emmisive_pbr.y = f3_uint(emmisive);
    }

    float3 get_emmisive() {
        return uint_f3(albedo_emmisive_pbr.y);
    }

    [mutating]
    void set_roughness(float roughness) {
    }

    float get_roughness() {
        return 0.0f;
    }

    [mutating]
    void set_ao(float ao) {

    }

    float get_ao() {
        return 0.0f;
    }

    [mutating]
    void set_metallic(float metallic) {

    }

    float get_metallic() {
        return 0.0f;
    }

    [mutating]
    void set_depth(float depth) {
        depth_stencil = asuint(depth);
    }

    float get_depth() {
        return asfloat(depth_stencil);
    }

    [mutating]
    void set_stencil(uint stencil) {

    }

    uint get_stencil() {
        return 0u;
    }
};