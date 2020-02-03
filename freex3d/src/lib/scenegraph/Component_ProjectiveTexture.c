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

#include "LinearAlgebra.h"
#include "Children.h"
#include <stdlib.h>


struct X3D_Node *tmpN;

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
		*sp = (float) *dp; 	
		sp ++; dp ++;
	}

	memcpy (fmt,spval,16*sizeof (float));
}


// For a Perspective Node
void compile_PerspectiveProjector (struct X3D_PerspectiveProjector *node) { 


	/* LookAt Matrix Complete */
	struct point_XYZ vec;
	int i;

	for (i=0; i<3; i++) node->_loc.c[i] = node->centerOfProjection.c[i];
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



void child_PerspectiveProjector (struct X3D_PerspectiveProjector *node) {

}
void render_PerspectiveProjector (struct X3D_PerspectiveProjector *node) {
	
	int i,j = 0;
	int flag = 0;
	static int datacount = 0;
	/*
	if(node->texture)
	{
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->texture,tmpN);
		render_node(tmpN);
	}
	*/
	GLDOUBLE cViewMat[16];
	GLDOUBLE invcViewMat[16];
	GLDOUBLE ViewMat[16];
	GLDOUBLE ProjMat[16];
	
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

	projPerspective((GLDOUBLE)node->fieldOfView,
		(GLDOUBLE)node->aspectRatio, // aspectRatio
		(GLDOUBLE)node->nearFar.p[0],(GLDOUBLE)node->nearFar.p[1], // near, far
		ProjMat);
	

	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glLoadMatrixd(bias);
	glMultMatrixd(ProjMat);
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
	/*
	if(tg->ProjectiveTextures._projTexGenMatCam0_Location != 0 || tg->ProjectiveTextures._projViewMat_Location != 0 || tg->ProjectiveTextures._projMap_forCam1_Location != 0)
	{
		convertDbtoFl(tg->ProjectiveTextures.data[datacount].TenLinearGexMat, f_TenLinearGexMatCam0);
		GLUNIFORMMATRIX4FV (tg->ProjectiveTextures._projTexGenMatCam0_Location,1,GL_FALSE, f_TenLinearGexMatCam0);
		
	}*/
	glPopMatrix();

 }


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

	projPerspective((GLDOUBLE)degree,
		(GLDOUBLE)node->aspectRatio, // aspectRatio
		(GLDOUBLE)node->nearDistance,(GLDOUBLE)node->farDistance, // near, far
		ProjMat);
	

	
	//glMatrixMode(GL_MODELVIEW);
	FW_GL_MATRIX_MODE(GL_MODELVIEW);
	//glPushMatrix();
	FW_GL_PUSH_MATRIX();
	//glLoadIdentity();
	FW_GL_LOAD_IDENTITY();

	//glLoadMatrixd(bias);
 	FW_GL_TRANSFORM_D(bias);
	//glMultMatrixd(ProjMat);
	FW_GL_TRANSFORM_D(ProjMat);
	//glMultMatrixd(ViewMat);
	FW_GL_TRANSFORM_D(ViewMat);
	//glGetDoublev(GL_MODELVIEW_MATRIX, TenLinearGexMatCam0);
	FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, TenLinearGexMatCam0);
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


 }


void fin_TextureProjectorPerspective (struct X3D_TextureProjectorPerspective *node) {
}

void prep_TextureProjectorPerspective(struct X3D_TextureProjectorPerspective *node) {

	COMPILE_IF_REQUIRED;

	render_TextureProjectorPerspective(node);

}
void child_TextureProjectorPerspective (struct X3D_TextureProjectorPerspective *node) {
}


void prep_PerspectiveProjector (struct X3D_PerspectiveProjector *node) {
	
	COMPILE_IF_REQUIRED;

	render_PerspectiveProjector(node);
}


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