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

/*	Aug 9, 2016, Castle Game Engine has given us/freewrl-sourceforge 
	special permission to implement their system
	for compositing shader permutations via special PLUGS, for use in libfreewrl.
	For use outside of libfreewrl, please consult Castle Game Engine
	http://castle-engine.sourceforge.net/compositing_shaders.php

	In the starting default/base shader, structured for web3d lighting model, with PLUG deckarations:
		...  PLUG: texture_apply (fragment_color, normal_eye_fragment) 

	In an additive effect shader:
        void PLUG_texture_color(inout vec4 texture_color,
          const in vec4 tex_coord)
        {

	The idea is to call the additive effect function from the main shader, where the same PLUG: <name> is
	1. paste the additive effect function at the bottom of the main shader, with a uniquized name
	2. put a forward declaration above the call
	3. call at the matching PLUG location
	4. (add in more effects same way)
	5. compile shaderparts and shaderprogram
	6. record permutation for re-use, as hash or bitflag of requirements/effects
*/
#include <config.h>
#include <system.h>
#include <system_threads.h>

#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>
#define TRUE 1
#define FALSE 0
//type
//  TShaderType = (vertex, geometry, fragment);
//  TShaderSource = array [TShaderType] of a string list;
//
enum TShaderType {
	SHADERPART_VERTEX,
	SHADERPART_GEOMETRY,
	SHADERPART_FRAGMENT
};
//var
//  { shader for the whole shape }
//  FinalShader: TShaderSource;


char *insertBefore(char *current, char *insert, char* wholeBuffer, int wholeBuffersize){
	//inserts insert at current location
	char *newcurrent, *here;
	int insertlen, wholelen, need, movesize;

	wholelen = strlen(current) +1; //plus 1 to get the \0 at the end in memmove
	insertlen = strlen(insert);
	need = wholelen + insertlen + 1;
	movesize = wholelen;
	newcurrent = current;
	if(need < wholeBuffersize){
		here = current;
		newcurrent = current + insertlen;
		memmove(newcurrent,current,movesize);
		memcpy(here,insert,insertlen);
	}else{
		ConsoleMessage("Not enough buffer for compositing shader buffsz=%d need=%d\n",wholeBuffersize,need);
	}
	//if(1){
	//	char *tmp = strdup(wholeBuffer);
	//	printf("strdup: %s",tmp);
	//	free(tmp);
	//}
	return newcurrent;
}
void removeAt(char *here, int len){
	//removes len bytes from string, moving tail bytes up by len bytes
	int wholelen, movesize;
	char *source;
	wholelen = strlen(here) + 1; //take the \0
	source = here + len;
	movesize = wholelen - len;
	memmove(here,source,movesize);
}

void extractPlugCall(char *start, char *PlugName,char *PlugParameters){
	//from the addon shader, get the name PLUG_<name>(declaredParameters)
	char *pns, *pne, *pps, *ppe;
	int len;
	pns = strstr(start,"PLUG: ");
	pns += strlen("PLUG: ");
	pne = strchr(pns,' ');
	len = pne - pns;
	strncpy(PlugName,pns,len);
	PlugName[len] = '\0';
	pps = strchr(pne,'(');
	ppe = strchr(pps,')') + 1;
	len = ppe - pps;
	strncpy(PlugParameters,pps,len);
	PlugParameters[len] = '\0';
	//printf("PlugName %s PlugParameters %s\n",PlugName,PlugParameters);
}
//{ Look for /* PLUG: PlugName (...) */ inside
	//given CodeForPlugDeclaration.
	//Return if any occurrence found. }
	//function LookForPlugDeclaration(
	//  CodeForPlugDeclaration: string list): boolean;
	//begin
int LookForPlugDeclarations( char * CodeForPlugDeclarations, int bsize, char *PlugName, char *ProcedureName, char *ForwardDeclaration) {
	//in the main code, look for matching PLUG: <PlugName>(plugparams) and place a call to ProcedureName(plugparams)
	//Result := false
	//for each S: string in CodeForPlugDeclaration do
	//begin
	int Result;
	//i = 0;
	//while(CodeForPlugDeclarations[i]){
		char *S;
		char MainPlugName[100], MainPlugParams[1000];
		//char PlugDeclarationBuffer[10000];
		int AnyOccurrencesHere = FALSE; //:= false
	Result = FALSE;
		//strcpy_s(PlugDeclarationBuffer,10000,*CodeForPlugDeclarations);
		S = CodeForPlugDeclarations;
		do {
			//while we can find an occurrence
			//  of /* PLUG: PlugName (...) */ inside S do
			//begin
			S = strstr(S,"/* PLUG: ");
			//if(S)
			//	printf("found PLUG:\n");
			//else
			//	printf("PLUG: not found\n");
			if(S){  ////where = strstr(haystack,needle)
				char ProcedureCallBuffer[500], *ProcedureCall;
				extractPlugCall(S,MainPlugName,MainPlugParams);
				if(!strcmp(MainPlugName,PlugName)){
					//found the place in the main shader to make the call to addon function
					//insert into S a call to ProcedureName,
					//with parameters specified inside the /* PLUG: PlugName (...) */,
					//right before the place where we found /* PLUG: PlugName (...) */
					ProcedureCall = ProcedureCallBuffer;
					sprintf(ProcedureCall,"%s%s;\n",ProcedureName,MainPlugParams);
					S = insertBefore(S,ProcedureCall,CodeForPlugDeclarations,bsize);
					AnyOccurrencesHere = TRUE; //:= true
					Result = TRUE; //:= true
				}else{
					//printf("found a PLUG: %s but doesn't match PLUG_%s\n",MainPlugName,PlugName);
				}
				S += strlen("/* PLUG:") + strlen(MainPlugName) + strlen(MainPlugParams);
			}
		}
		while(S); //AnyOccurrencesHere);
		//end

		//if AnyOccurrencesHere then
		if(AnyOccurrencesHere){
			//insert the PlugForwardDeclaration into S,
			//at the place of /* PLUG-DECLARATIONS */ inside
			//(or at the beginning, if no /* PLUG-DECLARATIONS */)
			S = CodeForPlugDeclarations;
			S = strstr(S,"/* PLUG-DECLARATIONS */");
			if(!S) S = CodeForPlugDeclarations;
			S = insertBefore(S,ForwardDeclaration,CodeForPlugDeclarations,bsize);
			//S = *CodeForPlugDeclarations;
			//*CodeForPlugDeclarations = strdup(PlugDeclarationBuffer);
			//FREE_IF_NZ(S);
		}else{
			printf("didn't find PLUG_%s\n",PlugName);
		}
	//	i++;
	//} //end
	return Result;
} //end


void replaceAll(char *buffer,int bsize, char *oldstr, char *newstr){
	char *found;
	while((found = strstr(buffer,oldstr))){
		removeAt(found,strlen(oldstr));
		insertBefore(found,newstr,buffer,bsize);
	}

}
//procedure Plug(
//  EffectPartType: TShaderType;
//  PlugValue: string;
//  CompleteCode: TShaderSource);
//char *strpbrk(const char *str1, const char *str2) finds the first character in the string str1 that matches any character specified in str2. This does not include the terminating null-characters.
void extractPlugName(char *start, char *PlugName,char *PlugDeclaredParameters){
	//from the addon shader, get the name PLUG_<name>(declaredParameters)
	char *pns, *pne, *pps, *ppe;
	int len;
	pns = strstr(start,"PLUG_");
	pns += strlen("PLUG_");
	//pne = strchr(pns,' ');
	pne = strpbrk(pns," (");
	len = pne - pns;
	strncpy(PlugName,pns,len);
	PlugName[len] = '\0';
	pps = strchr(pne,'(');
	ppe = strchr(pps,')') + 1;
	len = ppe - pps;
	strncpy(PlugDeclaredParameters,pps,len);
	PlugDeclaredParameters[len] = '\0';
	//printf("PlugName %s PlugDeclaredParameters %s\n",PlugName,PlugDeclaredParameters);
}
#define SBUFSIZE 65534 //32767 //must hold final size of composited shader part, could do per-gglobal-instance malloced buffer instead and resize to largest composited shader
#define PBUFSIZE 16384 //must hold largets PlugValue
int fw_strcpy_s(char *dest, int destsize, const char *source){
	int ier = -1;
	if(dest)
	if(source && strlen(source) < (unsigned)destsize){
		strcpy(dest,source);
		ier = 0;
	}
	return ier;
}
int fw_strcat_s(char *dest, int destsize, const char *source){
	int ier = -1;
	if(dest){
		int destlen = strlen(dest);
		if(source)
			if(strlen(source)+destlen < (unsigned)destsize){
				strcat(dest,source);
				ier = 0;
			}
	}
	return ier;
}
void Plug( int EffectPartType, const char *PlugValue, char **CompleteCode, int *unique_int)
{
	//Algo: 
	// outer loop: search for PLUG_<name> in (addon) effect
	//   inner loop: search for matching PLUG: <name> in main shader
	//     if found, paste addon into main:
	//        a) forward declaration at top, 
	//        b) method call at PLUG: point 
	//        c) method definition at bottom
	//var
	//  PlugName, ProcedureName, PlugForwardDeclaration: string;
	char PlugName[100], PlugDeclaredParameters[1000], PlugForwardDeclaration[1000], ProcedureName[100], PLUG_PlugName[100];
	char Code[SBUFSIZE], Plug[PBUFSIZE];
	int HasGeometryMain = FALSE, AnyOccurrences;
	char *found;
	int err;

	UNUSED(err);

	//var
	// Code: string list;
	//begin
	
	if(!CompleteCode[EffectPartType]) return;
	err = fw_strcpy_s(Code,SBUFSIZE, CompleteCode[EffectPartType]);
	err = fw_strcpy_s(Plug,PBUFSIZE, PlugValue);

	//HasGeometryMain := HasGeometryMain or
	//  ( EffectPartType = geometry and
	//    PlugValue contains 'main()' );
	HasGeometryMain = HasGeometryMain || 
		((EffectPartType == SHADERPART_GEOMETRY) && strstr("main(",Plug));

	//while we can find PLUG_xxx inside PlugValue do
	//begin
	found = Plug;
	do {
		found = strstr(found,"void PLUG_"); //where = strstr(haystack,needle)
		//if(!found)
		//	printf("I'd like to complain: void PLUG_ isn't found in the addon\n");
		if(found){
			//PlugName := the plug name we found, the "xxx" inside PLUG_xxx
			//PlugDeclaredParameters := parameters declared at PLUG_xxx function
			extractPlugName(found,PlugName,PlugDeclaredParameters);
			found += strlen("void PLUG_") + strlen(PlugName) + strlen(PlugDeclaredParameters);
			//{ Rename found PLUG_xxx to something unique. }
			//ProcedureName := generate new unique procedure name,
			//for example take 'plugged_' + some unique integer
			sprintf(ProcedureName,"%s_%d",PlugName,(*unique_int));
			(*unique_int)++;

			//replace inside PlugValue all occurrences of 'PLUG_' + PlugName
			//with ProcedureName
			sprintf(PLUG_PlugName,"%s%s","PLUG_",PlugName);
			replaceAll(Plug,PBUFSIZE,PLUG_PlugName,ProcedureName);

			//PlugForwardDeclaration := 'void ' + ProcedureName +
			//PlugDeclaredParameters + ';' + newline
			sprintf(PlugForwardDeclaration,"void %s%s;\n",ProcedureName,PlugDeclaredParameters);

			//AnyOccurrences := LookForPlugDeclaration(Code)
			AnyOccurrences = LookForPlugDeclarations(Code,SBUFSIZE, PlugName,ProcedureName,PlugForwardDeclaration);

			/* If the plug declaration is not found in Code, then try to find it
				in the final shader. This happens if Code is special for given
				light/texture effect, but PLUG_xxx is not special to the
				light/texture effect (it is applicable to the whole shape as well).
				For example, using PLUG_vertex_object_space inside
				the X3DTextureNode.effects. 
			*/
			//if not AnyOccurrences and
			//	Code <> Source[EffectPartType] then
			//	AnyOccurrences := LookForPlugDeclaration(Source[EffectPartType])
			//if(!AnyOccurrences && Code != Source[EffectPartType]){
			//	AnyOccurrences = LookForPlugDeclarations(Source[EffectPartType]);
			//}
			//if not AnyOccurrences then
			//	Warning('Plug name ' + PlugName + ' not declared')
			//}
			if(!AnyOccurrences){
				ConsoleMessage("Plug name %s not declared\n",PlugName);
			}
		}
	}while(found);
	//end

	/*{ regardless if any (and how many) plug points were found,
	always insert PlugValue into Code. This way EffectPart with a library
	of utility functions (no PLUG_xxx inside) also works. }*/
	//Code.Add(PlugValue)
	//printf("strlen Code = %d strlen PlugValue=%d\n",strlen(Code),strlen(PlugValue));
	err = fw_strcat_s(Code,SBUFSIZE,Plug);
	FREE_IF_NZ(CompleteCode[EffectPartType]);
	CompleteCode[EffectPartType] = strdup(Code);
} //end

void AddVersion( int EffectPartType, int versionNumber, char **CompleteCode){
	//puts #version <number> at top of shader, first line
	char Code[SBUFSIZE], line[1000];
	char *found;
	int err;

	UNUSED(err);

	if (!CompleteCode[EffectPartType]) return;
	err = fw_strcpy_s(Code, SBUFSIZE, CompleteCode[EffectPartType]);

	found = Code;
	if (found) {
		sprintf(line, "#version %d \n", versionNumber);
		insertBefore(found, line, Code, SBUFSIZE);
		FREE_IF_NZ(CompleteCode[EffectPartType]);
		CompleteCode[EffectPartType] = strdup(Code);
	}
}
void AddDefine0( int EffectPartType, const char *defineName, int defineValue, char **CompleteCode)
{
	//same as AddDEfine but you can say a number other than 1
	char Code[SBUFSIZE], line[1000];
	char *found;
	int err;

	UNUSED(err);

	if(!CompleteCode[EffectPartType]) return;
	err = fw_strcpy_s(Code,SBUFSIZE, CompleteCode[EffectPartType]);

	found = strstr(Code,"/* DEFINE"); 
	if(found){
		sprintf(line,"#define %s %d \n",defineName,defineValue);
		insertBefore(found,line,Code,SBUFSIZE);
		FREE_IF_NZ(CompleteCode[EffectPartType]);
		CompleteCode[EffectPartType] = strdup(Code);
	}
} 
void AddDefine( int EffectPartType, const char *defineName, char **CompleteCode){
	//adds #define <defineName> 1 to shader part, just above "/* DEFINES */" line in ShaderPart
	// char *CompleteCode[3] has char *vs *gs *fs parts, and will be realloced inside
	AddDefine0(EffectPartType,defineName,1,CompleteCode);
}

//procedure EnableEffects(
//  Effects: list of Effect nodes;
//  CompleteCode: TShaderSource);
//begin
//  for each Effect in Effects do
//    if Effect.enabled and
//       Effect.language matches renderer shader language then
//      for each EffectPart in Effect.parts do
//        Plug(EffectPart.type, GetUrl(EffectPart.url), CompleteCode)
//end
void EnableEffect(struct X3D_Node * node, char **CompletedCode, int *unique_int){
	int i, ipart;
	char *str;

	ipart=SHADERPART_VERTEX;
	struct X3D_Effect *effect = (struct X3D_Effect *)node;
	for(i=0;i<effect->parts.n;i++){
		struct X3D_EffectPart *part = (struct X3D_EffectPart*)effect->parts.p[i];
		if(part->_nodeType == NODE_EffectPart){
			if(!strcmp(part->type->strptr,"FRAGMENT"))
				ipart = SHADERPART_FRAGMENT;
			else if(!strcmp(part->type->strptr,"VERTEX"))
				ipart = SHADERPART_VERTEX;
			str = part->url.p[0]->strptr;
			if(!strncmp(str,"data:text/plain,",strlen("data:text/plain,")))
				str += strlen("data:text/plain,");
			Plug(ipart,str, CompletedCode, unique_int);
		}
	}
}
Stack *getEffectStack();
void EnableEffects( char **CompletedCode, int *unique_int){
	int i;
	Stack *effect_stack;
	effect_stack = getEffectStack();
	for(i=0;i<vectorSize(effect_stack);i++){
		struct X3D_Node *node = vector_get(struct X3D_Node*,effect_stack,i);
		if(node->_nodeType == NODE_Effect){
			EnableEffect(node,CompletedCode,unique_int);
		}
	}
}



//maxLights = 8;
//#if defined (GL_ES_VERSION_2_0)
//precision highp float;
//precision mediump float;
//#endif

/*
started with: http://svn.code.sf.net/p/castle-engine/code/trunk/castle_game_engine/src/x3d/opengl/glsl/template_mobile.vs
 castle						freewrl
 uniforms:
 castle_ModelViewMatrix		fw_ModelViewMatrix
 castle_ProjectionMatrix	fw_ProjectionMatrix
 castle_NormalMatrix		fw_NormalMatrix
 castle_MaterialDiffuseAlpha fw_FrontMaterial.diffuse.a
 castle_MaterialShininess	fw_FrontMaterial.shininess
 castle_SceneColor			fw_FrontMaterial.ambient
 castle_castle_UnlitColor	fw_FrontMaterial.emissive
							fw_FrontMaterial.specular
 per-vertex attributes
 castle_Vertex				fw_Vertex
 castle_Normal				fw_Normal
 castle_ColorPerVertex		fw_Color

 defines
 LIT
 COLOR_PER_VERTEX
 CASTLE_BUGGY_GLSL_READ_VARYING

define LIT if have Shape->appearance->material and NOT linepoints
define TWO if you have backface colors ie X3DTwoSidedMaterial
  http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/shape.html#TwoSidedMaterial
define LINE if you have Shape->appearance->material and is linepoints (lines and points use mat.emissive)
define TEX if you have texture
define CPV if colorNode && image_channels < 3
define MAT if material is valid
*/

/* Generic GLSL vertex shader.
   Used by ../castlerendererinternalshader.pas to construct the final shader.

   This is converted to template.vs.inc, and is then compiled
   in program's binary.
   When you change this file, rerun `make' and then recompile Pascal sources.
*/

static const GLchar *genericVertexGLES2 = "\
/* DEFINES */ \n\
#ifdef FULL \n\
#define attribute in \n\
#define varying out \n\
#endif //FULL \n\
/* Generic GLSL vertex shader, used on OpenGL ES. */ \n\
#ifdef MOBILE \n\
// we index into sampler arrays, OK for desktop, mobile needs GLES 3.1 and: \n\
// https://www.khronos.org/registry/OpenGL/extensions/OES/OES_gpu_shader5.txt \n\
#extension GL_OES_gpu_shader5 : require     //(or enable) \n\
#endif \n\
 \n\
uniform mat4 fw_ModelViewMatrix; \n\
uniform mat4 fw_ProjectionMatrix; \n\
uniform mat3 fw_NormalMatrix; \n\
#ifdef CUB \n\
uniform mat4 fw_ModelViewInverseMatrix; \n\
#endif //CUB \n\
attribute vec4 fw_Vertex; \n\
attribute vec3 fw_Normal; \n\
#if defined(LINETYPE) && defined(FULL) \n\
//desktop glsl 130 \n\
//glsl desktop version 130 can do flat instead of varying \n\
//which allows the provoking vertex (for GL_LINE_STRIP its the second vertex in a pair) \n\
//to output something to the frag that isnt interpolated - like .vert computed distance to prev \n\
flat out vec2 f_prev; \n\
flat out vec2 f_next; \n\
flat out float f_linestrip_end; //0 middle, 1 start, 2 end segment\n\
out vec2 v_curr; \n\
in vec3 a_prevVertex; \n\
in vec3 a_nextVertex; \n\
uniform int u_linetype; \n\
#endif //LINETYPE \n\
#if defined(LINETYPE) || defined(POINTP) \n\
uniform vec4 u_screenresolution; \n\
#endif // LINETYPE POINTP \n\
 \n\
//#ifdef TEX \n\
uniform mat4 fw_TextureMatrix[4]; \n\
uniform int nTexMatrix; \n\
attribute vec4 fw_MultiTexCoord0; \n\
attribute vec4 fw_MultiTexCoord1; \n\
attribute vec4 fw_MultiTexCoord2; \n\
attribute vec4 fw_MultiTexCoord3; \n\
uniform int nTexCoordChannels; \n\
uniform int flipuv; \n\
vec4 yupuv(in vec4 uv){ \n\
  //gltf uv are y-down, so we flag and send the flag here \n\
  vec4 yup = uv; \n\
  if(flipuv==1) yup.y = 1.0 - yup.y; \n\
  return yup; \n\
} \n\
//varying vec3 v_texC; \n\
varying vec3 fw_TexCoord[4]; \n\
#ifdef TEX3D \n\
uniform int tex3dUseVertex; \n\
#endif //TEX3D \n\
#ifdef TGEN \n\
 #define TCGT_CAMERASPACENORMAL    0  \n\
 #define TCGT_CAMERASPACEPOSITION    1 \n\
 #define TCGT_CAMERASPACEREFLECTION    2 \n\
 #define TCGT_COORD    3 \n\
 #define TCGT_COORD_EYE    4 \n\
 #define TCGT_NOISE    5 \n\
 #define TCGT_NOISE_EYE    6 \n\
 #define TCGT_SPHERE    7 \n\
 #define TCGT_SPHERE_LOCAL    8 \n\
 #define TCGT_SPHERE_REFLECT    9 \n\
 #define TCGT_SPHERE_REFLECT_LOCAL    10 \n\
 uniform int fw_textureCoordGenType; \n\
#endif //TGEN \n\
//#endif //TEX \n\
#ifdef FILL \n\
varying vec2 hatchPosition; \n\
#endif //FILL \n\
\n\
 \n\
varying vec4 castle_vertex_eye; \n\
varying vec3 castle_normal_eye; \n\
varying vec4 castle_Color; //DA diffuse ambient term \n\
 \n\
//uniform float castle_MaterialDiffuseAlpha; \n\
//uniform float castle_MaterialShininess; \n\
/* Color summed with all the lights. \n\
   Like gl_Front/BackLightModelProduct.sceneColor: \n\
   material emissive color + material ambient color * global (light model) ambient. \n\
*/ \n\
\n\
#ifdef LITE \n\
#define MAX_LIGHTS 8 \n\
uniform int lightcount; \n\
//uniform float lightRadius[MAX_LIGHTS]; \n\
uniform int lightType[MAX_LIGHTS];//ANGLE like this \n\
struct fw_LightSourceParameters { \n\
  float ambient;  \n\
  vec3 color;   \n\
  float intensity; \n\
  vec3 location;   \n\
  vec3 halfVector;  \n\
  vec3 direction; \n\
  float spotBeamWidth; \n\
  float spotCutoff; \n\
  vec3 Attenuations; \n\
  float lightRadius; \n\
}; \n\
\n\
uniform fw_LightSourceParameters fw_LightSource[MAX_LIGHTS] /* gl_MaxLights */ ;\n\
#endif //LITE \n\
\n\
//uniform vec3 castle_SceneColor; \n\
//uniform vec4 castle_UnlitColor; \n\
#ifdef UNLIT \n\
uniform vec4 fw_UnlitColor; \n\
#endif //UNLIT \n\
//#ifdef LIT \n\
struct fw_MaterialParameters { \n\
  vec3 diffuse; \n\
  vec3 emissive; \n\
  vec3 specular; \n\
  float ambient; \n\
  float shininess; \n\
  float occlusion; \n\
  float transparency; \n\
  vec3 baseColor; \n\
  float metallic; \n\
  float roughness; \n\
  int type; \n\
  // multitextures are disaggregated \n\
  int tindex[10]; \n\
  int mode[10]; \n\
  int source[10]; \n\
  int func[10]; \n\
  int nt; //total single textures \n\
  //iunit [0] normal [1] emissive [2] occlusion [3] diffuse OR base [4] shininess OR metallicRoughness [5] specular [6] ambient \n\
  int tcount[7]; //num single textures 1= one texture 0=no texture 2+ = multitexture \n\
  int tstart[7]; // where in packed tindex list to start looping \n\
  int cindex[7]; // which geometry multitexcoord channel 0=default \n\
}; \n\
uniform fw_MaterialParameters fw_FrontMaterial; \n\
//#ifdef TWO \n\
uniform fw_MaterialParameters fw_BackMaterial; \n\
//#endif //TWO \n\
#ifdef LIT \n\
varying vec3 castle_ColorES; //emissive shininess term \n\
vec3 castle_Emissive; \n\
#endif //LIT \n\
#ifdef FOG \n\
struct fogParams \n\
{  \n\
  vec4 fogColor; \n\
  float visibilityRange; \n\
  float fogScale; //applied on cpu side to visrange \n\
  int fogType; // 0 None, 1= FOGTYPE_LINEAR, 2 = FOGTYPE_EXPONENTIAL \n\
  // ifdefed int haveFogCoords; \n\
}; \n\
uniform fogParams fw_fogparams; \n\
#ifdef FOGCOORD \n\
attribute float fw_FogCoords; \n\
#endif \n\
#endif //FOG \n\
float castle_MaterialDiffuseAlpha; \n\
float castle_MaterialShininess; \n\
vec3 castle_SceneColor; \n\
vec4 castle_UnlitColor; \n\
vec4 castle_Specular; \n\
 \n\
#ifdef CPV \n\
attribute vec4 fw_Color; //castle_ColorPerVertex; \n\
varying vec4 cpv_Color; \n\
#endif //CPV \n\
#ifdef PARTICLE \n\
uniform vec3 particlePosition; \n\
uniform int fw_ParticleGeomType; \n\
#endif //PARTICLE \n\
#ifdef POINTP \n\
uniform float u_pointSize; \n\
uniform vec3 u_pointAttenuation; \n\
uniform vec2 u_pointSizeRange; \n\
uniform int u_pointtColorMode; \n\
uniform vec3 u_pointPosition; \n\
uniform int u_pointMethod; \n\
uniform float u_pointFogCoord; \n\
uniform vec4 u_pointCPV; \n\
#endif //POINTP \n\
#ifdef PROJTEX \n\
uniform mat4 projTexGenMatCam[8]; \n\
uniform int pCount; \n\
varying vec4 projTexCoord[8]; \n\
varying vec4 projTexNorm[8]; \n\
void vertProjCalTexCoord(void) { \n\
	for(int i=0;i<pCount;i++){ \n\
		projTexCoord[i] = projTexGenMatCam[i] * castle_vertex_eye; \n\
		projTexNorm[i] = projTexGenMatCam[i] * vec4((castle_vertex_eye.xyz + castle_normal_eye.xyz),1.0); \n\
	} \n\
} \n\
#endif //PROJTEX \n\
 \n\
 //literal string size break \n" "\
 vec3 dehomogenize(in mat4 matrix, in vec4 vector){ \n\
	vec4 tempv = vector; \n\
	if(tempv.w == 0.0) tempv.w = 1.0; \n\
	vec4 temp = matrix * tempv; \n\
	float winv = 1.0/temp.w; \n\
	return temp.xyz * winv; \n\
 } \n\
 bool approx(float a, float b){ \n\
	if( abs(a - b) < .0001 )return true; \n\
	return false; \n\
 } \n\
/* PLUG-DECLARATIONS */ \n\
void main(void) \n\
{ \n\
  #ifdef FOGCOORD \n\
  float fog_coord = fw_FogCoords; \n\
  #endif //FOGCOORD \n\
  #ifdef LIT \n\
  fw_MaterialParameters ourMat = fw_FrontMaterial; \n\
  castle_MaterialDiffuseAlpha = (1.0 - fw_FrontMaterial.transparency); \n\
  castle_MaterialShininess =	fw_FrontMaterial.shininess; \n\
  castle_SceneColor = fw_FrontMaterial.diffuse*fw_FrontMaterial.ambient; \n\
  castle_Specular =	vec4(fw_FrontMaterial.specular,1.0); \n\
  castle_Emissive = fw_FrontMaterial.emissive; \n\
  #ifdef LINE \n\
   castle_SceneColor = vec3(0.0,0.0,0.0); //line gets color from castle_Emissive \n\
  #endif //LINE\n\
  #else //LIT \n\
  //default unlits in case we dont set them \n\
  castle_UnlitColor = vec4(1.0,1.0,1.0,1.0); \n\
  castle_MaterialDiffuseAlpha = 1.0; \n\
  #endif //LIT \n\
  \n\
  #ifdef FILL \n\
  hatchPosition = fw_Vertex.xy; \n\
  #endif //FILL \n\
  //\n\
  vec4 vertex_object = fw_Vertex; \n\
  #ifdef PARTICLE \n\
  if(fw_ParticleGeomType != 4){ \n\
    vertex_object.xyz += particlePosition; \n\
  } \n\
  #endif //PARTICLE \n\
  vec3 normal_object = fw_Normal; \n\
  /* PLUG: vertex_object_space_change (vertex_object, normal_object) */ \n\
  /* PLUG: vertex_object_space (vertex_object, normal_object) */ \n\
   \n\
  #ifdef CASTLE_BUGGY_GLSL_READ_VARYING \n\
  /* use local variables, instead of reading + writing to varying variables, \n\
     when VARYING_NOT_READABLE */ \n\
  vec4 temp_castle_vertex_eye; \n\
  vec3 temp_castle_normal_eye; \n\
  vec4 temp_castle_Color; \n\
  #define castle_vertex_eye temp_castle_vertex_eye \n\
  #define castle_normal_eye temp_castle_normal_eye \n\
  #define castle_Color      temp_castle_Color \n\
  #endif //CASTLE_BUGGY_GLSL_READ_VARYING \n\
  \n\
  castle_vertex_eye = fw_ModelViewMatrix * vertex_object; \n\
  #if defined(LINETYPE) && defined(FULL) \n\
  if(u_linetype > 1){ \n\
	//get curr, prev, next into screenspace \n\
	//missing: screen aspect correction\n\
	vec4 curr = fw_ProjectionMatrix * castle_vertex_eye; \n\
	vec4 prev = fw_ProjectionMatrix * fw_ModelViewMatrix * vec4(a_prevVertex,1.0); \n\
	//vec4 next = fw_ProjectionMatrix * fw_ModelViewMatrix * vec4(a_nextVertex,1.0); \n\
	//projected coords are in -1 to 1 range \n\
	f_prev = ((prev.xyz/prev.w).xy*.5 + vec2(.5))*u_screenresolution.xy; \n\
	//f_next = (next.xyz/next.w).xy*u_screenresolution.xy*.5; \n\
	//using GL_LINE_STRIP the 2nd vertex is the provoking vertex so is next \n\
	f_next = ((curr.xyz/curr.w).xy*.5 + vec2(.5))*u_screenresolution.xy; \n\
	v_curr = ((curr.xyz/curr.w).xy*.5 + vec2(.5))*u_screenresolution.xy; \n\
	//float index aka findex method of determining start/end of polyline \n\
	float findex = a_nextVertex.x; \n\
	float fcount = a_nextVertex.y; \n\
	f_linestrip_end = 0; \n\
	if(approx(findex -1,0.0)) f_linestrip_end = 1; \n\
	if(approx(findex,fcount-1.0)) f_linestrip_end += 2; \n\
  } \n\
  #endif //LINETYPE \n\
  #ifdef PARTICLE \n\
  //sprite: align to viewer \n\
  if(fw_ParticleGeomType == 4){ \n\
	vec4 ppos = vec4(particlePosition,1.0); \n\
	vec4 particle_eye = fw_ModelViewMatrix * ppos; \n\
	ppos.x += 1.0; \n\
	vec4 particle_eye1 = fw_ModelViewMatrix * ppos; \n\
	float pscal = length(particle_eye1.xyz - particle_eye.xyz); \n\
	castle_vertex_eye = particle_eye + pscal*vertex_object; \n\
  } \n\
  #endif //PARTICLE \n\
  castle_normal_eye = normalize(fw_NormalMatrix * normal_object); \n\
  #ifdef PROJTEX \n\
	vertProjCalTexCoord(); \n\
  #endif //PROJETEX \n\
  \n\
  /* PLUG: vertex_eye_space (castle_vertex_eye, castle_normal_eye) */ \n\
   \n\
  #ifdef LIT \n\
  castle_ColorES = castle_Emissive; \n\
  castle_Color = vec4(castle_SceneColor, 1.0); \n\
	/* back Facing materials - flip the normal and grab back materials */ \n\
	vec3 E = -normalize(castle_vertex_eye.xyz); \n\
	vec3 N = normalize (castle_normal_eye); \n\
	bool backFacing = (dot(N,E) < 0.0); \n\
	if (backFacing) { \n\
		N = -N; \n\
		#ifdef TWO \n\
		ourMat = fw_BackMaterial; \n\
		#endif //TWO \n\
	} \n\
  vec3 vcolor = vec3(0.0,0.0,0.0); \n\
  /* PLUG: add_light_contribution2 (vcolor, castle_ColorES, castle_vertex_eye, N, ourMat.shininess, ourMat.ambient, ourMat.diffuse, ourMat.specular) */ \n\
  /* PLUG: add_light_contribution (vcolor, castle_vertex_eye, castle_normal_eye, castle_MaterialShininess) */ \n\
  castle_Color = vec4(vcolor, castle_MaterialDiffuseAlpha); \n\
  /* Clamp sum of lights colors to be <= 1. See template.fs for comments. */ \n\
  castle_Color.rgb = min(castle_Color.rgb, 1.0); \n\
  #else //LIT \n\
  castle_Color.rgb = castle_UnlitColor.rgb; \n\
  #endif //LIT \n\
  \n\
  #ifdef CPV //color per vertex \n\
  cpv_Color = fw_Color; \n\
  #endif //CPV \n\
  \n\
  //#ifdef TEX \n\
  vec4 texcoord = yupuv(fw_MultiTexCoord0); \n\
  #ifdef TEX3D \n\
  //to re-use vertex coords as texturecoords3D, we need them in 0-1 range: CPU calc of fw_TextureMatrix0 \n\
  if(tex3dUseVertex == 1) \n\
    texcoord = vec4(fw_Vertex.xyz,1.0); \n\
  #endif //TEX3D \n\
  #ifdef TGEN  \n\
  { \n\
    vec3 vertexNorm; \n\
    vec4 vertexPos; \n\
	vec3 texcoord3 = texcoord.xyz; \n\
    vertexNorm = normalize(fw_NormalMatrix * fw_Normal); \n\
    vertexPos = fw_ModelViewMatrix * fw_Vertex; \n\
    /* sphereEnvironMapping Calculation */  \n\
    vec3 u=normalize(vec3(vertexPos)); /* u is normalized position, used below more than once */ \n\
    vec3 r= reflect(u,vertexNorm); \n\
    if (fw_textureCoordGenType==TCGT_SPHERE) { /* TCGT_SPHERE  GL_SPHERE_MAP OpenGL Equiv */ \n\
      float m=2.0 * sqrt(r.x*r.x + r.y*r.y + (r.z*1.0)*(r.z*1.0)); \n\
      texcoord3 = vec3(r.x/m+0.5,r.y/m+0.5,0.0); \n\
    }else if (fw_textureCoordGenType==TCGT_CAMERASPACENORMAL) { \n\
      /* GL_REFLECTION_MAP used for sampling cubemaps */ \n\
      float dotResult = 2.0 * dot(u,r); \n\
      texcoord3 = vec3(u-r)*dotResult; \n\
    }else if (fw_textureCoordGenType==TCGT_COORD) { \n\
      /* 3D textures can use coords in 0-1 range */ \n\
      texcoord3 = fw_Vertex.xyz; //xyz; \n\
    } else { /* default usage - like default CubeMaps */ \n\
      vec3 u=normalize(vec3(fw_ProjectionMatrix * fw_Vertex)); /* myEyeVertex */  \n\
      texcoord3 =    reflect(u,vertexNorm); \n\
    } \n\
	texcoord.xyz = texcoord3; \n\
  } \n\
  #endif //TGEN \n\
  vec4 tcoord[4]; \n\
  tcoord[0] = texcoord; //fw_MultiTexCoord0; \n\
  tcoord[1] = yupuv(fw_MultiTexCoord1); \n\
  tcoord[2] = yupuv(fw_MultiTexCoord2); \n\
  tcoord[3] = yupuv(fw_MultiTexCoord3); \n\
  mat4 ttrans = mat4(1.0); \n\
  vec4 tc = vec4(0.0,0.0,0.0,1.0); \n\
  for(int i=0;i<4;i++){ \n\
	//spec rules: not enough transforms use identity, not enough coords use last ones\n\
	if(i < nTexMatrix) ttrans = fw_TextureMatrix[i]; \n\
	if(i < nTexCoordChannels) tc = tcoord[i]; \n\
	fw_TexCoord[i] = dehomogenize(ttrans, tc); \n\
  } \n\
  #ifdef FILL \n\
  hatchPosition = fw_TexCoord[0].xy; \n\
  #endif //FILL \n\
  \n\
 // #endif //TEX \n\
  \n\
  gl_Position = fw_ProjectionMatrix * castle_vertex_eye; \n\
  \n\
  #ifdef POINTP \n\
    //particle-system-like  PointSet points get special CPV, fogcoord handling, like pointPosition \n\
    #ifdef FOGCOORD \n\
	fog_coord = u_pointFogCoord; \n\
	#endif //FOGCOORD \n\
	#ifdef CPV \n\
	cpv_Color = u_pointCPV; \n\
	#endif //CPV \n\
	if(u_pointMethod == 2) { \n\
		//OBJECTSCALE - keep sprite-aligned but size fade with distance \n\
		vec4 ppos = vec4(u_pointPosition,1.0); \n\
		vec4 point_eye = fw_ModelViewMatrix * ppos; \n\
		ppos.x += 1.0; \n\
		vec4 point_eye1 = fw_ModelViewMatrix * ppos; \n\
		float pscal = length(point_eye1.xyz - point_eye.xyz); \n\
		castle_vertex_eye = point_eye + pscal*vertex_object*u_pointSize; \n\
	  gl_Position = fw_ProjectionMatrix * castle_vertex_eye; \n\
	}else { \n\
		vec4 ppos = vec4(u_pointPosition,1.0); \n\
		vec4 castle_vertex_eye = fw_ModelViewMatrix * ppos; \n\
		vec4 view_position = fw_ProjectionMatrix * castle_vertex_eye; \n\
		float pscal = 1.0; \n\
		if(u_pointMethod == 1) { \n\
			//simple screen scale \n\
			pscal = u_pointSize; \n\
		} else if(u_pointMethod == 3) { \n\
			// fancy attenuation, screen scale \n\
			float zdist = castle_vertex_eye.z; \n\
			pscal = u_pointAttenuation.x + zdist*u_pointAttenuation.y + zdist*zdist*u_pointAttenuation.z; \n\
			if(pscal > 0.0) pscal = u_pointSize/pscal; \n\
			else pscal = u_pointSize; \n\
			pscal = max(pscal,u_pointSizeRange.x); \n\
			pscal = min(pscal,u_pointSizeRange.y); \n\
		} \n\
		//convert from screen pixel size to view coords \n\
		vec2 view_point = (vec2(pscal)*vertex_object.xy / u_screenresolution.xy) *vec2(2.0)* view_position.w; \n\
		view_position.xy += view_point; \n\
		gl_Position = view_position; \n\
	} \n\
  #endif //POINTP \n\
  #ifdef CUB \n\
  //cubemap \n\
  vec4 camera = fw_ModelViewInverseMatrix * vec4(0.0,0.0,0.0,1.0); \n\
  //vec3 u = normalize( vec4(castle_vertex_eye - camera).xyz ); \n\
  vec3 u = normalize( vec4(vertex_object + camera).xyz ); \n\
  vec3 v = normalize(fw_Normal); \n\
  fw_TexCoord[0] = normalize(reflect(u,v)); //computed in object space \n\
  fw_TexCoord[0].st = -fw_TexCoord[0].st; //helps with renderman cubemap convention \n\
  #endif //CUB \n\
  #ifdef CASTLE_BUGGY_GLSL_READ_VARYING \n\
  #undef castle_vertex_eye \n\
  #undef castle_normal_eye \n\
  #undef castle_Color \n\
  castle_vertex_eye = temp_castle_vertex_eye; \n\
  castle_normal_eye = temp_castle_normal_eye; \n\
  castle_Color      = temp_castle_Color; \n\
  #endif //CASTLE_BUGGY_GLSL_READ_VARYING \n\
  \n\
  #ifdef FOG \n\
  #ifdef FOGCOORD \n\
  castle_vertex_eye.z = fog_coord; \n\
  #endif //FOGCOORD \n\
  #endif //FOG \n\
  #ifdef UNLIT \n\
  castle_Color = fw_UnlitColor; \n\
  #endif //UNLIT \n\
} \n";


/* Generic GLSL fragment shader.
   Used by ../castlerendererinternalshader.pas to construct the final shader.

   This is converted to template.fs.inc, and is then compiled
   in program's binary.
   When you change this file, rerun `make' and then recompile Pascal sources.
*/
/* 
	started with: http://svn.code.sf.net/p/castle-engine/code/trunk/castle_game_engine/src/x3d/opengl/glsl/template_mobile.fs
  defines:
  GL_ES_VERSION_2_0 - non-desktop
  HAS_GEOMETRY_SHADER - version 3+ gles
*/



















static const GLchar *genericFragmentGLES2 = "\
/* DEFINES */ \n\
#ifdef MOBILE \n\
precision mediump float; \n\
// we index into sampler arrays, OK for desktop, mobile needs GLES 3.1 and: \n\
// https://www.khronos.org/registry/OpenGL/extensions/OES/OES_gpu_shader5.txt \n\
#extension GL_OES_gpu_shader5 : require     //(or enable) \n\
//#else \n\
//precision highp float; \n\
#endif //MOBILE \n\
/* Generic GLSL fragment shader, used on OpenGL ES. */ \n\
 \n\
varying vec4 castle_Color; \n\
 \n\
#ifdef LITE \n\
#define MAX_LIGHTS 8 \n\
uniform int lightcount; \n\
//uniform float lightRadius[MAX_LIGHTS]; \n\
uniform int lightType[MAX_LIGHTS];//ANGLE like this \n\
struct fw_LightSourceParameters { \n\
	float ambient;  \n\
	vec3 color;   \n\
	float intensity; \n\
	vec3 location;   \n\
	vec3 halfVector;  \n\
	vec3 direction; \n\
	float spotBeamWidth; \n\
	float spotCutoff; \n\
	vec3 Attenuations; \n\
	float lightRadius; \n\
}; \n\
\n\
uniform fw_LightSourceParameters fw_LightSource[MAX_LIGHTS] /* gl_MaxLights */ ;\n\
#endif //LITE \n\
\n\
#ifdef CPV \n\
varying vec4 cpv_Color; \n\
#endif //CPV \n\
struct MaterialInfo \n\
{ \n\
    float perceptualRoughness;    // roughness value, as authored by the model creator (input to shader) \n\
    vec3 reflectance0;            // full reflectance color (normal incidence angle) \n\
	 \n\
    float alphaRoughness;         // roughness mapped to a more linear change in the roughness (proposed by [2]) \n\
    vec3 diffuseColor;            // color contribution from diffuse lighting \n\
	 \n\
    vec3 reflectance90;           // reflectance color at grazing angle \n\
    vec3 specularColor;           // color contribution from specular lighting \n\
}; \n\
\n\
/* PLUG-DECLARATIONS */ \n\
//#ifdef TEX \n\
uniform int textureCount; \n\
#ifdef CUB \n\
uniform samplerCube fw_Texture_unit0; \n\
#else //CUB \n\
uniform sampler2D fw_Texture_unit0; \n\
#endif //CUB \n\
varying vec3 fw_TexCoord[4]; \n\
#ifdef TEX3D \n\
uniform int tex3dTiles[3]; \n\
uniform int repeatSTR[3]; \n\
uniform int magFilter; \n\
#endif //TEX3D \n\
#ifdef TEX3DLAY \n\
uniform sampler2D fw_Texture_unit1; \n\
uniform sampler2D fw_Texture_unit2; \n\
uniform sampler2D fw_Texture_unit3; \n\
//uniform int textureCount; \n\
#endif //TEX3DLAY \n\
#if defined(MTEX) || defined(PROJTEX) \n\
uniform sampler2D fw_Texture_unit1; \n\
uniform sampler2D fw_Texture_unit2; \n\
uniform sampler2D fw_Texture_unit3; \n\
uniform ivec2 fw_Texture_mode0;  \n\
uniform ivec2 fw_Texture_mode1;  \n\
uniform ivec2 fw_Texture_mode2;  \n\
uniform ivec2 fw_Texture_mode3;  \n\
uniform ivec2 fw_Texture_source0;  \n\
uniform ivec2 fw_Texture_source1;  \n\
uniform ivec2 fw_Texture_source2;  \n\
uniform ivec2 fw_Texture_source3;  \n\
uniform int fw_Texture_function0;  \n\
uniform int fw_Texture_function1;  \n\
uniform int fw_Texture_function2;  \n\
uniform int fw_Texture_function3;  \n\
//uniform int textureCount; \n\
uniform vec4 mt_Color; \n\
void finalColCalcA(inout vec4 prevColour, in int mode, in int modea, in int func, in sampler2D tex, in vec2 texcoord){ \n\
	/* PLUG: finalColCalc ( prevColour, mode, modea, func, tex, texcoord ) */ \n\
} \n\
#endif //MTEX \n\
//#endif //TEX \n\
//literal string size break \n" "\
#ifdef POINTP \n\
uniform int u_pointColorMode; \n\
#endif //POINTP \n\
#if defined(LINETYPE) && defined(FULL) \n\
uniform int u_linetype; \n\
uniform float u_lineperiod; \n\
uniform float u_linewidth; \n\
uniform int u_linestrip_start_style; \n\
uniform int u_linestrip_end_style; \n\
uniform vec2 u_linetype_uv[128]; \n\
uniform vec3 u_linetype_tse[128]; \n\
uniform vec4 u_screenresolution; \n\
 \n\
flat in vec2 f_prev; \n\
flat in vec2 f_next; \n\
flat in float f_linestrip_end; \n\
in vec2 v_curr; \n\
bool approx(float a, float b){ \n\
	if( abs(a-b) < .0001) return true; \n\
	return false; \n\
} \n\
bool on_linetype(inout vec4 frag_color){ \n\
	bool on = true; \n\
	if(false){ \n\
		//procedural dashed line method (not used but works for simple dash)\n\
		float distance = length(v_curr - f_prev); \n\
		//info about cycle length \n\
		float period = 20.0; \n\
		float phase = mod(distance,20.0); \n\
		if(phase > 10.0) on = false; \n\
	}else{ \n\
		//parametric dashed line method \n\
		vec2 baseline = f_next - f_prev; \n\
		vec2 u_dir = normalize(baseline); \n\
		vec2 v_dir = normalize(cross(vec3(0,0,1),vec3(u_dir,0.0)).xy); \n\
		vec2 ubar; \n\
		//gl_FragCoord is relative to whole opengl window, we need viewport \n\
		vec2 vpcoord = gl_FragCoord.xy - u_screenresolution.pq; \n\
		ubar.s = dot(vpcoord.xy - f_prev, u_dir); \n\
		ubar.t = dot(vpcoord.xy - v_curr, v_dir); \n\
		float phase = mod(ubar.s, u_lineperiod); \n\
		vec2 uu = vec2(0.0,0.0); \n\
		bool gap = false; \n\
		vec2 dash; \n\
		vec3 tse = u_linetype_tse[int(phase)]; \n\
		uu = u_linetype_uv[int(phase)]; \n\
		gap = int(tse.x + .5) == 0; \n\
		dash = vec2(tse.y,tse.z); \n\
		vec2 ubarperiod = vec2(phase,ubar.t); \n\
		if(gap){ \n\
			on = false; \n\
		} else { \n\
			if( abs(ubarperiod.t - uu.t) > u_linewidth  ) on = false; \n\
			//if( length(ubarperiod-uu) > u_linewidth *.5 ) on = false; \n\
		} \n\
		//do fancy linestrip end if required and on linestrip end segment \n\
		if( u_linestrip_start_style > 0 || u_linestrip_end_style > 0) \n\
		if(!approx(f_linestrip_end,0.0)){ \n\
			//a line can be both start and and of polyline \n\
			bool s_start = approx(mod(f_linestrip_end,2.0),1.0); \n\
			bool s_end = approx(floor(f_linestrip_end/2.0),1.0); \n\
			if(s_start){ \n\
				vec2 uend = vec2(0.0); \n\
				if(u_linestrip_start_style == 1){ \n\
					//arrow end \n\
					float arrowlength = 14.0; \n\
					vec2 head = vec2(uend.s + arrowlength,0.0); \n\
					if(ubar.s < head.s){ \n\
						on = false; \n\
						vec2 range = head - ubar; \n\
						float d = 2.0*range.t + range.s; \n\
						if(d < arrowlength) on = true; \n\
					} \n\
				} else if(u_linestrip_start_style == 2){ \n\
					//round end \n\
					float radius = 6.0; \n\
					vec2 center = vec2(0.0); \n\
					center.s = (uend.s + radius); \n\
					vec2 diameter = uend + vec2(2.0*radius,0.0); \n\
					if(ubar.s < diameter.s){ \n\
						on = false; \n\
						if( length(ubar - center) <= radius ) on = true; \n\
					} \n\
				} \n\
			} \n\
			if(s_end){ \n\
				vec2 uend = vec2(0.0); \n\
				uend.s = dot(f_next - f_prev, u_dir); \n\
				//must be end 2.0 \n\
				if(u_linestrip_end_style == 1){ \n\
					//arrow end \n\
					float arrowlength = 14.0; \n\
					vec2 head = vec2(uend.s - arrowlength,0.0); \n\
					if(ubar.s > head.s){ \n\
						on = false; \n\
						vec2 range = ubar - head; \n\
						float d = 2.0*range.t + range.s; \n\
						if(d < arrowlength) on = true; \n\
					} \n\
				} else if(u_linestrip_end_style == 2){ \n\
					//round end \n\
					float radius = 6.0; \n\
					vec2 center = vec2(0.0); \n\
					center.s = (uend.s- radius); \n\
					vec2 diameter = uend - vec2(2.0*radius,0.0); \n\
					if(ubar.s > diameter.s){ \n\
						on = false; \n\
						if( length(ubar - center) <= radius ) on = true; \n\
					} \n\
				} \n\
			} \n\
		} \n\
	}\n\
	return on; \n\
}\n\
#endif //LINETYPE\n\
#ifdef FILL \n\
struct fillproperties { \n\
	vec4 HatchColour; \n\
	int HatchAlgo; \n\
	bool hatched; \n\
	bool filled; \n\
}; \n\
uniform fillproperties fillprops; \n\
varying vec2 hatchPosition; \n\
#endif //FILL \n\
//literal string size break \n" "\
#ifdef FOG \n\
struct fogParams \n\
{  \n\
	vec4 fogColor; \n\
	float visibilityRange; \n\
	float fogScale; \n\
	int fogType; // 0 None, 1= FOGTYPE_LINEAR, 2 = FOGTYPE_EXPONENTIAL \n\
	// ifdefed int haveFogCoords; \n\
}; \n\
uniform fogParams fw_fogparams; \n\
#endif //FOG \n\
 \n\
#ifdef HAS_GEOMETRY_SHADER \n\
#define castle_vertex_eye castle_vertex_eye_geoshader \n\
#define castle_normal_eye castle_normal_eye_geoshader \n\
#endif // HAS_GEOMETRY_SHADER \n\
 \n\
varying vec4 castle_vertex_eye; \n\
varying vec3 castle_normal_eye; \n\
//#ifdef LIT \n\
//#ifdef LITE \n\
//per-fragment lighting ie phong \n\
struct fw_MaterialParameters { \n\
	vec3 diffuse; \n\
	vec3 emissive; \n\
	vec3 specular; \n\
	float ambient; \n\
	float shininess; \n\
    float occlusion; \n\
	float transparency; \n\
	vec3 baseColor; \n\
	float metallic; \n\
	float roughness; \n\
	int type; \n\
	// multitextures are disaggregated \n\
	int tindex[10]; \n\
	int mode[10]; \n\
	int source[10]; \n\
	int func[10]; \n\
	int nt; //total single textures \n\
	//iunit [0] normal [1] emissive [2] occlusion [3] diffuse OR base [4] shininess OR metallicRoughness [5] specular [6] ambient \n\
	int tcount[7]; //num single textures 1= one texture 0=no texture 2+ = multitexture \n\
	int tstart[7]; // where in packed tindex list to start looping \n\
	int cindex[7]; // which geometry multitexcoord channel 0=default \n\
}; \n\
uniform fw_MaterialParameters fw_FrontMaterial; \n\
//#ifdef TWO \n\
uniform fw_MaterialParameters fw_BackMaterial; \n\
//#endif //TWO \n\
#ifdef LIT \n\
//#ifdef LITE \n\
//vec3 castle_ColorES; \n\
//#else //LITE \n\
//per-vertex lighting - interpolated Emissive-specular \n\
varying vec3 castle_ColorES; //emissive shininess term \n\
//#endif //LITE \n\
#endif //LIT\n\
//#if defined(TEX) || defined(PROJTEX) \n\
//shared sampler2D array -PTM or PBR use \n\
uniform sampler2D textureUnit[16]; \n\
//#endif //defined(TEX) || defined(PROJTEX \n\
#ifdef PROJTEX \n\
//per projector: \n\
uniform int pbackCull[8]; \n\
uniform int ntdesc[8]; \n\
uniform int pCount; \n\
varying vec4 projTexCoord[8]; \n\
varying vec4 projTexNorm[8]; \n\
//per texture descriptor (projector 1:m texdescriptor m:1 sampler): \n\
uniform int tunits[16]; \n\
uniform int modes[16]; \n\
uniform int sources[16]; \n\
uniform int funcs[16]; \n\
vec4 fragProjCalTexCoord(in vec4 frag_color) { \n\
	int k=0; \n\
	for(int i=0;i<pCount;i++) { \n\
		if( projTexCoord[i].q > 0.0 ){ \n\
			vec4 pp = projTexCoord[i]; \n\
			bool inside = (-pp.w < pp.x) && (pp.x < pp.w); \n\
			inside = inside && (-pp.w < pp.y) && (pp.y < pp.w); \n\
			inside = inside && (-pp.w < pp.z) && (pp.z < pp.w); \n\
			if(inside){ \n\
				bool facingProjector = true; \n\
				vec3 pptex = pp.xyz/pp.w; \n\
				if(pbackCull[i] == 1){ \n\
					vec3 pn = projTexNorm[i].xyz/projTexNorm[i].w; \n\
					//if(!gl_FrontFacing) pn = -pn; \n\
					vec3 nvec = normalize(pptex.xyz-pn); \n\
					vec3 peye = vec3(0.0,0.0,1.0); //normalize(pc); \n\
					float dotval = dot(nvec,peye); \n\
					facingProjector = (dotval > 0.0); \n\
				} \n\
				if(facingProjector){ \n\
					//parallel/ortho \n\
					vec2 ptex = pptex.xy; \n\
					ptex.x = (ptex.x * .5) + .5; \n\
					ptex.y = (ptex.y * .5) + .5; \n\
					int ndesc = ntdesc[i]; \n\
					vec4 prev = frag_color; \n\
					for(int j=0;j<ndesc;j++,k++){ \n\
						int kk = tunits[k]; \n\
						int modea = int(modes[k] / 100); \n\
						int mode = modes[k] - 100*modea; \n\
						finalColCalcA(prev, mode, modea, funcs[k], textureUnit[kk], ptex); \n\
					} \n\
					frag_color = prev;\n\
				} \n\
			} \n\
		} \n\
	} \n\
	return frag_color; \n\
} \n\
#endif //PROJTEX \n\
/* Wrapper for calling PLUG texture_coord_shift */ \n\
vec2 texture_coord_shifted(in vec2 tex_coord) \n\
{ \n\
	/* PLUG: texture_coord_shift (tex_coord) */ \n\
	return tex_coord; \n\
} \n\
//literal string size break \n" "\
//statics for multitexturing function\n\
vec3 mtex_specular; \n\
vec4 mtex_diffuse; \n\
//PHYSICAL LIGHTING >> \n\
// https://github.com/KhronosGroup/glTF-Sample-Viewer \n\
const float M_PI = 3.141592653589793; \n\
// sRGB to linear approximation \n\
const float GAMMA = 2.2; \n\
vec4 SRGBtoLINEAR(vec4 srgbIn) \n\
{ \n\
	return vec4(pow(srgbIn.xyz, vec3(GAMMA)), srgbIn.w); \n\
} \n\
const float INV_GAMMA = 1.0 / GAMMA; \n\
// linear to sRGB approximation \n\
vec3 LINEARtoSRGB(vec3 color) \n\
{ \n\
	return pow(color, vec3(INV_GAMMA)); \n\
} \n\
// << PhYSICAL LIGHTING \n\
//GETTERS \n\
fw_MaterialParameters mat = fw_FrontMaterial; \n\
// material.maps: iunit [0] normal [1] emissive [2] occlusion [3] diffuse OR base [4] shininess OR metallicRoughness [5] specular [6] ambient \n\
vec4 sample_map(int iunit, bool apply_gamma){ \n\
	#ifdef NOT_MTEX //not working \n\
		vec4 nc = vec4(1.0,1.0,1.0,1.0); \n\
		//vec4 nc = vec4(0.0,0.0,0.0,1.0); \n\
		int istart = mat.tstart[iunit];\n\
		int ndesc = mat.tcount[iunit]; \n\
		vec4 prev = nc; \n\
		for(int k=istart;k<(istart+ndesc);k++){ \n\
			int kk = mat.tindex[k]; \n\
			int modea = int(mat.mode[k] / 100); \n\
			int mode = mat.mode[k] - 100*modea; \n\
			vec3 ptex = fw_TexCoord[mat.cindex[iunit]]; \n\
			finalColCalcA(prev, mode, modea, mat.func[k], textureUnit[kk], ptex.xy); \n\
			//vec4 ncc = texture2D(textureUnit[kk],ptex.xy); \n\
			//prev.rgb = clamp(prev.rgb + ncc.rgb,0.0,1.0); \n\
		} \n\
		nc = prev; \n\
	#else //MTEX \n\
	#define CONFORMANT 1 \n\
	#ifdef CONFORMANT \n\
	int index = mat.tindex[mat.tstart[iunit]]; \n\
	vec2 tc = fw_TexCoord[mat.cindex[iunit]].xy; \n\
	vec4 nc = vec4(0); \n\
	#ifdef FULL \n\
	switch(index) { \n\
		case 0: nc = texture2D(textureUnit[0],tc); break; \n\
		case 1: nc = texture2D(textureUnit[1],tc); break; \n\
		case 2: nc = texture2D(textureUnit[2],tc); break; \n\
		case 3: nc = texture2D(textureUnit[3],tc); break; \n\
		case 4: nc = texture2D(textureUnit[4],tc); break; \n\
		case 5: nc = texture2D(textureUnit[5],tc); break; \n\
		case 6: nc = texture2D(textureUnit[6],tc); break; \n\
		case 7: nc = texture2D(textureUnit[7],tc); break; \n\
		case 8: nc = texture2D(textureUnit[8],tc); break; \n\
		case 9: nc = texture2D(textureUnit[9],tc); break; \n\
		case 10: nc = texture2D(textureUnit[10],tc); break; \n\
		case 11: nc = texture2D(textureUnit[11],tc); break; \n\
		case 12: nc = texture2D(textureUnit[12],tc); break; \n\
		case 13: nc = texture2D(textureUnit[13],tc); break; \n\
		case 14: nc = texture2D(textureUnit[14],tc); break; \n\
		case 15: nc = texture2D(textureUnit[15],tc); break; \n\
		default: break; \n\
	} \n\
	#else //FULL \n\
		//glsl 1.20 that goes with opengl 2.1 has trouble with switch \n\
		if(index < 8){ \n\
			if(index < 4){ \n\
				if(index < 2){ \n\
					if(index == 1) \n\
						nc = texture2D(textureUnit[1],tc); \n\
					else \n\
						nc = texture2D(textureUnit[0],tc); \n\
				}else{ \n\
					if(index == 3) \n\
						nc = texture2D(textureUnit[3],tc); \n\
					else \n\
						nc = texture2D(textureUnit[2],tc); \n\
				} \n\
			}else{\n\
				if(index < 6) { \n\
					if(index == 5) \n\
						nc = texture2D(textureUnit[5],tc); \n\
					else \n\
						nc = texture2D(textureUnit[4],tc); \n\
				}else{ \n\
					if(index == 7) \n\
						nc = texture2D(textureUnit[7],tc); \n\
					else \n\
						nc = texture2D(textureUnit[6],tc); \n\
				} \n\
			}\n\
		}else{ \n\
			if(index < 12){\n\
				if(index < 10){ \n\
					if(index == 9) \n\
						nc = texture2D(textureUnit[9],tc); \n\
					else \n\
						nc = texture2D(textureUnit[8],tc); \n\
				}else{ \n\
					if(index == 11) \n\
						nc = texture2D(textureUnit[11],tc); \n\
					else \n\
						nc = texture2D(textureUnit[10],tc); \n\
				} \n\
			}else{ \n\
				if(index < 14) { \n\
					if(index == 13) \n\
						nc = texture2D(textureUnit[13],tc); \n\
					else \n\
						nc = texture2D(textureUnit[12],tc); \n\
				}else{ \n\
					if(index == 15) \n\
						nc = texture2D(textureUnit[15],tc); \n\
					else \n\
						nc = texture2D(textureUnit[14],tc); \n\
				} \n\
			} \n\
		} \n\
	#endif //FULL \n\
	#else //CONFORMANT \n\
	vec4 nc = texture2D(textureUnit[mat.tindex[mat.tstart[iunit]]],fw_TexCoord[mat.cindex[iunit]].xy); \n\
	#endif //CONVORMANT \n\
	if(apply_gamma) nc = SRGBtoLINEAR(nc); \n\
	#endif //MTEX \n\
	return nc; \n\
} \n\
vec3 getNormal(){ \n\
	int normal_image = 0; \n\
	vec3 N = normalize (castle_normal_eye); \n\
	if (!gl_FrontFacing) //backFacing \n\
		N = -N; \n\
	if(mat.tcount[normal_image] > 0){ \n\
		// https://learnopengl.com/Advanced-Lighting/Normal-Mapping  \n\
		//texture transform applied in vertex shader \n\
		vec2 UV = fw_TexCoord[mat.cindex[normal_image]].xy; \n\
			\n\
		// Retrieve the tangent space matrix \n\
		vec3 pos_dx = dFdx(castle_vertex_eye.xyz); \n\
		vec3 pos_dy = dFdy(castle_vertex_eye.xyz); \n\
		vec3 tex_dx = dFdx(vec3(UV, 0.0)); \n\
		vec3 tex_dy = dFdy(vec3(UV, 0.0)); \n\
		vec3 t = (tex_dy.t * pos_dx - tex_dx.t * pos_dy) / (tex_dx.s * tex_dy.t - tex_dy.s * tex_dx.t); \n\
			\n\
		t = normalize(t - N * dot(N, t)); \n\
		vec3 b = normalize(cross(N, t)); \n\
		mat3 tbn = mat3(t, b, N); \n\
		//vec4 nc = texture2D(textureUnit[mat.tindex[mat.tstart[0]]],fw_TexCoord[mat.cindex[normal_image]].xy); \n\
		vec4 nc = sample_map(normal_image,false); \n\
		N = normalize(tbn * (2.0 * nc.xyz - 1.0)); \n\
	} \n\
	return N; \n\
} \n\
vec3 getEmissive(){ \n\
	vec3 E = mat.emissive; \n\
	int emissive_image = 1; \n\
	if(mat.type > 0 && mat.tcount[emissive_image] > 0){ \n\
		vec4 ec = sample_map(emissive_image,true); \n\
		E.rgb *= ec.rgb; \n\
	} \n\
	return E; \n\
} \n\
float getAlpha(){ \n\
	float A = 1.0; \n\
	if(mat.type > 0) { \n\
		A -= mat.transparency; \n\
		int transparency_image = 3; //diffuse or base image \n\
		if(mat.type == 1) transparency_image = 1; //emissive image \n\
		if(mat.tcount[transparency_image] > 0) { \n\
			vec4 dc = sample_map(transparency_image,false); \n\
			A *= dc.a; \n\
		} \n\
	} \n\
	return A; \n\
} \n\
float getOcclusion(){ \n\
	float occ = 0.0; \n\
	if(mat.type == 2 || mat.type == 3) { \n\
		int occlusion_image = 2; \n\
		if(mat.tcount[occlusion_image] > 0) { \n\
			occ = mat.occlusion; //occlusionStrength  \n\
			vec4 oc = sample_map(occlusion_image,false); \n\
			occ *= oc.r; //only the red \n\
		} \n\
	} \n\
	return occ; \n\
} \n\
vec3 getDiffuse(){ \n\
	vec3 D = mat.diffuse; \n\
	int diffuse_image = 3; \n\
	if(mat.type == 2 && mat.tcount[diffuse_image] > 0){ \n\
		vec4 dc = sample_map(diffuse_image,true); \n\
		D.rgb *= dc.rgb; \n\
	} \n\
	return D; \n\
} \n\
float getShininess() { \n\
	float S = mat.shininess; \n\
	int shininess_image = 4; \n\
	if(mat.type == 2 && mat.tcount[shininess_image] > 0){ \n\
		vec4 sc = sample_map(shininess_image,false); \n\
		S *= sc.a; \n\
	} \n\
	return S; \n\
} \n\
vec3 getSpecular() { \n\
	vec3 S = mat.specular; \n\
	int specular_image = 5; \n\
	if(mat.type == 2 && mat.tcount[specular_image] > 0){ \n\
		vec4 sc = sample_map(specular_image,true); \n\
		S.rgb *= sc.rgb; \n\
	} \n\
	return S; \n\
} \n\
float getAmbient(){ \n\
	float amb = mat.ambient; \n\
	int ambient_image = 6; \n\
	if(mat.type == 2 && mat.tcount[ambient_image] > 0){ \n\
		vec4 ac = sample_map(ambient_image,true); \n\
		amb *= ac.r; \n\
	} \n\
	return amb; \n\
} \n\
vec3 getBaseColor(){ \n\
	vec3 B = mat.baseColor; \n\
	int base_image = 3; \n\
	if(mat.type == 3 && mat.tcount[base_image] > 0){ \n\
		vec4 bc = sample_map(base_image,true); \n\
		B.rgb *= bc.rgb; \n\
	} \n\
	return B; \n\
} \n\
float getMetallic(){ \n\
	float met = mat.metallic; \n\
	int metallic_image = 4; \n\
	if(mat.type == 3 && mat.tcount[metallic_image] > 0){ \n\
		vec4 mr = sample_map(metallic_image,false); \n\
		met *= mr.b; \n\
	} \n\
	return met; \n\
} \n\
float getRoughness(){ \n\
	float rou = mat.roughness; \n\
	int roughness_image = 4; //same as metallic \n\
	if(mat.type == 3 && mat.tcount[roughness_image] > 0){ \n\
		vec4 mr = sample_map(roughness_image,false); \n\
		rou *= mr.g; \n\
	} \n\
	return rou; \n\
} \n\
vec4 getVertexColor() { \n\
	//H: this is supposed to be from vertex shader \n\
	vec4 color = vec4(1.0); \n\
	#ifdef CPV \n\
		color = cpv_Color; \n\
	#endif //CPV \n\
	return color; \n\
} \n\
vec4 getDiffuseOrBase(){ \n\
	return vec4(getBaseColor()*getDiffuse(),getAlpha()); \n\
} \n\
vec4 getDiffuseFactor() { \n\
	// https://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/lighting.html#Lightingoff \n\
	// table 17-2, 17-3 logic here \n\
	// function returns ODrgb (lit) or Irgb (unlit) \n\
	// MODT - freewrl out-of-spec option: alwasy modulate texture with diffuse \n\
	// MODC - illuminance texture, modulate texture with any CPV/CPF or mat.diffuse if no CPV \n\
	// MODA - texture has no interesting alpha, use material.diffuse.a \n\
	vec4 dcolor = vec4(1.0); \n\
	float mixcpv = 0.0; \n\
	#ifdef CPV \n\
	mixcpv = 1.0; \n\
	#endif //CPV \n\
	#ifdef TEX \n\
		#ifndef MODC \n\
		mixcpv = 0.0; \n\
		#endif //MODC \n\
	#endif //TEX \n\
	vec4 IC = getVertexColor(); \n\
	vec4 D = getDiffuseOrBase(); \n\
	dcolor *= mix(D,IC,mixcpv); \n\
	#ifdef TEX \n\
	if(textureCount > 0){ \n\
		vec3 N = getNormal(); \n\
		vec4 tcolor = vec4(1.0); \n\
		#if defined(MODT) || defined(MODC) \n\
			tcolor.rgb = dcolor.rgb; \n\
		#endif //MODT || MODC \n\
		/* PLUG: texture_apply (tcolor, N) */ \n\
		dcolor.rgb = tcolor.rgb; \n\
		#ifdef MODA \n\
			dcolor.a *= tcolor.a; \n\
		#else //MODA \n\
			dcolor.a = tcolor.a; \n\
		#endif //MODA \n\
	} \n\
	#endif //TEX \n\
	return dcolor; \n\
} \n\
vec4 getGouraudColor() { \n\
	vec4 dcolor = castle_Color; \n\
	#ifdef LIT\n\
	dcolor *= vec4(clamp(castle_ColorES + castle_Color.rgb,0.0,1.0),castle_Color.a); \n\
	#endif //LIT \n\
	#ifdef CPV \n\
		dcolor = cpv_Color; \n\
	#endif //CPV \n\
	#ifdef TEX \n\
	if(textureCount > 0){ \n\
		vec3 N = getNormal(); \n\
		vec4 tcolor = vec4(1.0); \n\
		//#if defined(MODT) || defined(MODC) \n\
			tcolor.rgb = dcolor.rgb; \n\
		//#endif //MODT || MODC \n\
		/* PLUG: texture_apply (tcolor, N) */ \n\
		dcolor.rgb = tcolor.rgb; \n\
		#ifdef MODA \n\
			dcolor.a *= tcolor.a; \n\
		#else //MODA \n\
			dcolor.a = tcolor.a; \n\
		#endif //MODA \n\
	} \n\
	#endif //TEX \n\
	return dcolor; \n\
} \n\
//literal string size break \n" "\
void main(void) \n\
{ \n\
//STEP0 MATERIALS \n\
	#ifdef LIT \n\
	mtex_specular = castle_ColorES; \n\
	mtex_diffuse = castle_Color; \n\
	#endif //LIT \n\
	mat = fw_FrontMaterial; \n\
	/* back Facing materials - flip the normal and grab back materials */ \n\
	//bool backFacing = (dot(N,E) < 0.0); \n\
	if (!gl_FrontFacing){ //backFacing) { \n\
		#ifdef TWO \n\
		mat = fw_BackMaterial; \n\
		#endif //TWO \n\
	} \n\
	vec3 N = getNormal(); \n\
	\n\
//STEP1 INITIALIZE \n\
	vec4 fragment_color; \n\
	#ifdef LINE \n\
		vec4 dcolor = vec4(1.0); \n\
		dcolor.rgb = getEmissive(); \n\
		#ifdef CPV \n\
		dcolor= getVertexColor(); \n\
		#endif //CVP \n\
		#ifdef POINTP \n\
			#ifdef TEX \n\
			if(textureCount > 0){ \n\
				vec3 N = getNormal(); \n\
				vec4 tcolor = vec4(1); \n\
				/* PLUG: texture_apply (tcolor, N) */ \n\
				if(u_pointColorMode == 1) dcolor.a = tcolor.a; \n\
				if(u_pointColorMode == 2) dcolor = tcolor; \n\
				if(u_pointColorMode == 3) { \n\
					dcolor.rgb += tcolor.rgb; \n\
					dcolor.a = tcolor.a;; \n\
				} \n\
			} \n\
			#endif //TEX \n\
		#endif //POINTP \n\
		fragment_color = dcolor; \n\
		#if defined(LINETYPE) && defined(FULL) \n\
		if(u_linetype > 1) \n\
			if(!on_linetype(fragment_color)){ \n\
				discard; \n\
				//fragment_color.a = 0.0; \n\
			} \n\
		#endif //LINETYPE \n\
	#else //LINE \n\
		vec4 diffuseFactor = getDiffuseFactor(); \n\
		fragment_color =  diffuseFactor; \n\
//STEP2 LIGHTS \n\
	#ifndef PHONG \n\
		fragment_color = getGouraudColor(); \n\
	#endif //not PHONG \n\
	#ifdef PHONG \n\
	//per-fragment lighting aka PHONG \n\
	if(mat.type == 2){ \n\
		//MAT_REGULAR \n\
		#ifdef LITE \n\
		//start over with the color, since we have material and lighting in here \n\
		vec3 cumulative_specular = vec3(0.0,0.0,0.0); \n\
		vec3 cumulative_diffuse = vec3(0.0,0.0,0.0); \n\
		float shiny = getShininess(); \n\
		float amby = getAmbient(); \n\
		vec3 diffy = diffuseFactor.rgb; //getDiffuse(); \n\
		vec3 specy = getSpecular(); \n\
		vec3 normy = getNormal(); \n\
		float occy = getOcclusion(); \n\
		/* PLUG: add_light_contribution2 (cumulative_diffuse, cumulative_specular, castle_vertex_eye, normy, shiny, amby, diffy, specy) */ \n\
		fragment_color.rgb = cumulative_diffuse + cumulative_specular; \n\
		fragment_color.rgb *= (1.0 - occy); \n\
		//fragment_color.rgb = clamp(fragment_color.rgb,0.0,1.0); \n\
		#endif //LITE \n\
	} else if(mat.type == 3){ \n\
		//MAT_PHYSICAL \n\
		#ifdef LITE \n\
		float metallic = getMetallic(); \n\
		float perceptualRoughness = getRoughness(); \n\
		vec3 baseColor = getBaseColor(); \n\
		//unlit \n\
		vec3 specularColor= vec3(0.0); \n\
	    vec3 f0 = vec3(0.04); \n\
		// ?? baseColor *= getVertexColor().xyz; //hunh? \n\
		vec3 diffuseColor = baseColor.rgb * (vec3(1.0) - f0) * (1.0 - metallic); \n\
		specularColor = mix(f0, baseColor.rgb, metallic); \n\
		//lit \n\
		float alphaRoughness = perceptualRoughness * perceptualRoughness; \n\
		// Compute reflectance. \n\
		float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b); \n\
		vec3 specularEnvironmentR0 = specularColor.rgb; \n\
		// Anything less than 2% is physically impossible and is instead considered to be shadowing. \n\
		vec3 specularEnvironmentR90 = vec3(clamp(reflectance * 50.0, 0.0, 1.0)); \n\
		MaterialInfo materialInfo = MaterialInfo( \n\
			perceptualRoughness, \n\
			specularEnvironmentR0, \n\
			alphaRoughness, \n\
			diffuseColor, \n\
			specularEnvironmentR90, \n\
			specularColor \n\
		); \n\
		// LIGHTING \n\
		vec3 color = vec3(0.0, 0.0, 0.0); \n\
		vec3 normal = getNormal(); \n\
		vec3 view = normalize(- castle_vertex_eye.xyz); //hunh?? thought our v_Position was already in Eye space \n\
		float occy = getOcclusion(); \n\
		//color += apply_lights_physical( materialInfo, normal, view ); \n\
		/* PLUG: add_light_physical (color, castle_vertex_eye.xyz, normal, materialInfo ) */  \n\
		color *= (1.0 - occy); \n\
		fragment_color.rgb = color; \n\
		#endif //LITE \n\
	} \n\
	#endif //PHONG \n\
	\n\
	#ifdef FILL \n\
	//fillPropCalc(fragment_color, hatchPosition); \n\
	/* PLUG: fragment_fillPropertiesApply (fragment_color, hatchPosition) */ \n\
	#endif //FILL \n\
	\n\
	#ifdef NOT_TEX \n\
	if(textureCount > 0){ \n\
		#ifndef MODA \n\
		fragment_color.a = 1.0; \n\
		#endif //MODA \n\
		#ifndef MODC \n\
		fragment_color.rgb = vec3(1.0); \n\
		#endif //MODC \n\
		/* PLUG: texture_apply (fragment_color, N) */ \n\
	} \n\
	#endif //TEX \n\
  \n\
//STEP3 PROJECTORS AND IBL image based lighting \n\
	#ifdef PROJTEX \n\
	fragment_color = fragProjCalTexCoord(fragment_color); \n\
	#endif //PROJTEX \n\
	\n\
//STEP4 OCCLUSION \n\
	/* PLUG: steep_parallax_shadow_apply (fragment_color) */ \n\
//STEP5 EMISSIVE \n\
	if(mat.type == 1) { \n\
		fragment_color.rgb = getEmissive(); \n\
		fragment_color.a = getAlpha(); \n\
	}else if(mat.type > 1){ \n\
		fragment_color.rgb += getEmissive(); \n\
	} \n\
	#endif //LINE \n\
	#ifdef NOT_LINE \n\
	fragment_color.rgb = getEmissive(); \n\
	#endif //LINE \n\
	#ifdef NOT_CPV \n\
	if(mat.type == 0) { \n\
		fragment_color = cpv_Color; //no mat to modulate with \n\
	}else{ \n\
		#ifndef MODC \n\
		fragment_color = vec4(1.0); \n\
		#endif //MODC \n\
		fragment_color *= cpv_Color; //CPV modulates prior \n\
	} \n\
	#endif //CPV \n\
	\n\
//STEP6 FOG \n\
	#ifdef  NOT_FOG \n\
		gl_FragColor = vec4(0.0,1.0,1.0,1.0); \n\
		return; \n\
	#endif //FOG \n\
	/* PLUG: fog_apply (fragment_color, N) */ \n\
	\n\
	fragment_color.rgb = LINEARtoSRGB(fragment_color.rgb); \n\
	gl_FragColor = fragment_color; \n\
	\n\
	/* PLUG: fragment_end (gl_FragColor) */ \n\
} \n";



static const GLchar *plug_fragment_fillProperties_apply = "\
//FILL \n\
float either(float x, float y){ \n\
	//returns 1 if either are > 0, else 0 \n\
	return step(.5,x+y); \n\
} \n\
float inrange(float curpos, float fx, float linewidth){ \n\
	//returns 1.0 if on line, else 0.0 \n\
	//return step(fx-linewidth*.5,curpos) - step(fx+linewidth*.5,curpos);; \n\
	//either in this cycle or (with +linewidth) the prior cycle \n\
	float fxfloor = floor(fx+linewidth); \n\
	//return step(ffx,curpos) - step(fract(ffx+linewidth),curpos); \n\
	return either(step(fx,curpos) - step(fx+linewidth,curpos),step(fx-fxfloor,curpos) - step(fx-fxfloor+linewidth,curpos)); \n\
	//return either(step(fx,curpos) - step(fx+linewidth,curpos),step(fx-1.0,curpos) - step(fx+linewidth-1.0,curpos)); \n\
} \n\
float inrange3(float curpos, float fx, float linewidth, float cycle_height){ \n\
	//returns 1.0 if on line, else 0.0 \n\
	float inside = 0.0; \n\
	if(cycle_height < 1.0){ \n\
		float ncycle = 1.0/cycle_height; \n\
		int ny = int(ceil(ncycle)) +2; \n\
		for(int i=0;i<ny;i++){ \n\
			float ffx = fx + float(i-2)*cycle_height; \n\
			inside = either(inside,step(ffx,curpos)-step(ffx+linewidth,curpos)); \n\
		} \n\
	}else if(cycle_height > 1.0){ \n\
		float ncycle = cycle_height; \n\
		int ny = int(ceil(ncycle)) +1; \n\
		for(int i=0;i<ny;i++){ \n\
			float ffx = fx - float(i); \n\
			inside = either(inside,step(ffx,curpos)-step(ffx+linewidth,curpos)); \n\
		} \n\
	} else { \n\
		inside = either(step(fx,curpos) - step(fx+linewidth,curpos),step(fx-1.0,curpos) - step(fx+linewidth-1.0,curpos)); \n\
	} \n\
	return inside; \n\
} \n\
float rand(float n){return fract(sin(n) * 43758.5453123);} \n\
float noise(float p){ \n\
	float fl = floor(p); \n\
  float fc = fract(p); \n\
	return mix(rand(fl), rand(fl + 1.0), fc); \n\
} \n\
float rand(vec2 c){ \n\
	return fract(sin(dot(c.xy ,vec2(12.9898,78.233))) * 43758.5453); \n\
} \n\
//literal string size break \n" "\
//#ifndef FULL \n\
void PLUG_fragment_fillPropertiesApply(inout vec4 prevColour, vec2 MCposition) { \n\
	// written as procedural texture \n\
	// http://learnwebgl.brown37.net/10_surface_properties/texture_mapping_procedural.html \n\
	// https://thebookofshaders.com/05/ \n\
	// https://isotc.iso.org/livelink/livelink/fetch/-8916524/8916549/8916590/6208440/class_pages/hatchstyle.html \n\
	// instead of y = f(x), you set fx = f(current_x)); (where y would need to be, to be on the line) \n\
	// then test if current_y is in range(fx-linewidth/2,fx+linewidth/2) \n\
	// the x and y are more conveniently processed in cycle-space if you have a repeating pattern \n\
	// so if your pattern repeats 10 times per 1 unit of texture coordinates, your cycle is 1/10 = .1 in size \n\
	vec4 colour; \n\
	vec2 position; // position in cycle, as cycle fraction \n\
	float cyclesize; //in texcoords \n\
	float linewidth; //in cycle space \n\
	position = MCposition; // /HatchScale; \n\
	vec2 percent = vec2(0); //fraction of background color to show, usually 1 or 0 \n\
	float fx; // f(x) evaluated at x = cyclepostion.x \n\
	float fxrange; //normally the pattern is square, if not fxrange is the height, assuming width is 1 \n\
	\n\
	int ha = fillprops.HatchAlgo; \n\
	switch(ha) { \n\
	case 0: // horizontal lines \n\
	case 1: \n\
		cyclesize = .1; \n\
		linewidth = .5; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = .5; \n\
		percent.x = 1.0 - inrange(position.y,fx,linewidth); \n\
		break; \n\
	case 2: // vertical lines \n\
		cyclesize = .1; \n\
		linewidth = .5; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = .5; \n\
		percent.x = 1.0 - inrange(position.x,fx,linewidth); \n\
		break; \n\
	case 3: // positive diagonals \n\
		cyclesize = .1; \n\
		linewidth = .5; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = position.x; \n\
		percent.x = 1.0 - inrange(position.y,fx,linewidth); \n\
		break; \n\
	case 4: //negative diagonals \n\
		cyclesize = .1; \n\
		linewidth = .5; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = 1.0-position.x; \n\
		percent.x = 1.0 - inrange(position.y,fx,linewidth); \n\
		break; \n\
	case 5: // # hv cross hatching \n\
		cyclesize = .1; \n\
		linewidth = .25; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = .5; \n\
		percent.y = inrange(position.y,fx,linewidth); \n\
		percent.x = inrange(position.x,fx,linewidth); \n\
		percent.x = 1.0 - either(percent.x, percent.y); \n\
		break; \n\
	case 6: // diagonal crosshatch \n\
		cyclesize = .1; \n\
		linewidth = .25; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = position.x; \n\
		percent.x = inrange(position.y,fx,linewidth); \n\
		fx = 1.0-position.x; \n\
		percent.y = inrange(position.y,fx,linewidth); \n\
		percent.x = 1.0 - either(percent.x,percent.y); \n\
		break; \n\
	case 7: //7 positive diagonals wide \n\
		cyclesize = .2; \n\
		linewidth = .25; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = position.x; \n\
		percent.x = 1.0 - inrange(position.y,fx,linewidth); \n\
		break; \n\
	case 8: //8 double positive diagonals, candycane \n\
		cyclesize = .4; \n\
		linewidth = .15; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = position.x; \n\
		percent.x = inrange(position.y,fx,linewidth); \n\
		cyclesize = .4; \n\
		linewidth = .15; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = position.x + .25; \n\
		percent.y = inrange(position.y,fx,linewidth); \n\
		percent.x = 1.0 - either(percent.x,percent.y); \n\
		break; \n\
	case 9: //9 positive diagonal dash-diagonal \n\
		//solid diagonal \n\
		cyclesize = .4; \n\
		linewidth = .15; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = position.x; \n\
		percent.x = inrange(position.y,fx,linewidth); \n\
		//dash it with negative diagonal \n\
		cyclesize = .4; \n\
		linewidth = .5; \n\
		fx = 1.0 - position.x; \n\
		percent.y = inrange(position.y,fx,linewidth); \n\
		percent.x = percent.x*percent.y; \n\
		//solid diagonal \n\
		cyclesize = .4; \n\
		linewidth = .15; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = position.x + .5; \n\
		percent.y = inrange(position.y,fx,linewidth); \n\
		percent.x = 1.0 - either(percent.x,percent.y); \n\
		break; \n\
	case 10: //10 wide diagonal crosshatch \n\
		cyclesize = .2; \n\
		linewidth = .2; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = position.x; \n\
		percent.x = inrange(position.y,fx,linewidth); \n\
		fx = 1.0-position.x; \n\
		percent.y = inrange(position.y,fx,linewidth); \n\
		percent.x = 1.0 - either(percent.x,percent.y); \n\
		break; \n\
	case 11: //11 positive diagonal railroad \n\
		//negative diagonal for railroad ties \n\
		cyclesize = .2; \n\
		linewidth = .15; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = 1.0 - position.x; \n\
		percent.x = inrange(position.y,fx,linewidth); \n\
		//dash it with positive diagonal \n\
		cyclesize = .2; \n\
		linewidth = .5; \n\
		fx = position.x; \n\
		percent.y = inrange(position.y,fx,linewidth); \n\
		percent.x = percent.x*percent.y; \n\
		//HV cross hatch to remove every second tie \n\
		cyclesize = .2; \n\
		linewidth = .5; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = .5; \n\
		percent.y = inrange(position.y,fx,linewidth); \n\
		percent.x = percent.x*percent.y; \n\
		//add solid diagonals \n\
		cyclesize = .1; \n\
		linewidth = .25; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = position.x + .5; \n\
		percent.y = inrange(position.y,fx,linewidth); \n\
		percent.x = 1.0 - either(percent.x,percent.y); \n\
		break; \n\
	case 12: // 12 4 +diag, 4 spaces \n\
		//diagonal fill \n\
		cyclesize = .1; \n\
		linewidth = .4; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = position.x; \n\
		percent.x = inrange(position.y,fx,linewidth); \n\
		//remove diagonals \n\
		cyclesize = .8; \n\
		linewidth = .5; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = position.x; \n\
		percent.y = inrange(position.y,fx,linewidth); \n\
		percent.x = 1.0 - percent.x*percent.y; \n\
		break; \n\
	case 13: //13 cork horizontal dashes \n\
		//horizontals \n\
		cyclesize = .1; \n\
		linewidth = .25; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = .5; \n\
		percent.x = inrange(position.y,fx,linewidth); \n\
		//dash using +ve diags \n\
		cyclesize = .3; \n\
		linewidth = .25; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = .5 * position.x; \n\
		fxrange = .5 * 1.0; \n\
		percent.y = 1.0 - inrange3(position.y,fx,linewidth,fxrange); \n\
		percent.x = 1.0 - percent.x*percent.y; \n\
		break; \n\
	case 14: //steps over +ve diags \n\
		//HV grid for steps \n\
		cyclesize = .1; \n\
		linewidth = .25; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = .5; \n\
		percent.y = inrange(position.y,fx,linewidth); \n\
		percent.x = inrange(position.x,fx,linewidth); \n\
		percent.x = either(percent.x, percent.y); \n\
		//clear out parts of grid with +ve diag \n\
		cyclesize = .2; \n\
		linewidth = .45; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = position.x + .52; \n\
		percent.y = 1.0 - inrange(position.y,fx,linewidth); \n\
		percent.x = percent.x*percent.y; \n\
		//add +ve diag over steps \n\
		cyclesize = .2; \n\
		linewidth = .15; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = position.x + .2; \n\
		percent.y = inrange(position.y,fx,linewidth); \n\
		percent.x = 1.0 - either(percent.x,percent.y); \n\
		break; \n\
	case 15: // titaniaum diag diag-dash diag \n\
		//diagonal for dashing \n\
		cyclesize = .4; \n\
		linewidth = .05; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = position.x + .25; \n\
		percent.x = inrange(position.y,fx,linewidth); \n\
		//dash with negative diagonal \n\
		cyclesize = .3; \n\
		linewidth = .2; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = 1.0 - position.x; \n\
		percent.y = 1.0 - inrange(position.y,fx,linewidth); \n\
		percent.x = percent.x*percent.y; \n\
		//solid diagonals \n\
		cyclesize = .2; \n\
		linewidth = .1; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = position.x; \n\
		percent.y = inrange(position.y,fx,linewidth); \n\
		percent.x = 1.0 - either(percent.x,percent.y); \n\
		break; \n\
	case 16: //marble diag-dash \n\
		//diagonal for dashing \n\
		cyclesize = .2; \n\
		linewidth = .1; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = position.x; \n\
		percent.x = inrange(position.y,fx,linewidth); \n\
		//dash with negative diagonal \n\
		cyclesize = .2; \n\
		linewidth = .2; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = 1.0 - position.x; \n\
		percent.y = 1.0 - inrange(position.y,fx,linewidth); \n\
		percent.x = 1.0 - percent.x*percent.y; \n\
		break; \n\
	case 17: //earth 5 diags erasing -ve diags \n\
		//negaative diags \n\
		cyclesize = .1; \n\
		linewidth = .1; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = 1.0 - .5*position.x; \n\
		fxrange = 1.0 - .5*1.0; \n\
		percent.x = inrange3(position.y,fx,linewidth,fxrange); \n\
		//clear gaps with thick +ve diags \n\
		cyclesize = .5; \n\
		linewidth = .4; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = position.x; \n\
		percent.y = 1.0 - inrange(position.y,fx,linewidth); \n\
		percent.x = percent.x*percent.y; \n\
		// add 5 diagonals in gap \n\
		vec2 percent2 = vec2(0.0); \n\
		//diagonal fill \n\
		cyclesize = .05; \n\
		linewidth = .2; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = position.x; \n\
		percent2.x = inrange(position.y,fx,linewidth); \n\
		//remove diagonals \n\
		cyclesize = .5; \n\
		linewidth = .5; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = position.x +.5; \n\
		percent2.y = 1.0 - inrange(position.y,fx,linewidth); \n\
		percent2.x = percent2.x*percent2.y; \n\
		percent.x = 1.0 - either(percent.x,percent2.x); \n\
		break; \n\
	case 18: //sand randcom dots \n\
		cyclesize = 1.0; \n\
		linewidth = .1; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = rand(position); \n\
		position.x = linewidth*floor(position.x/linewidth); \n\
		//fx = noise(position.x*position.y); \n\
		percent.x = inrange(position.y,fx,linewidth); \n\
		percent.y = inrange(position.x,fx,linewidth); \n\
		percent.x = 1.0 - either(percent.x,percent.y); \n\
		break; \n\
	case 19: //repeating stanggerd rows of dots \n\
		// use a find diagonal crosshatch \n\
		cyclesize = .05; \n\
		linewidth = .5; \n\
		position = fract(MCposition/cyclesize); \n\
		fx = position.x; \n\
		percent.x = inrange(position.y,fx,linewidth); \n\
		fx = 1.0-position.x; \n\
		percent.y = inrange(position.y,fx,linewidth); \n\
		percent.x = either(percent.x,percent.y); \n\
		break; \n\
	} \n\
	\n\
	if (fillprops.filled) {colour = prevColour;} else { colour=vec4(0.,0.,0.,0); }\n\
	if (fillprops.hatched) { \n\
		//colour = mix(fillprops.HatchColour, colour, useBrick.x * useBrick.y); \n\
		colour = mix(fillprops.HatchColour, colour, percent.x ); \n\
	} \n\
	prevColour = colour; \n\
} \n\
\n";

static const GLchar *plug_fragment_fillProperties_apply_120 = "\
//FILL \n\
float either(float x, float y){ \n\
	//returns 1 if either are > 0, else 0 \n\
	return step(.5,x+y); \n\
} \n\
float inrange(float curpos, float fx, float linewidth){ \n\
	//returns 1.0 if on line, else 0.0 \n\
	//return step(fx-linewidth*.5,curpos) - step(fx+linewidth*.5,curpos);; \n\
	//either in this cycle or (with +linewidth) the prior cycle \n\
	float fxfloor = floor(fx+linewidth); \n\
	//return step(ffx,curpos) - step(fract(ffx+linewidth),curpos); \n\
	return either(step(fx,curpos) - step(fx+linewidth,curpos),step(fx-fxfloor,curpos) - step(fx-fxfloor+linewidth,curpos)); \n\
	//return either(step(fx,curpos) - step(fx+linewidth,curpos),step(fx-1.0,curpos) - step(fx+linewidth-1.0,curpos)); \n\
} \n\
float inrange3(float curpos, float fx, float linewidth, float cycle_height){ \n\
	//returns 1.0 if on line, else 0.0 \n\
	float inside = 0.0; \n\
	if(cycle_height < 1.0){ \n\
		float ncycle = 1.0/cycle_height; \n\
		int ny = int(ceil(ncycle)) +2; \n\
		for(int i=0;i<ny;i++){ \n\
			float ffx = fx + float(i-2)*cycle_height; \n\
			inside = either(inside,step(ffx,curpos)-step(ffx+linewidth,curpos)); \n\
		} \n\
	}else if(cycle_height > 1.0){ \n\
		float ncycle = cycle_height; \n\
		int ny = int(ceil(ncycle)) +1; \n\
		for(int i=0;i<ny;i++){ \n\
			float ffx = fx - float(i); \n\
			inside = either(inside,step(ffx,curpos)-step(ffx+linewidth,curpos)); \n\
		} \n\
	} else { \n\
		inside = either(step(fx,curpos) - step(fx+linewidth,curpos),step(fx-1.0,curpos) - step(fx+linewidth-1.0,curpos)); \n\
	} \n\
	return inside; \n\
} \n\
float rand(float n){return fract(sin(n) * 43758.5453123);} \n\
float noise(float p){ \n\
	float fl = floor(p); \n\
  float fc = fract(p); \n\
	return mix(rand(fl), rand(fl + 1.0), fc); \n\
} \n\
float rand(vec2 c){ \n\
	return fract(sin(dot(c.xy ,vec2(12.9898,78.233))) * 43758.5453); \n\
} \n\
//literal string size break \n" "\
//#else //FULL \n\
//literal string size break \n" "\
void PLUG_fragment_fillPropertiesApply(inout vec4 prevColour, vec2 MCposition) { \n\
	// written as procedural texture \n\
	// http://learnwebgl.brown37.net/10_surface_properties/texture_mapping_procedural.html \n\
	// https://thebookofshaders.com/05/ \n\
	// https://isotc.iso.org/livelink/livelink/fetch/-8916524/8916549/8916590/6208440/class_pages/hatchstyle.html \n\
	// instead of y = f(x), you set fx = f(current_x)); (where y would need to be, to be on the line) \n\
	// then test if current_y is in range(fx-linewidth/2,fx+linewidth/2) \n\
	// the x and y are more conveniently processed in cycle-space if you have a repeating pattern \n\
	// so if your pattern repeats 10 times per 1 unit of texture coordinates, your cycle is 1/10 = .1 in size \n\
	vec4 colour; \n\
	vec2 position; // position in cycle, as cycle fraction \n\
	float cyclesize; //in texcoords \n\
	float linewidth; //in cycle space \n\
	position = MCposition; // /HatchScale; \n\
	vec2 percent = vec2(0); //fraction of background color to show, usually 1 or 0 \n\
	float fx; // f(x) evaluated at x = cyclepostion.x \n\
	float fxrange; //normally the pattern is square, if not fxrange is the height, assuming width is 1 \n\
	\n\
	int ha = fillprops.HatchAlgo; \n\
	if(ha < 10){ \n\
		if(ha < 5) { \n\
			if(ha < 3) { \n\
				if(ha < 2) { // horizontal lines \n\
					//case 1: \n\
					cyclesize = .1; \n\
					linewidth = .5; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = .5; \n\
					percent.x = 1.0 - inrange(position.y,fx,linewidth); \n\
				}else if(ha == 2) { \n\
					//case 2: // vertical lines \n\
					cyclesize = .1; \n\
					linewidth = .5; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = .5; \n\
					percent.x = 1.0 - inrange(position.x,fx,linewidth); \n\
				} \n\
			}else{ //ha < 3 \n\
				if(ha == 3) { \n\
				//case 3: // positive diagonals \n\
					cyclesize = .1; \n\
					linewidth = .5; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = position.x; \n\
					percent.x = 1.0 - inrange(position.y,fx,linewidth); \n\
				}else{ \n\
					//case 4: //negative diagonals \n\
					cyclesize = .1; \n\
					linewidth = .5; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = 1.0-position.x; \n\
					percent.x = 1.0 - inrange(position.y,fx,linewidth); \n\
				}; \n\
			} //if else ha < 3 \n\
		}else{ //ha < 5 \n\
			if(ha < 8) { \n\
				if(ha == 5) { \n\
					//case 5: // # hv cross hatching \n\
					cyclesize = .1; \n\
					linewidth = .25; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = .5; \n\
					percent.y = inrange(position.y,fx,linewidth); \n\
					percent.x = inrange(position.x,fx,linewidth); \n\
					percent.x = 1.0 - either(percent.x, percent.y); \n\
				}else if(ha==6){ \n\
					//case 6: // diagonal crosshatch \n\
					cyclesize = .1; \n\
					linewidth = .25; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = position.x; \n\
					percent.x = inrange(position.y,fx,linewidth); \n\
					fx = 1.0-position.x; \n\
					percent.y = inrange(position.y,fx,linewidth); \n\
					percent.x = 1.0 - either(percent.x,percent.y); \n\
				}else if(ha==7){ \n\
					//case 7: //7 positive diagonals wide \n\
					cyclesize = .2; \n\
					linewidth = .25; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = position.x; \n\
					percent.x = 1.0 - inrange(position.y,fx,linewidth); \n\
				} \n\
		}else{ //ha < 8 \n\
			if(ha == 8) { \n\
				//case 8: //8 double positive diagonals, candycane \n\
				cyclesize = .4; \n\
				linewidth = .15; \n\
				position = fract(MCposition/cyclesize); \n\
				fx = position.x; \n\
				percent.x = inrange(position.y,fx,linewidth); \n\
				cyclesize = .4; \n\
				linewidth = .15; \n\
				position = fract(MCposition/cyclesize); \n\
				fx = position.x + .25; \n\
				percent.y = inrange(position.y,fx,linewidth); \n\
				percent.x = 1.0 - either(percent.x,percent.y); \n\
			}else if(ha==9) { \n\
				//case 9: //9 positive diagonal dash-diagonal \n\
				//solid diagonal \n\
				cyclesize = .4; \n\
				linewidth = .15; \n\
				position = fract(MCposition/cyclesize); \n\
				fx = position.x; \n\
				percent.x = inrange(position.y,fx,linewidth); \n\
				//dash it with negative diagonal \n\
				cyclesize = .4; \n\
				linewidth = .5; \n\
				fx = 1.0 - position.x; \n\
				percent.y = inrange(position.y,fx,linewidth); \n\
				percent.x = percent.x*percent.y; \n\
				//solid diagonal \n\
				cyclesize = .4; \n\
				linewidth = .15; \n\
				position = fract(MCposition/cyclesize); \n\
				fx = position.x + .5; \n\
				percent.y = inrange(position.y,fx,linewidth); \n\
				percent.x = 1.0 - either(percent.x,percent.y); \n\
			}\n\
			} //if-else ha < 8 \n\
		} // if-else ha < 5 \n\
	} else { //ha<10 \n\
		if(ha < 15) { \n\
			if(ha < 13) {\n\
				if(ha == 10) { \n\
					//case 10: //10 wide diagonal crosshatch \n\
					cyclesize = .2; \n\
					linewidth = .2; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = position.x; \n\
					percent.x = inrange(position.y,fx,linewidth); \n\
					fx = 1.0-position.x; \n\
					percent.y = inrange(position.y,fx,linewidth); \n\
					percent.x = 1.0 - either(percent.x,percent.y); \n\
				}else if(ha==11){ \n\
					//case 11: //11 positive diagonal railroad \n\
					//negative diagonal for railroad ties \n\
					cyclesize = .2; \n\
					linewidth = .15; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = 1.0 - position.x; \n\
					percent.x = inrange(position.y,fx,linewidth); \n\
					//dash it with positive diagonal \n\
					cyclesize = .2; \n\
					linewidth = .5; \n\
					fx = position.x; \n\
					percent.y = inrange(position.y,fx,linewidth); \n\
					percent.x = percent.x*percent.y; \n\
					//HV cross hatch to remove every second tie \n\
					cyclesize = .2; \n\
					linewidth = .5; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = .5; \n\
					percent.y = inrange(position.y,fx,linewidth); \n\
					percent.x = percent.x*percent.y; \n\
					//add solid diagonals \n\
					cyclesize = .1; \n\
					linewidth = .25; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = position.x + .5; \n\
					percent.y = inrange(position.y,fx,linewidth); \n\
					percent.x = 1.0 - either(percent.x,percent.y); \n\
				}else if(ha==12){ \n\
					//case 12: // 12 4 +diag, 4 spaces \n\
					//diagonal fill \n\
					cyclesize = .1; \n\
					linewidth = .4; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = position.x; \n\
					percent.x = inrange(position.y,fx,linewidth); \n\
					//remove diagonals \n\
					cyclesize = .8; \n\
					linewidth = .5; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = position.x; \n\
					percent.y = inrange(position.y,fx,linewidth); \n\
					percent.x = 1.0 - percent.x*percent.y; \n\
				} \n\
			} else { //ha < 13 \n\
				if(ha == 13) { \n\
					//case 13: //13 cork horizontal dashes \n\
					//horizontals \n\
					cyclesize = .1; \n\
					linewidth = .25; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = .5; \n\
					percent.x = inrange(position.y,fx,linewidth); \n\
					//dash using +ve diags \n\
					cyclesize = .3; \n\
					linewidth = .25; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = .5 * position.x; \n\
					fxrange = .5 * 1.0; \n\
					percent.y = 1.0 - inrange3(position.y,fx,linewidth,fxrange); \n\
					percent.x = 1.0 - percent.x*percent.y; \n\
				}else{ \n\
					//case 14: //steps over +ve diags \n\
					//HV grid for steps \n\
					cyclesize = .1; \n\
					linewidth = .25; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = .5; \n\
					percent.y = inrange(position.y,fx,linewidth); \n\
					percent.x = inrange(position.x,fx,linewidth); \n\
					percent.x = either(percent.x, percent.y); \n\
					//clear out parts of grid with +ve diag \n\
					cyclesize = .2; \n\
					linewidth = .45; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = position.x + .52; \n\
					percent.y = 1.0 - inrange(position.y,fx,linewidth); \n\
					percent.x = percent.x*percent.y; \n\
					//add +ve diag over steps \n\
					cyclesize = .2; \n\
					linewidth = .15; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = position.x + .2; \n\
					percent.y = inrange(position.y,fx,linewidth); \n\
					percent.x = 1.0 - either(percent.x,percent.y); \n\
				} \n\
			} //if-else ha < 13 \n\
		} else { //ha < 15 \n\
			if(ha < 18) { \n\
				if(ha == 15) { \n\
					//case 15: // titaniaum diag diag-dash diag \n\
					//diagonal for dashing \n\
					cyclesize = .4; \n\
					linewidth = .05; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = position.x + .25; \n\
					percent.x = inrange(position.y,fx,linewidth); \n\
					//dash with negative diagonal \n\
					cyclesize = .3; \n\
					linewidth = .2; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = 1.0 - position.x; \n\
					percent.y = 1.0 - inrange(position.y,fx,linewidth); \n\
					percent.x = percent.x*percent.y; \n\
					//solid diagonals \n\
					cyclesize = .2; \n\
					linewidth = .1; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = position.x; \n\
					percent.y = inrange(position.y,fx,linewidth); \n\
					percent.x = 1.0 - either(percent.x,percent.y); \n\
				}else if(ha == 16) { \n\
					//case 16: //marble diag-dash \n\
					//diagonal for dashing \n\
					cyclesize = .2; \n\
					linewidth = .1; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = position.x; \n\
					percent.x = inrange(position.y,fx,linewidth); \n\
					//dash with negative diagonal \n\
					cyclesize = .2; \n\
					linewidth = .2; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = 1.0 - position.x; \n\
					percent.y = 1.0 - inrange(position.y,fx,linewidth); \n\
					percent.x = 1.0 - percent.x*percent.y; \n\
				} else if(ha == 17) { \n\
					//case 17: //earth 5 diags erasing -ve diags \n\
					//negaative diags \n\
					cyclesize = .1; \n\
					linewidth = .1; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = 1.0 - .5*position.x; \n\
					fxrange = 1.0 - .5*1.0; \n\
					percent.x = inrange3(position.y,fx,linewidth,fxrange); \n\
					//clear gaps with thick +ve diags \n\
					cyclesize = .5; \n\
					linewidth = .4; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = position.x; \n\
					percent.y = 1.0 - inrange(position.y,fx,linewidth); \n\
					percent.x = percent.x*percent.y; \n\
					// add 5 diagonals in gap \n\
					vec2 percent2 = vec2(0.0); \n\
					//diagonal fill \n\
					cyclesize = .05; \n\
					linewidth = .2; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = position.x; \n\
					percent2.x = inrange(position.y,fx,linewidth); \n\
					//remove diagonals \n\
					cyclesize = .5; \n\
					linewidth = .5; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = position.x +.5; \n\
					percent2.y = 1.0 - inrange(position.y,fx,linewidth); \n\
					percent2.x = percent2.x*percent2.y; \n\
					percent.x = 1.0 - either(percent.x,percent2.x); \n\
				} \n\
			}else{ //ha < 18 \n\
				if(ha == 18) { \n\
					//case 18: //sand randcom dots \n\
					cyclesize = 1.0; \n\
					linewidth = .1; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = rand(position); \n\
					position.x = linewidth*floor(position.x/linewidth); \n\
					//fx = noise(position.x*position.y); \n\
					percent.x = inrange(position.y,fx,linewidth); \n\
					percent.y = inrange(position.x,fx,linewidth); \n\
					percent.x = 1.0 - either(percent.x,percent.y); \n\
				} else if(ha == 19) { \n\
					//case 19: //repeating stanggerd rows of dots \n\
					// use a find diagonal crosshatch \n\
					cyclesize = .05; \n\
					linewidth = .5; \n\
					position = fract(MCposition/cyclesize); \n\
					fx = position.x; \n\
					percent.x = inrange(position.y,fx,linewidth); \n\
					fx = 1.0-position.x; \n\
					percent.y = inrange(position.y,fx,linewidth); \n\
					percent.x = either(percent.x,percent.y); \n\
				} \n\
			} //if-else ha < 18 \n\
		} //if-else ha < 15 \n\
	} //if-else ha < 10 \n\
	\n\
	if (fillprops.filled) {colour = prevColour;} else { colour=vec4(0.,0.,0.,0); }\n\
	if (fillprops.hatched) { \n\
		//colour = mix(fillprops.HatchColour, colour, useBrick.x * useBrick.y); \n\
		colour = mix(fillprops.HatchColour, colour, percent.x ); \n\
	} \n\
	prevColour = colour; \n\
} \n\
//#endif //FULL \n\
\n";


static const GLchar *plug_finalColCalc = "\
#if defined(MTEX) || defined(PROJTEX) \n\
#define MTMODE_ADD	1\n \
#define MTMODE_ADDSIGNED	2\n \
#define MTMODE_ADDSIGNED2X	3\n \
#define MTMODE_ADDSMOOTH	4\n \
#define MTMODE_BLENDCURRENTALPHA	5\n \
#define MTMODE_BLENDDIFFUSEALPHA	6\n \
#define MTMODE_BLENDFACTORALPHA	7\n \
#define MTMODE_BLENDTEXTUREALPHA	8\n \
#define MTMODE_DOTPRODUCT3	9\n \
#define MTMODE_MODULATE	10\n \
#define MTMODE_MODULATE2X	11\n \
#define MTMODE_MODULATE4X	12\n \
#define MTMODE_MODULATEALPHA_ADDCOLOR	13\n \
#define MTMODE_MODULATEINVALPHA_ADDCOLOR	14\n \
#define MTMODE_MODULATEINVCOLOR_ADDALPHA	15\n \
#define MTMODE_OFF	16\n \
#define MTMODE_REPLACE	17\n \
#define MTMODE_SELECTARG1	18\n \
#define MTMODE_SELECTARG2	19\n \
#define MTMODE_SUBTRACT	20\n \
#define MTSRC_DIFFUSE	1 \n\
#define MTSRC_FACTOR	2 \n\
#define MTSRC_SPECULAR	3 \n\
#define MTFN_ALPHAREPLICATE	0 \n\
#define MTFN_COMPLEMENT	1 \n\
#define MT_DEFAULT -1 \n\
\n\
void PLUG_finalColCalc(inout vec4 prevColour, in int mode, in int modea, in int func, in sampler2D tex, in vec2 texcoord) { \n\
  vec4 texel = texture2D(tex,texcoord); \n\
  vec4 rv = vec4(1.,0.,1.,1.);   \n\
  if (mode==MTMODE_OFF) {  \n\
    rv = vec4(prevColour); \n\
  } else if (mode==MTMODE_REPLACE) { \n\
    rv = vec4(texture2D(tex, texcoord)); \n\
  }else if (mode==MTMODE_MODULATE) {  \n\
    vec3 ct,cf;  \n\
    float at,af;  \n\
    cf = prevColour.rgb;  \n\
    af = prevColour.a;  \n\
    ct = texel.rgb;  \n\
    at = texel.a;  \n\
    rv = vec4(ct*cf, at*af);  \n\
  } else if (mode==MTMODE_MODULATE2X) {  \n\
    vec3 ct,cf;  \n\
    float at,af;  \n\
    cf = prevColour.rgb;  \n\
    af = prevColour.a;  \n\
    ct = texel.rgb;  \n\
    at = texel.a;  \n\
    rv = vec4(vec4(ct*cf, at*af)*vec4(2.,2.,2.,2.));  \n\
  }else if (mode==MTMODE_MODULATE4X) {  \n\
    vec3 ct,cf;  \n\
    float at,af;  \n\
    cf = prevColour.rgb; \n\
    af = prevColour.a;  \n\
    ct = texel.rgb;  \n\
    at = texel.a;  \n\
    rv = vec4(vec4(ct*cf, at*af)*vec4(4.,4.,4.,4.));  \n\
  }else if (mode== MTMODE_ADDSIGNED) { \n\
    rv = vec4 (prevColour + texel - vec4 (0.5, 0.5, 0.5, -.5));  \n\
  } else if (mode== MTMODE_ADDSIGNED2X) { \n\
    rv = vec4 ((prevColour + texel - vec4 (0.5, 0.5, 0.5, -.5))*vec4(2.,2.,2.,2.));  \n\
  } else if (mode== MTMODE_ADD) { \n\
    rv= vec4 (prevColour + texel);  \n\
  } else if (mode== MTMODE_SUBTRACT) { \n\
    rv = vec4 (texel - prevColour); //jas had prev - tex \n\
  } else if (mode==MTMODE_ADDSMOOTH) {  \n\
    rv = vec4 (prevColour + (prevColour - vec4 (1.,1.,1.,1.)) * texel);  \n\
  } else if (mode==MTMODE_BLENDDIFFUSEALPHA) {  \n\
    rv = vec4 (mix(prevColour,texel,castle_Color.a)); \n\
  } else if (mode==MTMODE_BLENDTEXTUREALPHA) {  \n\
    rv = vec4 (mix(prevColour,texel,texel.a)); \n\
  } else if (mode==MTMODE_BLENDFACTORALPHA) {  \n\
    rv = vec4 (mix(prevColour,texel,mt_Color.a)); \n\
  } else if (mode==MTMODE_BLENDCURRENTALPHA) {  \n\
    rv = vec4 (mix(prevColour,texel,prevColour.a)); \n\
  } else if (mode==MTMODE_SELECTARG1) {  \n\
    rv = texel;  \n\
  } else if (mode==MTMODE_SELECTARG2) {  \n\
    rv = prevColour;  \n\
  } \n\
  if(modea != 0){ \n\
    if (modea==MTMODE_OFF) {  \n\
      rv.a = prevColour.a; \n\
    } else if (modea==MTMODE_REPLACE) { \n\
      rv.a = 1.0; \n\
    }else if (modea==MTMODE_MODULATE) {  \n\
      float at,af;  \n\
      af = prevColour.a;  \n\
      at = texel.a;  \n\
      rv.a = at*af;  \n\
    } else if (modea==MTMODE_MODULATE2X) {  \n\
      float at,af;  \n\
      af = prevColour.a;  \n\
      at = texel.a;  \n\
      rv.a = at*af*2.0;  \n\
    }else if (modea==MTMODE_MODULATE4X) {  \n\
      float at,af;  \n\
      af = prevColour.a;  \n\
      at = texel.a;  \n\
      rv.a = at*af*4.0;  \n\
    }else if (modea== MTMODE_ADDSIGNED) { \n\
      rv.a = (prevColour.a + texel.a + .5);  \n\
    } else if (modea== MTMODE_ADDSIGNED2X) { \n\
      rv.a = ((prevColour.a + texel.a + .5))*2.0;  \n\
    } else if (modea== MTMODE_ADD) { \n\
      rv.a = prevColour.a + texel.a;  \n\
    } else if (modea== MTMODE_SUBTRACT) { \n\
      rv.a = texel.a - prevColour.a;  //jas had prev - texel \n\
    } else if (modea==MTMODE_ADDSMOOTH) {  \n\
      rv.a = (prevColour.a + (prevColour.a - 1.)) * texel.a;  \n\
    } else if (modea==MTMODE_BLENDDIFFUSEALPHA) {  \n\
      rv.a = mix(prevColour.a,texel.a,castle_Color.a); \n\
    } else if (modea==MTMODE_BLENDTEXTUREALPHA) {  \n\
      rv.a = mix(prevColour.a,texel.a,texel.a); \n\
    } else if (modea==MTMODE_BLENDFACTORALPHA) {  \n\
      rv.a = mix(prevColour.a,texel.a,mt_Color.a); \n\
    } else if (modea==MTMODE_BLENDCURRENTALPHA) {  \n\
      rv.a = mix(prevColour.a,texel.a,prevColour.a); \n\
    } else if (modea==MTMODE_SELECTARG1) {  \n\
      rv.a = texel.a;  \n\
    } else if (modea==MTMODE_SELECTARG2) {  \n\
      rv.a = prevColour.a;  \n\
    } \n\
  } \n\
  if(func == MTFN_COMPLEMENT){ \n\
	//rv = vec4(1.0,1.0,1.0,1.0) - rv; \n\
	rv = vec4( vec3(1.0,1.0,1.0) - rv.rgb, rv.a); \n\
  }else if(func == MTFN_ALPHAREPLICATE){ \n\
	rv = vec4(rv.a,rv.a,rv.a,rv.a); \n\
  } \n\
  prevColour = rv;  \n\
} \n\
#endif //defined(MTEX) || defined(PROJTEX) \n";




const char *getGenericVertex(void){
	return genericVertexGLES2; //genericVertexDesktop
}
const char *getGenericFragment(){
	return genericFragmentGLES2; //genericFragmentDesktop;
}
#include "../scenegraph/Component_Shape.h"

static const GLchar *plug_fragment_end_anaglyph =	"\
void PLUG_fragment_end (inout vec4 finalFrag){ \n\
	float gray = dot(finalFrag.rgb, vec3(0.299, 0.587, 0.114)); \n\
	finalFrag = vec4(gray,gray,gray, finalFrag.a); \n\
}\n";

//TEXTURE 3D
/*	
	4 scenarios:
	1. GL has texture3D/EXT_texture3D/OES_texture3D
	2. GL no texture3D - emulate
	A. 3D image: source imagery is i) 3D image or ii) composed image with image layers all same size
	B. 2D layers: source imagery is composed image with z < 7 layers and layers can be different sizes
		1					2
	A	texture3D			tiled texture2D	
	B	multi texure2D		multi texture2D

	for tiled texture2D, there are a few ways to do the tiles:
	a) vertical strip: nx x (ny * nz) - our first attempt
		layer 0 at top, and sequential layers follow down
	b) squarish tiled: ix = iy = ceil(sqrt(nz)); (nx*ix) x (ny*iy)
		there will be blank squares. Order:
		y-first layer order: fill column, first at top left, before advancing ix right
		(option: x-first layer order: fill row, first at top left, before advancing down iy)
*/

// TILED METHOD FOR TEXTURE3D 
//	texture3D emulator via TILED texture2D
//  reason for emulating: 2016 GLES2 via ANGLEPROJECT(gles emulator over DirectX on windows)
//     doesn't have Texture3D or Texture3DOES or Texture3DEXT.
//  reason for TILES: an oblong Y-STRIP approach exceded max texture size in Y (but had lots left in X)
//     desktop computer max_size (of 2D image in one dimension) 16384
//     android phone max_size 4096
//     and so would be resampled (blurry) in y and good in x
//     using tiles means room for more full z slices ie 256x256x256 == 4096x4096 == 16M, 
//			512x512x512 == 134M == 16384x16384/2, and therefore less blurry images
//  tiles start in upper left with z=0, increase in y,
//  then when hit ny tiles in a y strip, move right one tile, and restart at top
//  uniform tex3dTiles[3] = {nx,ny,z}
//  example ny = 4, nx = 3, z = 11
//  1  5  9
//  2  6  10
//  3  7  11
//  4  8
//  
static const GLchar *plug_fragment_texture3D_apply_volume =	"\n\
vec4 texture3Demu0( sampler2D sampler, in vec3 texcoord3, in int magfilter){ \n\
  vec4 sample = vec4(0.0); \n\
  #ifdef TEX3D \n\
  //TILED method (vs Y strip method) \n\
  vec3 texcoord = texcoord3; \n\
  //texcoord.z = 1.0 - texcoord.z; //flip z from RHS to LHS\n\
  float depth = max(1.0,float(tex3dTiles[2])); \n\
  if(repeatSTR[0] == 0) texcoord.x = clamp(texcoord.x,0.0001,.9999); \n\
  else texcoord.x = mod(texcoord.x,1.0); \n\
  if(repeatSTR[1] == 0) texcoord.y = clamp(texcoord.y,0.0001,.9999); \n\
  else texcoord.y = mod(texcoord.y,1.0); \n\
  if(repeatSTR[2] == 0) texcoord.z = clamp(texcoord.z,0.0001,.9999); \n\
  else texcoord.z = mod(texcoord.z,1.0); \n\
  vec4 texel; \n\
  int izf = int(floor(texcoord.z*depth)); //floor z \n\
  int izc = int(ceil(texcoord.z*depth));  //ceiling z \n\
  izc = izc == tex3dTiles[2] ? izc - 1 : izc; //clamp int z \n\
  vec4 ftexel, ctexel; \n\
  \n\
  int nx = tex3dTiles[0]; //0-11 \n\
  int ny = tex3dTiles[1]; \n\
  float fnx = 1.0/float(nx); //.1\n\
  float fny = 1.0/float(ny); \n\
  int ix = izc / ny; //60/11=5\n\
  int ixny = ix * ny; //5*11=55\n\
  int iy = izc - ixny; //60-55=5 modulus remainder \n\
  float cix = float(ix); //5 \n\
  float ciy = float(iy); \n\
  float xxc = (cix + texcoord.s)*fnx; //(5 + .5)*.1 = .55\n\
  float yyc = (ciy + texcoord.t)*fny; \n\
  ix = izf / ny; \n\
  ixny = ix * ny; \n\
  iy = izf - ixny; //modulus remainder \n\
  float fix = float(ix); \n\
  float fiy = float(iy); \n\
  float xxf = (fix + texcoord.s)*fnx; \n\
  float yyf = (fiy + texcoord.t)*fny; \n\
  \n\
  vec2 ftexcoord, ctexcoord; //texcoord is 3D, ftexcoord and ctexcoord are 2D coords\n\
  ftexcoord.s = xxf; \n\
  ftexcoord.t = yyf; \n\
  ctexcoord.s = xxc; \n\
  ctexcoord.t = yyc; \n\
  ftexel = texture2D(sampler,ftexcoord.st); \n\
  ctexel = texture2D(sampler,ctexcoord.st); \n\
  float fraction = mod(texcoord.z*depth,1.0); \n\
  if(magfilter == 1) \n\
	texel = mix(ctexel,ftexel,1.0-fraction); //lerp GL_LINEAR \n\
  else \n\
	texel = ftexel; //fraction > .5 ? ctexel : ftexel; //GL_NEAREST \n\
  sample = texel; \n\
  #endif //TEX3D \n\
  return sample; \n\
} \n\
vec4 texture3Demu( sampler2D sampler, in vec3 texcoord3){ \n\
	//use uniform magfilter \n\
	return texture3Demu0( sampler, texcoord3, magFilter); \n\
} \n\
void PLUG_texture3D( inout vec4 sample, in vec3 texcoord3 ){ \n\
	sample = texture3Demu(fw_Texture_unit0,texcoord3); \n\
} \n\
void PLUG_texture_apply (inout vec4 finalFrag, in vec3 normal_eye_fragment ){ \n\
\n\
	vec4 sample; \n\
	sample = texture3Demu(fw_Texture_unit0,fw_TexCoord[0]); \n\
	finalFrag *= sample; \n\
  \n\
}\n";



static const GLchar *plug_fragment_texture3Dlayer_apply =	"\
void PLUG_texture_apply (inout vec4 finalFrag, in vec3 normal_eye_fragment ){ \n\
\n\
  #ifdef TEX3DLAY \n\
  vec3 texcoord = fw_TexCoord[0]; \n\
  texcoord.z = 1.0 - texcoord.z; //flip z from RHS to LHS\n\
  float depth = max(1.0,float(textureCount-1)); \n\
  float delta = 1.0/depth; \n\
  if(repeatSTR[0] == 0) texcoord.x = clamp(texcoord.x,0.0001,.9999); \n\
  else texcoord.x = mod(texcoord.x,1.0); \n\
  if(repeatSTR[1] == 0) texcoord.y = clamp(texcoord.y,0.0001,.9999); \n\
  else texcoord.y = mod(texcoord.y,1.0); \n\
  if(repeatSTR[2] == 0) texcoord.z = clamp(texcoord.z,0.0001,.9999); \n\
  else texcoord.z = mod(texcoord.z,1.0); \n\
  int flay = int(floor(texcoord.z*depth)); \n\
  int clay = int(ceil(texcoord.z*depth)); \n\
  vec4 ftexel, ctexel; \n\
  //flay = 0; \n\
  //clay = 1; \n\
  if(flay == 0) ftexel = texture2D(fw_Texture_unit0,texcoord.st);  \n\
  if(clay == 0) ctexel = texture2D(fw_Texture_unit0,texcoord.st);  \n\
  if(flay == 1) ftexel = texture2D(fw_Texture_unit1,texcoord.st);  \n\
  if(clay == 1) ctexel = texture2D(fw_Texture_unit1,texcoord.st);  \n\
  if(flay == 2) ftexel = texture2D(fw_Texture_unit2,texcoord.st);  \n\
  if(clay == 2) ctexel = texture2D(fw_Texture_unit2,texcoord.st);  \n\
  if(flay == 3) ftexel = texture2D(fw_Texture_unit3,texcoord.st);  \n\
  if(clay == 3) ctexel = texture2D(fw_Texture_unit3,texcoord.st); \n\
  float fraction = mod(texcoord.z*depth,1.0); \n\
  vec4 texel; \n\
  if(magFilter == 1) \n\
	texel = mix(ctexel,ftexel,(1.0-fraction)); //lerp GL_LINEAR \n\
  else \n\
	texel = fraction > .5 ? ctexel : ftexel; //GL_NEAREST \n\
  finalFrag *= texel; \n\
  #endif //TEX3DLAY \n\
  \n\
}\n";

//MULTITEXTURE
// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/texturing.html#MultiTexture
  /* PLUG: texture_apply (fragment_color, normal_eye_fragment) */
static const GLchar *plug_fragment_texture_apply =	"\
void PLUG_texture_apply (inout vec4 finalFrag, in vec3 normal_eye_fragment ){ \n\
 \n\
  #ifdef MTEX \n\
  vec4 source; \n\
  int isource,iasource, mode; \n\
  //vec4 matdiff_color = finalFrag; \n\
  //finalFrag = texture2D(fw_Texture_unit0, fw_TexCoord[0].st) * finalFrag; \n\
  if(textureCount>0){ \n\
    if(fw_Texture_mode0[0] != MTMODE_OFF) { \n\
      isource = fw_Texture_source0[0]; //castle-style dual sources \n\
      iasource = fw_Texture_source0[1]; \n\
      if(isource == MT_DEFAULT) source = finalFrag; \n\
      else if(isource == MTSRC_DIFFUSE) source = mtex_diffuse; \n\
      else if(isource == MTSRC_SPECULAR) source = vec4(mtex_specular,1.0); \n\
      else if(isource == MTSRC_FACTOR) source = mt_Color; \n\
      if(iasource != 0){ \n\
        if(iasource == MT_DEFAULT) source.a = finalFrag.a; \n\
        else if(iasource == MTSRC_DIFFUSE) source.a = mtex_diffuse.a; \n\
        else if(iasource == MTSRC_SPECULAR) source.a = 1.0; \n\
        else if(iasource == MTSRC_FACTOR) source.a = mt_Color.a; \n\
      } \n\
      finalColCalcA(source,fw_Texture_mode0[0],fw_Texture_mode0[1],fw_Texture_function0, fw_Texture_unit0,fw_TexCoord[0].st); \n\
      finalFrag = source; \n\
    } \n\
  } \n\
  if(textureCount>1){ \n\
    if(fw_Texture_mode1[0] != MTMODE_OFF) { \n\
      isource = fw_Texture_source1[0]; //castle-style dual sources \n\
      iasource = fw_Texture_source1[1]; \n\
      if(isource == MT_DEFAULT) source = finalFrag; \n\
      else if(isource == MTSRC_DIFFUSE) source = mtex_diffuse; \n\
      else if(isource == MTSRC_SPECULAR) source = vec4(mtex_specular,1.0); \n\
      else if(isource == MTSRC_FACTOR) source = mt_Color; \n\
      if(iasource != 0){ \n\
        if(iasource == MT_DEFAULT) source.a = finalFrag.a; \n\
        else if(iasource == MTSRC_DIFFUSE) source.a = mtex_diffuse.a; \n\
        else if(iasource == MTSRC_SPECULAR) source.a = 1.0; \n\
        else if(iasource == MTSRC_FACTOR) source.a = mt_Color.a; \n\
      } \n\
      finalColCalcA(source,fw_Texture_mode1[0],fw_Texture_mode1[1],fw_Texture_function1, fw_Texture_unit1,fw_TexCoord[1].st); \n\
      finalFrag = source; \n\
    } \n\
  } \n\
  if(textureCount>2){ \n\
    if(fw_Texture_mode2[0] != MTMODE_OFF) { \n\
      isource = fw_Texture_source2[0]; //castle-style dual sources \n\
      iasource = fw_Texture_source2[1]; \n\
      if(isource == MT_DEFAULT) source = finalFrag; \n\
      else if(isource == MTSRC_DIFFUSE) source = mtex_diffuse; \n\
      else if(isource == MTSRC_SPECULAR) source = vec4(mtex_specular,1.0); \n\
      else if(isource == MTSRC_FACTOR) source = mt_Color; \n\
      if(iasource != 0){ \n\
        if(iasource == MT_DEFAULT) source.a = finalFrag.a; \n\
        else if(iasource == MTSRC_DIFFUSE) source.a = mtex_diffuse.a; \n\
        else if(iasource == MTSRC_SPECULAR) source.a = 1.0; \n\
        else if(iasource == MTSRC_FACTOR) source.a = mt_Color.a; \n\
      } \n\
      finalColCalcA(source,fw_Texture_mode2[0],fw_Texture_mode2[1],fw_Texture_function2,fw_Texture_unit2,fw_TexCoord[2].st); \n\
      finalFrag = source; \n\
    } \n\
  } \n\
  if(textureCount>3){ \n\
    if(fw_Texture_mode3[0] != MTMODE_OFF) { \n\
      isource = fw_Texture_source3[0]; //castle-style dual sources \n\
      iasource = fw_Texture_source3[1]; \n\
      if(isource == MT_DEFAULT) source = finalFrag; \n\
      else if(isource == MTSRC_DIFFUSE) source = mtex_diffuse; \n\
      else if(isource == MTSRC_SPECULAR) source = vec4(mtex_specular,1.0); \n\
      else if(isource == MTSRC_FACTOR) source = mt_Color; \n\
      if(iasource != 0){ \n\
        if(iasource == MT_DEFAULT) source.a = finalFrag.a; \n\
        else if(iasource == MTSRC_DIFFUSE) source.a = mtex_diffuse.a; \n\
        else if(iasource == MTSRC_SPECULAR) source.a = 1.0; \n\
        else if(iasource == MTSRC_FACTOR) source.a = mt_Color.a; \n\
      } \n\
      finalColCalcA(source,fw_Texture_mode3[0],fw_Texture_mode3[1],fw_Texture_function3,fw_Texture_unit3,fw_TexCoord[3].st); \n\
      finalFrag = source; \n\
    } \n\
  } \n\
  #else //MTEX \n\
  /* ONE TEXTURE */ \n\
  #ifdef CUB \n\
  finalFrag = textureCube(fw_Texture_unit0, fw_TexCoord[0]) * finalFrag; \n\
  #else //CUB \n\
  finalFrag = texture2D(fw_Texture_unit0, fw_TexCoord[0].st) * finalFrag; \n\
  #endif //CUB \n\
  #endif //MTEX \n\
  \n\
}\n";



/* PLUG: add_light_physical (color, view, normal, materialInfo ); */
static const GLchar *plug_frag_lighting_physical = "\n\
struct AngularInfo \n\
{ \n\
	float NdotL; // cos angle between normal and light direction \n\
	float NdotV; // cos angle between normal and view direction \n\
	float NdotH; // cos angle between normal and half vector \n\
	float LdotH; // cos angle between light direction and half vector \n\
	float VdotH; // cos angle between view direction and half vector \n\
	vec3 padding; \n\
}; \n\
AngularInfo getAngularInfo(vec3 pointToLight, vec3 normal, vec3 view) \n\
{ \n\
	// Standard one-letter names \n\
	vec3 n = normalize(normal); // Outward direction of surface point \n\
	vec3 v = normalize(view);   // Direction from surface point to view \n\
	vec3 l = normalize(pointToLight); // Direction from surface point to light \n\
	vec3 h = normalize(l + v); // Direction of the vector between l and v \n\
	float NdotL = clamp(dot(n, l), 0.0, 1.0); \n\
	float NdotV = clamp(dot(n, v), 0.0, 1.0); \n\
	float NdotH = clamp(dot(n, h), 0.0, 1.0); \n\
	float LdotH = clamp(dot(l, h), 0.0, 1.0); \n\
	float VdotH = clamp(dot(v, h), 0.0, 1.0); \n\
	AngularInfo ai = AngularInfo( \n\
		NdotL, \n\
		NdotV, \n\
		NdotH, \n\
		LdotH, \n\
		VdotH, \n\
		vec3(0, 0, 0) \n\
	); \n\
	return ai; \n\
} \n\
// Lambert lighting \n\
// see https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/ \n\
vec3 diffuse(MaterialInfo materialInfo) \n\
{ \n\
    return materialInfo.diffuseColor / M_PI; \n\
} \n\
// TFresnel reflectance F() \n\
vec3 specularReflection(MaterialInfo materialInfo, AngularInfo angularInfo) \n\
{ \n\
	return materialInfo.reflectance0 + (materialInfo.reflectance90 - materialInfo.reflectance0) * pow(clamp(1.0 - angularInfo.VdotH, 0.0, 1.0), 5.0); \n\
} \n\
// Smith Joint GGX \n\
// Note: Vis = G / (4 * NdotL * NdotV) \n\
float visibilityOcclusion(MaterialInfo materialInfo, AngularInfo angularInfo) \n\
{ \n\
	float NdotL = angularInfo.NdotL; \n\
	float NdotV = angularInfo.NdotV; \n\
	float alphaRoughnessSq = materialInfo.alphaRoughness * materialInfo.alphaRoughness; \n\
	float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0 - alphaRoughnessSq) + alphaRoughnessSq); \n\
	float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0 - alphaRoughnessSq) + alphaRoughnessSq); \n\
		\n\
	float GGX = GGXV + GGXL; \n\
	if (GGX > 0.0) \n\
	{ \n\
		return 0.5 / GGX; \n\
	} \n\
	return 0.0; \n\
} \n\
// model the distribution of microfacet normals (aka D()) \n\
float microfacetDistribution(MaterialInfo materialInfo, AngularInfo angularInfo) \n\
{ \n\
	float alphaRoughnessSq = materialInfo.alphaRoughness * materialInfo.alphaRoughness; \n\
	float f = (angularInfo.NdotH * alphaRoughnessSq - angularInfo.NdotH) * angularInfo.NdotH + 1.0; \n\
	return alphaRoughnessSq / (M_PI * f * f); \n\
} \n\
vec3 getPointShade(vec3 pointToLight, MaterialInfo materialInfo, vec3 normal, vec3 view) \n\
{ \n\
	AngularInfo angularInfo = getAngularInfo(pointToLight, normal, view); \n\
	if (angularInfo.NdotL > 0.0 || angularInfo.NdotV > 0.0) \n\
	{ \n\
		// microfacet specular shading model \n\
		vec3 F = specularReflection(materialInfo, angularInfo); \n\
		float Vis = visibilityOcclusion(materialInfo, angularInfo); \n\
		float D = microfacetDistribution(materialInfo, angularInfo); \n\
		// Calculation of analytical lighting contribution \n\
		vec3 diffuseContrib = (1.0 - F) * diffuse(materialInfo); \n\
		vec3 specContrib = F * Vis * D; \n\
		// reflectance (BRDF) scaled by the energy of the light (cosine law) \n\
		return angularInfo.NdotL * (diffuseContrib + specContrib); \n\
	} \n\
	return vec3(0.0, 0.0, 0.0); \n\
} \n\
void PLUG_add_light_physical (inout vec3 vertexcolor, in vec3 myPosition, in vec3 myNormal, in struct MaterialInfo mat){ \n\
	//working in eye space: eye is at 0,0,0 looking generally in direction 0,0,-1 \n\
	//myPosition, myNormal - of surface vertex, in eyespace \n\
	int i; \n\
	vec3 N = normalize (myNormal); \n\
		\n\
	vec3 E = -normalize(myPosition.xyz); \n \
		\n\
	// apply the lights to this material \n\
	// weird but ANGLE needs constant loop \n\
	for (i=0; i<lightcount; i++) {\n\
		float on = 1.0; //we only send active/on lights to shader, so this is for radius \n\
		float spot = 1.0; \n\
		float attenuation = 1.0; //directional default \n\
		fw_LightSourceParameters light = fw_LightSource[i]; \n\
		int myLightType = lightType[i]; \n\
		// VP vector of light direction and distance \n\
		vec3 VP = light.location.xyz - myPosition.xyz; \n\
		vec3 L = -light.direction; //directional light \n\
		if(myLightType < 2){ \n\
			//point and spot \n\
			L = normalize(VP); \n\
			float D = length(VP);  // distance to vertex \n\
			// are we within range? \n\
			if (D > light.lightRadius) on = 0.0; \n\
			attenuation = 1.0/max(1.0,(light.Attenuations.x + (light.Attenuations.y * D) + (light.Attenuations.z *D*D))); \n\
		} \n\
		vec3 shade = getPointShade(-VP, mat, -N, -E); \n\
		if (myLightType==1) { \n\
			// SpotLight  \n\
			spot = 0.0; \n\
			float spotDot = dot (-L,light.direction); \n\
			// check against spotCosCutoff \n\
			if (spotDot > light.spotCutoff) { \n\
				if(spotDot > light.spotBeamWidth) { \n\
					spot = 1.0; \n\
				} else { \n\
					spot = (spotDot - light.spotCutoff)/(light.spotBeamWidth - light.spotCutoff); \n\
				} \n\
			} \n\
		} \n\
		vertexcolor   += on * attenuation * spot * light.color * light.intensity * shade; \n\
		//vertexcolor   += shade; //vec3(0.0,1.0,1.0); \n\
	} \n\
	vertexcolor = clamp(vertexcolor, 0.0, 1.0); \n\
} \n\
";

//add_light_contribution (castle_Color, castle_vertex_eye, castle_normal_eye, castle_MaterialShininess)
// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/lighting.html#Lightingequations
// simplified thoery: lightOut = emissive + f(light_in,material,light_eqn)
// ADS: Ambient + Diffuse + Specular
// http://www.matrix44.net/cms/notes/opengl-3d-graphics/the-ads-lighting-model
// http://http.developer.nvidia.com/CgTutorial/cg_tutorial_chapter05.html
// incoming eyeposition and eyenormal are of the surface vertex and normal
// .. in the view/eye coordinate system (so eye is at 0,0,0 and eye direction is 0,0,-1

static const GLchar *plug_vertex_lighting_ADSLightModel = "\n\
/* use ADSLightModel here the ADS colour is returned from the function.  */ \n\
void PLUG_add_light_contribution2 (inout vec3 vertexcolor, inout vec3 specularcolor, in vec4 myPosition, in vec3 myNormal, \n\
		in float mat_shininess, in float mat_ambient, in vec3 mat_diffuse, in vec3 mat_specular){ \n\
	//working in eye space: eye is at 0,0,0 looking generally in direction 0,0,-1 \n\
	//myPosition, myNormal - of surface vertex, in eyespace \n\
	//vertexcolor - diffuse+ambient -will be replaced or modulated by texture color \n\
	//specularcolor - specular+emissive or non-diffuse (emissive added outside this function) \n\
	//algo: uses Blinn-Phong specular reflection: half-vector pow(N*H,shininess) \n\
	// https://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/lighting.html#Lightingequations \n\
	// fog and emissive are done elsewhere, this function does: \n\
	// SUM(on[i] x attenuation[i] x spot[i] x ILrgb[i] x (ambient[i] + diffuse[i] + specular[i])) \n\
	int i; \n\
	vec3 N = normalize (myNormal); \n\
		\n\
	vec3 E = -normalize(myPosition.xyz); \n \
		\n\
	// apply the lights to this material \n\
	// weird but ANGLE needs constant loop \n\
	vec3 sum_vertex = vec3(0.,0.,0.); \n\
	vec3 sum_specular = vec3(0.,0.,0.); \n\
	for (i=0; i<lightcount; i++) {\n\
		vec3 diffuse = vec3(0., 0., 0.); \n\
		vec3 ambient = vec3(0., 0., 0.); \n\
		vec3 specular = vec3(0., 0., 0.); \n\
		float on = 1.0; //we only send active/on lights to shader, so this is for radius \n\
		float spot = 1.0; \n\
		float attenuation = 1.0; //directional default \n\
		fw_LightSourceParameters light = fw_LightSource[i]; \n\
		int myLightType = lightType[i]; \n\
		// VP vector of light direction and distance \n\
		vec3 VP = light.location.xyz - myPosition.xyz; \n\
		vec3 L = -light.direction; //directional light \n\
		if(myLightType < 2){ \n\
			//point and spot \n\
			L = normalize(VP); \n\
			float D = length(VP);  // distance to vertex \n\
			// are we within range? \n\
			if (D > light.lightRadius) on = 0.0; \n\
			attenuation = 1.0/max(1.0,(light.Attenuations.x + (light.Attenuations.y * D) + (light.Attenuations.z *D*D))); \n\
		} \n\
		float NdotL = max(dot(N, L), 0.0); //Lambertian diffuse term \n\
		//specular reflection models, phong or blinn-phong \n\
		//#define PHONG 1 \n\
		#ifdef PHONG \n\
			//Phong \n\
			vec3 R = normalize(-reflect(L,N)); \n\
			float RdotE = max(dot(R,E),0.0); \n\
			float specbase = RdotE; \n\
			// assume shader gets shininess in 0 to 1 range, and scales it to 0 to 128 range here \n\
			float specpow = mat_shininess*128.0; \n\
		#else //PHONG \n\
			//Blinn-Phong \n\
			vec3 H = normalize(L + E); //halfvector x3d specs this is L+v/|L+v|\n\
			float NdotH = max(dot(N,H),0.0); \n\
			float specbase = NdotH; \n\
			float specpow = mat_shininess*128.0; \n\
		#endif //PHONG \n\
		float powerFactor = 0.0; // for light dropoff \n\
		if (specbase > 0.0) { \n\
			powerFactor = pow(specbase,specpow); \n\
			// tone down the power factor if mat_shininess borders 0 \n\
		} \n\
			\n\
		ambient += light.ambient * mat_diffuse * mat_ambient; \n\
		specular += light.intensity * mat_specular *powerFactor; \n\
		diffuse += light.intensity * mat_diffuse * NdotL; \n\
		if (myLightType==1) { \n\
			// SpotLight  \n\
			spot = 0.0; \n\
			float spotDot = dot (-L,light.direction); \n\
			// check against spotCosCutoff \n\
			if (spotDot > light.spotCutoff) { \n\
				if(spotDot > light.spotBeamWidth) { \n\
					spot = 1.0; \n\
				} else { \n\
					spot = (spotDot - light.spotCutoff)/(light.spotBeamWidth - light.spotCutoff); \n\
				} \n\
			} \n\
		} \n\
		sum_vertex   += on * attenuation * spot * light.color * (ambient + diffuse); \n\
		sum_specular += on * attenuation * spot * light.color * (specular); \n\
	} \n\
	vertexcolor = clamp(sum_vertex + vertexcolor, 0.0, 1.0); \n\
	specularcolor = clamp(sum_specular + specularcolor, 0.0, 1.0); \n\
} \n\
";

// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/lighting.html#t-foginterpolant
// PLUG: fog_apply (fragment_color, normal_eye_fragment)
static const GLchar *plug_fog_apply =	"\
void PLUG_fog_apply (inout vec4 finalFrag, in vec3 normal_eye_fragment ){ \n\
	float ff = 1.0; \n\
	float depth = abs(castle_vertex_eye.z/castle_vertex_eye.w); \n\
	if(fw_fogparams.fogType > 0){ \n\
		ff = 0.0;  \n\
		if(fw_fogparams.fogType == 1){ //FOGTYPE_LINEAR \n\
			if(depth < fw_fogparams.visibilityRange) \n\
			ff = (fw_fogparams.visibilityRange-depth)/fw_fogparams.visibilityRange; \n\
		} else { //FOGTYPE_EXPONENTIAL \n\
			if(depth < fw_fogparams.visibilityRange){ \n\
				ff = exp(-depth/(fw_fogparams.visibilityRange -depth) ); \n\
				ff = clamp(ff, 0.0, 1.0);  \n\
			} \n\
		} \n\
		finalFrag = mix(finalFrag,fw_fogparams.fogColor,1.0 - ff);  \n\
	} \n\
} \n\
";

static const GLchar *vertex_plug_clip_apply =	"\
#ifdef CLIP \n\
#define FW_MAXCLIPPLANES 4 \n\
uniform int fw_nclipplanes; \n\
uniform vec4 fw_clipplanes[FW_MAXCLIPPLANES]; \n\
varying float fw_ClipDistance[FW_MAXCLIPPLANES]; \n\
 \n\
void PLUG_vertex_object_space (in vec4 vertex_object, in vec3 normal_object){ \n\
  for ( int i=0; i<fw_nclipplanes; i++ ) \n\
    fw_ClipDistance[i] = dot( fw_clipplanes[i], vertex_object); \n\
} \n\
#endif //CLIP \n\
";

static const GLchar *frag_plug_clip_apply =	"\
#ifdef CLIP \n\
#define FW_MAXCLIPPLANES 4 \n\
uniform int fw_nclipplanes; \n\
varying float fw_ClipDistance[FW_MAXCLIPPLANES]; \n\
void PLUG_fog_apply (inout vec4 finalFrag, in vec3 normal_eye_fragment ){ \n\
	for(int i=0;i<fw_nclipplanes;i++) { \n\
      //if(normal_eye_fragment.z > fw_ClipDistance[i]) discard;  \n\
	  if(fw_ClipDistance[i] < 0.0) discard; \n\
	} \n\
} \n\
#endif //CLIP \n\
";


//assumes little endian
void printBitsB(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for (i=size-1;i>=0;i--)
    {
        for (j=7;j>=0;j--)
        {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
		printf(" ");
    }
    printf("\n");
}


#if defined(GL_ES_VERSION_2_0)
static int isMobile = TRUE;
#else
static int isMobile = FALSE;
#endif
int get_GLSL_max_version(){
	static int once = FALSE;
	static float glsl_version = 0.0f;
	static int max_shader_version = 130;
	if(!once){
		const GLubyte * glsl_version_str = glGetString ( GL_SHADING_LANGUAGE_VERSION);
		sscanf(glsl_version_str,"%f",&glsl_version);
		max_shader_version = (int)(glsl_version * 100.0f + .4f);
		ConsoleMessage("GLSL shader max version %s %d\n", glsl_version_str, max_shader_version );
		once = TRUE;
	}
	return max_shader_version;
}
#define DESIRE(whichOne,zzz) ((whichOne & zzz)==zzz)
int getSpecificShaderSourceCastlePlugs (const GLchar **vertexSource, const GLchar **fragmentSource, shaderflagsstruct whichOne) 
{
	//for building the Builtin (similar to fixed-function pipeline, except from shader parts)
	//in OpenGL_Utils.c L.2553 set usingCastlePlugs = 1 to get in here.
	//whichone - a bitmask of shader requirements, one bit for each requirement, so shader permutation can be built

	int retval, unique_int;
	char *CompleteCode[3];
	char *vs, *fs;
	static int GLSL_max_version = 0;
	retval = FALSE;
	if(whichOne.usershaders ) //& USER_DEFINED_SHADER_MASK) 
		return retval; //not supported yet as of Aug 9, 2016
	retval = TRUE;
	if(!GLSL_max_version){
		//const GLubyte * glsl_version_str = glGetString ( GL_SHADING_LANGUAGE_VERSION);
		//sscanf(glsl_version_str,"%f",&glsl_version);
		//max_shader_version = (int)(glsl_version * 100.0f + .4f);
		//printf("GLSL shader version support %s %4.2f %d\n", glsl_version_str, glsl_version, max_shader_version );
		//once = TRUE;
		GLSL_max_version = get_GLSL_max_version();
	}
	//generic
	vs = strdup(getGenericVertex());
	fs = strdup(getGenericFragment());
	//printf("size of frag shader %d\n",strlen(fs));  //MS vc has literal string size limit 65535

	CompleteCode[SHADERPART_VERTEX] = vs;
	CompleteCode[SHADERPART_GEOMETRY] = NULL;
	CompleteCode[SHADERPART_FRAGMENT] = fs;

	// what we really have here: UberShader with CastlePlugs
	// UberShader: one giant shader peppered with #ifdefs, and you add #defines at the top for permutations
	// CastlePlugs: allows users to add effects on to uberShader with PLUGs
	// - and internally, we can do a few permutations with PLUGs too

	if(isMobile){
		AddVersion(SHADERPART_VERTEX, 100, CompleteCode); //lower precision floats
		AddVersion(SHADERPART_FRAGMENT, 100, CompleteCode); //lower precision floats
		AddDefine(SHADERPART_FRAGMENT,"MOBILE",CompleteCode); //lower precision floats
	}else{
		if(GLSL_max_version >= 130) {
			AddVersion(SHADERPART_VERTEX, 130, CompleteCode); //lower precision floats
			AddVersion(SHADERPART_FRAGMENT, 130, CompleteCode); //lower precision floats
			AddDefine(SHADERPART_VERTEX, "FULL", CompleteCode); //lower precision floats
			AddDefine(SHADERPART_FRAGMENT, "FULL", CompleteCode); //lower precision floats
		}else{
			AddVersion(SHADERPART_VERTEX, GLSL_max_version, CompleteCode); //lower precision floats
			AddVersion(SHADERPART_FRAGMENT, GLSL_max_version, CompleteCode); //lower precision floats
		}
	}

	// printBitsB(sizeof(int),&whichOne.base); //debugging _shaderflags


	unique_int = 0; //helps generate method name PLUG_xxx_<unique_int> to avoid clash when multiple PLUGs supplied for same PLUG point
	//Add in:
	//Lit
	//Fog
	//analglyph
	if(DESIRE(whichOne.base,WANT_ANAGLYPH))
		Plug(SHADERPART_FRAGMENT,plug_fragment_end_anaglyph,CompleteCode,&unique_int);  //works, converts frag to gray
	//color per vertex
	if DESIRE(whichOne.base,COLOUR_MATERIAL_SHADER) {
		AddDefine(SHADERPART_VERTEX,"CPV",CompleteCode);
		AddDefine(SHADERPART_FRAGMENT,"CPV",CompleteCode);
	}
	if(DESIRE(whichOne.base,MODULATE_COLOR)){
		//we have a grayscale / illuminance image, modulate any color-per-vertes/face
		AddDefine(SHADERPART_VERTEX,"MODC",CompleteCode);
		AddDefine(SHADERPART_FRAGMENT,"MODC",CompleteCode);
	}
	if(DESIRE(whichOne.base,MODULATE_ALPHA)){
		// image texture doesn't havve an interesting alpha, use material alpha
		AddDefine(SHADERPART_FRAGMENT,"MODA",CompleteCode);
	}
	if(DESIRE(whichOne.base,MODULATE_TEXTURE)){
		// freewrl out-of-spec menu option: change single texture replace prior to texture modulate material diffuse
		AddDefine(SHADERPART_FRAGMENT,"MODT",CompleteCode);
	}
	//material appearance
	//2 material appearance
	//phong vs gourard
	if(DESIRE(whichOne.base,MATERIAL_APPEARANCE_SHADER) || DESIRE(whichOne.base,TWO_MATERIAL_APPEARANCE_SHADER)
		|| DESIRE(whichOne.base,PHYSICAL_MATERIAL_APPEARANCE_SHADER) || DESIRE(whichOne.base,UNLIT_MATERIAL_APPEARANCE_SHADER)){
		//we have a material node of some type
		AddDefine(SHADERPART_VERTEX,"LIT",CompleteCode);
		AddDefine(SHADERPART_FRAGMENT,"LIT",CompleteCode);

		if(DESIRE(whichOne.base,TWO_MATERIAL_APPEARANCE_SHADER)){
			AddDefine(SHADERPART_FRAGMENT,"TWO",CompleteCode);
			AddDefine(SHADERPART_VERTEX,"TWO",CompleteCode);
		}
		
		if(DESIRE(whichOne.base,SHADINGSTYLE_GOURAUD) || DESIRE(whichOne.base,MULTI_TEX_APPEARANCE_SHADER)){
			//when we say gouraud in freewrl, we really mean per-vertex lighting, in vertex shader
			AddDefine(SHADERPART_VERTEX,"LITE",CompleteCode);  //add some lights
			//with v4 Appearance.backMaterial, you could have physical on one side, and regular on the other - both
			if(DESIRE(whichOne.base,MATERIAL_APPEARANCE_SHADER) || DESIRE(whichOne.base,TWO_MATERIAL_APPEARANCE_SHADER))
				Plug(SHADERPART_VERTEX,plug_vertex_lighting_ADSLightModel,CompleteCode,&unique_int); //use lights
		} if(DESIRE(whichOne.base,SHADINGSTYLE_PHONG)){
			//when we say phong in freewrl, we really mean per-fragment lighting in fragment shader
			AddDefine(SHADERPART_FRAGMENT,"LITE",CompleteCode);  //add some lights
			//with v4 Appearance.backMaterial, you could have physical on one side, and regular on the other - both
			if(DESIRE(whichOne.base,PHYSICAL_MATERIAL_APPEARANCE_SHADER))
				Plug(SHADERPART_FRAGMENT,plug_frag_lighting_physical,CompleteCode,&unique_int); //use lights
			if(DESIRE(whichOne.base,MATERIAL_APPEARANCE_SHADER) || DESIRE(whichOne.base,TWO_MATERIAL_APPEARANCE_SHADER))
				Plug(SHADERPART_FRAGMENT,plug_vertex_lighting_ADSLightModel,CompleteCode,&unique_int); //use lights
			AddDefine(SHADERPART_FRAGMENT,"PHONG",CompleteCode);
		}
		//lines and points with material (rendered emissive)
		if( DESIRE(whichOne.base,HAVE_LINEPOINTS_COLOR) ) {
			AddDefine(SHADERPART_VERTEX,"LINE",CompleteCode);
			AddDefine(SHADERPART_FRAGMENT,"LINE",CompleteCode);
		}
	}
	//textureCoordinategen
	//cubemap texure
	//one tex appearance
	//multi tex appearance
	//cubemap tex
	/*	http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/lighting.html#Lightingon
		"The Material's transparency field modulates the alpha in the texture. Hence, 
		a transparency of 0 will result in an alpha equal to that of the texture. 
		A transparency of 1 will result in an alpha of 0 regardless of the value in the texture."
		That doesn't seem to me to be what browsers Octaga, InstantReality, or Cortona are doing, 
		and its not what table 17-3 and the Lighting equation say is happening. 
		In the table, alpha is never 'modulated' ie there's never an alpha= AT * (1-TM) term.
		- freewrl version 3 and vivaty do modulate.
		I've put a define to set if you don't want modulation ie table 17-3.
		If you do want to modulate ie the above quote "to modulate", comment out the define
		I put a mantis issue to web3d.org for clarification Aug 16, 2016
	*/
	int colCalc_loaded = FALSE;
	if(DESIRE(whichOne.base,HAVE_UNLIT_COLOR)){
		//used by particles
		AddDefine(SHADERPART_VERTEX,"UNLIT",CompleteCode);
		AddDefine(SHADERPART_FRAGMENT,"UNLIT",CompleteCode);
	}
	if(DESIRE(whichOne.base,HAVE_PROJECTIVETEXTURE)){
		AddDefine(SHADERPART_VERTEX,"PROJTEX",CompleteCode);
		AddDefine(SHADERPART_FRAGMENT,"PROJTEX",CompleteCode);
		AddDefine(SHADERPART_FRAGMENT,"TEX",CompleteCode);
		if(!colCalc_loaded) Plug(SHADERPART_FRAGMENT,plug_finalColCalc,CompleteCode,&unique_int);	
		colCalc_loaded = TRUE;
	}
	if (DESIRE(whichOne.base,ONE_TEX_APPEARANCE_SHADER) ||
		DESIRE(whichOne.base,HAVE_TEXTURECOORDINATEGENERATOR) ||
		DESIRE(whichOne.base,HAVE_CUBEMAP_TEXTURE) ||
		DESIRE(whichOne.base,MULTI_TEX_APPEARANCE_SHADER)) {

		AddDefine(SHADERPART_VERTEX,"TEX",CompleteCode);
		AddDefine(SHADERPART_FRAGMENT,"TEX",CompleteCode);
		if(DESIRE(whichOne.base,HAVE_TEXTURECOORDINATEGENERATOR) )
			AddDefine(SHADERPART_VERTEX,"TGEN",CompleteCode);
		if(DESIRE(whichOne.base,TEX3D_SHADER)){
			//in theory, if the texcoordgen "COORD" and TextureTransform3D are set in scenefile 
			// and working properly in freewrl, then don't need TEX3D for that node in VERTEX shader
			// which is using tex3dbbox (shape->_extent reworked) to get vertex coords in 0-1 range
			// x Sept 4, 2016 either TextureTransform3D or CoordinateGenerator "COORD" isn't working right for Texture3D
			// so we're using the bbox method
			//vertex str texture coords computed same for both volume and layered tex3d
			AddDefine(SHADERPART_VERTEX,"TEX3D",CompleteCode); 
			AddDefine(SHADERPART_FRAGMENT,"TEX3D",CompleteCode); 
			//fragment part different:
			if(DESIRE(whichOne.base,TEX3D_LAYER_SHADER)){
				//up to 6 textures, with lerp between floor,ceil textures
				AddDefine(SHADERPART_FRAGMENT,"TEX3DLAY",CompleteCode);
				Plug(SHADERPART_FRAGMENT,plug_fragment_texture3Dlayer_apply,CompleteCode,&unique_int);
			}else{
				//TEX3D_VOLUME_SHADER
				//AddDefine(SHADERPART_FRAGMENT,"TEX3D",CompleteCode);
				Plug(SHADERPART_FRAGMENT,plug_fragment_texture3D_apply_volume,CompleteCode,&unique_int);
			}
		}else {
			if(DESIRE(whichOne.base,HAVE_CUBEMAP_TEXTURE)){
				AddDefine(SHADERPART_VERTEX,"CUB",CompleteCode);
				AddDefine(SHADERPART_FRAGMENT,"CUB",CompleteCode);
			} else if(DESIRE(whichOne.base,MULTI_TEX_APPEARANCE_SHADER)){
				// https://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/texturing.html#MultiTexture 
				//- source can be DIFFUSE or SPECULAR, from Gauraud (vertex) lighting
				AddDefine(SHADERPART_VERTEX,"MTEX",CompleteCode);
				AddDefine(SHADERPART_FRAGMENT,"MTEX",CompleteCode);
			}

			if(!colCalc_loaded) Plug(SHADERPART_FRAGMENT,plug_finalColCalc,CompleteCode,&unique_int);	
			colCalc_loaded = TRUE;
			Plug(SHADERPART_FRAGMENT,plug_fragment_texture_apply,CompleteCode,&unique_int);

		}
	}

	//fill properties / hatching
	if(DESIRE(whichOne.base,FILL_PROPERTIES_SHADER)) {
		AddDefine(SHADERPART_VERTEX,"FILL",CompleteCode);		
		AddDefine(SHADERPART_FRAGMENT,"FILL",CompleteCode);
		if(GLSL_max_version >= 130) {
			Plug(SHADERPART_FRAGMENT,plug_fragment_fillProperties_apply,CompleteCode,&unique_int);
		}else{
			Plug(SHADERPART_FRAGMENT,plug_fragment_fillProperties_apply_120,CompleteCode,&unique_int);
		}
	}
	//LINETYPES
	if(DESIRE(whichOne.base,LINE_PROPERTIES_SHADER)) {
		AddDefine(SHADERPART_VERTEX,"LINETYPE",CompleteCode);		
		AddDefine(SHADERPART_FRAGMENT,"LINETYPE",CompleteCode);		
	}
	//POINT PROPERTIES 
	if(DESIRE(whichOne.base,POINT_PROPERTIES_SHADER)) {
		AddDefine(SHADERPART_VERTEX,"POINTP",CompleteCode);		
		AddDefine(SHADERPART_FRAGMENT,"POINTP",CompleteCode);	
	}
	//FOG
	if(DESIRE(whichOne.base,FOG_APPEARANCE_SHADER)){
		AddDefine(SHADERPART_VERTEX,"FOG",CompleteCode);		
		AddDefine(SHADERPART_FRAGMENT,"FOG",CompleteCode);	
		if(DESIRE(whichOne.base,HAVE_FOG_COORDS))
			AddDefine(SHADERPART_VERTEX,"FOGCOORD",CompleteCode);
		Plug(SHADERPART_FRAGMENT,plug_fog_apply,CompleteCode,&unique_int);	
	}
	//CLIPPLANE
	//FOG
	if(DESIRE(whichOne.base,CLIPPLANE_SHADER)){
		AddDefine(SHADERPART_VERTEX,"CLIP",CompleteCode);	
		Plug(SHADERPART_VERTEX,vertex_plug_clip_apply,CompleteCode,&unique_int);	
		AddDefine(SHADERPART_FRAGMENT,"CLIP",CompleteCode);	
		Plug(SHADERPART_FRAGMENT,frag_plug_clip_apply,CompleteCode,&unique_int);	
	}
	if(DESIRE(whichOne.base,PARTICLE_SHADER)){
		AddDefine(SHADERPART_VERTEX,"PARTICLE",CompleteCode);
	}
	//EFFECTS - castle game engine effect nodes X3D_Effect with plugs applied here
	EnableEffects(CompleteCode,&unique_int);

	// stripUnusedDefines(CompleteCode);
    // http://freecode.com/projects/unifdef/  example: unifdef -UTEX -UGMTEX shader.vs > out.vs will strip the TEX and MTEX sections out
	// or hack something like https://github.com/evanplaice/pypreprocessor 
	// dug9 hacked py3 for shader pre-processing: http://dug9.users.sourceforge.net/web3d/tests/largetexture/preprocessor_dug9_3T.py

	*fragmentSource = CompleteCode[SHADERPART_FRAGMENT]; //original_fragment; //fs;
	*vertexSource = CompleteCode[SHADERPART_VERTEX]; //original_vertex; //vs;
	//printf("size of finished fragment shader %d bytes\n",strlen(*fragmentSource));
//#define DEBUGSHADER 1
#ifdef DEBUGSHADER
	{
		//after writing to file you can run unifdef, sunifdef or coan on output to see reduced shader
		// http://coan2.sourceforge.net/
		//coan source -m composed_shader.vert > shader.vert
		FILE *fp = fopen("C:/tmp/composed_shader.vert","w+");
		fwrite(*vertexSource,strlen(*vertexSource),1,fp);
		fclose(fp);
		fp = fopen("C:/tmp/composed_shader.frag","w+");
		fwrite(*fragmentSource,strlen(*fragmentSource),1,fp);
		fclose(fp);
		printf("wrote shader\n");
	}
#endif //DEBUGSHADER
#undef DEBUGSHADER

	return retval;
}

// START MIT, VOLUME RENDERING >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/* Generic GLSL vertex shader, used on OpenGL ES. */
static const GLchar *volumeVertexGLES2 = " \n\
uniform mat4 fw_ModelViewMatrix; \n\
uniform mat4 fw_ProjectionMatrix; \n\
attribute vec4 fw_Vertex; \n\
 \n\
/* PLUG-DECLARATIONS */ \n\
 \n\
varying vec4 castle_vertex_eye; \n\
varying vec4 castle_Color; \n\
 \n\
void main(void) \n\
{ \n\
  vec4 vertex_object = fw_Vertex; \n\
  vec3 normal_object = vec3(0.0); \n\
  /* PLUG: vertex_object_space (vertex_object, normal_object) */ \n\
  castle_vertex_eye = fw_ModelViewMatrix * vertex_object; \n\
   \n\
   castle_Color = vec4(1.0,.5,.5,1.0); \n\
  \n\
  gl_Position = fw_ProjectionMatrix * castle_vertex_eye; \n\
   \n\
} \n\
";







/* Generic GLSL fragment shader, used on OpenGL ES. */
static const GLchar *volumeFragmentGLES2 = " \n\
/* DEFINES */ \n\
#ifdef MOBILE \n\
//precision highp float; \n\
precision mediump float; \n\
#endif //MOBILE \n\
 \n\
 vec4 HeatMapColor(float value, float minValue, float maxValue) \n\
{ \n\
	//used for debugging. If min=0,max=1 then magenta is 0, blue,green,yellow, red is 1 \n\
	vec4 ret; \n\
    int HEATMAP_COLORS_COUNT; \n\
    vec4 colors[6]; \n\
	HEATMAP_COLORS_COUNT = 6; \n\
	colors[0] = vec4(0.32, 0.00, 0.32, 1.0); \n\
    colors[1] = vec4( 0.00, 0.00, 1.00, 1.00); \n\
    colors[2] = vec4(0.00, 1.00, 0.00, 1.00); \n\
    colors[3] = vec4(1.00, 1.00, 0.00, 1.00); \n\
    colors[4] = vec4(1.00, 0.60, 0.00, 1.00); \n\
    colors[5] = vec4(1.00, 0.00, 0.00, 1.00); \n\
    float ratio=(float(HEATMAP_COLORS_COUNT)-1.0)*clamp((value-minValue)/(maxValue-minValue),0.0,1.0); \n\
    int indexMin=int(floor(ratio)); \n\
    int indexMax= indexMin+1 < HEATMAP_COLORS_COUNT-1 ? indexMin+1 : HEATMAP_COLORS_COUNT-1; \n\
    ret = mix(colors[indexMin], colors[indexMax], ratio-float(indexMin)); \n\
	if(value < minValue) ret = vec4(0.0,0.0,0.0,1.0); \n\
	if(value > maxValue) ret = vec4(1.0,1.0,1.0,1.0); \n\
	return ret; \n\
} \n\
vec4 debug_color; \n\
float hash( float n ) \n\
{ \n\
    return fract(sin(n)*43758.5453); \n\
} \n\
float noise( vec3 xyz ) \n\
{ \n\
    // The noise function returns a value in the range -1.0f -> 1.0f \n\
    vec3 p = floor(xyz); \n\
    vec3 f = fract(xyz); \n\
	\n\
    f = f*f*(3.0-2.0*f); \n\
    float n = p.x + p.y*57.0 + 113.0*p.z; \n\
	\n\
    return mix(mix(mix( hash(n+0.0), hash(n+1.0),f.x), \n\
                   mix( hash(n+57.0), hash(n+58.0),f.x),f.y), \n\
               mix(mix( hash(n+113.0), hash(n+114.0),f.x), \n\
                   mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z); \n\
} \n\
vec3 noise3( in vec3 xyz, in float range ){ \n\
	vec3 rxyz = vec3(xyz); \n\
	rxyz.x += noise(xyz)*range; \n\
	rxyz.y += noise(xyz)*range; \n\
	rxyz.z += noise(xyz)*range; \n\
	return rxyz; \n\
} \n\
 \n\
varying vec4 castle_vertex_eye; \n\
varying vec4 castle_Color; \n\
uniform mat4 fw_ModelViewProjInverse; \n\
//uniform float fw_FocalLength; \n\
uniform vec4 fw_viewport; \n\
uniform vec3 fw_dimensions; \n\
//uniform vec3 fw_RayOrigin; \n\
uniform sampler2D fw_Texture_unit0; \n\
uniform sampler2D fw_Texture_unit1; \n\
uniform sampler2D fw_Texture_unit2; \n\
uniform sampler2D fw_Texture_unit3; \n\
#ifdef TEX3D \n\
uniform int tex3dTiles[3]; \n\
uniform int repeatSTR[3]; \n\
uniform int magFilter; \n\
#endif //TEX3D \n\
#ifdef SEGMENT \n\
uniform int fw_nIDs; \n\
uniform int fw_enableIDs[10]; \n\
uniform int fw_surfaceStyles[2]; \n\
uniform int fw_nStyles; \n\
vec4 texture3Demu( sampler2D sampler, in vec3 texcoord3); \n\
vec4 texture3Demu0( sampler2D sampler, in vec3 texcoord3, int magfilter); \n\
bool inEnabledSegment(in vec3 texcoords, inout int jstyle){ \n\
	bool inside = true; \n\
	jstyle = 1; //DEFAULT \n\
	vec4 segel = texture3Demu0(fw_Texture_unit1,texcoords,0); \n\
	//convert from GL_FLOAT 0-1 to int 0-255 \n\
	//Q. is there a way to do int images in GLES2? \n\
	int ID = int(floor(segel.a * 255.0 + .1)); \n\
	//debug_color = HeatMapColor(float(ID),0.0,5.0); \n\
	//debug_color.a = .2; \n\
	if(ID < fw_nIDs){ \n\
		//specs: The indices of this array corresponds to the segment identifier. \n\
		inside = fw_enableIDs[ID] == 0 ? false : true; \n\
	} \n\
	if(inside){ \n\
		int kstyle = fw_nStyles-1; \n\
		kstyle = ID < fw_nStyles ? ID : kstyle; \n\
		jstyle = fw_surfaceStyles[kstyle]; \n\
		jstyle = jstyle == 1 ? 0 : jstyle; \n\
	} \n\
	return inside; \n\
} \n\
#endif //SEGMENT \n\
#ifdef ISO \n\
uniform float fw_stepSize; \n\
uniform float fw_tolerance; \n\
uniform float fw_surfaceVals[]; \n\
uniform int fw_nVals; \n\
uniform int fw_surfaceStyles[]; \n\
uniform int fw_nStyles; \n\
#endif //ISO \n\
 \n\
struct Ray { \n\
  vec3 Origin; \n\
  vec3 Dir; \n\
}; \n\
struct AABB { \n\
  vec3 Min; \n\
  vec3 Max; \n\
}; \n\
bool IntersectBox(Ray r, AABB aabb, out float t0, out float t1) \n\
{ \n\
    vec3 invR = 1.0 / r.Dir; \n\
    vec3 tbot = invR * (aabb.Min-r.Origin); \n\
    vec3 ttop = invR * (aabb.Max-r.Origin); \n\
    vec3 tmin = min(ttop, tbot); \n\
    vec3 tmax = max(ttop, tbot); \n\
    vec2 t = max(tmin.xx, tmin.yz); \n\
    t0 = max(t.x, t.y); \n\
    t = min(tmax.xx, tmax.yz); \n\
    t1 = min(t.x, t.y); \n\
    return t0 <= t1; \n\
} \n\
/* PLUG-DECLARATIONS */ \n\
 \n\
vec3 fw_TexCoord[1]; \n\
#ifdef CLIP \n\
#define FW_MAXCLIPPLANES 4 \n\
uniform int fw_nclipplanes; \n\
uniform vec4 fw_clipplanes[FW_MAXCLIPPLANES]; \n\
bool clip (in vec3 vertex_object){ \n\
  bool iclip = false; \n\
  for ( int i=0; i<fw_nclipplanes; i++ ) { \n\
    if( dot( fw_clipplanes[i], vec4(vertex_object,1.0)) < 0.0) \n\
		iclip = true; \n\
  } \n\
  return iclip; \n\
} \n\
#endif //CLIP \n\
vec3 vertex_eye; \n\
vec3 normal_eye; \n\
vec4 raysum; \n\
void main(void) \n\
{ \n\
	debug_color = vec4(0.0); \n\
	float maxDist = length(fw_dimensions); //1.414214; //sqrt(2.0); \n\
	int numSamples = 128; \n\
	float fnumSamples = float(numSamples); \n\
	float stepSize = maxDist/fnumSamples; \n\
	float densityFactor = 5.0/fnumSamples; //.88; // 1.0=normal H3D, .5 see deeper  \n\
	 \n\
    vec4 fragment_color; \n\
	//vec4 raysum; \n\
    vec3 rayDirection; \n\
	//convert window to frustum \n\
    rayDirection.xy = 2.0 * (gl_FragCoord.xy - fw_viewport.xy) / fw_viewport.zw - vec2(1.0); \n\
	rayDirection.z = 0.0; \n\
	vec3 rayOrigin; // = fw_RayOrigin; \n\
	//the equivalent of gluUnproject \n\
	//by unprojecting 2 points on ray here, this should also work with ortho viewpoint \n\
	vec4 ray4 = vec4(rayDirection,1.0); \n\
	vec4 org4 = ray4; \n\
	//if I back up the ray origin by -1.0 the front plane clipping works properly \n\
	ray4.z = 0.0; //1.0; \n\
	org4.z = -1.0; //0.0; \n\
	ray4 = fw_ModelViewProjInverse * ray4; \n\
	org4 = fw_ModelViewProjInverse * org4; \n\
	ray4 /= ray4.w; \n\
	org4 /= org4.w; \n\
	rayDirection = normalize(ray4.xyz - org4.xyz); \n\
	rayOrigin = org4.xyz; \n\
	\n\
    Ray eye = Ray( rayOrigin, rayDirection); \n\
	vec3 half_dimensions = fw_dimensions * .5; \n\
	vec3 minus_half_dimensions = half_dimensions * -1.0; \n\
	AABB aabb = AABB(minus_half_dimensions,half_dimensions); \n\
	\n\
    float tnear, tfar; \n\
    IntersectBox(eye, aabb, tnear, tfar); \n\
    if (tnear < 0.0) tnear = 0.0; \n\
    vec3 rayStart = eye.Origin + eye.Dir * tnear; \n\
    vec3 rayStop = eye.Origin + eye.Dir * tfar; \n\
    // Perform the ray marching: \n\
    vec3 pos = rayStart; \n\
    vec3 step = normalize(rayStop-rayStart) * stepSize; \n\
    float totaltravel = distance(rayStop, rayStart); \n\
	float travel = totaltravel; \n\
    float T = 1.0; \n\
    vec3 Lo = vec3(0.0); \n\
	normal_eye = rayDirection; \n\
	vec3 pos2 = pos; \n\
    // Transform from object space to texture coordinate space: \n\
	pos2 = (pos2+half_dimensions)/fw_dimensions; \n\
	pos2 = clamp(pos2,0.001,.999); \n\
	raysum = vec4(0.0); \n\
	float depth = 0.0; \n\
	float lastdensity; \n\
	float lastdensity_iso; \n\
	\n\
    for (int i=0; i < numSamples; ++i) { \n\
		//raysum = HeatMapColor(travel,0.0,2.0); \n\
		//break; \n\
       // ...lighting and absorption stuff here... \n\
		pos2 = pos; \n\
		vertex_eye = pos2; \n\
	    // Transform from object space to texture coordinate space: \n\
		pos2 = (pos2+half_dimensions)/fw_dimensions; \n\
		//pos2.z = 1.0 - pos2.z; //RHS to LHS \n\
		pos2 = clamp(pos2,0.001,.999); \n\
		vec3 texcoord3 = pos2; \n\
		bool iclip = false; \n\
		#ifdef CLIP \n\
		iclip = clip(vertex_eye); //clip(totaltravel - travel); \n\
		#endif //CLIP \n\
		if(!iclip) { \n\
			fragment_color = vec4(1.0,0.0,1.0,1.0); //do I need a default? seems not \n\
			/* PLUG: texture3D ( fragment_color, texcoord3) */ \n\
			#ifdef SEGMENT \n\
			int jstyle = 1; \n\
			if(inEnabledSegment(texcoord3,jstyle)){ \n\
			#endif //SEGMENT \n\
			//assuming we had a scalar input image and put L into .a, \n\
			// and computed gradient and put in .rgb : \n\
			float density = fragment_color.a; //recover the scalar value \n\
			vec3 gradient = fragment_color.rgb - vec3(.5,.5,.5); //we added 127 to (-127 to 127) in CPU gradient computation\n\
			//vec4 voxel = vec4(density,density,density,density); //this is where the black visual voxels come from\n\
			vec4 voxel = vec4(density,density,density,density); //this is where the black visual voxels come from\n\
			\n\
			#ifdef ISO \n\
			if(i==0){ \n\
				lastdensity = density; \n\
				lastdensity_iso = 0.0; \n\
			} \n\
			int MODE = fw_nVals == 1 ? 1 : 3; \n\
			MODE = fw_stepSize != 0.0 && MODE == 1 ? 2 : 1; \n\
			#ifdef ISO_MODE3 \n\
			if(MODE == 3){ \n\
				for(int i=0;i<fw_nVals;i++){ \n\
					float iso = fw_surfaceVals[i]; \n\
					if( sign( density - iso) != sign( lastdensity - iso) && length(gradient) > fw_tolerance ){ \n\
						voxel.a = 1.0; \n\
						int jstyle = min(i,fw_nStyles-1); \n\
						jstyle = fw_surfaceStyles[jstyle]; \n\
						if(jstyle == 1){ \n\
							/* PLUG: voxel_apply_DEFAULT (voxel, gradient) */ \n\
						} else if(jstyle == 2) { \n\
							/* PLUG: voxel_apply_OPACITY (voxel, gradient) */ \n\
						} else if(jstyle == 3) { \n\
							/* PLUG: voxel_apply_BLENDED (voxel, gradient) */ \n\
						} else if(jstyle == 4) { \n\
							/* PLUG: voxel_apply_BOUNDARY (voxel, gradient) */ \n\
						} else if(jstyle == 5) { \n\
							/* PLUG: voxel_apply_CARTOON (voxel, gradient) */ \n\
						} else if(jstyle == 6) { \n\
							/* PLUG: voxel_apply_DEFAULT (voxel, gradient) */ \n\
						} else if(jstyle == 7) { \n\
							/* PLUG: voxel_apply_EDGE (voxel, gradient) */ \n\
						} else if(jstyle == 8) { \n\
							/* PLUG: voxel_apply_PROJECTION (voxel, gradient) */ \n\
						} else if(jstyle == 9) { \n\
							/* PLUG: voxel_apply_SHADED (voxel, gradient) */ \n\
						} else if(jstyle == 10) { \n\
							/* PLUG: voxel_apply_SILHOUETTE (voxel, gradient) */ \n\
						} else if(jstyle == 11) { \n\
							/* PLUG: voxel_apply_TONE (voxel, gradient) */ \n\
						} \n\
					} else { \n\
						voxel = vec4(0.0); //similar to discard \n\
					} \n\
				} \n\
				lastdensity = density; \n\
			} \n\
			#else //ISO_MODE3 \n\
			if(MODE == 1){ \n\
				float iso = fw_surfaceVals[0]; \n\
				if( sign( density - iso) != sign( lastdensity - iso) && length(gradient) > fw_tolerance ){ \n\
					//debug_color = HeatMapColor(iso,0.0,.3); \n\
					voxel.a = 1.0; \n\
					/* PLUG: voxel_apply (voxel, gradient) */ \n\
				} else { \n\
					voxel = vec4(0.0); //similar to discard \n\
				} \n\
				lastdensity = density; \n\
			} else if(MODE == 2){ \n\
				float iso = fw_surfaceVals[0]; \n\
				float density_iso = density / fw_stepSize; \n\
				if( sign( density_iso - iso) != sign( lastdensity_iso - iso) && length(gradient) > fw_tolerance ){ \n\
					voxel.a = 1.0; \n\
					/* PLUG: voxel_apply (voxel, gradient) */ \n\
				} else { \n\
					voxel = vec4(0.0); //similar to discard \n\
				} \n\
				lastdensity = density; \n\
				lastdensity_iso = density_iso; \n\
			}  \n\
			#endif //ISO_MODE3 \n\
			#else //ISO \n\
			#ifdef SEGMENT \n\
			//debug_color = HeatMapColor(float(jstyle),1.0,12.0); \n\
			//debug_color.a = .2; \n\
			if(jstyle == 1){ \n\
				/* PLUG: voxel_apply_DEFAULT (voxel, gradient) */ \n\
			} else if(jstyle == 2) { \n\
				/* PLUG: voxel_apply_OPACITY (voxel, gradient) */ \n\
			} else if(jstyle == 3) { \n\
				/* PLUG: voxel_apply_BLENDED (voxel, gradient) */ \n\
			} else if(jstyle == 4) { \n\
				/* PLUG: voxel_apply_BOUNDARY (voxel, gradient) */ \n\
			} else if(jstyle == 5) { \n\
				/* PLUG: voxel_apply_CARTOON (voxel, gradient) */ \n\
			} else if(jstyle == 6) { \n\
				/* PLUG: voxel_apply_DEFAULT (voxel, gradient) */ \n\
			} else if(jstyle == 7) { \n\
				/* PLUG: voxel_apply_EDGE (voxel, gradient) */ \n\
			} else if(jstyle == 8) { \n\
				/* PLUG: voxel_apply_PROJECTION (voxel, gradient) */ \n\
			} else if(jstyle == 9) { \n\
				/* PLUG: voxel_apply_SHADED (voxel, gradient) */ \n\
			} else if(jstyle == 10) { \n\
				/* PLUG: voxel_apply_SILHOUETTE (voxel, gradient) */ \n\
			} else if(jstyle == 11) { \n\
				/* PLUG: voxel_apply_TONE (voxel, gradient) */ \n\
			} \n\
			#else //SEGMENT \n\
			//non-iso rendering styles \n\
			//void PLUG_voxel_apply (inout vec4 voxel, inout vec3 gradient) \n\
			/* PLUG: voxel_apply (voxel, gradient) */ \n\
			#endif //SEGMENT \n\
			#endif //ISO \n\
			density = voxel.a; \n\
			//debug_color = HeatMapColor(densityFactor,0.134,.135); \n\
			T = (1.0 - raysum.a); \n\
			raysum.a += density * T; \n\
			raysum.rgb += voxel.rgb  * T * density; \n\
			if(raysum.a > .99) { \n\
				break; \n\
			} \n\
			#ifdef SEGMENT \n\
			} //if inEnabledSegment \n\
			#endif //SEGMENT \n\
		} //iclip \n\
		travel -= stepSize; \n\
		depth += stepSize; \n\
		if(travel <= 0.0) break; \n\
		pos += step; \n\
		\n\
    }  \n\
	//void PLUG_ray_apply (inout vec4 raysum) \n\
	/* PLUG: ray_apply (raysum) */ \n\
	if(true) gl_FragColor = raysum; \n\
	else gl_FragColor = debug_color; \n\
} \n\
";


//void PLUG_voxel_apply (inout vec4 voxel, inout vec3 gradient) { 

// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/volume.html#OpacityMapVolumeStyle
// opacity with intensity == intensity lookup/transferFunction

static const GLchar *plug_voxel_DEFAULT =	"\
void voxel_apply_DEFAULT (inout vec4 voxel, inout vec3 gradient) { \n\
	float alpha = voxel.a; \n\
	//voxel.a = voxel.r; \n\
	//voxel.rgb = vec3(alpha); \n\
} \n\
void PLUG_voxel_apply_DEFAULT (inout vec4 voxel, inout vec3 gradient) { \n\
	voxel_apply_DEFAULT(voxel,gradient); \n\
} \n\
void PLUG_voxel_apply (inout vec4 voxel, inout vec3 gradient) { \n\
	voxel_apply_DEFAULT(voxel,gradient); \n\
} \n\
";

// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/volume.html#OpacityMapVolumeStyle
static const GLchar *plug_voxel_OPACITY =	"\
uniform int fw_opacTexture; \n\
//uniform sampler2D fw_Texture_unit3; \n\
void voxel_apply_OPACITY (inout vec4 voxel, inout vec3 gradient) { \n\
	if(fw_opacTexture == 1){ \n\
		vec4 lookup_color; \n\
		float lum = voxel.r; \n\
		vec2 texcoord = vec2(lum,0.0); \n\
		//this is too simple for the lookups in the specs \n\
		//http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/volume.html#t-transferFunctionTextureCoordinateMapping \n\
		lookup_color = texture2D(fw_Texture_unit3,texcoord); \n\
		voxel.rgb = lookup_color.rgb; \n\
		voxel.a = lum; \n\
	}else{ \n\
		//like default \n\
		float alpha = voxel.a; \n\
		voxel.a = voxel.r; \n\
		voxel.rgb = vec3(alpha); \n\
	} \n\
} \n\
void PLUG_voxel_apply_OPACITY (inout vec4 voxel, inout vec3 gradient) { \n\
	voxel_apply_OPACITY(voxel, gradient); \n\
} \n\
void PLUG_voxel_apply (inout vec4 voxel, inout vec3 gradient) { \n\
	voxel_apply_OPACITY(voxel, gradient); \n\
} \n\
";

// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/volume.html#BlendedVolumeStyle
static const GLchar *plug_voxel_BLENDED =	"\
void PLUG_voxel_apply (inout vec4 voxel, inout vec3 gradient) { \n\
} \n\
";

// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/volume.html#BoundaryEnhancementVolumeStyle
static const GLchar *plug_voxel_BOUNDARY =	"\n\
uniform float fw_boundaryOpacity; \n\
uniform float fw_retainedOpacity; \n\
uniform float fw_opacityFactor; \n\
void voxel_apply_BOUNDARY (inout vec4 voxel, inout vec3 gradient) { \n\
	float magnitude = length(gradient); \n\
	float factor = (fw_retainedOpacity + fw_boundaryOpacity*pow(magnitude,fw_opacityFactor) ); \n\
	voxel.a = voxel.a * factor; \n\
	//debug_color = HeatMapColor(factor,0.0,1.0); \n\
} \n\
void PLUG_voxel_apply_BOUNDARY (inout vec4 voxel, inout vec3 gradient) { \n\
	voxel_apply_BOUNDARY(voxel, gradient); \n\
} \n\
void PLUG_voxel_apply (inout vec4 voxel, inout vec3 gradient) { \n\
	voxel_apply_BOUNDARY(voxel, gradient); \n\
} \n\
";

// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/volume.html#CartoonVolumeStyle
static const GLchar *plug_voxel_CARTOON =	"\n\
uniform int fw_colorSteps; \n\
uniform vec4 fw_orthoColor; \n\
uniform vec4 fw_paraColor; \n\
void voxel_apply_CARTOON (inout vec4 voxel, inout vec3 gradient) { \n\
	float len = length(gradient); \n\
	if(len > 0.01) { \n\
		vec3 ng = normalize(gradient); \n\
		float ndotv = dot(normal_eye,ng); \n\
		if(ndotv > 0.01 && voxel.a > 0.0) { \n\
			ndotv = floor(ndotv/float(fw_colorSteps))*float(fw_colorSteps); \n\
			vec4 color = mix(fw_orthoColor,fw_paraColor,ndotv); \n\
			//voxel.rgb = color.rgb*voxel.a; \n\
			voxel.rgb = color.rgb; \n\
		} else { \n\
			voxel = vec4(0.0); //similar to discard \n\
		} \n\
	} else { \n\
		voxel = vec4(0.0); //similar to discard \n\
	} \n\
} \n\
void PLUG_voxel_apply_CARTOON (inout vec4 voxel, inout vec3 gradient) { \n\
	voxel_apply_CARTOON(voxel, gradient); \n\
} \n\
void PLUG_voxel_apply (inout vec4 voxel, inout vec3 gradient) { \n\
	voxel_apply_CARTOON(voxel, gradient); \n\
} \n\
";

// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/volume.html#ComposedVolumeStyle
static const GLchar *plug_voxel_COMPOSED =	"\
void PLUG_voxel_apply (inout vec4 voxel, inout vec3 gradient) { \n\
} \n\
";

// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/volume.html#EdgeEnhancementVolumeStyle
static const GLchar *plug_voxel_EDGE =	"\
uniform float fw_cosGradientThreshold; \n\
uniform vec4 fw_edgeColor; \n\
void voxel_apply_EDGE (inout vec4 voxel, inout vec3 gradient) { \n\
	float len = length(gradient); \n\
	if(len > 0.01) { \n\
		vec3 ng = normalize(gradient); \n\
		float ndotv = abs(dot(normal_eye,ng));  \n\
		if( ndotv < fw_cosGradientThreshold ) { \n\
			voxel = mix(voxel,fw_edgeColor,1.0 -ndotv); \n\
		} \n\
	} \n\
} \n\
void PLUG_voxel_apply_EDGE (inout vec4 voxel, inout vec3 gradient) { \n\
	voxel_apply_EDGE(voxel, gradient); \n\
} \n\
void PLUG_voxel_apply (inout vec4 voxel, inout vec3 gradient) { \n\
	voxel_apply_EDGE(voxel, gradient); \n\
} \n\
";

// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/volume.html#ProjectionVolumeStyle
static const GLchar *plug_voxel_PROJECTION =	"\n\
uniform float fw_intensityThreshold; \n\
uniform int fw_projType; \n\
float MAXPROJ = 0.0; \n\
float MINPROJ = 1.0; \n\
float AVEPROJ = 0.0; \n\
float LMIP = 0.0; \n\
vec4 RGBAPROJ; \n\
int PROJCOUNT = 0; \n\
void voxel_apply_PROJECTION (inout vec4 voxel, inout vec3 gradient) { \n\
	PROJCOUNT++; \n\
	float cval = length(voxel.rgb); \n\
	if(fw_projType == 1){ \n\
		//MIN \n\
		if(cval < MINPROJ){ \n\
			MINPROJ = cval; \n\
			RGBAPROJ = voxel; \n\
		} \n\
	}else if(fw_projType == 2){ \n\
		//MAX \n\
		if(fw_intensityThreshold > 0.0){ \n\
			//LMIP \n\
			if(LMIP == 0.0) { \n\
				LMIP = cval; \n\
				RGBAPROJ = voxel; \n\
			} \n\
		} else { \n\
			//MIP \n\
			if(cval > MAXPROJ){ \n\
				MAXPROJ = cval; \n\
				RGBAPROJ = voxel; \n\
			} \n\
		} \n\
	}else if(fw_projType==3){ \n\
		//AVERAGE \n\
		AVEPROJ += cval; \n\
		RGBAPROJ += voxel; \n\
	} \n\
} \n\
void PLUG_voxel_apply_PROJECTION (inout vec4 voxel, inout vec3 gradient) { \n\
	voxel_apply_PROJECTION(voxel, gradient); \n\
} \n\
void PLUG_voxel_apply (inout vec4 voxel, inout vec3 gradient) { \n\
	voxel_apply_PROJECTION(voxel, gradient); \n\
} \n\
void PLUG_ray_apply (inout vec4 raysum) { \n\
	float value = 0.0; \n\
	vec4 color = vec4(1.0); \n\
	if(fw_projType == 1){ \n\
		//MIN \n\
		value = MINPROJ; \n\
		color = RGBAPROJ; \n\
	}else if(fw_projType == 2){ \n\
		//MAX \n\
		if(fw_intensityThreshold > 0.0){ \n\
			//LMIP \n\
			value = LMIP; \n\
			color = RGBAPROJ; \n\
		} else { \n\
			//MIP \n\
			value = MAXPROJ; \n\
			color = RGBAPROJ; \n\
		} \n\
	}else if(fw_projType==3){ \n\
		//AVERAGE \n\
		value = AVEPROJ / float(PROJCOUNT); \n\
		color = RGBAPROJ / float(PROJCOUNT); \n\
	} \n\
	//raysum.rgb = color.rgb * color.a; \n\
	//raysum.a = color.a; \n\
	\n\
	raysum.rgb = vec3(value,value,value); \n\
//	raysum.a = 1.0 - value; \n\
	//raysum.a = value;\n\
	//raysum.a = color.a; \n\
	//raysum = color; \n\
} \n\
";

// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/volume.html#ShadedVolumeStyle
static const GLchar *plug_voxel_SHADED =	"\
#ifdef LITE \n\
#define MAX_LIGHTS 8 \n\
uniform int lightcount; \n\
//uniform float lightRadius[MAX_LIGHTS]; \n\
uniform int lightType[MAX_LIGHTS];//ANGLE like this \n\
struct fw_LightSourceParameters { \n\
  float ambient;  \n\
  vec3 color;   \n\
  float intensity; \n\
  vec3 location;   \n\
  vec3 halfVector;  \n\
  vec3 direction; \n\
  float spotBeamWidth; \n\
  float spotCutoff; \n\
  vec3 Attenuations; \n\
  float lightRadius; \n\
}; \n\
\n\
uniform fw_LightSourceParameters fw_LightSource[MAX_LIGHTS] /* gl_MaxLights */ ;\n\
#endif //LITE \n\
 \n\
#ifdef FOG \n\
struct fogParams \n\
{  \n\
  vec4 fogColor; \n\
  float visibilityRange; \n\
  float fogScale; \n\
  int fogType; // 0 None, 1= FOGTYPE_LINEAR, 2 = FOGTYPE_EXPONENTIAL \n\
  // ifdefed int haveFogCoords; \n\
}; \n\
uniform fogParams fw_fogparams; \n\
#endif //FOG \n\
#ifdef LIT \n\
#ifdef LITE \n\
//per-fragment lighting ie phong \n\
struct fw_MaterialParameters { \n\
  vec3 diffuse; \n\
  vec3 emissive; \n\
  vec3 specular; \n\
  float ambient; \n\
  float shininess; \n\
  float occlusion; \n\
  float transparency; \n\
  vec3 baseColor; \n\
  float metallic; \n\
  float roughness; \n\
  int type; \n\
  // multitextures are disaggregated \n\
  int tindex[10]; \n\
  int mode[10]; \n\
  int source[10]; \n\
  int func[10]; \n\
  int nt; //total single textures \n\
  //iunit [0] normal [1] emissive [2] occlusion [3] diffuse OR base [4] shininess OR metallicRoughness [5] specular [6] ambient \n\
  int tcount[7]; //num single textures 1= one texture 0=no texture 2+ = multitexture \n\
  int tstart[7]; // where in packed tindex list to start looping \n\
  int cindex[7]; // which geometry multitexcoord channel 0=default \n\
}; \n\
uniform fw_MaterialParameters fw_FrontMaterial; \n\
#ifdef TWO \n\
uniform fw_MaterialParameters fw_BackMaterial; \n\
#endif //TWO \n\
vec3 castle_ColorES; \n\
#else //LITE \n\
//per-vertex lighting - interpolated Emissive-specular \n\
varying vec3 castle_ColorES; //emissive shininess term \n\
#endif //LITE \n\
#endif //LIT\n\
uniform int fw_phase; \n\
uniform int fw_lighting; \n\
uniform int fw_shadows; \n\
void voxel_apply_SHADED (inout vec4 voxel, inout vec3 gradient) { \n\
	float len = length(gradient); \n\
	vec3 ng = vec3(0.0); \n\
	if(len > 0.0) \n\
	  ng = normalize(gradient); \n\
	vec3 color = vec3(1.0); \n\
	#ifdef LIT \n\
	vec3 castle_ColorES = fw_FrontMaterial.specular; \n\
	color.rgb = fw_FrontMaterial.diffuse.rgb; \n\
	#else //LIT \n\
	color.rgb = vec3(0,0,0.0,0.0); \n\
	vec3 castle_ColorES = vec3(0.0,0.0,0.0); \n\
	#endif //LIT	\n\
	// void add_light_contribution2(inout vec3 vertexcolor, inout vec3 specularcolor, in vec4 myPosition, in vec3 myNormal, \n\
	//   in float mat_shininess, in float mat_ambient, in vec3 mat_diffuse, in vec3 mat_specular); \n\
	vec4 vertex_eye4 = vec4(vertex_eye,1.0); \n\
	/* PLUG: add_light_contribution2 (color, castle_ColorES, vertex_eye4, ng, fw_FrontMaterial.shininess, fw_FrontMaterial.ambient, fw_FrontMaterial.diffuse, fw_FrontMaterial..specular) */ \n\
	// voxel.rgb = color.rgb; \n\
	color = mix(color,castle_ColorES,dot(ng,normal_eye)); \n\
	voxel.rgb = color.rgb; \n\
	//voxel.rgb = voxel.rgb * color.rgb; \n\
} \n\
void PLUG_voxel_apply_SHADED (inout vec4 voxel, inout vec3 gradient) { \n\
	voxel_apply_SHADED(voxel, gradient); \n\
} \n\
void PLUG_voxel_apply (inout vec4 voxel, inout vec3 gradient) { \n\
	voxel_apply_SHADED(voxel, gradient); \n\
} \n\
";

// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/volume.html#SilhouetteEnhancementVolumeStyle
static const GLchar *plug_voxel_SILHOUETTE =	"\n\
uniform float fw_BoundaryOpacity; \n\
uniform float fw_RetainedOpacity; \n\
uniform float fw_Sharpness; \n\
void voxel_apply_SILHOUETTE (inout vec4 voxel, inout vec3 gradient) { \n\
	float len = length(gradient); \n\
	if(len > 0.01) { \n\
		vec3 ng = normalize(gradient); \n\
		float ndotv = abs(dot(ng,normal_eye)); \n\
		float factor = (fw_RetainedOpacity + fw_BoundaryOpacity*pow(1.0 - ndotv,fw_Sharpness)); \n\
		//float factor = (fw_RetainedOpacity + pow(fw_BoundaryOpacity*(1.0 - ndotv),fw_Sharpness)); \n\
		//debug_color = HeatMapColor(factor,0.0,1.0); \n\
		voxel.a = voxel.a * factor; \n\
	} \n\
} \n\
void PLUG_voxel_apply_SILHOUETTE (inout vec4 voxel, inout vec3 gradient) { \n\
	voxel_apply_SILHOUETTE(voxel, gradient); \n\
} \n\
void PLUG_voxel_apply (inout vec4 voxel, inout vec3 gradient) { \n\
	voxel_apply_SILHOUETTE(voxel, gradient); \n\
} \n\
";

// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/volume.html#ToneMappedVolumeStyle
static const GLchar *plug_voxel_TONE =	"\
uniform vec4 fw_coolColor; \n\
uniform vec4 fw_warmColor; \n\
void voxel_apply_TONE (inout vec4 voxel, inout vec3 gradient) { \n\
	float len = length(gradient); \n\
	if(len > 0.0) { \n\
		vec3 color; \n\
		vec3 ng = normalize(gradient); \n\
		//vec3 L = normalize(vec3(-.707,-.707,.707)); \n\
		float cc = (1.0 + dot(normal_eye,ng))*.5; \n\
		//float cc = (1.0 + dot(L,ng))*.5; \n\
		//debug_color = HeatMapColor(cc,0.0,1.0); \n\
		color = mix(fw_coolColor.rgb,fw_warmColor.rgb,cc); \n\
		voxel = vec4(color,voxel.a); \n\
	} else { \n\
		voxel.a = 0.0; \n\
		//debug_color = vec4(0.0); \n\
	} \n\
} \n\
void PLUG_voxel_apply_TONE (inout vec4 voxel, inout vec3 gradient) { \n\
	voxel_apply_TONE(voxel, gradient); \n\
} \n\
void PLUG_voxel_apply (inout vec4 voxel, inout vec3 gradient) { \n\
	voxel_apply_TONE(voxel, gradient); \n\
} \n\
";

// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/volume.html#BlendedVolumeStyle
// blended (BlendedVolumeStyle) works (was interpreted and implemented by dug9, Oct 2016)
//  by rendering 2 volumedatas to 2 fbo textures, then running
// this shader to blend the 2 fbo textures and spit out fragments just
// where the box is. Use with the regular volume Vertex shader so
// only frags over the box show, and so we get box depth for depth blending
// with the main scene
static const GLchar *volumeBlendedFragmentGLES2 = " \n\
/* DEFINES */ \n\
#ifdef MOBILE \n\
//precision highp float; \n\
precision mediump float; \n\
#endif //MOBILE \n\
 vec4 HeatMapColor(float value, float minValue, float maxValue) \n\
{ \n\
	//used for debugging. If min=0,max=1 then magenta is 0, blue,green,yellow, red is 1 \n\
	vec4 ret; \n\
    int HEATMAP_COLORS_COUNT; \n\
    vec4 colors[6]; \n\
	HEATMAP_COLORS_COUNT = 6; \n\
	colors[0] = vec4(0.32, 0.00, 0.32, 1.0); \n\
    colors[1] = vec4( 0.00, 0.00, 1.00, 1.00); \n\
    colors[2] = vec4(0.00, 1.00, 0.00, 1.00); \n\
    colors[3] = vec4(1.00, 1.00, 0.00, 1.00); \n\
    colors[4] = vec4(1.00, 0.60, 0.00, 1.00); \n\
    colors[5] = vec4(1.00, 0.00, 0.00, 1.00); \n\
    float ratio=(float(HEATMAP_COLORS_COUNT)-1.0)*clamp((value-minValue)/(maxValue-minValue),0.0,1.0); \n\
    int indexMin=int(floor(ratio)); \n\
    int indexMax= indexMin+1 < HEATMAP_COLORS_COUNT-1 ? indexMin+1 : HEATMAP_COLORS_COUNT-1; \n\
    ret = mix(colors[indexMin], colors[indexMax], ratio-float(indexMin)); \n\
	if(value < minValue) ret = vec4(0.0,0.0,0.0,1.0); \n\
	if(value > maxValue) ret = vec4(1.0,1.0,1.0,1.0); \n\
	return ret; \n\
} \n\
vec4 debug_color; \n\
 \n\
uniform vec4 fw_viewport; \n\
uniform sampler2D fw_Texture_unit0; \n\
uniform sampler2D fw_Texture_unit1; \n\
uniform sampler2D fw_Texture_unit2; \n\
uniform sampler2D fw_Texture_unit3; \n\
uniform float fw_iwtc1; \n\
uniform float fw_iwtc2; \n\
uniform int fw_iwtf1; \n\
uniform int fw_iwtf2; \n\
uniform int fw_haveTransfers; \n\
vec3 weightcolor( in vec3 color, in int func, in float wt, in float ov,  in float oblend, in sampler2D table){ \n\
	vec3 ret; \n\
	if(func == 1){ \n\
		ret = color * wt; \n\
	}else if(func == 2){ \n\
		ret = color * ov; \n\
	}else if(func == 3){ \n\
		ret = color * oblend; \n\
	}else if(func == 4){ \n\
		ret = color * (1.0 - oblend); \n\
	}else if(func == 5){ \n\
		vec2 texcoord = color.rg;\n\
		ret = color * texture2D(table,texcoord).r; \n\
	} \n\
	return ret; \n\
} \n\
float weightalpha( in float alpha, in int func, in float wt, in float ov, in float oblend, in sampler2D table){ \n\
	float ret; \n\
	if(func == 1){ \n\
		ret = alpha * wt; \n\
	}else if(func == 2){ \n\
		ret = alpha * ov; \n\
	}else if(func == 3){ \n\
		ret = alpha * oblend; \n\
	}else if(func == 4){ \n\
		ret = alpha * (1.0 - oblend); \n\
	}else if(func == 5){ \n\
		vec2 texcoord = vec2(alpha,0);\n\
		ret = alpha * texture2D(table,texcoord).r; \n\
	} \n\
	return ret; \n\
} \n\
void main(void) \n\
{ \n\
    vec2 fc = (gl_FragCoord.xy - fw_viewport.xy) / fw_viewport.zw; \n\
	vec4 frag0 = texture2D(fw_Texture_unit0,fc); \n\
	vec4 frag1 = texture2D(fw_Texture_unit1,fc); \n\
	vec3 cv = frag0.rgb; \n\
	float ov = frag0.a; \n\
	vec3 cblend = frag1.rgb; \n\
	float oblend = frag1.a; \n\
	vec3 cvw, cbw; \n\
	float ovw, obw; \n\
	cvw = weightcolor(cv,fw_iwtf1,fw_iwtc1,ov,oblend,fw_Texture_unit2); \n\
	ovw = weightalpha(ov,fw_iwtf1,fw_iwtc1,ov,oblend,fw_Texture_unit2); \n\
	cbw = weightcolor(cblend,fw_iwtf2,fw_iwtc2,ov,oblend,fw_Texture_unit3); \n\
	obw = weightalpha(oblend,fw_iwtf2,fw_iwtc2,ov,oblend,fw_Texture_unit3); \n\
	vec3 cg = clamp( cvw + cbw, 0.0, 1.0); \n\
	float og = clamp(ovw + obw, 0.0, 1.0); \n\
	\n\
	gl_FragColor = vec4(cg,og); \n\
} \n\
";


const char *getVolumeVertex(void){
	return volumeVertexGLES2; //genericVertexDesktop
}
const char *getVolumeFragment(){
	return volumeFragmentGLES2; //genericFragmentDesktop;
}
int getSpecificShaderSourceVolume (const GLchar **vertexSource, const GLchar **fragmentSource, shaderflagsstruct whichOne) 
{
	//for building the Builtin (similar to fixed-function pipeline, except from shader parts)
	//in OpenGL_Utils.c L.2553 set usingCastlePlugs = 1 to get in here.
	//whichone - a bitmask of shader requirements, one bit for each requirement, so shader permutation can be built

	int retval, unique_int;
	unsigned int volflags;
	unsigned char volflag[7];
	int kflags,i,k;
	char *CompleteCode[3];
	char *vs, *fs;
	retval = FALSE;
	if(whichOne.usershaders ) //& USER_DEFINED_SHADER_MASK) 
		return retval; //not supported yet as of Aug 9, 2016
	retval = TRUE;

	//generic
	vs = strdup(getVolumeVertex());
	fs = strdup(getVolumeFragment());
		
	CompleteCode[SHADERPART_VERTEX] = vs;
	CompleteCode[SHADERPART_GEOMETRY] = NULL;
	if(whichOne.volume == SHADERFLAGS_VOLUME_STYLE_BLENDED << 4){
		CompleteCode[SHADERPART_FRAGMENT] = STRDUP(volumeBlendedFragmentGLES2);

	}else{
		CompleteCode[SHADERPART_FRAGMENT] = fs;
	}

	// what we really have here: UberShader with CastlePlugs
	// UberShader: one giant shader peppered with #ifdefs, and you add #defines at the top for permutations
	// CastlePlugs: allows users to add effects on to uberShader with PLUGs
	// - and internally, we can do a few permutations with PLUGs too

	if(isMobile){
		AddVersion(SHADERPART_VERTEX, 100, CompleteCode); //lower precision floats
		AddVersion(SHADERPART_FRAGMENT, 100, CompleteCode); //lower precision floats
		AddDefine(SHADERPART_FRAGMENT,"MOBILE",CompleteCode); //lower precision floats
	}else{
		//desktop, emulating GLES2
		AddVersion(SHADERPART_VERTEX, 110, CompleteCode); //lower precision floats
		AddVersion(SHADERPART_FRAGMENT, 110, CompleteCode); //lower precision floats
	}

	if(whichOne.volume == SHADERFLAGS_VOLUME_STYLE_BLENDED << 4){
		*fragmentSource = CompleteCode[SHADERPART_FRAGMENT]; //original_fragment; //fs;
		*vertexSource = CompleteCode[SHADERPART_VERTEX]; //original_vertex; //vs;
		return retval;
	}

	unique_int = 0; //helps generate method name PLUG_xxx_<unique_int> to avoid clash when multiple PLUGs supplied for same PLUG 

	//if(DESIRE(whichOne.volume,TEX3D_SHADER)){
	AddDefine(SHADERPART_FRAGMENT,"TEX3D",CompleteCode); 
	Plug(SHADERPART_FRAGMENT,plug_fragment_texture3D_apply_volume,CompleteCode,&unique_int); //uses TILED
	//}

	if(DESIRE(whichOne.volume,SHADERFLAGS_VOLUME_DATA_BASIC)){
		AddDefine(SHADERPART_FRAGMENT,"BASIC",CompleteCode); 
	}
	if(DESIRE(whichOne.volume,SHADERFLAGS_VOLUME_DATA_ISO)){
		AddDefine(SHADERPART_FRAGMENT,"ISO",CompleteCode); 
		if(DESIRE(whichOne.volume,SHADERFLAGS_VOLUME_DATA_ISO_MODE3)){
			AddDefine(SHADERPART_FRAGMENT,"ISO_MODE3",CompleteCode); 
		}
	}
	if(DESIRE(whichOne.volume,SHADERFLAGS_VOLUME_DATA_SEGMENT)){
		AddDefine(SHADERPART_FRAGMENT,"SEGMENT",CompleteCode); 
	}
	if(DESIRE(whichOne.base,CLIPPLANE_SHADER)){
		AddDefine(SHADERPART_FRAGMENT,"CLIP",CompleteCode);	
		// we use a special function in frag for clip for volume
	}

	//unsigned int 32 bits - 4 for basic, leaves 28/4 = max 7 styles
	//work from left to right (the order declared), skip any 0/null/empties
	volflags = whichOne.volume;
	// this was just here, but is defined above. volflag[7];
	kflags = 0;
	for(i=0;i<7;i++){
		int iflag = (volflags >> (7-i)*4) & 0xF;
		if(iflag){
			volflag[kflags] = iflag;
			kflags++;
		}
	}
	//now volflag[] is in the order declared with no 0s/nulls 
	for(k=0;k<kflags;k++){
		switch(volflag[k]){
		case SHADERFLAGS_VOLUME_STYLE_DEFAULT:
			AddDefine(SHADERPART_FRAGMENT,"DEFAULT",CompleteCode); 
			Plug(SHADERPART_FRAGMENT,plug_voxel_DEFAULT,CompleteCode,&unique_int);
			break;
		case SHADERFLAGS_VOLUME_STYLE_OPACITY:
			AddDefine(SHADERPART_FRAGMENT,"OPACITY",CompleteCode); 
			Plug(SHADERPART_FRAGMENT,plug_voxel_OPACITY,CompleteCode,&unique_int);
			break;
		case SHADERFLAGS_VOLUME_STYLE_BLENDED:
			AddDefine(SHADERPART_FRAGMENT,"BLENDED",CompleteCode); 
			Plug(SHADERPART_FRAGMENT,plug_voxel_BLENDED,CompleteCode,&unique_int);
			break;
		case SHADERFLAGS_VOLUME_STYLE_BOUNDARY:
			AddDefine(SHADERPART_FRAGMENT,"BOUNDARY",CompleteCode); 
			Plug(SHADERPART_FRAGMENT,plug_voxel_BOUNDARY,CompleteCode,&unique_int);
			break;
		case SHADERFLAGS_VOLUME_STYLE_CARTOON:
			AddDefine(SHADERPART_FRAGMENT,"CARTOON",CompleteCode); 
			Plug(SHADERPART_FRAGMENT,plug_voxel_CARTOON,CompleteCode,&unique_int);
			break;
		case SHADERFLAGS_VOLUME_STYLE_COMPOSED:
			AddDefine(SHADERPART_FRAGMENT,"COMPOSED",CompleteCode); 
			Plug(SHADERPART_FRAGMENT,plug_voxel_COMPOSED,CompleteCode,&unique_int);
			break;
		case SHADERFLAGS_VOLUME_STYLE_EDGE:
			AddDefine(SHADERPART_FRAGMENT,"EDGE",CompleteCode); 
			Plug(SHADERPART_FRAGMENT,plug_voxel_EDGE,CompleteCode,&unique_int);
			break;
		case SHADERFLAGS_VOLUME_STYLE_PROJECTION:
			AddDefine(SHADERPART_FRAGMENT,"PROJECTION",CompleteCode); 
			Plug(SHADERPART_FRAGMENT,plug_voxel_PROJECTION,CompleteCode,&unique_int);
			break;
		case SHADERFLAGS_VOLUME_STYLE_SHADED:
			AddDefine(SHADERPART_FRAGMENT,"SHADED",CompleteCode); 
			Plug(SHADERPART_FRAGMENT,plug_voxel_SHADED,CompleteCode,&unique_int);
			//if you want a plug within a plug, then if you include the higher level
			// plug first, then the lower level plug should find its tartget in the CompleteCode
			AddDefine(SHADERPART_FRAGMENT,"LIT",CompleteCode); 
			AddDefine(SHADERPART_FRAGMENT,"LITE",CompleteCode); 
			Plug(SHADERPART_FRAGMENT,plug_vertex_lighting_ADSLightModel,CompleteCode,&unique_int);
			break;
		case SHADERFLAGS_VOLUME_STYLE_SILHOUETTE:
			AddDefine(SHADERPART_FRAGMENT,"SILHOUETTE",CompleteCode); 
			Plug(SHADERPART_FRAGMENT,plug_voxel_SILHOUETTE,CompleteCode,&unique_int);
			break;
		case SHADERFLAGS_VOLUME_STYLE_TONE:
			AddDefine(SHADERPART_FRAGMENT,"TONE",CompleteCode); 
			Plug(SHADERPART_FRAGMENT,plug_voxel_TONE,CompleteCode,&unique_int);
			break;
		default:
			//if 0, just skip
			break;
		}
	}


	//shader doesn't compile?
	//in visual studio, this is a good place to get the composed shader source, then paste into
	// an editor that has line numbers, to get to the ERROR line
	*fragmentSource = CompleteCode[SHADERPART_FRAGMENT]; //original_fragment; //fs;
	*vertexSource = CompleteCode[SHADERPART_VERTEX]; //original_vertex; //vs;
//#define DEBUGSHADER 1
#ifdef DEBUGSHADER
	{
		//after writing to file you can run unifdef, sunifdef or coan on output to see reduced shader
		// http://coan2.sourceforge.net/
		//coan source -m composed_shader.vert > shader.vert
		FILE *fp = fopen("C:/tmp/composed_shader.vert","w+");
		fwrite(*vertexSource,strlen(*vertexSource),1,fp);
		fclose(fp);
		fp = fopen("C:/tmp/composed_shader.frag","w+");
		fwrite(*fragmentSource,strlen(*fragmentSource),1,fp);
		fclose(fp);
	}
#endif //DEBUGSHADER
	return retval;

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< END MIT, VOLUME RENDERING
