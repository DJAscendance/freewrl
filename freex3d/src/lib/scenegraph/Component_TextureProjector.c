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
	struct X3D_Node* depthTexture;
	int size;
	int idepthtexture;
	double matproj[16];
	double matview[16];
	//projector section
	struct X3D_Node* texture;
	int itexture;
	//double matmodelviewproj[16];
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
		projectorrep->idepthtexture = -1;
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



int get_bound_image(struct X3D_Node *node);
int getGlTextureNumberFromTextureNode(struct X3D_Node *textureNode);
int getTextureSizeFromTextureNode(struct X3D_Node *textureNode, int *ixyz);
int getTextureDescriptors(struct X3D_Node *textureNode, int *textures, int *modes, int *sources, int *funcs, int *width, int *height, int *samplr);
void resend_textureprojector_matrix()
{
	//called from render_shape to refresh uniform before shade draw
	int pcount,tcount;
	s_shader_capabilities_t *me;
	ppComponent_TextureProjector p;
	ttglobal tg = gglobal();
	p = (ppComponent_TextureProjector)tg->Component_TextureProjector.prv;

    me = getAppearanceProperties()->currentShaderProperties;

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
	int MAX_PROJ = 8;
	int MAX_TDESC = 16;
	int MAX_TEX = 4;
	tcount = min(p->projector_stack->n,MAX_PROJ);
	pcount = 0;
	int nunit = 0;
	int kdesc = 0;
	int unitTextures[4];
	GLint saveTextureStackTop = tg->RenderFuncs.textureStackTop;
	PRINT_GL_ERROR_IF_ANY("BEGIN resend_textureprojector_matrix");

	for(int i=0;i<tcount;i++)
	{
		float TenLinearGexMatCam0f[16];
		usehit *ptuple;
		GLint texture;
		if(me->ptmGenMatCam[i] > -1){
			ptuple = vector_get_ptr(usehit, p->projector_stack, i);
			double matfull[16];
			struct X3D_TextureProjector* ptm = (struct X3D_TextureProjector*)ptuple->node;
			struct X3D_ProjectorRep* projrep = (struct X3D_ProjectorRep*)ptm->_intern;
			matmultiplyFULL(matfull, ptuple->mvm, projrep->matproj);
			double2float(TenLinearGexMatCam0f, matfull,16);
			//double2float(TenLinearGexMatCam0f, ptuple->userdata, 16);
			GLUNIFORMMATRIX4FV (me->ptmGenMatCam[i],1,GL_FALSE, TenLinearGexMatCam0f);
			//GLUNIFORM1I(me->projectorType[i],ptuple->type);
			//backCull in theory could automatically always do it, 
			// or projector->backCull=TRUE default, 
			// and turn off when Gl_CULL_FACE is off, meaning web3d solid=FALSE
			// X HOWEVER freewrl Feb 2020 isn't reliably discriminating solid=true/false for different geometry types
			// - THEREFORE we will let projector->backCull be definitive and scene authors will set manually until freewrl solid is fixed
			GLUNIFORM1I(me->ptmbackCull[i],ptm->backCull);
			GLUNIFORM1I(me->ptmshadows[i], ptm->shadows);
			GLUNIFORM1F(me->ptmshadowIntensity[i], ptm->shadowIntensity);
		//GLUNIFORM1I(me->ptmdepthmap[i], projrep->idepthtexture);
			//GLUNIFORM1I(me->pbackCull[i], (ptuple->backCull && getAppearanceProperties()->cullFace)?1:0); 

			int ntdesc = 0; //number of texture descriptors in this projector
			struct X3D_NODE * tlist[4];
			int modes[4];
			int sources[4];
			int funcs[4];
			int textures[4];
			int width[4], height[4], samplr[4];

			int toffset = 4;
			//glActiveTexture(GL_TEXTURE0+toffset+pcount); 
			//glActiveTexture(GL_TEXTURE0 + next_textureUnit2D());
			render_node(projrep->texture);
			PRINT_GL_ERROR_IF_ANY("MIDDLE resend_textureprojector_matrix");

			ntdesc = getTextureDescriptors(projrep->texture,textures, modes,sources, funcs, width, height, samplr);
			GLUNIFORM1I(me->ntdesc[i],ntdesc);
			for(int j=0;j<ntdesc;j++,kdesc++){
				// re-use texture sampler if mulitple projectors and multitextures refer to same GLint texture 1:1 sampler2D
				int kunit;
				//texture = ptuple->texture;
				texture = textures[j];

				nunit = min(nunit++,MAX_TEX); //for fun, if we go over MAX_TEX we'll just over-write last one
				kunit = nunit-1;
				int ksamp = share_or_next_material_sampler_index_2D(texture); //returns i as in GL_TEXTUREi next available
				int itextureunit = tunit2D(ksamp); //returns index into shader sampler2D textureUnit[itextureunit] 
				glUniform1i(me->textureUnit[ksamp],itextureunit); //tunit(kkunit));
				GLUNIFORM1I(me->tunits[kdesc],ksamp); //tunits like PBR tindex - an array saying which sampler2D textureUnit[tunit[kdesc]]
				//glActiveTexture(GL_TEXTURE0);

				GLUNIFORM1I(me->modes[kdesc],modes[j]);
				GLUNIFORM1I(me->sources[kdesc],sources[j]);
				GLUNIFORM1I(me->funcs[kdesc],funcs[j]);
			}
			if (ptm->shadows) {
				PRINT_GL_ERROR_IF_ANY("before shadow resend_textureprojector_matrix");

				//render_node(projrep->depthTexture);

				texture = projrep->idepthtexture;
				int ksamp = share_or_next_material_sampler_index_2D(texture); //does bind and activetexture
				int itextureunit = tunit2D(ksamp);
				//glActiveTexture(GL_TEXTURE1);
				//glBindTexture(GL_TEXTURE_2D, texture);
				glUniform1i(me->textureUnit[ksamp], itextureunit);
				PRINT_GL_ERROR_IF_ANY("after [ksamp], texture resend_textureprojector_matrix");
				glUniform1i(me->ptmdepthmap[i], ksamp);
				PRINT_GL_ERROR_IF_ANY("after shadow resend_textureprojector_matrix");
			}
			pcount++;
			tg->RenderFuncs.textureStackTop = saveTextureStackTop; //keep this frmo building up
		}
	}
	GLUNIFORM1I(me->ptmCount,pcount);
	PRINT_GL_ERROR_IF_ANY("END resend_textureprojector_matrix");
}
//void compile_shadowMap(struct X3D_Node* node); // Component_Lighting
//void render_shadowMap(struct X3D_Node* node);
void compile_TextureProjector (struct X3D_TextureProjector *node) { 

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
//	if (node->shadows) compile_shadowMap(X3D_NODE(node));
	MARK_NODE_COMPILED;
}

/* Projective Texture gluLookAt */
void projLookAt(GLDOUBLE eyex, GLDOUBLE eyey, GLDOUBLE eyez,
				GLDOUBLE centerx, GLDOUBLE centery, GLDOUBLE centerz,
				GLDOUBLE upx, GLDOUBLE upy, GLDOUBLE upz, GLDOUBLE *matrix);
void projPerspective(GLDOUBLE fovy, GLDOUBLE aspect, GLDOUBLE zNear, GLDOUBLE zFar, GLDOUBLE *matrix);
void printmatrix2(GLDOUBLE* mat,char* description );
void render_TextureProjector (struct X3D_TextureProjector *node) {
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
			matcopy(ptuple.mvm, eye2projector);
			//ptuple.userdata = projrep->matproj;
			//matcopy(ptuple.proj, projrep->matproj);
			texture = tg->RenderFuncs.boundTextureStack[tg->RenderFuncs.textureStackTop];
			projrep->itexture = texture;
			projrep->texture = tmpN;
			projectorTable_push(ptuple);
			if (node->global && node->shadows) {
				//shadowTable_push(ptuple);
				//render_shadowMap(X3D_NODE(node));
			}
		}

	} //if(node->on)
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
void render_TextureProjectorParallel (struct X3D_TextureProjectorParallel *node) {
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
		mesa_Ortho((GLDOUBLE)node->fieldOfView.p[0],(GLDOUBLE)node->fieldOfView.p[2],(GLDOUBLE)node->fieldOfView.p[1],(GLDOUBLE)node->fieldOfView.p[3],
			(GLDOUBLE)node->nearDistance, (GLDOUBLE)node->farDistance,orthoMat);
		matcopy(projrep->matproj, orthoMat);
		//matidentity4d(tempmat);
//		matmultiplyFULL(tempmat,bias,tempmat); //in shader now
		//matmultiplyFULL(tempmat,orthoMat,tempmat);

		//D. COMBINE PROJECTION AND EYE-TO-PROJECTOR TRANSFORMS
		//matmultiplyFULL(projrep->matmodelviewproj,eye2projector,orthoMat);
	
		{
			float aspectRatio, denom, *fov;
			fov = node->fieldOfView.p;
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
			matcopy(ptuple.mvm, eye2projector);
			//matcopy(ptuple.proj, projrep->matproj);
			//ptuple.userdata = projrep->matproj;
			texture = tg->RenderFuncs.boundTextureStack[tg->RenderFuncs.textureStackTop];
			projrep->itexture = texture;
			projrep->texture = tmpN;
			projectorTable_push(ptuple);
			if (node->global && node->shadows) {
				//shadowTable_push(ptuple);
			}
		}


	} //if(node->on)
}


void prep_TextureProjectorParallel(struct X3D_TextureProjectorParallel *node)
{
	if (!renderstate()->render_light) return;
	/* this will be a global textureprojector here... */
	render_TextureProjectorParallel(node);
}



void render_TextureProjectors(struct X3D_Node *sibAffector){
	switch(sibAffector->_nodeType){
		case NODE_TextureProjectorParallel:
			render_TextureProjectorParallel((struct X3D_TextureProjectorParallel*)sibAffector);
			break;
		case NODE_TextureProjector:
		default:
			render_TextureProjector((struct X3D_TextureProjector*)sibAffector);
			break;
	}
}
void fin_TextureProjectors(struct X3D_Node *sibAffector){
	switch(sibAffector->_nodeType){
		case NODE_TextureProjectorParallel:
			fin_TextureProjectorParallel((struct X3D_TextureProjectorParallel*)sibAffector);
			break;
		case NODE_TextureProjector:
		default:
			fin_TextureProjector((struct X3D_TextureProjector*)sibAffector);
			break;
	}
}

void sib_prep_TextureProjector(struct X3D_Node *parent, struct X3D_Node *sibAffector){
	if ( renderstate()->render_light != VF_globalLight){
		shaderflagsstruct shaderflags;
		shaderflags = getShaderFlags();
		shaderflags.base |= HAVE_PROJECTIVETEXTURE;
		pushShaderFlags(shaderflags);

		render_TextureProjectors(sibAffector);
	}

}

void sib_fin_TextureProjector(struct X3D_Node *parent, struct X3D_Node *sibAffector){
	if (renderstate()->render_light != VF_globalLight) {
		fin_TextureProjectors(sibAffector);
		popShaderFlags();
	}
}


