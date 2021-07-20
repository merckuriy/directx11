cbuffer MatrixBuffer
{
	matrix world;
	matrix wvp;
	matrix wvplight;
};

cbuffer LightBuffer2
{
	float3 lightPosition;
	float pad;
};

struct VertexInputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 lightViewPosition : TEXCOORD1;
	float3 lightPos : TEXCOORD2;
};

PixelInputType VS(VertexInputType input)
{
	PixelInputType output;
        
	input.position.w = 1.0f;

	output.position = mul(input.position, wvp); 
	output.lightViewPosition = mul(input.position, wvplight);

	output.tex = input.tex;
	output.normal = normalize(mul(input.normal, (float3x3)world));

	// Вычисление позиции вершины в мире
	float4 worldPosition = mul(input.position, world);

	// Позиции света относительно вершины
	output.lightPos = normalize(lightPosition.xyz - worldPosition.xyz);

	return output;
}