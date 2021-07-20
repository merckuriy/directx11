cbuffer ConstantBuffer
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 vLightDir[2];
	float4 vLightColor[2];
	float4 vOutputColor;
};

struct VS_INPUT
{
	float4 Pos : POSITION;
	float3 Norm : NORMAL;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float3 Norm : TEXCOORD0;
};


PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul(input.Pos, World);
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);
	output.Norm = mul(input.Norm, World); //Вычисляем нормаль.

	return output; 
}


float4 PS(PS_INPUT input) : SV_Target
{
	float4 finalColor = vOutputColor;
	//Нормируем вектор нормали (длина равна 1).
	input.Norm = normalize(input.Norm);

	//Складываем цвета от 2-х источников.
	for(int i = 0; i < 2; i++){
		//saturate: если x<0, то вернёт 0, если x>1, то 1, иначе x - rgb цвет.
		//dot: скалярное произведение 2-х векторов (в результате будет число). 
		//Скалярное произведение с углом > 90градусов меньше 0, поэтому цвет не падает.
		finalColor += saturate(dot((float3)vLightDir[i], input.Norm) * vLightColor[i]);
	}

	return finalColor;
}

float4 PSSolid(PS_INPUT input) : SV_Target
{
	return vOutputColor;
}