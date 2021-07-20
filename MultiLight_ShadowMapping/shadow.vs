cbuffer MatrixBuffer
{
	matrix world;
	matrix wvp;
	matrix wvplight;
	matrix wvplight2;
};

cbuffer LightBuffer2
{
	float3 lightPosition;
	float pad;
	float3 lightPosition2;
	float pad2;
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
	float4 lightViewPosition2 : TEXCOORD3;
	float3 lightPos2 : TEXCOORD4;
};

PixelInputType VS(VertexInputType input)
{
	PixelInputType output;
        
	input.position.w = 1.0f;

	output.position = mul(input.position, wvp); 
	output.lightViewPosition = mul(input.position, wvplight);
	output.lightViewPosition2 = mul(input.position, wvplight2);

	output.tex = input.tex;
	output.normal = normalize(mul(input.normal, (float3x3)world));

	// Вычисление позиции вершины в мире
	float4 worldPosition = mul(input.position, world);

	// Позиции света относительно вершины
	output.lightPos = normalize(lightPosition.xyz - worldPosition.xyz);
	output.lightPos2 = normalize(lightPosition2.xyz - worldPosition.xyz);

	return output;
}