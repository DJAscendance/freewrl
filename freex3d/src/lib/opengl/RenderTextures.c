/*


Texturing during Runtime 
texture enabling - works for single texture, for multitexture. 

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



#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../opengl/OpenGL_Utils.h"
#include "../scenegraph/Component_Shape.h"
#include "../scenegraph/RenderFuncs.h"
#include "../scenegraph/LinearAlgebra.h"
#include "../scenegraph/PolyRep.h"
#include "Textures.h"
#include "Material.h"


struct multiTexParams {
int multitex_mode[2];
int multitex_source[2];
int multitex_function;
};

typedef struct pRenderTextures{
	struct multiTexParams textureParameterStack[MAX_MULTITEXTURE];
	int textureUnit_used;
	GLint texture_in_unit[32];
	GLint sampler_type[32];

}* ppRenderTextures;

void *RenderTextures_constructor(){
	void *v = MALLOCV(sizeof(struct pRenderTextures));
	memset(v,0,sizeof(struct pRenderTextures));
	return v;
}
void RenderTextures_init(struct tRenderTextures *t){
	t->prv = RenderTextures_constructor();
	{
		ppRenderTextures p = (ppRenderTextures)t->prv;
		/* variables for keeping track of status */
		t->textureParameterStack = (void *)p->textureParameterStack;
		p->textureUnit_used = 0;
	}
}

unsigned char* generate_checkerboard_texture_data_RGBA(int size8, int divisions8, int black255, int white255, int opacity255) {
	int texSize = size8;
	int checkerSize = texSize / divisions8;
	unsigned char* texdata = malloc(texSize * texSize * 4);
	//initialize to black transparent
	memset(texdata, 0, texSize * texSize * 4);
	unsigned char black[4];
	unsigned char white[4];
	unsigned char* pixel;
	black[0] = black[1] = black[2] = black255; black[3] = opacity255;
	white[0] = white[1] = white[2] = white255; white[3] = opacity255;
	for (int i = 0; i < texSize; ++i) {
		for (int j = 0; j < texSize; ++j) {
			int ii = i / checkerSize;
			int jj = j / checkerSize;
			int evenii = (ii % 2) == 0;
			int evenjj = (jj % 2) == 0;

			int drawWhite = evenii == evenjj;
			pixel = black;
			if (drawWhite) 
				pixel = white;
			int location = (i * texSize + j) * 4;
			memcpy(&texdata[location], pixel, 4);
		}
	}
	return texdata;
}
static GLuint checkerboard_texture2D = 0;
static GLuint checkerboard_textureCube = 0;
static unsigned char* checkerboard_data = NULL;
static int checkerboard_size = 64;
void compile_checkerboard_texture2D() {
	if (checkerboard_texture2D < 1) {
		PRINT_GL_ERROR_IF_ANY("compile_checkerboard_texture2D start");

		int texSize = checkerboard_size;
		int divisions = 8;
		int black = 0;
		int white = 255;
		int opacity = 127;
		unsigned char* texdata = generate_checkerboard_texture_data_RGBA(texSize, divisions, black, white, opacity);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glGenTextures(1, &checkerboard_texture2D);
		glBindTexture(GL_TEXTURE_2D, checkerboard_texture2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSize, texSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, texdata);
		//free(texdata);
		checkerboard_data = texdata;
		PRINT_GL_ERROR_IF_ANY("compile_checkerboard_texture2D end");

	}
}
void compile_checkerboard_textureCube() {
	if (checkerboard_textureCube < 1) {
		compile_checkerboard_texture2D();
		PRINT_GL_ERROR_IF_ANY("compile_checkerboard_textureCube start");

		glActiveTexture(GL_TEXTURE0);
		PRINT_GL_ERROR_IF_ANY("compile_checkerboard_textureCube very early");
		glGenTextures(1, &checkerboard_textureCube);
		glBindTexture(GL_TEXTURE_CUBE_MAP, checkerboard_textureCube);
		PRINT_GL_ERROR_IF_ANY("compile_checkerboard_textureCube early");
		//glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA, checkerboard_size, checkerboard_size);
		PRINT_GL_ERROR_IF_ANY("compile_checkerboard_textureCube middle");
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		//-allocates storage for all 6 faces
		for (int face = 0; face < 6; face++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, 0, GL_RGBA, checkerboard_size, checkerboard_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkerboard_data);
			//glTextureSubImage3D(GL_TEXTURE_2D_ARRAY, //face
			//	0, //level
			//	0, 0, //x,y offset
			//	face, //Z offset is the face
			//	checkerboard_size, checkerboard_size, //size of face
			//	1, //one face at a time (depth)
			//	GL_RGBA, //format
			//	GL_UNSIGNED_BYTE, //type
			//	checkerboard_data); //Data if you have it)
			PRINT_GL_ERROR_IF_ANY("compile_checkerboard_textureCube face");

		}
		PRINT_GL_ERROR_IF_ANY("compile_checkerboard_textureCube end");

	}
}
//void render_checkerboard_default_texture() {
//	compile_checkerboard_texture2D();
//	//glActiveTexture(GL_TEXTURE0);
//	//glBindTexture(GL_TEXTURE_2D, checkerboard_texture);
//}
GLuint getCheckerboardTexture2D() {
	compile_checkerboard_texture2D();
	return checkerboard_texture2D;
}
GLuint getCheckerboardTextureCube() {
	compile_checkerboard_textureCube();
	return checkerboard_textureCube;
}
void clear_textureUnit_used(){
	//call this in child_shape just before you start sending data / textures to the shader program
	ppRenderTextures p;
	ttglobal tg = gglobal();
	p = (ppRenderTextures)tg->RenderTextures.prv;
	//render_checkerboard_default_texture();
	for (int i = 0; i < 16; i++) {
		glBindTextureUnit(i, 0);
		//glBindTextureUnit(i, checkerboard_texture2D);
		//glBindTextureUnit(i, checkerboard_textureCube);
	}
	p->textureUnit_used = 0;  //start at 1 and leave TEXTURE0 for debugging?
}
int next_textureUnit(){
	ppRenderTextures p;
	ttglobal tg = gglobal();
	p = (ppRenderTextures)tg->RenderTextures.prv;
	p->textureUnit_used++;
	return p->textureUnit_used -1;
}

int textureUnit_used(){
	ppRenderTextures p;
	ttglobal tg = gglobal();
	p = (ppRenderTextures)tg->RenderTextures.prv;
	return p->textureUnit_used;
}
int bind_or_share_next_textureUnit(const int samplerType, GLint texture){
	// call this when sending textures to the shader 
	// benefits over gl_activeTexture + glBindTexture:
	// this one automatically
	// a) checks if this texture is already bound, perhaps by another PTM projector, or another material
	//   and if so return the texture unit OR
	// b) if not already bound, increments the texture unit, binds (and returns its  texture unit index

	ppRenderTextures p;
	ttglobal tg = gglobal();
	p = (ppRenderTextures)tg->RenderTextures.prv;

	//check if sharable
	int unit = -1;
	for(int i=0;i<p->textureUnit_used;i++){
		if(p->texture_in_unit[i] == texture && samplerType == p->sampler_type[i]){
			unit = i;
			break;
		}
	}
	if(unit == -1) {
		unit = next_textureUnit();
		p->texture_in_unit[unit] = texture;
		p->sampler_type[unit] = samplerType;
		glBindTextureUnit(unit, 0); //clears all targets for a unit, gl 4.5 https://www.khronos.org/opengl/wiki/Sampler_(GLSL)
		glActiveTexture(GL_TEXTURE0+unit); 
		glBindTexture(samplerType,texture);
	}
	return unit;
}



/* function params */
//static void passedInGenTex(struct textureVertexInfo *genTex);

/* which texture unit are we going to use? is this texture not OFF?? Should we set the
   background coloUr??? Larry the Cucumber, help! */

static int setActiveTexture (int c, GLint *texUnit, GLint *texMode) 
{
	ppRenderTextures p;
	ttglobal tg = gglobal();
	p = (ppRenderTextures)tg->RenderTextures.prv;

	/* which texture unit are we working on? */
    
	/* tie each fw_TextureX uniform into the correct texture unit */
    
	/* here we assign the texture unit to a specific number. NOTE: in the current code, this will ALWAYS
	 * be [0] = 0, [1] = 1; etc. */
	texUnit[c] = c;

#ifdef TEXVERBOSE
	if (getAppearanceProperties()->currentShaderProperties != NULL) {
		printf ("setActiveTexture %d, boundTextureStack is %d, sending to uniform %d\n",c,
			tg->RenderFuncs.boundTextureStack[c],
			getAppearanceProperties()->currentShaderProperties->TextureUnit[c]);
	} else {
		printf ("setActiveTexture %d, boundTextureStack is %d, sending to uniform [NULL--No Shader]\n",c,
			tg->RenderFuncs.boundTextureStack[c]);
	}
#endif
    
	/* is this a MultiTexture, or just a "normal" single texture?  When we
	 * bind_image, we store a pointer for the texture parameters. It is
	 * NULL, possibly different for MultiTextures */

	if (p->textureParameterStack[c].multitex_mode[0] == INT_ID_UNDEFINED) {
        
		#ifdef TEXVERBOSE
		printf ("setActiveTexture - simple texture NOT a MultiTexture \n"); 
		#endif

		/* should we set the coloUr to 1,1,1,1 so that the material does not show
		 * through a RGB texture?? */
		/* only do for the first texture if MultiTexturing */
		if (c == 0) {
			#ifdef TEXVERBOSE
			printf ("setActiveTexture - firsttexture  \n"); 
			#endif
			texMode[c]= GL_MODULATE;
		} else {
			texMode[c]=GL_ADD;
		}

	} else {
	/* printf ("muititex source for %d is %d\n",c,tg->RenderTextures.textureParameterStack[c].multitex_source); */
		if (p->textureParameterStack[c].multitex_source[0] != MTMODE_OFF) {
		} else {
			//glDisable(GL_TEXTURE_2D); /* DISABLE_TEXTURES */
			//return FALSE;
			// we do OFF right in the shader
		}
	}


	PRINT_GL_ERROR_IF_ANY("");

	return TRUE;
}


/* lets disable texture transforms here */
void textureTransform_end(void) {
	int j;
	ttglobal tg = gglobal();
    
#ifdef TEXVERBOSE
	printf ("start of textureTransform_end\n");
#endif

	/* DISABLE_TEXTURES */
	/* setting this ENSURES that items, like the HUD, that are not within the normal
	   rendering path do not try and use textures... */
	FW_GL_MATRIX_MODE(GL_TEXTURE);
	int ntransforms;
	fw_glGetInteger(GL_TEXTURE_STACK_DEPTH, &ntransforms);
	//for(j=0;j<tg->RenderFuncs.textureStackTop;j++)
	for (j = 0; j < ntransforms; j++)
		FW_GL_POP_MATRIX(); //pushed in passedInGenTex

	tg->RenderFuncs.textureStackTop = 0;
	tg->RenderFuncs.texturenode = NULL;
	FW_GL_MATRIX_MODE(GL_MODELVIEW);
}

/* did we have a TextureTransform in the Appearance node? */
void do_textureTransform0 (struct X3D_Node *textureNode, int ttnum, char **tmap, int *igen, int *parameter_n, float *parameter) {
	*tmap = NULL;
	*igen = TCGT_REGULAR;
	/* is this a simple TextureTransform? */
	if (textureNode->_nodeType == NODE_TextureTransform) {
		//ConsoleMessage ("do_textureTransform, node is indeed a NODE_TextureTransform");
		struct X3D_TextureTransform* ttt = (struct X3D_TextureTransform*)textureNode;
		*tmap = ttt->mapping ? ttt->mapping->strptr : NULL;
		/*  Render transformations according to spec.*/
		//http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/texturing.html#TextureTransform
		//specs say 'translate, rotate, then scale'
		FW_GL_TRANSLATE_F(-((ttt->center).c[0]), -((ttt->center).c[1]), 0);		/*  5*/
		FW_GL_SCALE_F(((ttt->scale).c[0]), ((ttt->scale).c[1]), 1);			/*  4*/
		FW_GL_ROTATE_RADIANS(ttt->rotation, 0, 0, 1);					/*  3*/
		FW_GL_TRANSLATE_F(((ttt->center).c[0]), ((ttt->center).c[1]), 0);		/*  2*/
		FW_GL_TRANSLATE_F(((ttt->translation).c[0]), ((ttt->translation).c[1]), 0);	/*  1*/
	} else if (textureNode->_nodeType == NODE_TextureTransformGenerator) {
		struct X3D_TextureTransformGenerator* ttg = (struct X3D_TextureTransformGenerator*)textureNode;
		*tmap = ttg->mapping ? ttg->mapping->strptr : NULL;
		*igen = findFieldInARR((ttg)->mode->strptr, TEXTURECOORDINATEGENERATOR, TEXTURECOORDINATEGENERATOR_COUNT);
		//any matrix prep that would simplify GPU / vertex shader side?
		//any clock-time animations?
		//any routed parameter[] updates?
	} else  if (textureNode->_nodeType == NODE_MultiTextureTransform) {
		/* is this a MultiTextureTransform? */
		struct X3D_MultiTextureTransform *mtt = (struct X3D_MultiTextureTransform *) textureNode;
		if (ttnum < mtt->textureTransform.n) {
			struct X3D_TextureTransform *ttt = (struct X3D_TextureTransform *) mtt->textureTransform.p[ttnum];
			/* is this a simple TextureTransform? */
			if (ttt->_nodeType == NODE_TextureTransform) {
				*tmap = ttt->mapping ? ttt->mapping->strptr : NULL;

				/*  Render transformations according to spec.*/
				FW_GL_TRANSLATE_F(-((ttt->center).c[0]), -((ttt->center).c[1]), 0);		/*  5*/
				FW_GL_SCALE_F(((ttt->scale).c[0]), ((ttt->scale).c[1]), 1);			/*  4*/
				FW_GL_ROTATE_RADIANS(ttt->rotation, 0, 0, 1);					/*  3*/
				FW_GL_TRANSLATE_F(((ttt->center).c[0]), ((ttt->center).c[1]), 0);		/*  2*/
				FW_GL_TRANSLATE_F(((ttt->translation).c[0]), ((ttt->translation).c[1]), 0);	/*  1*/
			} else if(ttt->_nodeType == NODE_TextureTransformGenerator){
				struct X3D_TextureTransformGenerator* ttg = (struct X3D_TextureTransformGenerator*)ttt;
				*tmap = ttg->mapping ? ttg->mapping->strptr : NULL;
				*igen = findFieldInARR((ttg)->mode->strptr, TEXTURECOORDINATEGENERATOR, TEXTURECOORDINATEGENERATOR_COUNT);

				*parameter_n = ttg->parameter.n;
				memcpy(parameter, ttg->parameter.p, ttg->parameter.n * sizeof(float));
				//any matrix prep that would simplify GPU / vertex shader side?
				//any clock-time animations?
				//any routed parameter[] updates?
			} else {
				static int once = 0;
				if(!once){
					printf ("MultiTextureTransform expected a textureTransform for texture %d, got %d %s \n",
					ttnum, ttt->_nodeType, stringNodeType(ttt->_nodeType));
					once = 1;
				}
			}
		} else {
			static int once = 0;
			if(!once){
				printf ("not enough transforms in MultiTextureTransform -will fill with Identity matrix\n");
				once = 1;
			}
		}
	} else if (textureNode->_nodeType == NODE_TextureTransform3D) {
		//ConsoleMessage ("do_textureTransform, node is indeed a NODE_TextureTransform");
		struct X3D_TextureTransform3D  *ttt = (struct X3D_TextureTransform3D *) textureNode;
		*tmap = ttt->mapping ? ttt->mapping->strptr : NULL;

		/*  Render transformations according to spec.*/
		FW_GL_TRANSLATE_F(-((ttt->center).c[0]),-((ttt->center).c[1]), -((ttt->center).c[2]));		/*  5*/
		FW_GL_SCALE_F(((ttt->scale).c[0]),((ttt->scale).c[1]),((ttt->scale).c[2]));			/*  4*/
		FW_GL_ROTATE_RADIANS(ttt->rotation.c[3], ttt->rotation.c[0],ttt->rotation.c[1],ttt->rotation.c[2]);
		FW_GL_TRANSLATE_F(((ttt->center).c[0]),((ttt->center).c[1]), ((ttt->center).c[2]));		/*  2*/
		FW_GL_TRANSLATE_F(((ttt->translation).c[0]), ((ttt->translation).c[1]), ((ttt->translation).c[2]));	/*  1*/
	} else if (textureNode->_nodeType == NODE_TextureTransformMatrix3D) {
		//ConsoleMessage ("do_textureTransform, node is indeed a NODE_TextureTransform");
		int i;
		double mat[16];
		struct X3D_TextureTransformMatrix3D  *ttt = (struct X3D_TextureTransformMatrix3D *) textureNode;
		*tmap = ttt->mapping ? ttt->mapping->strptr : NULL;

		for(i=0;i<16;i++)
			mat[i] = (double)ttt->matrix.c[i];
		FW_GL_SETDOUBLEV(GL_TEXTURE_MATRIX,mat);
	} else {
		static int once = 0;
		if(!once){
			printf ("expected a textureTransform node, got %d %s\n",textureNode->_nodeType, stringNodeType(textureNode->_nodeType));
			once = 1;
		}
	}

	//FW_GL_MATRIX_MODE(GL_MODELVIEW);
}
//void do_textureTransform(struct X3D_Node* textureNode, int ttnum) {
//	char *tmap;
//	do_textureTransform0(textureNode, ttnum, &tmap);
//}
/***********************************************************************************/
int isMultiTexture(struct X3D_Node *node){
	int ret = FALSE;
	if(node && node->_nodeType == NODE_MultiTexture)
		ret = TRUE;
	return ret;
}
int isTex3D(struct X3D_Node *node);


int is_cubeMap(struct X3D_Node* node) {
	int iret = FALSE;
	if (node) {
		switch (node->_nodeType) {
		case NODE_ComposedCubeMapTexture:
		case NODE_ImageCubeMapTexture:
		case NODE_GeneratedCubeMapTexture:
			iret = TRUE;
			break;
		}
	}
	return iret;
}
int is_or_has_cubeMap(struct X3D_Node* node) {
	int ret = FALSE;
	if (!node) return ret;
	struct X3D_Node** p;
	p = &node;
	int n = 1;
	if (node->_nodeType == NODE_MultiTexture) {
		struct X3D_MultiTexture* mnode = (struct X3D_MultiTexture*)node;
		p = mnode->texture.p;
		n = mnode->texture.n;
	}
	for (int i = 0; i < n; i++) {
		switch (p[i]->_nodeType) {
		case NODE_ComposedCubeMapTexture:
		case NODE_ImageCubeMapTexture:
		case NODE_GeneratedCubeMapTexture:
			ret = TRUE;
			break;
		default:
			break;
		}
	}
	return ret;
}
int is_or_has_Tex3D(struct X3D_Node* node);
char* trans_name_from_texture_mapping(char* mmap, char* scratch) {
	char* aret = NULL;
	if (mmap) {
		aret = scratch;
		memset(scratch, 0, 10);
		//advanced way - look for "C0T2" style string from gltf_loader and split, else just copy
		if (mmap[0] == 'T') scratch[0] = mmap[1];
		else if (mmap[2] == 'T') scratch[0] = mmap[3];
		else strcpy(scratch, mmap);
	}
	return aret;
}
char* coord_name_from_texture_mapping(char* mmap, char* scratch) {
	char* aret = NULL;
	if (mmap) {
		aret = scratch;
		memset(scratch, 0, 10);
		//advanced way - look for "C0T2" style string from gltf_loader and split, else just copy
		if (mmap[0] == 'C') scratch[0] = mmap[1];
		else if (mmap[2] == 'C') scratch[0] = mmap[3];
		else strcpy(scratch, mmap);
	}
	return aret;
}
int find_in_char_list(char *name, char** list, int n) {
	int iret = -1;
	if(list && n && name)
		for(int i=0;i<n;i++)
			if (strcmp(name, list[i]) == 0) {
				iret = i;
				break;
			}
	return iret;
}
// https://stackoverflow.com/questions/3911400/how-to-pass-2d-array-matrix-in-a-function-in-c
int find_or_make_combo_by_index(int itrans, int icoord, int combo_list[][2], int* ncombo) {
	int found = 0;
	int iret = -1;
	for (int i = 0; i < *ncombo; i++) {
		// Q. in freewrl does icoord == -1 mean the same as icoord == 0, both refer to first texcoord?
		if (combo_list[i][0] == icoord && combo_list[i][1] == itrans) {
			found = TRUE;
			iret = i;
			break;
		}
	}
	if (!found) {
		iret = (*ncombo);
		combo_list[iret][0] = icoord;
		combo_list[iret][1] = itrans;
		(*ncombo)++;
	}
	return iret; //this will be the index into frag shader fw_TexCoord[iret] for a given texture.

}
int find_or_make_combo(char* trans_name, char* coord_name, char** coord_name_list, int ntcoord, 
	char** trans_name_list, int ntrans, int combo_list[][2], int* ncombo) {
	int itrans = find_in_char_list(trans_name, trans_name_list, ntrans);
	int icoord = find_in_char_list(coord_name, coord_name_list, ntcoord);
	int iret = find_or_make_combo_by_index(itrans, icoord, combo_list, ncombo);
	//int found = 0;
	//int iret = -1;
	//for (int i = 0; i < *ncombo; i++) {
	//	// Q. in freewrl does icoord == -1 mean the same as icoord == 0, both refer to first texcoord?
	//	if (combo_list[i][0] == icoord && combo_list[i][1] == itrans) {
	//		found = TRUE;
	//		iret = i;
	//		break;
	//	}
	//}
	//if (!found) {
	//	iret = (*ncombo);
	//	combo_list[iret][0] = icoord;
	//	combo_list[iret][1] = itrans;
	//	(* ncombo)++;
	//}
	return iret; //this will be the index into frag shader fw_TexCoord[iret] for a given texture.
}
int next_or_last_sampler_compatible_trans(int lasttrans, int ntrans, int jsamplr, int* igen) {
	int itrans = -1;
	//find the prior compatible if any
	for (int i = 0; i < lasttrans + 1; i++){
		if (jsamplr == 0 && igen[i] == TCGT_REGULAR) itrans = i;
		if (jsamplr == 1 && igen[i] != TCGT_REGULAR) itrans = i;
	}
	//over-ride with the next compatible if any
	for (int i = lasttrans + 1; i < ntrans; i++) {
		if (jsamplr == 1 && igen[i] != TCGT_REGULAR) {
			itrans = i;
			break;
		}
		if (jsamplr == 0 && igen[i] == TCGT_REGULAR) {
			itrans = i;
			break;
		}
	}
	return itrans;
}


int getTextureDescriptors(struct X3D_Node* textureNode, int* textures, int* modes, int* sources, int* funcs, int* width, int* height, int* samplr);
GLint tunit(int index);
void textureTransform_start() {
	int c;
	int i, isStrict, isMulti, isIdentity, ntransforms[2];
	GLint texUnit[MAX_MULTITEXTURE];
	//GLint tunit[MAX_MULTITEXTURE];
	GLint texMode[MAX_MULTITEXTURE];
	s_shader_capabilities_t* me;
	struct X3D_Node* tnode;
	int itmap[MAX_MULTITEXTURE];
	int igen[MAX_MULTITEXTURE];

	int parameter_n;
	float parameter[7];

	int icombo[MAX_MULTITEXTURE + 2][2];
	int immap[MAX_MULTITEXTURE];

	int ncombo;
	char* tmap[MAX_MULTITEXTURE];
	ppRenderTextures p;
	ttglobal tg = gglobal();
	p = (ppRenderTextures)tg->RenderTextures.prv;
	tnode = tg->RenderFuncs.texturenode;

	me = getAppearanceProperties()->currentShaderProperties;

	//PRINT_GL_ERROR_IF_ANY("TT_start_start");

#ifdef TEXVERBOSE
	printf("passedInGenTex, using passed in genTex, textureStackTop %d\n", tg->RenderFuncs.textureStackTop);
	printf("passedInGenTex, cubeFace %d\n", getAppearanceProperties()->cubeFace);
#endif 
	int oldway = 0;
	FW_GL_MATRIX_MODE(GL_TEXTURE);
	if (oldway) for (int i = 0; i < MAX_MULTITEXTURE; i++) immap[i] = 0;
	for (int i = 0; i < MAX_MULTITEXTURE + 2; i++) {
		icombo[i][0] = icombo[i][1] = -1; //[0] index of texcoord ie 0 = TEXCOORD_0, -1 means 0, [1] index of textrans, -1 means idenity
	}
	ncombo = 1; // de-minimus for shapeless things

	parameter_n = 0;
	//printf ("passedInGenTex, B\n");
	isStrict = 1;  //web3d specs say if its a multitexture, 
		//and you give it a single textureTransform instead of multitexturetransform 
		//it should ignore the singleTextureTransform and use identities. 
		//strict: This is a change of functionality for freewrl Aug 31, 2016

	if (!tg->RenderFuncs.shapenode) {
		//May 1 2022 annoying Background comes through here - please fix
		//we still use ubershader on background textures, send de-minimus uniforms to vertex shader
		glUniform1i(me->cmap[0], icombo[0][0]);
		glUniform1i(me->tmap[0], icombo[0][1]);
		glUniform1i(me->ntexcombo, ncombo);
	}
	else
	{
		// April 29, 2022: cases not handled: .mapping to 2 textrans, but only one texcoord or vice versa
		// khronos gltf GreenChair has one texcoord, and multiple textrans 
		// https://github.com/KhronosGroup/3DC-Certification/tree/main/models 
		// https://github.khronos.org/3DC-Sample-Viewer/
		// tests\multitexture\multimap_material_2trans_1coord.x3dv
		// May 2, 2022 change of concept of operations
		// vertex shader produces out fw_TexCoord[4] from combinations of input texcoord and textrans
		// frag shader / material.texture needs to know which combination, by index, for use in sample_map()
		// here we prepare combinations, up to one for each texture, from given and default textrans and texcoords
		// we store the combo index in the material.texture.cmap, 
		// and send vertex shader 2 values for each combo: index to texcoord or -1, index to textran or -1
		// -1 means default: for texcoord that's texcoord[0], for textrans that's identity
		// benefit over old concept: frag fw_TexCoord[] length and index is decoupled from geometry.texcoord[] order, length and index
		//  allowing more combinations of textrans and texcoords
		struct matpropstruct* matprop;
		struct X3D_Node* gn = NULL;
		int ntcoord = 0;
		char** ccmap = NULL;
		struct X3D_Shape* sn = (struct X3D_Shape*)tg->RenderFuncs.shapenode;
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node*, sn->geometry, gn);

		//step 1 get all the texcoords from geometry, and their ccmap names, and ntcoord count
		ntcoord = 1;
		if (gn && gn->_intern) {
			struct X3D_GeomRep* gr = (struct X3D_GeomRep*)gn->_intern;
			if (gr->itype == 2) {
				struct X3D_PolyRep* prep = (struct X3D_PolyRep*)gn->_intern;
				ccmap = prep->map;
				ntcoord = prep->ntcoord;
				//for (int i = 0; i < 4; i++)
				//	printf("gn.map[%d]=%p\n", i, (prep->map[i]);
			}
			else if (gr->itype == 3) {
				// gltf_loader puts geometry in BufferGeometry._intern = MeshRep
				struct X3D_MeshRep* mrep = (struct X3D_MeshRep*)gn->_intern;
				ntcoord = mrep->nuv;
				static char* mrallmap[] = { "0","1","2","3" }; //gltf_loader uses texcoord index as map string
				static char* nullmap[4] = { NULL,NULL,NULL,NULL };
				static char* usemap[4];
				memcpy(usemap, nullmap, 4 * sizeof(char*));
				ccmap = usemap;
				for (int i = 0; i < ntcoord; i++)
					ccmap[i] = mrallmap[i];
			}
		}
		//if(ccmap) for (int i = 0; i < 4; i++)
		//	printf("ccmap[%d]=%p\n", i, ccmap[i]);

		//step 2 get all the textrans, and their tmap names and ntrans count
		matprop = getAppearanceProperties();
		//unconditionally load any supplied texture transforms
		for (int i = 0; i < MAX_MULTITEXTURE; i++) {
			tmap[i] = NULL;
			igen[i] = TCGT_REGULAR;
		}
		struct X3D_Node* tt = getThis_textureTransform();
		int ntrans = 0;
		int ntrans_specified = 0;
		if (tt != NULL) {
			switch (tt->_nodeType) {
			case NODE_TextureTransform:
			case NODE_TextureTransform3D:
			case NODE_TextureTransformMatrix3D:
			case NODE_TextureTransformGenerator:
				ntrans = 1;
				break;
			case NODE_MultiTextureTransform:
				ntrans = ((struct X3D_MultiTextureTransform*)(tt))->textureTransform.n;
				break;
			default:
				ntrans = 0;
			}
			for (int i = 0; i < ntrans; i++) {
				FW_GL_PUSH_MATRIX(); //POPPED in textureTransform_end
				FW_GL_LOAD_IDENTITY();
				do_textureTransform0(tt, i, &tmap[i], &igen[i], & parameter_n, parameter);

			}
		}
		ntrans_specified = ntrans;
		int ngen = 0;
		for (int ig = 0; ig < ntrans; ig++) if (igen[ig] != TCGT_REGULAR) ngen++;
		//printf("ntrans %d ngen %d is_or_has_cubemap %d\n", ntrans, ngen, is_or_has_cubeMap(tnode));
		if (is_or_has_cubeMap(tnode) && !ngen) {
			FW_GL_PUSH_MATRIX(); //POPPED in textureTransform_end
			FW_GL_LOAD_IDENTITY();
			igen[ntrans] = TCGT_CAMERASPACEREFLECTIONVECTOR;
			ntrans++;
			//printf("adding igen for cubemap \n");
		}
		//add any computed 3D texture matrices
		if (is_or_has_Tex3D(tnode) && !ngen) {
			if (tg->RenderFuncs.shapenode) {
				//_if_ no TextureTransform3D was explicitly specified for Texture3D, 
				//_and_ no textureCoordinate3D or textureCoordinate4D was explicilty specified with the goem node
				//_then_ bounding box of shape, in local coordinates, is used to scale/translate
				//geometry vertices into 0-1 range on each axis for re-use as default texture3D coordinates
				float bbox[6], * bmin, * bmax;
				if (gn) {
					//first vec3 is minimum xyz
					bmin = bbox;
					bmax = &bbox[3];
					for (i = 0; i < 3; i++) {
						bmin[i] = gn->_extent[i * 2 + 1];
						bmax[i] = gn->_extent[i * 2];
					}
					//second vec3 is 1/size - so can be applied directly in vertex shader
					vecdif3f(bmax, bmax, bmin);
					for (i = 0; i < 3; i++) {
						if (bmax[i] != 0.0f)
							bmax[i] = 1.0f / bmax[i];
						else
							bmax[i] = 1.0f;
					}
					//if(fabs(bmin[0]) > 10.0f)
					//	printf("bbox shift [%f %f %f] scale [%f %f %f]\n",bmin[0],bmin[1],bmin[2],bmax[0],bmax[1],bmax[2]);

					//special default texture transform for 3D textures posing as 2D textures

					//the order of applying transform elements seems reversed for texture transforms
					//H: related to order of operands in mat * vec in shader:
					//   fw_TexCoord[0] = vec3(fw_TextureMatrix0 *vec4(texcoord,1.0)); 
					// but sign on elements is what you expect
					//flip z from RHS to LHS in fragment shader plug_tex3d apply
					//printf("default tt\n");
					FW_GL_PUSH_MATRIX(); //POPPED in textureTransform_end
					FW_GL_LOAD_IDENTITY();
					igen[ntrans] = TCGT_REGULAR;
					ntrans++;
					FW_GL_SCALE_F(bmax[0], bmax[1], bmax[2]);
					FW_GL_TRANSLATE_F(-bmin[0], -bmin[1], -bmin[2]);
				}
			}
		}
		glUniform1i(me->nTexMatrix, ntrans); //see also sendMatricesToShader and 	FW_GL_MATRIX_MODE(GL_TEXTURE); above
		for (int ig = 0; ig < MAX_MULTITEXTURE; ig++)
			glUniform1i(me->tgen[ig], igen[ig]);
		// ideally parameters would be 1:1 with texture stage / single texture. For now, allow one stage of multitexture to have refraction parameters.

		glUniform1i(me->parameter_n, parameter_n);
		for (int ig = 0; ig < parameter_n; ig++)
			glUniform1f(me->parameter[ig], parameter[ig]);
		//Step 3 go over appearance and/or material textures, and generate up to 1 combo (texcoord,textrans) for each
		//pair appearance.textures with texture coordinates
		int ntextures = tg->RenderFuncs.textureStackTop;
		// defaults for appearance.multitexture:
		ncombo = 0;
		if (tnode && X3D_APPEARANCE(sn->appearance)->texture == tnode) {
			//texture is coming from appearance.texture
			if (0) {
				for (int i = 0; i < tg->RenderFuncs.textureStackTop; i++) {
					int itrans = min(i, ntrans - 1);
					int icoord = min(i, ntcoord - 1);
					int jcombo = find_or_make_combo_by_index(itrans, icoord, &icombo[0], &ncombo);
					matprop->fw_FrontMaterial.cmap[i] = jcombo;
				}
			}
			else {
				struct X3D_Node** p;
				int n = 1;
				p = &tnode;
				if (tnode->_nodeType == NODE_MultiTexture) {
					struct X3D_MultiTexture* mnode = (struct X3D_MultiTexture*)tnode;
					p = mnode->texture.p;
					n = mnode->texture.n;
				}
				for (int i = 0; i < n; i++) {
					int jsamplr = is_cubeMap(p[i]) ? 1 : isTex3D(p[i]) ? 2 : 0;
					int itrans = i > ntrans_specified - 1 ? -1 : i; // min(i, ntrans - 1);
					if (itrans == -1) {
						itrans = next_or_last_sampler_compatible_trans(i-1, ntrans, jsamplr, igen);
					}
					int icoord = min(i, ntcoord - 1);  //next_or_last_sampler_compatible_coord(icoord, ntcoord, jsamplr);
					int jcombo = find_or_make_combo_by_index(itrans, icoord, &icombo[0], &ncombo);
					matprop->fw_FrontMaterial.cmap[i] = jcombo;
				}
			}
		}
		else {
			// by TextureCoordinate.mapping and material.xxxTextureMapping if they exist
			char** mmapf = matprop->fw_FrontMaterial.map;
			char** mmapb = matprop->fw_BackMaterial.map;
			for (int i = 0; i < 7; i++) {
				char scratch1[20], scratch2[20];
				char* trans_name;
				char* coord_name;
				int jcombo;
				// front material
				if (mmapf && mmapf[i]) {
					trans_name = trans_name_from_texture_mapping(mmapf[i], scratch1);
					coord_name = coord_name_from_texture_mapping(mmapf[i], scratch2);
					jcombo = find_or_make_combo(trans_name, coord_name, ccmap, ntcoord,
						tmap, ntrans, &icombo[0], &ncombo);
					matprop->fw_FrontMaterial.cmap[i] = jcombo;
				}
				//back material
				if (mmapb && mmapb[i]) {
					trans_name = trans_name_from_texture_mapping(mmapb[i], scratch1);
					coord_name = coord_name_from_texture_mapping(mmapb[i], scratch2);
					jcombo = find_or_make_combo(trans_name, coord_name, ccmap, ntcoord,
						tmap, ntrans, icombo, &ncombo);
					matprop->fw_BackMaterial.cmap[i] = jcombo;
				}
			}
		}
		for (int i = 0; i < ncombo; i++) {
			//printf("icmap[%d] = %d uniform %d\n", i, icombo[i][0], me->cmap[i]);
			//printf("itmap[%d] = %d uniform %d\n", i, icombo[i][1], me->tmap[i]);
			glUniform1i(me->cmap[i], icombo[i][0]);
			glUniform1i(me->tmap[i], icombo[i][1]);
		}
		glUniform1i(me->ntexcombo, ncombo);
		//printf("ncombo=%d\n", ncombo);

	}

	/* set up the selected shader for this texture(s) config */
	//PRINT_GL_ERROR_IF_ANY("TT_start_middle");

	if (me != NULL) {
		tnode = tg->RenderFuncs.texturenode;
		//printf ("passedInGenTex, we have tts %d tc %d\n",tg->RenderFuncs.textureStackTop, me->textureCount);

		if (me->textureCount != -1) {
			glUniform1i(me->textureCount, tg->RenderFuncs.textureStackTop);
		}
		if(tg->RenderFuncs.textureStackTop){
			if(isMultiTexture(tg->RenderFuncs.texturenode)){
				struct X3D_MultiTexture * mtnode = (struct X3D_MultiTexture *)tg->RenderFuncs.texturenode;
				glUniform4f(me->multitextureColor,mtnode->color.c[0],mtnode->color.c[1],mtnode->color.c[2],mtnode->alpha);
			}
		}
		if (tg->RenderFuncs.textureStackTop) {
			//June 2022 new approach: cubemaps are textures and should be treated as such
			{
				struct matpropstruct* myap = getAppearanceProperties();
				struct fw_MaterialParameters* mp;

				int textures[4], modes[4], sources[4], funcs[4], width[4], height[4], samplr[4];
				GLint saveTextureStackTop = tg->RenderFuncs.textureStackTop;
				int ntdesc = getTextureDescriptors(tnode, textures, modes, sources, funcs, width, height,samplr);
				// https://www.web3d.org/documents/specifications/19775-1/V4.0/Part01/components/shape.html#CoexistenceMaterialTexturesWithAppearanceTexture
				// material.maps: iuse [0] normal [1] emissive [2] occlusion [3] diffuse OR base [4] shininess OR metallicRoughness [5] specular [6] ambient 
				//material.type: 0 NONE 1 UNLIT 2 DEFUSE/SPECULAR 3 PHYSICAL/PBR
				mp = &myap->fw_FrontMaterial;
				int iuse = 3; // MAT_REGULAR;
				if (mp->type < 2) iuse = 1;       //if none or UnlitMaterial, put appearance textures in emissive iuse
				else if (mp->type == 2) iuse = 3; // if diffuse/specular Material put appearance textures in diffuse iuse
				else if (mp->type == 3) iuse = 3; // if PhysicalMaterial (PBR) put appearance textures in base iuse 
				int nt = mp->nt;  //assume appearance.texture has fwFrontMaterial all to itself, no material.texture to coordinte with
				mp->tcount[iuse] += ntdesc;
				mp->tstart[iuse] = nt;
				mp->cindex[iuse] = 0; //appearance.texture - cindex (coordinate index) 1:1 singletexture m:1 multitexture
					// material.texture - cindex 1:1 xxxTexture 1:1 xxxTexture.multitexture 1:m multitexture.singletexture
				for (int j = 0; j < ntdesc; j++) {
					int kunit, iunit;
					kunit = iunit = 0;
					mp->samplr[nt] = samplr[j];
					if (mp->samplr[nt] == 1) {
						if (0) {
							GLenum target;
							printf("%s ", stringNodeType(tnode->_nodeType));
							glGetTextureParameteriv(textures[j], GL_TEXTURE_TARGET, (GLint*)&target);
							switch (target) {
							case GL_TEXTURE_CUBE_MAP: printf("CUBE MAP \n"); break;
							case GL_TEXTURE_2D: printf("texture2D\n"); break;
							case GL_TEXTURE_3D: printf("texture3D\n"); break;
							case GL_TEXTURE_2D_ARRAY: printf("GL_TEXTURE_2D_ARRAY\n");
							default: printf("unknown %d \n",target); break;
							}
						}
						PRINT_GL_ERROR_IF_ANY("TT_start_ bfor bind cube");

						kunit = share_or_next_material_sampler_index_Cube(textures[j]);//returns index into shader samplerCube texterUnitCube[kunit]
						//kunit = share_or_next_material_sampler_index_Cube(getCheckerboardTextureCube());//returns index into shader samplerCube texterUnitCube[kunit]
						PRINT_GL_ERROR_IF_ANY("TT_start_ aftr bind cube");
						glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
						PRINT_GL_ERROR_IF_ANY("TT_start_ aftr seamless");

					}
					else {
						kunit = share_or_next_material_sampler_index_2D(textures[j]);//returns index into shader sampler2D texterUnit[kunit]
					}
					mp->tindex[nt] = kunit;
					mp->source[nt] = sources[j];
					mp->mode[nt] = modes[j];
					mp->func[nt] = funcs[j];
					//if(oldway) 
					//	mp->cmap[nt] = immap[j];
					//mp->cmap[nt] = icombo[j][0]; // immap[j]; //assigned above? or is this different?
					glUniform1i(me->myMaterialCmap[nt], mp->cmap[nt]); //would be mp->cmap[iuse] in material?
					if (mp->samplr[nt] == 1) {
						iunit = tunitCube(kunit);//returns i as in GL_TEXTUREi, to be stored in samplerCube textureUnitCube[kunit]
						glUniform1i(me->textureUnitCube[kunit], iunit);
					}
					else {
						iunit = tunit2D(kunit);//returns i as in GL_TEXTUREi, to be stored in sampler2D textureUnit[kunit]
						glUniform1i(me->textureUnit[kunit], iunit);
					}
					mp->binding[nt] = iunit; //for debugging around here (already sent to shader)
					//printf("textureUnit(Cube)[%d]=%d ogl %d samplr %d\n", kunit, iunit,textures[j], samplr[j]);

					glUniform1i(me->myMaterialTindex[nt], mp->tindex[nt]);
					glUniform1i(me->myMaterialMode[nt], mp->mode[nt]);
					glUniform1i(me->myMaterialSource[nt], mp->source[nt]);
					glUniform1i(me->myMaterialFunc[nt], mp->func[nt]);
					glUniform1i(me->myMaterialSampler[nt], mp->samplr[nt]);
					nt++;
				}
				mp->nt = nt;
				//something about the multitexture..
				//printf("sampler type samplr[%d]=%d start[%d]=%d ", iuse, mp->samplr[iuse], iuse, mp->tstart[iuse]); //samplr 0=2D 1=cube
				//something about the first sub-texture in the multitexture..
				//printf("textureUnit(Cube)[%d]=%d\n", mp->tindex[mp->tstart[iuse]], mp->binding[mp->tstart[iuse]] );
				GLUNIFORM1I(me->myMaterialCindex[iuse], mp->cindex[iuse]);
				GLUNIFORM1I(me->myMaterialTcount[iuse], mp->tcount[iuse]);
				GLUNIFORM1I(me->myMaterialTstart[iuse], mp->tstart[iuse]);

				tg->RenderFuncs.textureStackTop = saveTextureStackTop; //keep this frmo building up
			}
		}
	#ifdef TEXVERBOSE
	} else {
		printf (" NOT sending in %d i+tu+mode because currentShaderProperties is NULL\n",tg->RenderFuncs.textureStackTop);
	#endif
	}

	FW_GL_MATRIX_MODE(GL_MODELVIEW);

	PRINT_GL_ERROR_IF_ANY("TT_start_finish");
}

void textureCoord_send(struct textureVertexInfo *genTex) {
	// Oct 2016 refactoring before particleSystem
	// moved texturetransform stuff out of here and into (below) textureTransform_start()
	int c;
	//int isIdentity; //isMulti, isStrict,i,  
	//GLint texUnit[MAX_MULTITEXTURE];
	//GLint texMode[MAX_MULTITEXTURE];
	s_shader_capabilities_t *me;
	struct textureVertexInfo *genTexPtr;
	// OLDCODE struct X3D_Node *tnode;

	// OLDCODE ppRenderTextures p;
	ttglobal tg = gglobal();
	// OLDCODE p = (ppRenderTextures)tg->RenderTextures.prv;
	// OLDCODE tnode = tg->RenderFuncs.texturenode;

    me = getAppearanceProperties()->currentShaderProperties;

	genTexPtr = genTex;
	//for (c=0; c < tg->RenderFuncs.textureStackTop; c++) {
	c = 0;
	while(genTexPtr){ //PBR: send all you got and say how many (channels)
		if(genTexPtr->VBO)
			FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,genTexPtr->VBO);

		if (genTexPtr->pre_canned_textureCoords != NULL) {
			/* simple shapes, like Boxes and Cones and Spheres will have pre-canned arrays */
			FW_GL_TEXCOORD_POINTER (2,GL_FLOAT,0,genTexPtr->pre_canned_textureCoords,c);
		}else{
			FW_GL_TEXCOORD_POINTER (genTexPtr->TC_size, 
				genTexPtr->TC_type,
				genTexPtr->TC_stride,
				genTexPtr->TC_pointer,c);
		}
		//genTexPtr = genTexPtr->next ? genTexPtr->next : genTexPtr; //duplicate the prior coords if not enough for all MultiTextures
		genTexPtr = genTexPtr->next; //PBR: send all you got, and say how many (channels)
		c++;
	}
	glUniform1i(me->nTexCoordChannels,c);  //PBR: send all you got, and say how many (channels)
	//printf("nTexCoordChannels = %d uniform= %d\n", c, me->nTexCoordChannels);
	glUniform1i(me->flipuv, 0);
}
