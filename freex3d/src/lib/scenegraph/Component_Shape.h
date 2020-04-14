/*


Proximity sensor macro.

*/

/****************************************************************************
    This file is part of the FreeWRL/FreeX3D Distribution.

    Copyright 2009 CRC Canada. (http://www.crc.gc.ca)

    FreeWRL/FreeX3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FreeWRL/FreeX3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FreeWRL/FreeX3D.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/



#ifndef __FREEWRL_SCENEGRAPH_SHAPE_H__
#define __FREEWRL_SCENEGRAPH_SHAPE_H__


/*******************************************************/


/*

 Bit-wise operations here - these can be OR'd together to
 create the specific shader we want.

 DO NOT MESS UP THE BITS! (look at these in binary for 
 proper or-ing of the values)

Sept 25, 2016:
shaderflags changed from int to struct { int, int, int }
{
 base, built from bit flags, and is also a fallback if userShader is desired but doesn't compile
 effect (castle Effect)
 user shader number (programmableShader)
}
- Could have done one long long int with 3 ranges, or int[3] instead; struct seems handy.
- In general needed more breathing room, especially for new effects which are bit mask or-able together, unlike
  user shaders that do only one user shader at a time
- now to test if its a usershader, just test if .usershaders != 0 (they start at 1)
- need to memset(,0,) the struct if creating fresh
- if need more bits in the future, add another member or change one to longlong
	 and look for places where we see if its == ie in opengl_utils.c
	 if (me->whichOne.base == rq_cap0.base && me->whichOne.effects == rq_cap0.effects && me->whichOne.usershaders == rq_cap0.usershaders) {

*/

typedef struct {
int base;
int effects;
int usershaders; 
int volume;
} shaderflagsstruct;

shaderflagsstruct getShaderFlags();
s_shader_capabilities_t *getMyShaders(shaderflagsstruct);



#define NO_APPEARANCE_SHADER 0x0001
#define MATERIAL_APPEARANCE_SHADER 0x0002
#define TWO_MATERIAL_APPEARANCE_SHADER 0x0004
#define PHYSICAL_MATERIAL_APPEARANCE_SHADER 0x0008
#define UNLIT_MATERIAL_APPEARANCE_SHADER 0x0010
#define ONE_TEX_APPEARANCE_SHADER 0x0020
#define MULTI_TEX_APPEARANCE_SHADER 0x0040

/* PolyRep (etc) color field present */
#define COLOUR_MATERIAL_SHADER 0x00080

/*  - fillProperties present */
#define FILL_PROPERTIES_SHADER 0x00100

/*  - lines, points */
#define HAVE_LINEPOINTS_COLOR 0x00200
#define HAVE_LINEPOINTS_APPEARANCE 0x00400

/* TextureCoordinateGenerator */
#define HAVE_TEXTURECOORDINATEGENERATOR 0x00800

/* CubeMapTexturing */
#define HAVE_CUBEMAP_TEXTURE   0x01000
/* more OR-able flags for compositing shaders */
#define FOG_APPEARANCE_SHADER	0X02000
#define HAVE_FOG_COORDS			0x04000
#define MODULATE_COLOR			0x08000
#define MODULATE_ALPHA			0x10000
#define MODULATE_TEXTURE		0x20000
#define SHADINGSTYLE_FLAT		0x40000
#define SHADINGSTYLE_GOURAUD	0x80000
#define SHADINGSTYLE_PHONG		0x100000
#define SHADINGSTYLE_WIRE		0x200000
#define MAT_FIRST				0x400000
#define WANT_ANAGLYPH			0x800000
#define TEX3D_SHADER			0X1000000
#define TEX3D_LAYER_SHADER		0x2000000
#define CLIPPLANE_SHADER		0x4000000
#define PARTICLE_SHADER			0X8000000
#define HAVE_UNLIT_COLOR		0x10000000
#define HAVE_PROJECTIVETEXTURE	0X20000000
#define LINE_PROPERTIES_SHADER	0X40000000
#define POINT_PROPERTIES_SHADER 0x80000000
//can go up to 2^32 - for future components like volume, particle, hanim 

//goes into flags.volume
#define SHADERFLAGS_VOLUME_DATA_BASIC		0x001
#define SHADERFLAGS_VOLUME_DATA_SEGMENT		0x002
#define SHADERFLAGS_VOLUME_DATA_ISO			0x004
#define SHADERFLAGS_VOLUME_DATA_ISO_MODE3	0x008
//#define SHADERFLAGS_VOLUME_STYLE_OPACITY	0x001
//#define SHADERFLAGS_VOLUME_STYLE_BLENDED	0x002
//#define SHADERFLAGS_VOLUME_STYLE_BOUNDARY	0x004
//#define SHADERFLAGS_VOLUME_STYLE_CARTOON	0x008
//#define SHADERFLAGS_VOLUME_STYLE_COMPOSED	0x010
//#define SHADERFLAGS_VOLUME_STYLE_EDGE		0x020
//#define SHADERFLAGS_VOLUME_STYLE_PROJECTION	0x040
//#define SHADERFLAGS_VOLUME_STYLE_SHADED		0x080
//#define SHADERFLAGS_VOLUME_STYLE_SILHOUETTE	0x100
//#define SHADERFLAGS_VOLUME_STYLE_TONE		0x200

#define SHADERFLAGS_VOLUME_STYLE_DEFAULT	1
#define SHADERFLAGS_VOLUME_STYLE_OPACITY	2
#define SHADERFLAGS_VOLUME_STYLE_BLENDED	3
#define SHADERFLAGS_VOLUME_STYLE_BOUNDARY	4
#define SHADERFLAGS_VOLUME_STYLE_CARTOON	5
#define SHADERFLAGS_VOLUME_STYLE_COMPOSED	6
#define SHADERFLAGS_VOLUME_STYLE_EDGE		7
#define SHADERFLAGS_VOLUME_STYLE_PROJECTION	8
#define SHADERFLAGS_VOLUME_STYLE_SHADED		9
#define SHADERFLAGS_VOLUME_STYLE_SILHOUETTE	10
#define SHADERFLAGS_VOLUME_STYLE_TONE		11
/*******************************************************/


struct fw_MaterialParameters {
	float diffuse[3];   //MAT_REGULAR
	float emissive[3];
	float specular[3];  //MAT_REGULAR
	float ambient;      //MAT_REGULAR
	float shininess;    //MAT_REGULAR
	float transparency; 
	float baseColor[3]; //MAT_PHYSICAL
	float metallic;     //MAT_PHYSICAL
	float roughness;    //MAT_PHYSICAL 
	int type; //MAT_TYPE: 0 MAT_NONE 1 MAT_EMISSIVE 2 MAT_REGULAR 3 MAT_PHYSICAL
	// used in frag, for texture maps:
	int transdex; // which tindex to use for transparency -1 None, else 0-3
	// multi-te4xtues are dis-aggregated at send-to-shader stage
	int tindex[10]; //texture unit indexes, 
	int mode[10];  //multitexture modulate mode
	int source[10]; //multitexture modulate mode
	int func[10]; //multitexture modulate mode
	int nt; // number of single texture maps 0 if none
	// [0] normal [1] emissive [2] diffuse OR baseColor [3] specular/shiny OR metallic/roughness [4] ambient
	struct X3D_Node *textures[5]; //ambient,normal,diffuse,specularshiny or roughnessmetallic,emissive,
	int tcount[5]; // for material.textureXXX if its a single texture 1, if multitexture n
	int tstart[5]; // where in tindex to start looping
	int cindex[5]; //texture coordinate channel
	//int mtex[5];   //flag = 1 if it's a multitexture / needs multitexture functionality applied
	int mt; // number of multitextures 0 if none, just a CPU-side flag to set MTEX in shader, don't send
};

// helpers for sharing sampler2D (and texture units)
void clear_material_samplers();
int share_or_next_material_sampler_index(GLint texture);
GLint tunit(int index);

struct matpropstruct {
	/* material properties for current shape */
	struct fw_MaterialParameters fw_FrontMaterial;
	struct fw_MaterialParameters fw_BackMaterial;

	/* which shader is active; 0 = no shader active */
	s_shader_capabilities_t *currentShaderProperties;

	//float	transparency;
	//GLfloat	emissionColour[3];
	GLint	cubeFace;	/* for cubemapping, if 0, not cube mapping */
	int 	cullFace;	/* is this single-sided or two-sided? Simply used to reduce calls to
						GL_ENABLE(GL_CULL_FACE), etc */

	/* for FillProperties, and LineProperties, line type (NOT pointsize) */
	bool hatchedBool;
	bool filledBool;
	GLfloat hatchPercent[2];
	GLfloat hatchScale[2]; //non-web3d standard but keep it for now
	GLfloat hatchColour[4];
	int hatchAlgo;

	//linetypes
	int linetype;
	float lineperiod;
	float linewidth; //this goes direct to our frag shader
	float * linetype_uv;
	float * linetype_tse;
	int linestrip_start_style;
	int linestrip_end_style;
	// points now specified in shader, not via an opengl call 
	GLfloat pointSize;   //this goes to old opengl internal geometry shader

	//TextureCoordinateGenerator value - a "TCGT_XXX" type
	int texCoordGeneratorType;
};

struct matpropstruct* getAppearanceProperties();
void setUserShaderNode(struct X3D_Node *me);


#define MIN_NODE_TRANSPARENCY 0.0f
#define MAX_NODE_TRANSPARENCY 0.99f  /* if 1.0, then occlusion culling will cause flashing */

#define RENDER_MATERIAL_SUBNODES(which) \
	{ struct X3D_Node *tmpN;   \
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, which,tmpN) \
		if(tmpN) { \
			render_node(tmpN); \
		} \
	}


#define SET_SHADER_SELECTED_FALSE(x3dNode) \
	switch (X3D_NODE(x3dNode)->_nodeType) { \
		case NODE_ComposedShader: \
			X3D_COMPOSEDSHADER(x3dNode)->isSelected = FALSE; \
			break; \
		case NODE_ProgramShader: \
			X3D_PROGRAMSHADER(x3dNode)->isSelected = FALSE; \
			break; \
		case NODE_PackagedShader: \
			X3D_PROGRAMSHADER(x3dNode)->isSelected = FALSE; \
			break; \
		default: { \
			/* this is NOT a shader; should we say something, or just \
			   ignore? Lets ignore, for now */ \
		} \
	}

#define SET_FOUND_GOOD_SHADER(x3dNode) \
	switch (X3D_NODE(x3dNode)->_nodeType) { \
		case NODE_ComposedShader: \
			foundGoodShader = X3D_COMPOSEDSHADER(x3dNode)->isValid; \
			X3D_COMPOSEDSHADER(x3dNode)->isSelected = foundGoodShader; \
			break; \
		case NODE_ProgramShader: \
			foundGoodShader = X3D_PROGRAMSHADER(x3dNode)->isValid; \
			X3D_PROGRAMSHADER(x3dNode)->isSelected = foundGoodShader; \
			break; \
		case NODE_PackagedShader: \
			foundGoodShader = X3D_PROGRAMSHADER(x3dNode)->isValid; \
			X3D_PACKAGEDSHADER(x3dNode)->isSelected = foundGoodShader; \
			break; \
		default: { \
			/* this is NOT a shader; should we say something, or just \
			   ignore? Lets ignore, for now */ \
		} \
	}
#endif /* __FREEWRL_SCENEGRAPH_SHAPE_H__ */
