
float4x4 gMatWorldViewProj;

float4 VertexMain( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}

float4 PixelMain() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}