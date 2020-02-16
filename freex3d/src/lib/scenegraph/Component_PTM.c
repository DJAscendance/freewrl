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


struct projective_Texdata {
    struct Uni_String *des;
	GLDOUBLE TenLinearGexMat[16];
};

struct projector_tuple {
    struct Uni_String *des;
	GLDOUBLE TenLinearGexMat[16];
	int global;
	GLuint texture;
	struct X3D_Node * textureNode;
};

typedef struct pComponent_PTM{
	struct Vector *projector_stack; //activeProjectiveTextureTable;
	//textureTableIndexStruct_s* loadThisProjectiveTexture;

	/* current index into loadparams that texture thread is working on */
	int currentlyWorkingOn;// = -1;
	int textureInProcess;// = -1;
	struct projective_Texdata data[4];
}* ppComponent_PTM;

void *Component_PTM_constructor(){
	void *v = malloc(sizeof(struct pComponent_PTM));
	memset(v,0,sizeof(struct pComponent_PTM));
	return v;
}
void Component_PTM_init(struct tComponent_PTM *t){
	//public

	//private 
	
	t->prv = Component_PTM_constructor();
	{
		ppComponent_PTM p = (ppComponent_PTM)t->prv;
		//p->activeProjectiveTextureTable = NULL;
		p->projector_stack = newStack(struct projector_tuple);

		//t->data = &p->data;
		/* current index into loadparams that texture thread is working on */
		p->currentlyWorkingOn = -1;

		p->textureInProcess = -1;
	}
}

void Component_PTM_clear(struct tComponent_PTM *t){
	//public
	//private
	{
		ppComponent_PTM p = (ppComponent_PTM)t->prv;
	}
}


void projectorTable_clear(){
	//called once per frame, before the search for global=true projectors
	//will clear any global=true projectors from last frame
	ppComponent_PTM p;
	ttglobal tg = gglobal();
	p = (ppComponent_PTM)tg->Component_PTM.prv;
	clearStack(p->projector_stack);
}
void projectorTable_push(struct projector_tuple *ptuple ){
	//called when we find a global=true, on=true projector, and
	//called in sib_prep for a global=false, on=false projector
	ppComponent_PTM p;
	ttglobal tg = gglobal();
	p = (ppComponent_PTM)tg->Component_PTM.prv;
	//we need a deep copy because the ptm node can't hold it
	// because it can be DEF/USED with different transform each use
	stack_push(struct projector_tuple,p->projector_stack,*ptuple);

}
void projectorTable_pop(){
	//called in sib_fin for a global=false, on=true projector
	ppComponent_PTM p;
	ttglobal tg = gglobal();
	p = (ppComponent_PTM)tg->Component_PTM.prv;
	if(p->projector_stack->n < 1)
		printf("ouch from projectorTable_opo()\n");
	stack_pop(struct projector_tuple,p->projector_stack);

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
int getTextureTableIndexFromFromTextureNode(struct X3D_Node *node);
int getGlTextureNumberFromTextureNode(struct X3D_Node *textureNode);
int getTextureSizeFromTextureNode(struct X3D_Node *textureNode, int *ixyz);
void resend_textureprojector_matrix()
{
	//called from render_shape to refresh uniform before shade draw
	int pcount,tcount;
	s_shader_capabilities_t *me;
	struct projective_Texdata *data;
	ppComponent_PTM p;
	ttglobal tg = gglobal();
	p = (ppComponent_PTM)tg->Component_PTM.prv;
	data = p->data;
	//data = (struct projective_Texdata*)tg->Component_PTM.data;

    me = getAppearanceProperties()->currentShaderProperties;


	tcount = min(p->projector_stack->n,4);
	pcount = 0;
	for(int i=0;i<tcount;i++)
	{
		float TenLinearGexMatCam0f[16];
		struct projector_tuple *ptuple;
		GLint texture;
		//int tti;
		if(me->projTexGenMatCam[i] > -1){
			ptuple = vector_get_ptr(struct projector_tuple, p->projector_stack, i);
			double2float(TenLinearGexMatCam0f, ptuple->TenLinearGexMat,16);
			GLUNIFORMMATRIX4FV (me->projTexGenMatCam[i],1,GL_FALSE, TenLinearGexMatCam0f);
			texture = ptuple->texture;
			int toffset = 4;
			//print_bound_textures("start");
			glActiveTexture(GL_TEXTURE0+toffset+pcount); 

			render_node(ptuple->textureNode);
			//tti = getTextureTableIndexFromFromTextureNode(ptuple->textureNode);
			texture = getGlTextureNumberFromTextureNode(ptuple->textureNode);

			glActiveTexture(GL_TEXTURE0+toffset+pcount); 
			glBindTexture(GL_TEXTURE_2D,texture); 

			glUniform1i(me->textureUnit[i],pcount+toffset);
			glActiveTexture(GL_TEXTURE0);
			//print_bound_textures("end");
			pcount++;
		}
	}
	GLUNIFORM1I(me->pCount,pcount);

}

void compile_TextureProjectorPerspective (struct X3D_TextureProjectorPerspective *node) { 


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

/* Projective Texture gluLookAt */
void projLookAt(GLDOUBLE eyex, GLDOUBLE eyey, GLDOUBLE eyez,
				GLDOUBLE centerx, GLDOUBLE centery, GLDOUBLE centerz,
				GLDOUBLE upx, GLDOUBLE upy, GLDOUBLE upz, GLDOUBLE *matrix);
void printmatrix2(GLDOUBLE* mat,char* description );
void render_TextureProjectorPerspective (struct X3D_TextureProjectorPerspective *node) {
	int i,j = 0;
	int flag = 0;
	static int datacount = 0;
	float degree = node->fieldOfView* 180/3.14;
	GLDOUBLE cViewMat[16];
	GLDOUBLE invcViewMat[16];
	GLDOUBLE ViewMat[16];
	GLDOUBLE ProjMat[16];
	GLint tex1;
	struct projective_Texdata *data;
	ppComponent_PTM p;
	ttglobal tg = gglobal();
	p = (ppComponent_PTM)tg->Component_PTM.prv;
	data = p->data;
	//data = (struct projective_Texdata*)tg->Component_PTM.data;
	
	RETURN_IF_RENDER_STATE_NOT_US
	COMPILE_IF_REQUIRED;

	if(node->on) {
		double tempmat[16];
		GLDOUBLE TenLinearGexMatCam0[16];
		GLDOUBLE modelview[16], modelviewnode[16], eye2projector[16], modelviewinv[16];
		struct X3D_Node *tmpN = NULL;

		if(node->global) tg->Component_PTM.globalProjector = TRUE;

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

		//B. INVERT modelviewnode (which transforms projector to eye) to get eye-to-projector
		matinverse(modelviewinv,modelview);
		//C. COMBINE MODELVIEW MATRIX WITH NODE-POSE MATRIX
		matmultiplyAFFINE(eye2projector,modelviewinv,ViewMat);


		//C. COMPUTE A PROJECTION MATRIX THAT INCLUDES CAMERA SPACE TO TEXTURE SPACE BIAS
		projPerspective((GLDOUBLE)degree,
			(GLDOUBLE)node->aspectRatio, // aspectRatio
			(GLDOUBLE)node->nearDistance,(GLDOUBLE)node->farDistance, // near, far
			ProjMat);

		matidentity4d(tempmat);
		matmultiplyFULL(tempmat,bias,tempmat);
		matmultiplyFULL(tempmat,ProjMat,tempmat);

		//D. COMBINE PROJECTION AND EYE-TO-PROJECTOR TRANSFORMS
		matmultiplyFULL(TenLinearGexMatCam0,eye2projector,tempmat);
	
	
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
							MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_TextureProjectorPerspective, aspectRatio));
							//printf("aspectRatio= %f\n",node->aspectRatio);
						}
					}
				}
			}


		}
		{
			GLuint texture;
			struct projector_tuple ptuple;
			ptuple.des = node->description;
			memcpy(ptuple.TenLinearGexMat, TenLinearGexMatCam0,16*sizeof (GLDOUBLE));
			ptuple.global = node->global;
			texture = tg->RenderFuncs.boundTextureStack[tg->RenderFuncs.textureStackTop];
			ptuple.texture = texture;
			ptuple.textureNode = tmpN;
			projectorTable_push(&ptuple);
		}

	} //if(node->on)
 }


void fin_TextureProjectorPerspective (struct X3D_TextureProjectorPerspective *node) 
{
	RETURN_IF_RENDER_STATE_NOT_US
	if(node->on)
		if(!node->global)
			projectorTable_pop(); //just pop local projectors that we pushed above - globals are cleared once per frame
}

void prep_TextureProjectorPerspective(struct X3D_TextureProjectorPerspective *node) {


	if (!renderstate()->render_light) return;
	/* this will be a global textureprojector here... */
	render_TextureProjectorPerspective(node);

}
void child_TextureProjectorPerspective (struct X3D_TextureProjectorPerspective *node) {
}


/////////////////////////////////////////////////////////////////////////////////////////////



void child_TextureProjectorParallel (struct X3D_TextureProjectorParallel *node) {
}
void fin_TextureProjectorParallel (struct X3D_TextureProjectorParallel *node) {
}


void compile_TextureProjectorParallel (struct X3D_TextureProjectorParallel *node) { 


	
	struct point_XYZ vec;
	int i;

	for (i=0; i<3; i++) node->_loc.c[i] = node->location.c[i];
	node->_loc.c[3] = 1.0f;
	
	vec.x = (double) node->direction.c[0];
	vec.y = (double) node->direction.c[1];
	vec.z = (double) node->direction.c[2];

	normalize_vector(&vec);

	node->_dir.c[0] = (float) vec.x;
	node->_dir.c[1] = (float) vec.y;
	node->_dir.c[2] = (float) vec.z;
	node->_dir.c[3] = 1.0f;

	vec.x = (double) node->upVector.c[0];
	vec.y = (double) node->upVector.c[1];
	vec.z = (double) node->upVector.c[2];

	normalize_vector(&vec);

	node->_upVec.c[0] = (float) vec.x;
	node->_upVec.c[1] = (float) vec.y;
	node->_upVec.c[2] = (float) vec.z;

	MARK_NODE_COMPILED;
}


void render_TextureProjectorParallel (struct X3D_TextureProjectorParallel *node) {
	
	int i,j = 0;
	int flag = 0;
	static int datacount = 0;
	
	
	
	GLDOUBLE cViewMat[16];
	GLDOUBLE invcViewMat[16];
	GLDOUBLE ViewMat[16];
	GLDOUBLE orthoMat[16];
	GLDOUBLE TenLinearGexMatCam0[16];
	
	GLint tex1;
	struct projective_Texdata *data;
	ppComponent_PTM p;
	ttglobal tg = gglobal();
	p = (ppComponent_PTM)tg->Component_PTM.prv;
	data = p->data;
	//data = (struct projective_Texdata*)tg->Component_PTM.data;
	
	
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	projLookAt((GLDOUBLE)node->_loc.c[0],(GLDOUBLE)node->_loc.c[1],(GLDOUBLE)node->_loc.c[2], 
		(GLDOUBLE)node->_dir.c[0],(GLDOUBLE)node->_dir.c[1],(GLDOUBLE)node->_dir.c[2],
		(GLDOUBLE)node->_upVec.c[0],(GLDOUBLE)node->_upVec.c[1],(GLDOUBLE)node->_upVec.c[2],ViewMat);
	
	glGetDoublev(GL_MODELVIEW_MATRIX, ViewMat);

	projOrtho((GLDOUBLE)node->fieldOfView.c[0],(GLDOUBLE)node->fieldOfView.c[1],(GLDOUBLE)node->fieldOfView.c[2],(GLDOUBLE)node->fieldOfView.c[3],
		(GLDOUBLE)node->nearDistance, (GLDOUBLE)node->farDistance,orthoMat);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glLoadMatrixd(bias);
	glMultMatrixd(orthoMat);
	glMultMatrixd(ViewMat);
	glGetDoublev(GL_MODELVIEW_MATRIX, TenLinearGexMatCam0);
	
	fw_glGetDoublev(GL_MODELVIEW_MATRIX, cViewMat);
	matinverse(invcViewMat,cViewMat);
	
	for(i=0; i<4; i++)
	{
		//if(node->description == tg->ProjectiveTextures.data[i].des)
		if(node->description == data[i].des)
		{
			flag = 1;
			break;
		}
		else flag = 0;
	}

	for(j=0; j<4; j++)
	{
		//if(tg->ProjectiveTextures.data[j].des == NULL && !flag)
		if(data[j].des == NULL && !flag)
		{
			//tg->ProjectiveTextures.data[j].des = node->description; 
			data[j].des = node->description; 
			////tg->ProjectiveTextures.data[j].TenLinearGexMat = TenLinearGexMatCam0;
			//memcpy (tg->ProjectiveTextures.data[j].TenLinearGexMat, TenLinearGexMatCam0,16*sizeof (GLDOUBLE));
			memcpy (data[j].TenLinearGexMat, TenLinearGexMatCam0,16*sizeof (GLDOUBLE));
			//datacount = j;
			break;
		}
	}
	
	if(node->texture)
	{
		struct X3D_Node *tmpN;
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->texture,tmpN);
		render_node(tmpN);
	}

	glPopMatrix();

 }
void prep_TextureProjectorParallel(struct X3D_TextureProjectorParallel *node)
{
	COMPILE_IF_REQUIRED;

	render_TextureProjectorParallel(node);
}



void render_TextureProjector(struct X3D_Node *sibAffector){
	switch(sibAffector->_nodeType){
		case NODE_TextureProjectorParallel:
			render_TextureProjectorParallel((struct X3D_TextureProjectorParallel*)sibAffector);
			break;
		case NODE_TextureProjectorPerspective:
		default:
			render_TextureProjectorPerspective((struct X3D_TextureProjectorPerspective*)sibAffector);
			break;
	}
}
void fin_TextureProjector(struct X3D_Node *sibAffector){
	switch(sibAffector->_nodeType){
		case NODE_TextureProjectorParallel:
			fin_TextureProjectorParallel((struct X3D_TextureProjectorParallel*)sibAffector);
			break;
		case NODE_TextureProjectorPerspective:
		default:
			fin_TextureProjectorPerspective((struct X3D_TextureProjectorPerspective*)sibAffector);
			break;
	}
}

void sib_prep_TextureProjector(struct X3D_Node *parent, struct X3D_Node *sibAffector){
	if ( renderstate()->render_light != VF_globalLight){
		shaderflagsstruct shaderflags;
		shaderflags = getShaderFlags();
		shaderflags.base |= HAVE_PROJECTIVETEXTURE;
		pushShaderFlags(shaderflags);

		render_TextureProjector(sibAffector);
	}

}

void sib_fin_TextureProjector(struct X3D_Node *parent, struct X3D_Node *sibAffector){
	if (renderstate()->render_light != VF_globalLight) {
		fin_TextureProjector(sibAffector);
		popShaderFlags();
	}
}


