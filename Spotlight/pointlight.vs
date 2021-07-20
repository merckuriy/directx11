cbuffer cbPerObject
{
	float4x4 WVP;
	float4x4 World;
};

struct VertexInputType
{
	float4 pos : POSITION;
	float2 tex : TEXCOORD;
	float3 normal : NORMAL;
};

struct PixelInputType
{
	float4 pos : SV_POSITION;
	float4 worldPos : POSITION;
	float2 tex : TEXCOORD;
	float3 normal : NORMAL;
};


PixelInputType VS(VertexInputType input)
{
	PixelInputType output;

	output.pos = mul(input.pos, WVP);
	output.worldPos = mul(input.pos, World);
	output.tex = input.tex;
	output.normal = mul(input.normal, World);

	return output;
}