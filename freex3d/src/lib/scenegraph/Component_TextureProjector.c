/*
=INSERT_TEMPLATE_HERE=

$Id: Component_Grouping.c,v 1.52 2013/08/01 12:55:35 crc_canada Exp $

X3D Grouping Component

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
#include "../vrml_parser/CRoutes.h"
#include "../main/headers.h"

#include "../opengl/OpenGL_Utils.h"
#include "../opengl/Textures.h"
#include "../opengl/Frustum.h"
#include "../opengl/Material.h"
#include "Renderfuncs.h"
#include "Component_Shape.h"
#include "LinearAlgebra.h"
#include "Vector.h"
#include "Children.h"
#include <stdlib.h>


//we'll share VF_globalLight render pass with global lights
// this filter will allow global=true on the VF_globalLight render_hier pass
// and wll allow global=false on regular pass
#define RETURN_IF_RENDER_STATE_NOT_US \
		if (renderstate()->render_light== VF_globalLight) { \
			if (!node->global) return;\
		} else { \
			if (node->global) return; \
			if(renderstate()->render_geom != VF_Geom) return; \
		}



// camera space (-1,-1) to (1,1)
// texture space (0,0) to (1,1)
// convert: scale by .5, and add .5
const GLDOUBLE bias[16] = { 0.5, 0.0, 0.0, 0.0,
0.0, 0.5, 0.0, 0.0,
0.0, 0.0, 0.5, 0.0,
0.5, 0.5, 0.5, 1.0 };


typedef struct pComponent_TextureProjector{
	struct Vector *projector_stack; //activeProjectiveTextureTable;
	//textureTableIndexStruct_s* loadThisProjectiveTexture;

	/* current index into loadparams that texture thread is working on */
	int currentlyWorkingOn;// = -1;
	int textureInProcess;// = -1;
}* ppComponent_TextureProjector;

void *Component_TextureProjector_constructor(){
	void *v = malloc(sizeof(struct pComponent_TextureProjector));
	memset(v,0,sizeof(struct pComponent_TextureProjector));
	return v;
}
void Component_TextureProjector_init(struct tComponent_TextureProjector *t){
	//public

	//private 
	
	t->prv = Component_TextureProjector_constructor();
	{
		ppComponent_TextureProjector p = (ppComponent_TextureProjector)t->prv;
		//p->activeProjectiveTextureTable = NULL;
		p->projector_stack = newStack(usehit);

		/* current index into loadparams that texture thread is working on */
		p->currentlyWorkingOn = -1;

		p->textureInProcess = -1;
	}
}

void Component_TextureProjector_clear(struct tComponent_TextureProjector *t){
	//public
	//private
	{
		ppComponent_TextureProjector p = (ppComponent_TextureProjector)t->prv;
	}
}

struct X3D_ProjectorRep {
	int itype; //=5, 0 PointRep 1 LineRep 2 PolyRep 3 MeshRep 4 TextureRep 5 LightRep 6 ProjectorRep
	//depth section
	Stack* depth_buffer_stack;
	int size;
	double matproj[16];
	double matview[16];
	//projector section
	struct X3D_Node* texture;
	int itexture;
};

void* set_ProjectorRep(void* _projectorrep)
{
	struct X3D_ProjectorRep* projectorrep = _projectorrep;
	if (!_projectorrep) {
		_projectorrep = MALLOC(struct X3D_ProjectorRep*, sizeof(struct X3D_ProjectorRep));
		memset(_projectorrep, 0, sizeof(struct X3D_ProjectorRep));
		projectorrep = (struct X3D_ProjectorRep*)_projectorrep;
		projectorrep->itype = 6;
		projectorrep->size = 1024; //size of shadow image, or for pointlight, size of each of 6 sides of cubemap
	}
	return projectorrep;
}




//void shadowTable_clear();
//void shadowTable_push(usehit ptuple);
//void shadowTable_pop();

void projectorTable_clear(){
	//called once per frame, before the search for global=true projectors
	//will clear any global=true projectors from last frame
	ppComponent_TextureProjector p;
	ttglobal tg = gglobal();
	p = (ppComponent_TextureProjector)tg->Component_TextureProjector.prv;
	clearStack(p->projector_stack);
}
void projectorTable_push(usehit ptuple ){
	//called when we find a global=true, on=true projector, and
	//called in sib_prep for a global=false, on=false projector
	ppComponent_TextureProjector p;
	ttglobal tg = gglobal();
	p = (ppComponent_TextureProjector)tg->Component_TextureProjector.prv;
	//we need a deep copy because the ptm node can't hold it
	// because it can be DEF/USED with different transform each use
	stack_push(usehit,p->projector_stack,ptuple);

}
void projectorTable_pop(){
	//called in sib_fin for a global=false, on=true projector
	ppComponent_TextureProjector p;
	ttglobal tg = gglobal();
	p = (ppComponent_TextureProjector)tg->Component_TextureProjector.prv;
	if(p->projector_stack->n < 1)
		printf("ouch from projectorTable_opo()\n");
	stack_pop(usehit,p->projector_stack);

}
int projectorTable_count() {
	ppComponent_TextureProjector p;
	ttglobal tg = gglobal();
	p = (ppComponent_TextureProjector)tg->Component_TextureProjector.prv;
	return p->projector_stack->n;
}
usehit* projectorTable_item(int i) {
	ppComponent_TextureProjector p;
	ttglobal tg = gglobal();
	p = (ppComponent_TextureProjector)tg->Component_TextureProjector.prv;
	return vector_get_ptr(usehit, p->projector_stack, i);
}
int projectorTable_node_use_count(struct X3D_Node* node) {
	int count = 0;
	for (int i = 0; i < projectorTable_count(); i++) {
		if (projectorTable_item(i)->node == node) count++;
	}
	return count;
}

void clear_bound_textures(){
	for(int i=0;i<16;i++){
		glActiveTexture(GL_TEXTURE0 + i); 
		glBindTexture(GL_TEXTURE_2D,0); 
	}
}

void print_bound_textures(char *str){
	GLint whichID;
	printf("ActiveTexture boundUnit %s\n",str);
	for(int i=0;i<16;i++){
		glActiveTexture(GL_TEXTURE0 + i); 
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &whichID); 
		printf("%12d  %12d\n",i,whichID);
	}
}

int old_waay = 0;

int get_bound_image(struct X3D_Node *node);
int getGlTextureNumberFromTextureNode(struct X3D_Node *textureNode);
int getTextureSizeFromTextureNode(struct X3D_Node *textureNode, int *ixyz);
int getTextureDescriptors(struct X3D_Node *textureNode, int *textures, int *modes, int *sources, int *funcs, int *width, int *height, int *samplr);
void PRINT_GL_ERROR(GLenum _global_gl_err);
void sendProjectorInfo()
{
	//called from render_shape to refresh uniform before shade draw
	int pcount,tcount;
	s_shader_capabilities_t *me;
	ppComponent_TextureProjector p;
	ttglobal tg = gglobal();
	p = (ppComponent_TextureProjector)tg->Component_TextureProjector.prv;

    me = getAppearanceProperties()->currentShaderProperties;
	GLuint myProg = me->myShaderProgram;

	// while the number of texture samplers are a limited resource in GLSL,
	// there could be many projectors re-using the same sampler.
	// to accommodate multitexture per projector, and avoid [][] 2 dimenstional arrays in GLSL
	// we have a single list of texture descriptors, and a tcounts[] that says how many texture descriptors per projector
	// per child_shape shader run:
	//   pcount; //number of projectors, <= MAX_PROJ
	// per sampler2D: 
	//   textureUnit[MAX_TEX]
	// per projector:
	//   projTexGenMatCam[MAX_PROJ]
	//   backCull[MAX_PROJ]
	//   ntdesc[MAX_PROJ]
	// per texture descriptor
	//.  tunits[MAX_TDESC]  //indexes into textureUnit[] array
	//   modes[MAX_TDESC]
	//   sources[MAX_TDESC]
	//   funcs[MAX_TDESC]
	GLint saveTextureStackTop = tg->RenderFuncs.textureStackTop;
	PRINT_GL_ERROR_IF_ANY("BEGIN resend_textureprojector_matrix");

	for (int i = 0; i < 8; i++) {
		//per projector
		char line[24];
		sprintf(line, "ptms[%d].GenMatCam", i);
		me->ptmGenMatCam[i] = GET_UNIFORM(myProg, line); //"projTexGenMatCam0"); //vertex shader matrix for projecting rays back to texture
		sprintf(line, "ptms[%d].backCull", i);
		me->ptmbackCull[i] = GET_UNIFORM(myProg, line);
		sprintf(line, "ptms[%d].color", i);
		me->ptmcolor[i] = GET_UNIFORM(myProg, line);
		sprintf(line, "ptms[%d].intensity", i);
		me->ptmintensity[i] = GET_UNIFORM(myProg, line);
		sprintf(line, "ptms[%d].shadows", i);
		me->ptmshadows[i] = GET_UNIFORM(myProg, line);
		sprintf(line, "ptms[%d].shadowIntensity", i);
		me->ptmshadowIntensity[i] = GET_UNIFORM(myProg, line);
		sprintf(line, "ptms[%d].depthmap", i);
		me->ptmdepthmap[i] = GET_UNIFORM(myProg, line);
		sprintf(line, "ptms[%d].tcount", i);
		me->ptmtcount[i] = GET_UNIFORM(myProg, line);
		sprintf(line, "ptms[%d].tstart", i);
		me->ptmtstart[i] = GET_UNIFORM(myProg, line);
		sprintf(line, "ptms[%d].type", i);
		me->ptmtype[i] = GET_UNIFORM(myProg, line);
	}
	for (int i = 0; i < 16; i++) {
		//per texture descriptor
		char line[24];
		sprintf(line, "tdescs[%d].tindex", i);
		me->tdtindex[i] = GET_UNIFORM(myProg, line);
		sprintf(line, "tdescs[%d].mode", i);
		me->tdmode[i] = GET_UNIFORM(myProg, line);
		sprintf(line, "tdescs[%d].source", i);
		me->tdsource[i] = GET_UNIFORM(myProg, line);
		sprintf(line, "tdescs[%d].func", i);
		me->tdfunc[i] = GET_UNIFORM(myProg, line);
		sprintf(line, "tdescs[%d].samplr", i);
		me->tdsamplr[i] = GET_UNIFORM(myProg, line);
	}
	me->ptmCount = GET_UNIFORM(myProg, "ptmCount");
	PRINT_GL_ERROR_IF_ANY("EARLY resend_textureprojector_matrix");

	int MAX_PROJ = 8;
	int MAX_TDESC = 16;
	int MAX_TEX = 4;
	int projcount = min(projectorTable_count(), MAX_PROJ);

	pcount = 0;

	int kdesc = 0;
	for(int j=0;j<projcount;j++)
	{
		float TenLinearGexMatCam0f[16];
		usehit *ptuple;
		GLint texture;
		usehit* uhit = projectorTable_item(j);
		struct X3D_Node* node = uhit->node;
		struct X3D_TextureProjector* ptm = X3D_TEXTUREPROJECTOR(node);
		struct X3D_TextureProjectorParallel* ppar = X3D_TEXTUREPROJECTORPARALLEL(node);
		struct X3D_TextureProjectorPoint* ppoint = X3D_TEXTUREPROJECTORPOINT(node);
		struct X3D_ProjectorRep* projrep = (struct X3D_ProjectorRep*)node->_intern;
		int projType = 0;
		//0 - projector
		//1 - projectorparallel
		//2 - projectorpoint
		switch (node->_nodeType) {
			case NODE_TextureProjector: projType = 0; break;
			case NODE_TextureProjectorParallel: projType = 1; break;
			case NODE_TextureProjectorPoint: projType = 2; break;
			default: break;
		}

		{
			if (old_waay) {
				double matfull[16];
				matmultiplyFULL(matfull, uhit->mvm, projrep->matproj);
				double2float(TenLinearGexMatCam0f, matfull, 16);
				GLUNIFORMMATRIX4FV(me->ptmGenMatCam[j], 1, GL_FALSE, TenLinearGexMatCam0f);
			}
			else {
				float w2l[16];
				{
					//following textureProjector
					double modelviewinv[16], eye2projector[16], matfull[16], mvm[16];
					matcopy(mvm, uhit->mvm);

					matinverse(modelviewinv, mvm);
					matmultiplyAFFINE(eye2projector, modelviewinv, projrep->matview);
					matmultiplyFULL(matfull, eye2projector, projrep->matproj);
					double2float(w2l, matfull, 16);
				}
				//printf("w2l\n");
				//for (int ii = 0; ii < 4; ii++){
				//	for (int jj = 0; jj < 4; jj++) printf("%f ", w2l[ii * 4 + jj]);
				//	printf("\n");
				//}
				GLUNIFORMMATRIX4FV(me->ptmGenMatCam[j], 1, GL_FALSE, w2l);

			}
			//backCull in theory could automatically always do it, 
			// or projector->backCull=TRUE default, 
			// and turn off when Gl_CULL_FACE is off, meaning web3d solid=FALSE
			// X HOWEVER freewrl Feb 2020 isn't reliably discriminating solid=true/false for different geometry types
			// - THEREFORE we will let projector->backCull be definitive and scene authors will set manually until freewrl solid is fixed
			GLUNIFORM1I(me->ptmbackCull[j],ptm->backCull);
			GLUNIFORM3FV(me->ptmcolor[j], 1, ptm->color.c);
			GLUNIFORM1F(me->ptmintensity[j], ptm->intensity);
			GLUNIFORM1I(me->ptmshadows[j], ptm->shadows);
			GLUNIFORM1F(me->ptmshadowIntensity[j], ptm->shadowIntensity);
			GLUNIFORM1I(me->ptmtype[j], projType);

			int ntdesc = 0; //number of texture descriptors in this projector
			struct X3D_NODE * tlist[4];
			int modes[4];
			int sources[4];
			int funcs[4];
			int textures[4];
			int width[4], height[4], samplr[4];

			render_node(projrep->texture);
			PRINT_GL_ERROR_IF_ANY("MIDDLE resend_textureprojector_matrix");

			ntdesc = getTextureDescriptors(projrep->texture,textures, modes,sources, funcs, width, height, samplr);
			GLUNIFORM1I(me->ptmtcount[j],ntdesc);
			PRINT_GL_ERROR_IF_ANY("M1 resend_textureprojector_matrix");
			GLUNIFORM1I(me->ptmtstart[j], kdesc);
			GLenum _global_gl_err = glGetError(); 
			while (_global_gl_err != GL_NONE) {
				PRINT_GL_ERROR(_global_gl_err);
				printf(" here: %s (%s:%d)\n", "resend_textureprojector_matrix", __FILE__, __LINE__);
				_global_gl_err = glGetError();
			}

			PRINT_GL_ERROR_IF_ANY("M2 resend_textureprojector_matrix");

			for(int i=0;i<ntdesc;i++,kdesc++){
				// re-use texture sampler if mulitple projectors and multitextures refer to same GLint texture 1:1 sampler2D
				int kunit, iunit;
				if (samplr[i] == 1) {
					if (0) {
						GLenum target;
						printf("%s ", stringNodeType(node->_nodeType));
						glGetTextureParameteriv(textures[i], GL_TEXTURE_TARGET, (GLint*)&target);
						switch (target) {
						case GL_TEXTURE_CUBE_MAP: printf("CUBE MAP \n"); break;
						case GL_TEXTURE_2D: printf("texture2D\n"); break;
						case GL_TEXTURE_3D: printf("texture3D\n"); break;
						case GL_TEXTURE_2D_ARRAY: printf("GL_TEXTURE_2D_ARRAY\n");
						default: printf("unknown %d \n", target); break;
						}
					}
					PRINT_GL_ERROR_IF_ANY("TT_start_ bfor bind cube");

					kunit = share_or_next_material_sampler_index_Cube(textures[i]);//returns index into shader samplerCube texterUnitCube[kunit]
					//kunit = share_or_next_material_sampler_index_Cube(getCheckerboardTextureCube());//returns index into shader samplerCube texterUnitCube[kunit]
					PRINT_GL_ERROR_IF_ANY("TT_start_ aftr bind cube");
					glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
					PRINT_GL_ERROR_IF_ANY("TT_start_ aftr seamless");
					iunit = tunitCube(kunit);//returns i as in GL_TEXTUREi, to be stored in samplerCube textureUnitCube[kunit]
					glUniform1i(me->textureUnitCube[kunit], iunit);
				}
				else {
					kunit = share_or_next_material_sampler_index_2D(textures[i]);//returns index into shader sampler2D texterUnit[kunit]
					iunit = tunit2D(kunit);//returns i as in GL_TEXTUREi, to be stored in sampler2D textureUnit[kunit]
					glUniform1i(me->textureUnit[kunit], iunit);
				}

				GLUNIFORM1I(me->tdtindex[kdesc],kunit); //tunits like PBR tindex - an array saying which sampler2D textureUnit[tunit[kdesc]]
				PRINT_GL_ERROR_IF_ANY("M4 resend_textureprojector_matrix");

				GLUNIFORM1I(me->tdmode[kdesc],modes[i]);
				GLUNIFORM1I(me->tdsource[kdesc],sources[i]);
				GLUNIFORM1I(me->tdfunc[kdesc],funcs[i]);
				GLUNIFORM1I(me->tdsamplr[kdesc], samplr[i]);
				PRINT_GL_ERROR_IF_ANY("M6 resend_textureprojector_matrix");

			}
			PRINT_GL_ERROR_IF_ANY("LATE resend_textureprojector_matrix");

			if (ptm->shadows) {
				struct X3D_Node* texnode = (struct X3D_Node*)vector_get(struct X3D_Node*, projrep->depth_buffer_stack, uhit->ivalue);
				int itexunit, iunit;
				textureTableIndexStruct_s* tti;
				if (texnode->_nodeType == NODE_GeneratedCubeMapTexture) {
					struct X3D_GeneratedCubeMapTexture* tex = (struct X3D_GeneratedCubeMapTexture*)texnode;
					tti = getTableIndex(tex->__textureTableIndex);
					PRINT_GL_ERROR_IF_ANY("sendLightInfo before bind_or_share");
					//				glEnable(GL_TEXTURE_CUBE_MAP);
					if (0) {
						GLuint target;
						glGetTextureParameteriv(tti->OpenGLTexture, GL_TEXTURE_TARGET, (GLint*)&target);
						switch (target) {
						case GL_TEXTURE_CUBE_MAP: printf("CUBE MAP \n"); break;
						case GL_TEXTURE_2D: printf("texture2D\n"); break;
						case GL_TEXTURE_3D: printf("texture3D\n"); break;
						case GL_TEXTURE_2D_ARRAY: printf("GL_TEXTURE_2D_ARRAY\n");
						default: printf("unknown %d \n", target); break;
						}

					}
					itexunit = share_or_next_material_sampler_index_Cube(tti->OpenGLTexture); // returns i as in GL_TEXTUREi, next available
					iunit = tunitCube(itexunit); //returns index into shader samplerCube textureUnitCube[iunit]
					PRINT_GL_ERROR_IF_ANY("sendProjectorInfo after bind_or_share");
					glUniform1i(me->textureUnitCube[iunit], itexunit); // iunit);
				}
				else {
					struct X3D_PixelTexture* tex = (struct X3D_PixelTexture*)texnode;
					tti = getTableIndex(tex->__textureTableIndex);
					PRINT_GL_ERROR_IF_ANY("sendProjectorInfo before bind_or_share");
					itexunit = share_or_next_material_sampler_index_2D(tti->OpenGLTexture); // returns i as in GL_TEXTUREi, next available
					iunit = tunit2D(itexunit); //returns index into shader sampler2D textureUnit[iunit]
					PRINT_GL_ERROR_IF_ANY("sendProjectorInfo after bind_or_share");
					glUniform1i(me->textureUnit[iunit], itexunit);
				}
				glUniform1i(me->ptmdepthmap[j], iunit);
				// use the same transforms for depth as for diffuse 
				
				//float w2l[16];
				//{
				//	//following textureProjector
				//	double modelviewinv[16], eye2projector[16], matfull[16], mvm[16];
				//	if (uhit->node->_nodeType == NODE_DirectionalLight)
				//		matmultiplyAFFINE(mvm, uhit->extra, uhit->mvm);
				//	else
				//		matcopy(mvm, uhit->mvm);

				//	matinverse(modelviewinv, mvm);
				//	matmultiplyAFFINE(eye2projector, modelviewinv, projrep->matview);
				//	matmultiplyFULL(matfull, eye2projector, projrep->matproj);
				//	double2float(w2l, matfull, 16);
				//}
				////printf("w2l\n");
				////for (int ii = 0; ii < 4; ii++){
				////	for (int jj = 0; jj < 4; jj++) printf("%f ", w2l[ii * 4 + jj]);
				////	printf("\n");
				////}
				//GLUNIFORMMATRIX4FV(me->lightMat[j], 1, GL_FALSE, w2l);
			}

			pcount++;
			tg->RenderFuncs.textureStackTop = saveTextureStackTop; //keep this frmo building up
		}
	}
	GLUNIFORM1I(me->ptmCount,pcount);
	PRINT_GL_ERROR_IF_ANY("END resend_textureprojector_matrix");
}

void generate_shadowmap_cube(usehit uhit, int index);
void generate_shadowmap_2D(usehit uhit, int index);


struct X3D_Node* make_depth_buffer_cube(int width, int height);
struct X3D_Node* make_depth_buffer(int width, int height);

int make_or_get_depth_buffer_projector(int index, struct X3D_Node* node) {
	node->_intern = set_ProjectorRep(node->_intern);
	struct X3D_ProjectorRep* projrep = (struct X3D_ProjectorRep*)node->_intern;
	if (!projrep->depth_buffer_stack) {
		projrep->depth_buffer_stack = newStack(struct X3D_Node*);
	}
	if (index > -1 && index < vectorSize(projrep->depth_buffer_stack)) return index;
	struct X3D_Node* depth_buffer_texture = NULL;
	if (node->_nodeType == NODE_TextureProjectorPoint)
		depth_buffer_texture = make_depth_buffer_cube(projrep->size, projrep->size);
	else
		depth_buffer_texture = make_depth_buffer(projrep->size, projrep->size);
	stack_push(struct X3D_Node*, projrep->depth_buffer_stack, X3D_NODE(depth_buffer_texture));
	return vectorSize(projrep->depth_buffer_stack) - 1;
}

/* Projective Texture gluLookAt */
void projLookAt(GLDOUBLE eyex, GLDOUBLE eyey, GLDOUBLE eyez,
				GLDOUBLE centerx, GLDOUBLE centery, GLDOUBLE centerz,
				GLDOUBLE upx, GLDOUBLE upy, GLDOUBLE upz, GLDOUBLE *matrix);
void projPerspective(GLDOUBLE fovy, GLDOUBLE aspect, GLDOUBLE zNear, GLDOUBLE zFar, GLDOUBLE *matrix);
void printmatrix2(GLDOUBLE* mat,char* description );

void compile_TextureProjector(struct X3D_TextureProjector* node) {

	node->_intern = set_ProjectorRep(node->_intern);
	/* LookAt Matrix Complete */
	float dir[3], up[3], cross1[3], cross2[3];
	veccopy3f(node->_loc.c, node->location.c);
	veccopy3f(dir, node->direction.c);
	veccopy3f(up, node->upVector.c);
	vecnormalize3f(dir, dir);
	vecnormalize3f(up, up);
	veccross3f(cross1, dir, up);
	vecnormalize3f(cross1, cross1);
	veccross3f(cross2, cross1, dir);
	vecnormalize3f(cross2, cross2);
	veccopy3f(node->_dir.c, dir);
	node->_dir.c[3] = 0.0f;
	veccopy3f(node->_upVec.c, up);
	node->_upVec.c[3] = 0.0f;
	//	if (node->shadows) compile_shadowMap(X3D_NODE(node));

	if (node->shadows)
		set_debug_quad_near_farplane(node->nearDistance, node->farDistance);

	MARK_NODE_COMPILED;
}

void render_TextureProjector0(struct X3D_Node* parent, struct X3D_TextureProjector *node) {
	int i,j = 0;
	int flag = 0;
	float degree = node->fieldOfView* 180.0/3.141596;
	GLDOUBLE cViewMat[16];
	GLDOUBLE invcViewMat[16];
	GLDOUBLE ViewMat[16];
	GLDOUBLE ProjMat[16];
	GLint tex1;
	ppComponent_TextureProjector p;
	ttglobal tg = gglobal();
	p = (ppComponent_TextureProjector)tg->Component_TextureProjector.prv;
	
	RETURN_IF_RENDER_STATE_NOT_US
	COMPILE_IF_REQUIRED;

	if(node->on) {
		double tempmat[16];
		//GLDOUBLE TenLinearGexMatCam0[16];
		GLDOUBLE modelview[16], modelviewnode[16], eye2projector[16], modelviewinv[16];
		struct X3D_Node *tmpN = NULL;
		struct X3D_ProjectorRep* projrep = (struct X3D_ProjectorRep*)node->_intern;

		if(node->global) tg->Component_TextureProjector.globalProjector = TRUE;

		//A. COMPUTE NODE-POSE MATRIX FOR: .position, .dir, .upVector
		//glMatrixMode(GL_MODELVIEW);
		FW_GL_MATRIX_MODE(GL_MODELVIEW);
		FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelview);

		{
			double loc[3],dir[3],up[3],eye[3];
			float2double(loc,node->_loc.c,3);
			float2double(dir,node->_dir.c,3);
			float2double(up,node->_upVec.c,3);
			vecdifd(eye,loc,dir);
			projLookAt(eye[0],eye[1],eye[2], loc[0],loc[1],loc[2], up[0],up[1],up[2],ViewMat);
		}
		matcopy(projrep->matview, ViewMat);
		//B. INVERT modelviewnode (which transforms projector to eye) to get eye-to-projector
		matinverse(modelviewinv,modelview);
		//C. COMBINE MODELVIEW MATRIX WITH NODE-POSE MATRIX
		matmultiplyAFFINE(eye2projector,modelviewinv,ViewMat);

		//C. COMPUTE A PROJECTION MATRIX THAT INCLUDES CAMERA SPACE TO TEXTURE SPACE BIAS
		projPerspective((GLDOUBLE)degree,
			(GLDOUBLE)node->aspectRatio, // aspectRatio = width/height see below, gets from image
			(GLDOUBLE)node->nearDistance,(GLDOUBLE)node->farDistance, // near, far
			ProjMat);
		matcopy(projrep->matproj, ProjMat);

		//matidentity4d(tempmat);
//		matmultiplyFULL(tempmat,bias,tempmat);
		//matmultiplyFULL(tempmat,ProjMat,tempmat);

		//D. COMBINE PROJECTION AND EYE-TO-PROJECTOR TRANSFORMS
		//matmultiplyFULL(projrep->matmodelviewproj,eye2projector,tempmat);
		//matmultiplyFULL(projrep->matmodelviewproj, eye2projector, ProjMat);

	
		if(node->texture)
		{
			POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->texture,tmpN);
			if(tmpN){
				int ixyz[3];
				float aspectRatio;
				if(getTextureSizeFromTextureNode(tmpN, ixyz)){
					if(ixyz[0] > 0 && ixyz[1] > 0){
						aspectRatio = (float)ixyz[0]/(float)ixyz[1];
						if(!APPROX(node->aspectRatio,aspectRatio)){
							node->aspectRatio = aspectRatio;
							MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_TextureProjector, aspectRatio));
							//printf("aspectRatio= %f\n",node->aspectRatio);
						}
					}
				}
			}


		}
		{
			GLuint texture;
			usehit ptuple;

			ptuple.node = X3D_NODE(node);
			ptuple.userdata = parent;
			if(old_waay)
				matcopy(ptuple.mvm, eye2projector); 
			else
				matcopy(ptuple.mvm, modelview); //concatonate in sendProjectorInfo
			//ptuple.userdata = projrep->matproj;
			//matcopy(ptuple.proj, projrep->matproj);
			texture = tg->RenderFuncs.boundTextureStack[tg->RenderFuncs.textureStackTop];
			projrep->itexture = texture;
			projrep->texture = tmpN;
			//if (node->global && node->shadows) {
			if (node->shadows) {
				int nuse = projectorTable_node_use_count(X3D_NODE(node));
				ptuple.ivalue = make_or_get_depth_buffer(nuse, X3D_NODE(node));
				generate_shadowmap_2D(ptuple, 0);
			}
			projectorTable_push(ptuple);


		}

	} //if(node->on)
 }
 void render_TextureProjector(struct X3D_TextureProjector* node) {
	 render_TextureProjector0(NULL, node);
 }

void fin_TextureProjector (struct X3D_TextureProjector *node) 
{
	RETURN_IF_RENDER_STATE_NOT_US
	if(node->on)
		if(!node->global)
			projectorTable_pop(); //just pop local projectors that we pushed above - globals are cleared once per frame
}

void prep_TextureProjector(struct X3D_TextureProjector *node) {


	if (!renderstate()->render_light) return;
	/* this will be a global textureprojector here... */
	render_TextureProjector(node);

}
void child_TextureProjector (struct X3D_TextureProjector *node) {
}


/////////////////////////////////////////////////////////////////////////////////////////////



void child_TextureProjectorParallel (struct X3D_TextureProjectorParallel *node) {
}
void fin_TextureProjectorParallel (struct X3D_TextureProjectorParallel *node) {

	RETURN_IF_RENDER_STATE_NOT_US
	if(node->on)
		if(!node->global)
			projectorTable_pop(); //just pop local projectors that we pushed above - globals are cleared once per frame

}

void compile_TextureProjectorParallel (struct X3D_TextureProjectorParallel *node) { 
	node->_intern = set_ProjectorRep(node->_intern);

	/* LookAt Matrix Complete */
	float dir[3], up[3], cross1[3],cross2[3];
	veccopy3f(node->_loc.c,node->location.c);
	veccopy3f(dir,node->direction.c);
	veccopy3f(up,node->upVector.c);
	vecnormalize3f(dir,dir);
	vecnormalize3f(up,up);
	veccross3f(cross1,dir,up);
	vecnormalize3f(cross1,cross1);
	veccross3f(cross2,cross1,dir);
	vecnormalize3f(cross2,cross2);
	veccopy3f(node->_dir.c,dir);
	node->_dir.c[3] = 0.0f;
	veccopy3f(node->_upVec.c,up);
	node->_upVec.c[3] = 0.0f;

	MARK_NODE_COMPILED;

}

void projOrtho (GLDOUBLE l, GLDOUBLE r, GLDOUBLE b,	GLDOUBLE t, 
				GLDOUBLE n, GLDOUBLE f,GLDOUBLE *matrix);
void mesa_Ortho(GLDOUBLE left, GLDOUBLE right, GLDOUBLE bottom, GLDOUBLE top, GLDOUBLE nearZ, GLDOUBLE farZ, GLDOUBLE *m);
void render_TextureProjectorParallel0(struct X3D_Node* parent, struct X3D_TextureProjectorParallel *node) {
	int i,j = 0;
	int flag = 0;
	//float degree = node->fieldOfView* 180/3.14;
	GLDOUBLE cViewMat[16];
	GLDOUBLE invcViewMat[16];
	GLDOUBLE ViewMat[16];
	GLDOUBLE orthoMat[16];
	GLint tex1;
	ppComponent_TextureProjector p;
	ttglobal tg = gglobal();
	p = (ppComponent_TextureProjector)tg->Component_TextureProjector.prv;
	
	RETURN_IF_RENDER_STATE_NOT_US
	COMPILE_IF_REQUIRED;

	if(node->on) {
		double tempmat[16];
		GLDOUBLE TenLinearGexMatCam0[16];
		GLDOUBLE modelview[16], modelviewnode[16], eye2projector[16], modelviewinv[16];
		struct X3D_Node *tmpN = NULL;
		struct X3D_ProjectorRep* projrep = (struct X3D_ProjectorRep*)node->_intern;

		if(node->global) tg->Component_TextureProjector.globalProjector = TRUE;

		//A. COMPUTE NODE-POSE MATRIX FOR: .position, .dir, .upVector
		//glMatrixMode(GL_MODELVIEW);
		FW_GL_MATRIX_MODE(GL_MODELVIEW);
		FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelview);
		{
			double loc[3],dir[3],up[3],eye[3];
			float2double(loc,node->_loc.c,3);
			float2double(dir,node->_dir.c,3);
			float2double(up,node->_upVec.c,3);
			vecdifd(eye,loc,dir);
			projLookAt(eye[0],eye[1],eye[2], loc[0],loc[1],loc[2], up[0],up[1],up[2],ViewMat);
		}
		matcopy(projrep->matview, ViewMat);
		//B. INVERT modelviewnode (which transforms projector to eye) to get eye-to-projector
		matinverse(modelviewinv,modelview);
		//C. COMBINE MODELVIEW MATRIX WITH NODE-POSE MATRIX
		matmultiplyAFFINE(eye2projector,modelviewinv,ViewMat);


		//C. COMPUTE A PROJECTION MATRIX THAT INCLUDES CAMERA SPACE TO TEXTURE SPACE BIAS
		// unsolved problem: specs show TPParallel.fieldOfView as SFVec4f, but OrthoViewpoint fov MFFloat. The 2 are incompatible in freewrl.
		float *fov;
		fov = node->fieldOfView.c;
		int method = 1;
		if (method == 0) {
			// July 3, 2022 - this doesn't have the right zone (near/farDistance), not working right
			mesa_Ortho((GLDOUBLE)fov[0], (GLDOUBLE)fov[2], (GLDOUBLE)fov[1], (GLDOUBLE)fov[3],
				(GLDOUBLE)node->nearDistance, (GLDOUBLE)node->farDistance, orthoMat);
			matcopy(projrep->matproj, orthoMat);
			printmatrix2(orthoMat, "orthoMat");
		}
		else {
			// this works a bit
			float size[3], center[3], * ll, * ur, zz[2];
			double mate[16], dcenter[3], dsize[3], matproj[16], matinv[16];
			ll = &fov[0];
			ur = &fov[2];
			zz[0] = node->nearDistance; zz[1] = node->farDistance;
			matidentity4d(mate);
			vecadd2f(center, ll, ur);
			center[2] = zz[0] + zz[1];
			vecscale3f(center, center, .5f);
			vecdif2f(size, ur, ll);
			size[2] = zz[1] - zz[0];
			mattranslate4d(mate, float2double(dcenter, center, 3));
			matscale4d(mate, float2double(dsize, size, 3));
			matinverse(matinv, mate);
			mesa_Ortho(-.5, .5, -.5, .5, -.5, .5, matproj);
			matmultiplyFULL(projrep->matproj, matproj, matinv);
			//printmatrix2(projrep->matproj, "orthoMat");

		}
		//D. COMBINE PROJECTION AND EYE-TO-PROJECTOR TRANSFORMS
		//matmultiplyFULL(projrep->matmodelviewproj,eye2projector,orthoMat);
	
		{
			float aspectRatio, denom;
			denom = fov[3]-fov[1];
			if(denom > 0.0){
				aspectRatio = (fov[2]-fov[0])/denom;
				if(!APPROX(node->aspectRatio,aspectRatio)){
					node->aspectRatio = aspectRatio;
					MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_TextureProjectorParallel, aspectRatio));
					//printf("aspectRatio= %f\n",node->aspectRatio);
				}
			}
		}

	
		if(node->texture)
		{
			POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->texture,tmpN);
		}
		{
			GLuint texture;
			usehit ptuple;
			ptuple.node = X3D_NODE(node);
			ptuple.userdata = parent;
			if (old_waay)
				matcopy(ptuple.mvm, eye2projector);
			else
				matcopy(ptuple.mvm, modelview);
			//matcopy(ptuple.proj, projrep->matproj);
			//ptuple.userdata = projrep->matproj;
			texture = tg->RenderFuncs.boundTextureStack[tg->RenderFuncs.textureStackTop];
			projrep->itexture = texture;
			projrep->texture = tmpN;
			//if (node->global && node->shadows) {
			if (node->shadows) {
				int nuse = projectorTable_node_use_count(X3D_NODE(node));
				ptuple.ivalue = make_or_get_depth_buffer(nuse, X3D_NODE(node));
				generate_shadowmap_2D(ptuple, 0);
			}
			projectorTable_push(ptuple);
		}


	} //if(node->on)
}
void render_TextureProjectorParallel(struct X3D_TextureProjectorParallel* node) {
	render_TextureProjectorParallel0(NULL, node);
}

void prep_TextureProjectorParallel(struct X3D_TextureProjectorParallel *node)
{
	if (!renderstate()->render_light) return;
	/* this will be a global textureprojector here... */
	render_TextureProjectorParallel(node);
}


void compile_TextureProjectorPoint(struct X3D_TextureProjectorPoint* node) {

	node->_intern = set_ProjectorRep(node->_intern);
	/* LookAt Matrix Complete */
	float dir[3], up[3], cross1[3], cross2[3];
	veccopy3f(node->_loc.c, node->location.c);
	veccopy3f(dir, node->direction.c);
	veccopy3f(up, node->upVector.c);
	vecnormalize3f(dir, dir);
	vecnormalize3f(up, up);
	veccross3f(cross1, dir, up);
	vecnormalize3f(cross1, cross1);
	veccross3f(cross2, cross1, dir);
	vecnormalize3f(cross2, cross2);
	veccopy3f(node->_dir.c, dir);
	node->_dir.c[3] = 0.0f;
	veccopy3f(node->_upVec.c, up);
	node->_upVec.c[3] = 0.0f;
	//	if (node->shadows) compile_shadowMap(X3D_NODE(node));
	MARK_NODE_COMPILED;
}

void render_TextureProjectorPoint0(struct X3D_Node* parent, struct X3D_TextureProjectorPoint* node) {
	int i, j = 0;
	int flag = 0;
	//float degree = node->fieldOfView * 180.0 / 3.141596;
	GLDOUBLE cViewMat[16];
	GLDOUBLE invcViewMat[16];
	GLDOUBLE ViewMat[16];
	GLDOUBLE ProjMat[16];
	GLint tex1;
	ppComponent_TextureProjector p;
	ttglobal tg = gglobal();
	p = (ppComponent_TextureProjector)tg->Component_TextureProjector.prv;

	RETURN_IF_RENDER_STATE_NOT_US
		COMPILE_IF_REQUIRED;

	if (node->on) {
		double tempmat[16];
		//GLDOUBLE TenLinearGexMatCam0[16];
		GLDOUBLE modelview[16], modelviewnode[16], eye2projector[16], modelviewinv[16];
		struct X3D_Node* tmpN = NULL;
		struct X3D_ProjectorRep* projrep = (struct X3D_ProjectorRep*)node->_intern;

		if (node->global) tg->Component_TextureProjector.globalProjector = TRUE;

		//A. COMPUTE NODE-POSE MATRIX FOR: .position, .dir, .upVector
		//glMatrixMode(GL_MODELVIEW);
		FW_GL_MATRIX_MODE(GL_MODELVIEW);
		FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelview);

		{
			double loc[3], dir[3], up[3], eye[3];
			float2double(loc, node->_loc.c, 3);
			float2double(dir, node->_dir.c, 3);
			float2double(up, node->_upVec.c, 3);
			vecdifd(eye, loc, dir);
			projLookAt(eye[0], eye[1], eye[2], loc[0], loc[1], loc[2], up[0], up[1], up[2], ViewMat);
		}
		matcopy(projrep->matview, ViewMat);
		//B. INVERT modelviewnode (which transforms projector to eye) to get eye-to-projector
		matinverse(modelviewinv, modelview);
		//C. COMBINE MODELVIEW MATRIX WITH NODE-POSE MATRIX
		matmultiplyAFFINE(eye2projector, modelviewinv, ViewMat);

		//C. COMPUTE A PROJECTION MATRIX THAT INCLUDES CAMERA SPACE TO TEXTURE SPACE BIAS
		//projPerspective((GLDOUBLE)degree,
		//	(GLDOUBLE)node->aspectRatio, // aspectRatio = width/height see below, gets from image
		//	(GLDOUBLE)node->nearDistance, (GLDOUBLE)node->farDistance, // near, far
		//	ProjMat);
		matidentity4d(ProjMat);
		matcopy(projrep->matproj, ProjMat);

		//matidentity4d(tempmat);
//		matmultiplyFULL(tempmat,bias,tempmat);
		//matmultiplyFULL(tempmat,ProjMat,tempmat);

		//D. COMBINE PROJECTION AND EYE-TO-PROJECTOR TRANSFORMS
		//matmultiplyFULL(projrep->matmodelviewproj,eye2projector,tempmat);
		//matmultiplyFULL(projrep->matmodelviewproj, eye2projector, ProjMat);


		//if (node->texture)
		//{
		//	POSSIBLE_PROTO_EXPANSION(struct X3D_Node*, node->texture, tmpN);
		//	if (tmpN) {
		//		int ixyz[3];
		//		float aspectRatio;
		//		if (getTextureSizeFromTextureNode(tmpN, ixyz)) {
		//			if (ixyz[0] > 0 && ixyz[1] > 0) {
		//				aspectRatio = (float)ixyz[0] / (float)ixyz[1];
		//				if (!APPROX(node->aspectRatio, aspectRatio)) {
		//					node->aspectRatio = aspectRatio;
		//					MARK_EVENT(X3D_NODE(node), offsetof(struct X3D_TextureProjector, aspectRatio));
		//					//printf("aspectRatio= %f\n",node->aspectRatio);
		//				}
		//			}
		//		}
		//	}


		//}
		if (node->texture)
		{
			POSSIBLE_PROTO_EXPANSION(struct X3D_Node*, node->texture, tmpN);
		}

		{
			GLuint texture;
			usehit ptuple;

			ptuple.node = X3D_NODE(node);
			ptuple.userdata = parent;
			if (old_waay)
				matcopy(ptuple.mvm, eye2projector);
			else
				matcopy(ptuple.mvm, modelview);
			//ptuple.userdata = projrep->matproj;
			//matcopy(ptuple.proj, projrep->matproj);
			texture = tg->RenderFuncs.boundTextureStack[tg->RenderFuncs.textureStackTop];
			projrep->itexture = texture;
			projrep->texture = tmpN;
			//if (node->global && node->shadows) {
			if (node->shadows) {
				int nuse = projectorTable_node_use_count(X3D_NODE(node));
				ptuple.ivalue = make_or_get_depth_buffer(nuse, X3D_NODE(node));
				generate_shadowmap_cube(ptuple, 0);
			}
			projectorTable_push(ptuple);
		}

	} //if(node->on)
}
void render_TextureProjectorPoint(struct X3D_TextureProjectorPoint* node) {
	render_TextureProjectorPoint0(NULL, node);
}

void fin_TextureProjectorPoint(struct X3D_TextureProjectorPoint* node)
{
	RETURN_IF_RENDER_STATE_NOT_US
		if (node->on)
			if (!node->global)
				projectorTable_pop(); //just pop local projectors that we pushed above - globals are cleared once per frame
}

void prep_TextureProjectorPoint(struct X3D_TextureProjectorPoint* node) {


	if (!renderstate()->render_light) return;
	/* this will be a global textureprojector here... */
	render_TextureProjectorPoint(node);
}
void child_TextureProjectorPoint(struct X3D_TextureProjectorPoint* node) {
}






//void render_TextureProjectors(struct X3D_Node *sibAffector){
//	switch(sibAffector->_nodeType){
//		case NODE_TextureProjectorParallel:
//			render_TextureProjectorParallel((struct X3D_TextureProjectorParallel*)sibAffector);
//			break;
//		case NODE_TextureProjector:
//		default:
//			render_TextureProjector((struct X3D_TextureProjector*)sibAffector);
//			break;
//	}
//}
//void fin_TextureProjectors(struct X3D_Node *sibAffector){
//	switch(sibAffector->_nodeType){
//		case NODE_TextureProjectorParallel:
//			fin_TextureProjectorParallel((struct X3D_TextureProjectorParallel*)sibAffector);
//			break;
//		case NODE_TextureProjector:
//		default:
//			fin_TextureProjector((struct X3D_TextureProjector*)sibAffector);
//			break;
//	}
//}

//void sib_prep_TextureProjector(struct X3D_Node *parent, struct X3D_Node *sibAffector){
//	if ( renderstate()->render_light != VF_globalLight){
//		shaderflagsstruct shaderflags;
//		shaderflags = getShaderFlags();
//		shaderflags.base |= HAVE_PROJECTIVETEXTURE;
//		pushShaderFlags(shaderflags);
//
//		render_TextureProjectors(sibAffector);
//	}
//
//}
//
//void sib_fin_TextureProjector(struct X3D_Node *parent, struct X3D_Node *sibAffector){
//	if (renderstate()->render_light != VF_globalLight) {
//		fin_TextureProjectors(sibAffector);
//		popShaderFlags();
//	}
//}

void sib_prep_TextureProjector(struct X3D_Node* parent, struct X3D_Node* sibAffector) {
	struct X3D_TextureProjector* projector = X3D_TEXTUREPROJECTOR(sibAffector);
	if (renderstate()->render_light != VF_globalLight && !renderstate()->render_depth && renderstate()->render_geom) {
		if (projector->global == FALSE && projector->on == TRUE) {
			shaderflagsstruct shaderflags;
			shaderflags = getShaderFlags();
			shaderflags.base |= HAVE_PROJECTIVETEXTURE;
			pushShaderFlags(shaderflags);

			switch (projector->_nodeType) {
			case NODE_TextureProjector:
				render_TextureProjector0(parent, (struct X3D_TextureProjector*)sibAffector);
				break;
			case NODE_TextureProjectorParallel:
				render_TextureProjectorParallel0(parent, (struct X3D_TextureProjectorParallel*)sibAffector);
				break;
			case NODE_TextureProjectorPoint:
				render_TextureProjectorPoint0(parent, (struct X3D_TextureProjectorPoint*)sibAffector);
				break;
			default:
				break;
			}
		}
	}
}
void sib_fin_TextureProjector(struct X3D_Node* parent, struct X3D_Node* sibAffector) {
	if (renderstate()->render_light != VF_globalLight && !renderstate()->render_depth && renderstate()->render_geom) {
		struct X3D_TextureProjector* projector = X3D_TEXTUREPROJECTOR(sibAffector);
		if (projector->global == FALSE && projector->on == TRUE) {
			projectorTable_pop();
			popShaderFlags();
		}
	}
}

