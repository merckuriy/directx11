cbuffer TessellationBuffer
{
	float tessellationAmount;
	float3 pad;
};

struct HullInputType
{
	float3 position : POSITION;
	float4 color : COLOR;
};

struct HullOutputType
{
	float3 position : POSITION;
	float4 color : COLOR;
};

struct ConstantOutputType
{
	//tessel. amount on each edge of a patch (заплатка, участок).
	//tri patch: u==0,v==0,w==0 tess. factors.
	float edges[3] : SV_TessFactor; 

	//tess. amount within (внутри) a patch surface.
	float inside : SV_InsideTessFactor;
};

//Patch Constant Function. Вероятно это и есть Tessellator Shader (2-ая стадия тесселяции), либо её часть
ConstantOutputType ColorPatchConstantFunction(InputPatch<HullInputType, 3> inputPatch, uint patchId : SV_PrimitiveID)
{    
	ConstantOutputType output; //patch-constant data.

	// Set the tessellation factors for the three edges of the triangle.
	output.edges[0] = tessellationAmount;
	output.edges[1] = tessellationAmount;
	output.edges[2] = tessellationAmount;

	// Set the tessellation factor for tessallating inside the triangle.
	output.inside = tessellationAmount;

	return output;
}

[domain("tri")] //domain - область. patch type used in the HS (tri, quad, or isoline)
[partitioning("integer")] //tess. scheme to be used in the hull shader (integer, fractional_even, fractional_odd, or pow2)
[outputtopology("triangle_cw")] //output primitive type for the tessellator.
[outputcontrolpoints(3)] //number of output control points (per thread) that will be created in the hull shader. 
						 //number of times the main function will be executed.
[patchconstantfunc("ColorPatchConstantFunction")] //separate function that outputs patch-constant data.

//SV_OutputControlPointID - index of the control point ID
//SV_PrimitiveID - id примитива.
//<HullInputType, 3> - массив из трёх HullInputType.
HullOutputType HS(InputPatch<HullInputType, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
	HullOutputType output;

	// Set the position and color for this control point as the output position and color.
	output.position = patch[pointId].position;
	output.color = patch[pointId].color;

    return output;
}