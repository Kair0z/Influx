


#if 0
// BINDLESS TEXTURES:
// https://alextardif.com/Bindless.html
// It's fairly common with bindless tables to name the HLSL spaces here to make it clear what the space is being used for.
#define _myTex2DSpace space1
Texture2D   _Texture2DTable[]  : register(t0, _myTex2DSpace);

// GOODBYE, Vertex Input Layouts
#define _bufferSpace space2
ByteAddressBuffer _BufferTable[] : register(t0, _bufferSpace);

struct Vertex
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : UV;
}

// CONSTANTS:
cbuffer _ConstantsPerScene
{
    float4x4    WVP;
};

cbuffer _ConstantsPerDraw
{
    float4x4    transform;
    uint        textureIndex; // can index into _Texture2DTable

    // Vertex buffer indexing:
    uint vertexBufferOffset;
    uint vertexBufferIndex;
}
#endif

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : UV;
};

PSInput VSMain(float3 position : POSITION, float4 color : COLOR, float3 normal : NORMAL, float2 uv : UV)
{
    //uint index = _ConstantsPerDraw.vertexBufferIndex;
    //uint offset = _ConstantsPerDraw.vertexOffset;
    //
    //Vertex vertex = _BufferTable[index].Load<MyVertexStructure>((vertexOffset + vertexId) * sizeof(MyVertexStructure));

    PSInput result;

    result.position = float4(position, 1.0f);
    result.color = color;
    result.normal = normal;
    result.uv = uv;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    // uint textureIndex = _ConstantsPerDraw.textureIndex;
    // 
    // float4 s = Texture2DTable[textureIndex].Sample(AlbedoSampler, uv0);
    return input.color;
}
