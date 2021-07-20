cbuffer MatrixBuffer
{
	float4x4 WVP;
};

struct VertexInputType
{
	float4 pos : POSITION;
	float2 tex : TEXCOORD0;
};

struct PixelInputType
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};

PixelInputType VS(VertexInputType input)
{
	PixelInputType output;

	output.pos = mul(input.pos, WVP);
	output.tex = input.tex;

	return output; 
}