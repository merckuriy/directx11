cbuffer MatrixBuffer
{
	float4x4 wvp;
};

struct VertexInputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

PixelInputType VS(VertexInputType input)
{
	PixelInputType output;
	output.position = mul(input.position, wvp);
	output.tex = input.tex;
	return output;
}