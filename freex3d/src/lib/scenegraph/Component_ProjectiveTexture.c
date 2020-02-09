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
#include "Children.h"
#include <stdlib.h>


struct X3D_Node *tmpN;

//we'll share VF_globalLight render pass with global lights
// this filter will allow global=true on the VF_globalLight render_hier pass
// and wll allow global=false on regular pass
#define RETURN_IF_RENDER_STATE_NOT_US \
		if (renderstate()->render_light== VF_globalLight) { \
			if (!node->global) return;\
			/* printf ("and this is a global light\n"); */\
		} else if (node->global) return; \
		/* else printf ("and this is a local light\n"); */


/*
GLfloat eyePlaneS[] = { 1.0, 0.0, 0.0, 0.0 };
GLfloat eyePlaneT[] = { 0.0, 1.0, 0.0, 0.0 };
GLfloat eyePlaneR[] = { 0.0, 0.0, 1.0, 0.0 };
GLfloat eyePlaneQ[] = { 0.0, 0.0, 0.0, 1.0 };
*/
static int dataCount=0;

static GLDOUBLE ProjViewMatCam0[16];
static GLDOUBLE ProjProjectionMatCam0[16];
GLDOUBLE TenLinearGexMatCam0[16];

float f_TenLinearGexMatCam0[16];
static float f_invcViewMat[16];

static float ViewMatrix[16] ={
	1,0,0,0,
	0,1,0,0,
	0,0,1,0,
	0,0,0,1
};


const GLDOUBLE bias[16] = { 0.5, 0.0, 0.0, 0.0,
0.0, 0.5, 0.0, 0.0,
0.0, 0.0, 0.5, 0.0,
0.5, 0.5, 0.5, 1.0 };


GLint loc_texgenmat;



bool flag = true;
GLuint projTexture;

//float TenLinearGexMatCam0[16];
//void convertDbtoFl(GLDOUBLE * dmt, float *fmt);


void resend_textureprojector_matrix()
{
	struct projective_Texdata *data;
	ttglobal tg = gglobal();
	data = (struct projective_Texdata*)tg->ProjectiveTextures.data;

	if(tg->ProjectiveTextures._projTexGenMatCam0_Location != 0 || tg->ProjectiveTextures._projViewMat_Location != 0 || tg->ProjectiveTextures._projMap_forCam1_Location != 0)
	{

		//convertDbtoFl(tg->ProjectiveTextures.data[0].TenLinearGexMat, TenLinearGexMatCam0);
		//convertDbtoFl(data[0].TenLinearGexMat, TenLinearGexMatCam0);
		float TenLinearGexMatCam0f[16];
		double2float(TenLinearGexMatCam0f,data[0].TenLinearGexMat,16);
		if(0){
		for(int j=0;j<4;j++)
		{
			printf("[ ");
			for(int i=0;i<4;i++)
				printf("%lf ",data[0].TenLinearGexMat[j*4 +i]);
			printf("]\n");
		}
		for(int j=0;j<4;j++)
		{
			printf("[ ");
			for(int i=0;i<4;i++)
				printf("%f ",TenLinearGexMatCam0f[j*4 +i]);
			printf("]\n");
		}
		}
		GLUNIFORMMATRIX4FV (tg->ProjectiveTextures._projTexGenMatCam0_Location,1,GL_FALSE, TenLinearGexMatCam0f);
	}

}



void convertDbtoFl(GLDOUBLE * dmt, float *fmt)
{
	float spval[16];
	int i;
	float *sp; 
	GLDOUBLE *dp;
	
	dp = dmt;
	sp = spval;

	/* convert GLDOUBLE to float */
	for (i=0; i<16; i++) {
		*sp = (float)*dp;
		sp ++; dp ++;
	}

	memcpy (fmt,spval,16*sizeof (float));
}


// For a Perspective Node
//void compile_PerspectiveProjector (struct X3D_PerspectiveProjector *node) { 
//
//
//	/* LookAt Matrix Complete */
//	struct point_XYZ vec;
//	int i;
//
//	for (i=0; i<3; i++) node->_loc.c[i] = node->centerOfProjection.c[i];
//	node->_loc.c[3] = 1.0f;/* 1 == this is a position, not a vector */
//	
//	vec.x = (double) node->direction.c[0];
//	vec.y = (double) node->direction.c[1];
//	vec.z = (double) node->direction.c[2];
//
//	normalize_vector(&vec);
//
//	node->_dir.c[0] = (float) vec.x;
//	node->_dir.c[1] = (float) vec.y;
//	node->_dir.c[2] = (float) vec.z;
//	node->_dir.c[3] = 1.0f;
//
//	vec.x = (double) node->upVector.c[0];
//	vec.y = (double) node->upVector.c[1];
//	vec.z = (double) node->upVector.c[2];
//
//	normalize_vector(&vec);
//
//	node->_upVec.c[0] = (float) vec.x;
//	node->_upVec.c[1] = (float) vec.y;
//	node->_upVec.c[2] = (float) vec.z;
//
//	MARK_NODE_COMPILED;
//	
//}



//void child_PerspectiveProjector (struct X3D_PerspectiveProjector *node) {
//
//}
//void render_PerspectiveProjector (struct X3D_PerspectiveProjector *node) {
//	
//	int i,j = 0;
//	int flag = 0;
//	static int datacount = 0;
//	/*
//	if(node->texture)
//	{
//		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->texture,tmpN);
//		render_node(tmpN);
//	}
//	*/
//	GLDOUBLE cViewMat[16];
//	GLDOUBLE invcViewMat[16];
//	GLDOUBLE ViewMat[16];
//	GLDOUBLE ProjMat[16];
//	
//	GLint tex1;
//	struct projective_Texdata *data;
//	ttglobal tg = gglobal();
//	data = (struct projective_Texdata*)tg->ProjectiveTextures.data;
//	
//	
//	
//	glMatrixMode(GL_MODELVIEW);
//	glPushMatrix();
//	glLoadIdentity();
//	
//	projLookAt((GLDOUBLE)node->_loc.c[0],(GLDOUBLE)node->_loc.c[1],(GLDOUBLE)node->_loc.c[2], 
//		(GLDOUBLE)node->_dir.c[0],(GLDOUBLE)node->_dir.c[1],(GLDOUBLE)node->_dir.c[2],
//		(GLDOUBLE)node->_upVec.c[0],(GLDOUBLE)node->_upVec.c[1],(GLDOUBLE)node->_upVec.c[2],ViewMat);
//	
//	glGetDoublev(GL_MODELVIEW_MATRIX, ViewMat);
//
//	projPerspective((GLDOUBLE)node->fieldOfView,
//		(GLDOUBLE)node->aspectRatio, // aspectRatio
//		(GLDOUBLE)node->nearFar.p[0],(GLDOUBLE)node->nearFar.p[1], // near, far
//		ProjMat);
//	
//
//	
//	glMatrixMode(GL_MODELVIEW);
//	glPushMatrix();
//	glLoadIdentity();
//	glLoadMatrixd(bias);
//	glMultMatrixd(ProjMat);
//	glMultMatrixd(ViewMat);
//	glGetDoublev(GL_MODELVIEW_MATRIX, TenLinearGexMatCam0);
//	
//	fw_glGetDoublev(GL_MODELVIEW_MATRIX, cViewMat);
//	matinverse(invcViewMat,cViewMat);
//	
//	for(i=0; i<4; i++)
//	{
//		//if(node->description == tg->ProjectiveTextures.data[i].des)
//		if(node->description == data[i].des)
//		{
//			flag = 1;
//			break;
//		}
//		else flag = 0;
//	}
//
//	for(j=0; j<4; j++)
//	{
//		//if(tg->ProjectiveTextures.data[j].des == NULL && !flag)
//		if(data[j].des == NULL && !flag)
//		{
//			//tg->ProjectiveTextures.data[j].des = node->description; 
//			data[j].des = node->description; 
//			////tg->ProjectiveTextures.data[j].TenLinearGexMat = TenLinearGexMatCam0;
//			//memcpy (tg->ProjectiveTextures.data[j].TenLinearGexMat, TenLinearGexMatCam0,16*sizeof (GLDOUBLE));
//			memcpy (data[j].TenLinearGexMat, TenLinearGexMatCam0,16*sizeof (GLDOUBLE));
//			//datacount = j;
//			break;
//		}
//	}
//	/*
//	if(tg->ProjectiveTextures._projTexGenMatCam0_Location != 0 || tg->ProjectiveTextures._projViewMat_Location != 0 || tg->ProjectiveTextures._projMap_forCam1_Location != 0)
//	{
//		convertDbtoFl(tg->ProjectiveTextures.data[datacount].TenLinearGexMat, f_TenLinearGexMatCam0);
//		GLUNIFORMMATRIX4FV (tg->ProjectiveTextures._projTexGenMatCam0_Location,1,GL_FALSE, f_TenLinearGexMatCam0);
//		
//	}*/
//	glPopMatrix();
//
// }


void compile_TextureProjectorPerspective (struct X3D_TextureProjectorPerspective *node) { 


	/* LookAt Matrix Complete */
	struct point_XYZ vec;
	int i;

	for (i=0; i<3; i++) node->_loc.c[i] = node->location.c[i];
	node->_loc.c[3] = 1.0f;/* 1 == this is a position, not a vector */
	
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
	ttglobal tg = gglobal();
	data = (struct projective_Texdata*)tg->ProjectiveTextures.data;
	
	RETURN_IF_RENDER_STATE_NOT_US
	COMPILE_IF_REQUIRED;
	
	if(node->on) {
		double tempmat[16];
		if(node->global) tg->ProjectiveTextures.globalProjector = TRUE;
		//glMatrixMode(GL_MODELVIEW);
		FW_GL_MATRIX_MODE(GL_MODELVIEW);
		//glPushMatrix();
		FW_GL_PUSH_MATRIX();
		//glLoadIdentity();
		FW_GL_LOAD_IDENTITY();
	
		projLookAt((GLDOUBLE)node->_loc.c[0],(GLDOUBLE)node->_loc.c[1],(GLDOUBLE)node->_loc.c[2], 
			(GLDOUBLE)node->_dir.c[0],(GLDOUBLE)node->_dir.c[1],(GLDOUBLE)node->_dir.c[2],
			(GLDOUBLE)node->_upVec.c[0],(GLDOUBLE)node->_upVec.c[1],(GLDOUBLE)node->_upVec.c[2],ViewMat);
	
		//glGetDoublev(GL_MODELVIEW_MATRIX, ViewMat);
		FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX,ViewMat);
		//viewmat is also on the modelview stack
		//printmatrix2(ViewMat,"ViewMat");

		projPerspective((GLDOUBLE)degree,
			(GLDOUBLE)node->aspectRatio, // aspectRatio
			(GLDOUBLE)node->nearDistance,(GLDOUBLE)node->farDistance, // near, far
			ProjMat);
		//printmatrix2(ProjMat,"ProjMat");

		//glMatrixMode(GL_MODELVIEW);
		FW_GL_MATRIX_MODE(GL_MODELVIEW);
		//glPushMatrix();
		FW_GL_PUSH_MATRIX();
		//glLoadIdentity();
		FW_GL_LOAD_IDENTITY();

		//glLoadMatrixd(bias);
 		FW_GL_MULTMATRIX_D(bias);
		//FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX,tempmat);
		//printmatrix2(tempmat,"bias applied");

		//glMultMatrixd(ProjMat);
		FW_GL_MULTMATRIX_D(ProjMat);
		//FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX,tempmat);
		//printmatrix2(tempmat,"projmat applied");

		//glMultMatrixd(ViewMat);
		FW_GL_MULTMATRIX_D(ViewMat);
		//glGetDoublev(GL_MODELVIEW_MATRIX, TenLinearGexMatCam0);
		FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, TenLinearGexMatCam0);
		//printmatrix2(TenLinearGexMatCam0,"TenLinearGexMatCam0");

		//fw_glGetDoublev(GL_MODELVIEW_MATRIX, cViewMat);
		FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, cViewMat);
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
				if(0){
					static double vc11[] = {2.396053, -1.649305, -0.681921, -0.613728, -0.248332, 3.069382, -0.551849, -0.496664, -3.009781, -1.649305, -0.681921, -0.613728, 2.586181, 0.687683, 4.635957, 5.172361};
					for(int k=0;k<4;k++)
					{
						printf("[ ");
						for(int i=0;i<4;i++)
							printf("%lf ",TenLinearGexMatCam0[k*4 +i]);
						printf("]\n");
					}
					if(1) memcpy(TenLinearGexMatCam0,vc11,16*sizeof(double));
					for(int k=0;k<4;k++)
					{
						printf("[ ");
						for(int i=0;i<4;i++)
							printf("%lf ",vc11[k*4 +i]);
						printf("]\n");
					}
					for(int k=0;k<4;k++)
					{
						printf("[ ");
						for(int i=0;i<4;i++)
							printf("%lf ",TenLinearGexMatCam0[k*4 +i]);
						printf("]\n");
					}
				}
				if(1){
					double out4[4], in4[4];
					in4[0] = .2; in4[1] = .2; in4[2] = 0.2; in4[3] = 1.0;
					transformFULL4d(out4,in4,TenLinearGexMatCam0);
					out4[0] /= out4[3];
					out4[1] /= out4[3];
					out4[2] /= out4[3];
					printf("transformed .2 .2 0 = %lf %lf %lf\n",out4[0],out4[1],out4[2]);
					out4[0] /= out4[2];
					out4[1] /= out4[2];
					printf("projected = %lf %lf \n",out4[0],out4[1]);
				}
				memcpy (data[j].TenLinearGexMat, TenLinearGexMatCam0,16*sizeof (GLDOUBLE));
				//datacount = j;
				break;
			}
		}
	
		if(node->texture)
		{
			POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->texture,tmpN);
			render_node(tmpN);
		}
		/*
		if(tg->ProjectiveTextures._projTexGenMatCam0_Location != 0 || tg->ProjectiveTextures._projViewMat_Location != 0 || tg->ProjectiveTextures._projMap_forCam1_Location != 0)
		{
			convertDbtoFl(tg->ProjectiveTextures.data[datacount].TenLinearGexMat, f_TenLinearGexMatCam0);
			GLUNIFORMMATRIX4FV (tg->ProjectiveTextures._projTexGenMatCam0_Location,1,GL_FALSE, f_TenLinearGexMatCam0);
		
		}*/
		//glPopMatrix();
		//I seem to need an extra pop
		FW_GL_POP_MATRIX();
		FW_GL_POP_MATRIX();

	} //if(node->on)
 }


void fin_TextureProjectorPerspective (struct X3D_TextureProjectorPerspective *node) {
}

void prep_TextureProjectorPerspective(struct X3D_TextureProjectorPerspective *node) {


	if (!renderstate()->render_light) return;
	/* this will be a global textureprojector here... */
	render_TextureProjectorPerspective(node);

}
void child_TextureProjectorPerspective (struct X3D_TextureProjectorPerspective *node) {
}


//void prep_PerspectiveProjector (struct X3D_PerspectiveProjector *node) {
//	
//	COMPILE_IF_REQUIRED;
//
//	render_PerspectiveProjector(node);
//}


void fin_PerspectiveProjector (struct X3D_PerspectiveProjector *node) {

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
	
	GLint tex1;
	struct projective_Texdata *data;
	ttglobal tg = gglobal();
	data = (struct projective_Texdata*)tg->ProjectiveTextures.data;
	
	
	
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



// For a Parallel Node
void compile_OrthoProjector (struct X3D_OrthoProjector *node) { 

}

void child_OrthoProjector (struct X3D_OrthoProjector *node) {
}

void prep_OrthoProjector (struct X3D_OrthoProjector *node) {

}

void fin_OrthoProjector (struct X3D_OrthoProjector *node) {

}

// For a ProjectiveTextureGroup Node
void compile_ProjectiveTextureGroup (struct X3D_ProjectiveTextureGroup *node) { 

}

void child_ProjectiveTextureGroup (struct X3D_ProjectiveTextureGroup *node) {
}

void prep_ProjectiveTextureGroup(struct X3D_ProjectiveTextureGroup *node) {

}

void fin_ProjectiveTextureGroup (struct X3D_ProjectiveTextureGroup *node) {

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
		popShaderFlags();
	}
}


