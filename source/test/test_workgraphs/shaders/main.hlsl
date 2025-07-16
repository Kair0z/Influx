// https://www.youtube.com/watch?v=n3rcbBdy-BA

struct Line
{
    float a, b;
};

[Shader("node")]
[NodeLaunch("thread")]
void launch(
    ThreadNodeInputRecord<Line> input,
    [MaxRecords(1)] NodeOutput<Line> line_mesh_node
)
{
    ThreadNodeOutputRecords<Line> d = 
        line_mesh_node.GetThreadNodeOutputRecords(1);
    d.Get(0) = input.Get(); 
    d.OutputComplete();
}

// MESH NODE
[Shader("node")]
[NodeLaunch("mesh")]
[NodeId("LineMeshNode", 0)]
[NodeDispatchGrid(1,1,1)][NumThreads(1,1,1)]
[OutputTopology("line")]
void ms_main(
    DispatchNodeInputRecord<Line> input,
    out indices uint2 lines[1],
    out vertices float4 vertices[2] : SV_POSITION
)
{
    SetMeshOutputCounts(2,1);
    lines[0]    = uint2(0,1);
    vertices[0] = float4(input.Get().a, 0, 1);
    vertices[1] = float4(input.Get().b, 0, 1);
}

float4 ps_main() : SV_TARGET
{
    return float4(0,0,0,1);
}