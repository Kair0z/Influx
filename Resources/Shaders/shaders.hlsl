


#if 0
// BINDLESS TEXTURES:
// https://alextardif.com/Bindless.html
// It's fairly common with bindless tables to name the HLSL spaces here to make it clear what the space is being used for.
#define _myTex2DSpace space1
Texture2D   _Texture2DTable[]  : register(t0, _myTex2DSpace);

// GOODBYE, Vertex Input Layouts
#define _bufferSpace space2
ByteAddressBuffer _BufferTable[] : register(t0, _bufferSpace);


#endif

cbuffer _ConstantsPerScene : register(b0)
{
    float4x4    WVP;
};

cbuffer _ConstantsPerDraw : register(b1)
{
    float4x4    transform;
}

struct Vertex
{
    float3 position : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : UV;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float3 normal : NORMAL;
    float2 uv : UV;
};

[shader("vertex")]
PSInput VSMain(Vertex vertex)
{
    //uint index = _ConstantsPerDraw.vertexBufferIndex;
    //uint offset = _ConstantsPerDraw.vertexOffset;
    //
    //Vertex vertex = _BufferTable[index].Load<MyVertexStructure>((vertexOffset + vertexId) * sizeof(MyVertexStructure));

    PSInput result;
    result.position = mul(float4(vertex.position, 1.0f), WVP);
    result.color = vertex.color;
    result.normal = vertex.normal;
    result.uv = vertex.uv;

    return result;
}

[shader("pixel")]
float4 PSMain(PSInput input) : SV_TARGET
{
    // uint textureIndex = _ConstantsPerDraw.textureIndex;
    // 
    // float4 s = Texture2DTable[textureIndex].Sample(AlbedoSampler, uv0);

    return input.color;
}
