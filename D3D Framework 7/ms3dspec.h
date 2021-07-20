#pragma once
#include <pshpack1.h>

#ifndef byte
typedef unsigned char byte;
#endif

#ifndef word
typedef unsigned short word;
#endif


struct MS3DHeader
{
	char id[10]; //всегда "MS3D000000"
	unsigned int version; //3
};

word nNumVertices = 0; // 2 bytes

struct MS3DVertex
{
	byte    flags;		// SELECTED | SELECTED2 | HIDDEN
	float   vertex[3]; 
	char    boneId;		// -1 = без скелета
	byte    referenceCount;
};

word nNumTriangles = 0; // 2 bytes

struct MS3DTriangle
{
	word    flags;		// SELECTED | SELECTED2 | HIDDEN
	word    vertexIndices[3];
	float   vertexNormals[3][3];
	float   s[3];
	float   t[3];
	byte    smoothingGroup; // 1 - 32
	byte    groupIndex;
};

//Группы нужны в случае нескольких материалов.
word nNumGroups = 0; // 2 bytes

struct MS3DGroup
{
	byte    flags;      // SELECTED | HIDDEN
	char    name[32];
	word    numtriangles;
	word    *triangleIndices;  // the groups group the triangles
	char    materialIndex;     // -1 = no material
};

word nNumMaterials = 0; // 2 bytes

struct MS3DMaterial
{
	char            name[32];
	float           ambient[4];
	float           diffuse[4];
	float           specular[4];
	float           emissive[4];
	float           shininess;		// 0.0f - 128.0f
	float           transparency;	// 0.0f - 1.0f
	char            mode;			// 0, 1, 2 is unused now
	char            texture[128];	// texture.bmp
	char            alphamap[128];	// alpha.bmp
};

#include <poppack.h>