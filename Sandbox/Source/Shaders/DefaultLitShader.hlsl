
struct VertexInput
{
	float4 position : POSITION;
	float4 color : COLOR;
};

struct PixelInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PixelInput VertexMain(VertexInput input)
{
	PixelInput output = (PixelInput)0;
	output.position = input.position;
	output.color = input.color;
	return output;
}

float4 PixelMain(PixelInput input) : SV_TARGET
{
	return float4(input.color.rgb, 1.0f);
}