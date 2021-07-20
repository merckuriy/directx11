struct VertexInputType
{
	float3 position : POSITION;
	float4 color : COLOR;
};

struct HullInputType
{
    float3 position : POSITION;
    float4 color : COLOR;
};

HullInputType VS(VertexInputType input)
{
	HullInputType output;
	output.position = input.position;
	output.color = input.color;
	return output;
}