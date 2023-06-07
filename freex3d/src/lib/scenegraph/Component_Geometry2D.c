/*


X3D Geometry2D  Component

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

#include "Collision.h"
#include "LinearAlgebra.h"
#include "../opengl/Frustum.h"
#include "../opengl/Material.h"
#include "Component_Geometry3D.h"
#include "../opengl/OpenGL_Utils.h"
#include "../opengl/Textures.h"

#include "Component_Shape.h"
#include "../scenegraph/RenderFuncs.h"
#include "../x3d_parser/Bindable.h"
#include "Polyrep.h"

#include <float.h>
#if defined(_MSC_VER) && _MSC_VER < 1500
#define cosf cos
#define sinf sin
#endif
#define SEGMENTS_PER_CIRCLE 36
#define PIE 10
#define CHORD 20
#define NONE 30

static void *createLines (float start, float end, float radius, int closed, int *size,float *_extent);

#define COMPILE_AND_GET_BOUNDS(myType,myField) \
void compile_##myType (struct X3D_##myType *node){ \
	float myminx = FLT_MAX; \
	float mymaxx = -FLT_MAX; \
	float myminy = FLT_MAX; \
	float mymaxy = -FLT_MAX; \
	int count; \
 \
	if (node->myField.n<=0) { \
		node->EXTENT_MIN_X = 0.0f; \
		node->EXTENT_MAX_X = 0.0f; \
		node->EXTENT_MIN_Y = 0.0f; \
		node->EXTENT_MAX_Y = 0.0f; \
	} else { \
		for (count = 0; count < node->myField.n; count++) { \
			if (node->myField.p[count].c[0] > mymaxx) mymaxx = node->myField.p[count].c[0]; \
			if (node->myField.p[count].c[0] < myminx) myminx = node->myField.p[count].c[0]; \
			if (node->myField.p[count].c[1] > mymaxy) mymaxy = node->myField.p[count].c[1]; \
			if (node->myField.p[count].c[1] < myminy) myminy = node->myField.p[count].c[1]; \
		} \
		node->EXTENT_MAX_X = mymaxx; \
		node->EXTENT_MIN_X = myminx; \
		node->EXTENT_MAX_Y = mymaxy; \
		node->EXTENT_MIN_Y = myminy; \
	} \
 \
	MARK_NODE_COMPILED \
}
/***********************************************************************************/
void* set_LineRep(void *_linerep, struct SFVec3f *points, struct SFVec2f *points2D, 
		struct SFColorRGBA *colorRgba, struct SFColor *color, float *fog,
		int nsegments, int *counts, int *starts);
void clear_LineRep(void *_linerep);
void render_LineRep(struct X3D_LineRep *linerep);

void compile_Arc2D (struct X3D_Arc2D *node) {
       /*  have to regen the shape*/
	struct SFVec2f *tmpptr_a, *tmpptr_b;
	int tmpint;
	static int start[1];

	MARK_NODE_COMPILED
	
	tmpint = 0;
	clear_LineRep(node->_intern);
	tmpptr_a = createLines (node->startAngle, node->endAngle, node->radius, NONE, &tmpint, node->_extent);

	/* perform the switch - worry about threading here without locking */
	node->__numPoints = 0;		/* tell us that it has zero points */
	tmpptr_b = node->__points.p;	/* old set of points, for freeing later */
	node->__points.p = tmpptr_a;	/* new points */
	node->__numPoints = tmpint;
	FREE_IF_NZ (tmpptr_b);
	/* switch completed */
	start[0] = 0;
	node->_intern = set_LineRep(node->_intern,NULL,node->__points.p,NULL,NULL,NULL,1,&node->__numPoints,start);
	
}

void render_Arc2D (struct X3D_Arc2D *node) {
	ttglobal tg = gglobal();
	COMPILE_IF_REQUIRED
	if (node->__numPoints>0) {	
		/* for BoundingBox calculations */
		setExtent( node->EXTENT_MAX_X, node->EXTENT_MIN_X, 
			node->EXTENT_MAX_Y, node->EXTENT_MIN_Y, 0.0f,0.0f,X3D_NODE(node));

	    LIGHTING_OFF
	    DISABLE_CULL_FACE
		render_LineRep((struct X3D_LineRep*)node->_intern);
		tg->Mainloop.trisThisLoop += node->__numPoints;
	}
}

/***********************************************************************************/
void compile_ArcClose2D (struct X3D_ArcClose2D *node){
        /*  have to regen the shape*/
	char *ct;
	struct SFVec2f *fp, *tp;
	//GLfloat *tp;
	struct SFVec2f *sfp, *stp;
	//GLfloat *stp;
	struct SFVec2f *ofp, *otp;
	//GLfloat *otp;
	int i,j,k;
	GLfloat id;
	GLfloat od;
	int tmpint;
	int simpleDisc;
	int closure;
	ushort *lindex;
	float start, end, radius, angle, angle_increment;
	int numPoints, arcpoints;

	MARK_NODE_COMPILED


	ct = node->closureType->strptr;
	//xx = node->closureType->len;
	tmpint = 0;

	if (strcmp(ct,"PIE") == 0) {
		closure = PIE;
	} else if (strcmp(ct,"CHORD") == 0) {
		closure = CHORD;
	} else {
		printf ("ArcClose2D, closureType %s invalid\n",node->closureType->strptr);
	}
	start = node->startAngle;
	end = node->endAngle;
	radius = node->radius;
	/* is this a circle? */
	simpleDisc =  APPROX(start,end);

	/* bounds check, and sort values */
	if(end < start)
		end += 2.0*PI;

	if (radius < 0.0) radius = 1.0f;

	if(0) if (start > end) {
		float tmp = start;
		start = end;
		end = tmp;
	}

	if (simpleDisc) {
		numPoints = SEGMENTS_PER_CIRCLE;
	} else {
		numPoints = (int) ((float)(SEGMENTS_PER_CIRCLE * (end - start))/(PI*2.0f));
		numPoints++; //one more point than segments
		if (numPoints>SEGMENTS_PER_CIRCLE) numPoints=SEGMENTS_PER_CIRCLE;
	}
	arcpoints = numPoints;
	//add one point for pie center or half-chord - we'll fan from this point.
	numPoints ++;

	tmpint = SEGMENTS_PER_CIRCLE+2;
	fp = sfp = MALLOC (struct SFVec2f *, sizeof(struct SFVec2f) * (numPoints));
	tp = stp = MALLOC (struct SFVec2f *, sizeof(struct SFVec2f) * (numPoints)); 
	lindex = MALLOC (ushort *, sizeof(ushort) * (numPoints*2)*2); //over malloc by a few. should be nsegs * 2 lines/seg * 2 lineEnds/line
	//if(!node->_gc) node->_gc = newVector(void *,4); H: FreeWRLPTR gets freed, no need for _gc
	//vector_pushBack(void*,node->_gc,lindex);

	/* initial TriangleFan point */
	(*fp).c[0] = 0.0f; (*fp).c[1] = 0.0f; fp++;
	(*tp).c[0] = 0.5f; (*tp).c[1] = 0.5f; tp++;

	angle = start;
	angle_increment = (end - start)/(float)(arcpoints -1);
	for (i=0,k=0,j=1;i<arcpoints;i++,k+=4,j++) {
		float x,y;
		x = cosf(angle);
		y = sinf(angle);
		(*fp).c[0] = node->radius * x;
		(*fp).c[1] = node->radius * y;
		fp++;

		lindex[k + 0] = 0;
		lindex[k + 1] = j;
		lindex[k + 2] = j;
		lindex[k + 3] = j+1;

		(*tp).c[0] = 0.5f + x*.5f; //center 0,0 in middle of texture
		(*tp).c[1] = 0.5f + y*.5f;	
		tp++;
		angle += angle_increment;
		angle = max(angle, start);
	}
	if(closure == CHORD){
		sfp[0].c[0] = .5f * (sfp[1].c[0] + sfp[arcpoints].c[0]); 
		sfp[0].c[1] = .5f * (sfp[1].c[1] + sfp[arcpoints].c[1]); 
		stp[0].c[0] = .5f * (stp[1].c[0] + stp[arcpoints].c[0]); 
		stp[0].c[1] = .5f * (stp[1].c[1] + stp[arcpoints].c[1]); 
	}
	node->__wireindices = lindex;


	/* compiling done, set up for rendering. thread safe */
	node->__numPoints = 0;
	ofp = node->__points.p;
	otp = node->__texCoords.p;
	node->__points.p = sfp;
	node->__texCoords.p = stp;
	node->__simpleDisk = simpleDisc;
	node->__numPoints = numPoints;
	FREE_IF_NZ (ofp);
	FREE_IF_NZ (otp);

	/* we can set the extents here... */
	{
        float myminx = FLT_MAX;
        float mymaxx = -FLT_MAX;
        float myminy = FLT_MAX;
        float mymaxy = -FLT_MAX;
		for (i=0; i<numPoints; i++) {
			/* do X first */
			if (sfp[i].c[0] > mymaxx) mymaxx = sfp[i].c[0];
			if (sfp[i].c[0] < myminx) myminx = sfp[i].c[0];
			fp++;
			/* do Y second */
			if (sfp[i].c[1] > mymaxy) mymaxy = sfp[i].c[1];
			if (sfp[i].c[1] < myminy) myminy = sfp[i].c[1];
		}

		node->EXTENT_MAX_X = myminx; //node->radius;
		node->EXTENT_MIN_X = mymaxx; // -node->radius;
		node->EXTENT_MAX_Y = myminy; //node->radius;
		node->EXTENT_MIN_Y = mymaxy; //-node->radius;
	}
}
#define DESIRE(whichOne,zzz) ((whichOne & zzz)==zzz)
void render_ArcClose2D (struct X3D_ArcClose2D *node){
	COMPILE_IF_REQUIRED
	if (node->__numPoints>0) {	
		struct textureVertexInfo mtf = {(GLfloat *)node->__texCoords.p,2,GL_FLOAT,0,NULL,NULL};
		/* for BoundingBox calculations */
		setExtent( node->EXTENT_MAX_X, node->EXTENT_MIN_X, 
			node->EXTENT_MAX_Y, node->EXTENT_MIN_Y, 0.0f,0.0f,X3D_NODE(node));

		CULL_FACE(node->solid)

		textureCoord_send(&mtf);
		FW_GL_VERTEX_POINTER (2,GL_FLOAT,0,(GLfloat *)node->__points.p);


		/* do the array drawing; sides are simple 0-1-2-3, 4-5-6-7, etc quads */
		if(DESIRE(getShaderFlags().base,SHADINGSTYLE_WIRE)){
			//wireframe triangles
			sendElementsToGPU(GL_LINES,((node->__numPoints-1)*4 -1 ),node->__wireindices); //should be segs x 2 lines/seg = (pts-1) x 2 lines / pt
		}else{
			sendArraysToGPU (GL_TRIANGLE_FAN, 0, node->__numPoints);
		}

		gglobal()->Mainloop.trisThisLoop += node->__numPoints;
	}
}

void compile_ArcClose2D_LINE (struct X3D_ArcClose2D *node) {
	//int xx;
	char *ct;
	struct SFVec2f *tmpptr_a, *tmpptr_b;
	int tmpint;

        /*  have to regen the shape*/
	MARK_NODE_COMPILED
		
	ct = node->closureType->strptr;
	//xx = node->closureType->len;
	tmpint = 0;
	tmpptr_a = NULL;

	if (strcmp(ct,"PIE") == 0) {
		tmpptr_a = createLines (node->startAngle,
			node->endAngle, node->radius, PIE, &tmpint,node->_extent);
	} else if (strcmp(ct,"CHORD") == 0) {
		tmpptr_a = createLines (node->startAngle,
			node->endAngle, node->radius, CHORD, &tmpint,node->_extent);
	} else {
		printf ("ArcClose2D, closureType %s invalid\n",node->closureType->strptr);
	}

	/* perform the switch - worry about threading here without locking */
	node->__numPoints = 0;		/* tell us that it has zero points */
	tmpptr_b = node->__points.p;	/* old set of points, for freeing later */
	node->__points.p = tmpptr_a;	/* new points */
	node->__numPoints = tmpint;
	FREE_IF_NZ (tmpptr_b);
	/* switch completed */
}


void render_ArcClose2D_LINE (struct X3D_ArcClose2D *node) {
	ttglobal tg = gglobal();
	COMPILE_IF_REQUIRED
	if (node->__numPoints>0) {	
		/* for BoundingBox calculations */
		setExtent( node->EXTENT_MAX_X, node->EXTENT_MIN_X, 
			node->EXTENT_MAX_Y, node->EXTENT_MIN_Y, 0.0f,0.0f,X3D_NODE(node));

	        LIGHTING_OFF
	        DISABLE_CULL_FACE


		FW_GL_VERTEX_POINTER (2,GL_FLOAT,0,(GLfloat *)node->__points.p);
        	sendArraysToGPU (GL_LINE_STRIP, 0, node->__numPoints);

		gglobal()->Mainloop.trisThisLoop += node->__numPoints;
	}
}
int isLeftSide2f(float* p1, float* p2, float* px) {
//https://en.wikipedia.org/wiki/Cross_product 
//vector 1 v1 = b - a
//vector 2 v2 = c - a
//sin(angle) = | v2xv1 / (| v1 | *| v2 | ) |

	float v1[3];
	float v2[3];
	float v3[3];
	vecset3f(v1, 0.0f, 0.0f, 0.0f);
	vecset3f(v2, 0.0f, 0.0f, 0.0f);
	vecdif2f(v1, p2, p1);
	vecdif2f(v2, px, p1);
	veccross3f(v3, v1, v2);
	vecscale3f(v3, v3, 1.0f / (veclength3f(v1) * veclength3f(v2)));  //sine(angle)
	float sineangle = v3[2];
	return sineangle < 0 ? -1 : (sineangle > 0 ? 1 : 0); //1 left -1 right 0 on-line
}
BOOL angleCounterClockwiseBetween(float a0, float a1, float angle) {
	// a0 < angle < a1 ? TRUE : FALSE
	// technique - get them all +ve angles and a1, angle > a0
	// but I invented this technique in 5 minutes, if not working please fix - dug9
	float na0, na1, nangle; //normalized angles
	na0 = atan2(sin(a0), cos(a0)) + 2*PI;
	na1 = atan2(sin(a1), cos(a1)) + 2*PI;
	nangle = atan2(sin(angle), cos(angle)) + 2*PI;
	if (na1 < na0) na1 += 2 * PI;
	if (nangle < na0) nangle += 2 * PI;
	if (nangle > na0 && nangle < na1) return TRUE;
	return FALSE;
}
void rendray_ArcClose2D(struct X3D_ArcClose2D* node) {
	//copy from rendray_Cylinder and hack
	float r, a0,a1, z;
	struct point_XYZ t_r1, t_r2;
	get_current_ray(&t_r1, &t_r2);

	r = node->radius;
	a0 = node->startAngle;
	a1 = node->endAngle;
	z = 0.0f;
	/* Caps */
	if (!ZEQ) {
		float zrat0 = (float)ZRAT(z);
		if (TRAT(zrat0)) {
			float cx = (float)MRATX(zrat0);
			float cy = (float)MRATY(zrat0);
			float rhit2 = cx * cx + cy * cy;
			if (r * r > rhit2 ) {
				//inside circle
				float angle = atan2(cy, cx);
				if(angleCounterClockwiseBetween(a0,a1,angle)){
					//inside pie
					if(!strcmp(node->closureType->strptr,"PIE"))
						rayhit(zrat0, cx, cy, z, 0, 0, 1, -1, -1, "arcclose2dpie");
					else {
						//closuretype chord
						//hypothesis if hitpoint is to the right of clockwise chord [start - end], then its inside
						float p1[2], p2[2], px[2];
						p1[0] = r * cos(a0);
						p1[1] = r * sin(a0);
						p2[0] = r * cos(a1);
						p2[1] = r * sin(a1);
						px[0] = cx;
						px[1] = cy;
						if (isLeftSide2f(p1, p2, px) < 0) {
							rayhit(zrat0, cx, cy, z, 0, 0, 1, -1, -1, "arcclose2dchord");
						}
					}
				}
			}
		}
	}
}

/***********************************************************************************/

void compile_Circle2D (struct X3D_Circle2D *node) {
	struct SFVec2f *tmpptr_a, *tmpptr_b;
	int tmpint;
	static int start[1];
       /*  have to regen the shape*/
	MARK_NODE_COMPILED
	
	clear_LineRep(node->_intern);
	tmpptr_a = createLines (0.0f, 0.0f, node->radius, NONE, &tmpint,node->_extent);

	/* perform the switch - worry about threading here without locking */
	node->__numPoints = 0;		/* tell us that it has zero points */
	tmpptr_b = node->__points.p;	/* old set of points, for freeing later */
	node->__points.p = tmpptr_a;	/* new points */
	node->__numPoints = tmpint;
	FREE_IF_NZ (tmpptr_b);
	/* switch completed */
	start[0] = 0;
	node->_intern = set_LineRep(node->_intern,NULL,node->__points.p,NULL,NULL,NULL,1,&node->__numPoints,start);
}

void render_Circle2D (struct X3D_Circle2D *node) {
	ttglobal tg = gglobal();
	COMPILE_IF_REQUIRED
	if (node->__numPoints>0) {	
		/* for BoundingBox calculations */
		setExtent( node->EXTENT_MAX_X, node->EXTENT_MIN_X, 
			node->EXTENT_MAX_Y, node->EXTENT_MIN_Y, 0.0f,0.0f,X3D_NODE(node));

	    LIGHTING_OFF
	    DISABLE_CULL_FACE
		render_LineRep((struct X3D_LineRep*)node->_intern);
		gglobal()->Mainloop.trisThisLoop += node->__numPoints;
	}
}

/***********************************************************************************/


//COMPILE_AND_GET_BOUNDS(Polyline2D,lineSegments)
float * extent6f_from_box2fn(float *extent6,float *p, int n);
void compile_Polyline2D (struct X3D_Polyline2D *node){
	static int start[1];
	extent6f_from_box2fn(node->_extent,(float*)node->lineSegments.p,node->lineSegments.n);
	MARK_NODE_COMPILED
	start[0] = 0;
	node->_intern = set_LineRep(node->_intern,NULL,node->lineSegments.p,NULL,NULL,NULL,1,&node->lineSegments.n,start);
}

void render_Polyline2D (struct X3D_Polyline2D *node){
	ttglobal tg = gglobal();

	COMPILE_IF_REQUIRED
	if (node->lineSegments.n>0) {
		/* for BoundingBox calculations */
		setExtent( node->EXTENT_MAX_X, node->EXTENT_MIN_X, 
			node->EXTENT_MAX_Y, node->EXTENT_MIN_Y, 0.0f,0.0f,X3D_NODE(node));

	        LIGHTING_OFF
	        DISABLE_CULL_FACE

		render_LineRep((struct X3D_LineRep*)node->_intern);
		gglobal()->Mainloop.trisThisLoop += node->lineSegments.n;
	}
}

/***********************************************************************************/

void compile_Polypoint2D(struct X3D_Polypoint2D* node) {
	int npoint = 0;
	float* points = NULL;

	/* do nothing, except get the extents here */
	MARK_NODE_COMPILED
	if (node->point.n > 0) {
		points = (float *)node->point.p;
		npoint = node->point.n;
	}
	findExtentInCoord0(X3D_NODE(node), npoint, points, 2);
	if(npoint)
		node->_intern = set_PointRep(node->_intern, points, 2, npoint, NULL, 4,0,NULL,0);
}

void render_Polypoint2D (struct X3D_Polypoint2D *node){
	ttglobal tg = gglobal();

	COMPILE_IF_REQUIRED

		LIGHTING_OFF
		DISABLE_CULL_FACE
	setExtent(node->EXTENT_MAX_X, node->EXTENT_MIN_X, node->EXTENT_MAX_Y,
			node->EXTENT_MIN_Y, node->EXTENT_MAX_Z, node->EXTENT_MIN_Z,
			X3D_NODE(node));

	if (!node->_intern) return;
	render_PointRep(node->_intern);
}

/***********************************************************************************/

void compile_Disk2D (struct X3D_Disk2D *node){
        /*  have to regen the shape*/
	struct SFVec2f *fp, *tp;
	//GLfloat *tp;
	struct SFVec2f *sfp, *stp;
	//GLfloat *stp;
	struct SFVec2f *ofp, *otp;
	//GLfloat *otp;
	int i,j,k;
	GLfloat id;
	GLfloat od;
	int tmpint;
	int simpleDisc;
	ushort *lindex;

	MARK_NODE_COMPILED


	/* bounds checking */
	if (node->innerRadius<0) {node->__numPoints = 0; return;}
	if (node->outerRadius<0) {node->__numPoints = 0; return;}

	/* is this a simple disc ? */
	if ((APPROX (node->innerRadius, 0.0)) || 
		(APPROX(node->innerRadius,node->outerRadius))) simpleDisc = TRUE;
	else simpleDisc = FALSE;

	/* is this a simple disk, or one with an inner circle cut out? */
	if (simpleDisc) {
		tmpint = SEGMENTS_PER_CIRCLE+2;
		fp = sfp = MALLOC (struct SFVec2f *, sizeof(struct SFVec2f) * (tmpint));
		tp = stp = MALLOC (struct SFVec2f *, sizeof(struct SFVec2f) * (tmpint)); //(GLfloat *, sizeof(GLfloat) * 2 * (tmpint));
		lindex = MALLOC (ushort *, sizeof(ushort) * (tmpint*2)*2); //over malloc by a few. should be nsegs * 2 lines/seg * 2 lineEnds/line
		//if(!node->_gc) node->_gc = newVector(void *,4); H: FreeWRLPTR gets freed, no need for _gc
		//vector_pushBack(void*,node->_gc,lindex);

		/* initial TriangleFan point */
		(*fp).c[0] = 0.0f; (*fp).c[1] = 0.0f; fp++;
		(*tp).c[0] = 0.5f; (*tp).c[1] = 0.5f; tp++;
		id = 2.0f;

		for (i=SEGMENTS_PER_CIRCLE,j=1,k=0; i >= 0; i--,j++,k+=4) {
			(*fp).c[0] = node->outerRadius * sinf(((float)PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE));
			(*fp).c[1] = node->outerRadius * cosf(((float)PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE));	
			fp++;

			lindex[k + 0] = 0;
			lindex[k + 1] = j;
			lindex[k + 2] = j;
			lindex[k + 3] = j+1;

			(*tp).c[0] = 0.5f + (sinf(((float)PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE))/id);
			(*tp).c[1] = 0.5f + (cosf(((float)PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE))/id);	
			tp++;
		}
		node->__wireindices = lindex;
	} else {
		tmpint = (SEGMENTS_PER_CIRCLE+1) * 2;
		fp = sfp = MALLOC (struct SFVec2f *, sizeof(struct SFVec2f) * 2 * tmpint);
		tp = stp = MALLOC (struct SFVec2f *, sizeof(struct SFVec2f) * (tmpint)); //MALLOC (GLfloat *, sizeof(GLfloat) * 2 * tmpint);
		lindex = MALLOC (ushort *, sizeof(ushort) * (tmpint*2) *2); //over malloc by a few, should be (nseg-1)*4 lines/seg * 2 lineEnds per line
		//if(!node->_gc) node->_gc = newVector(void *,4);
		//vector_pushBack(void*,node->_gc,lindex);

		/* texture scaling params */
		od = 2.0f;
		id = node->outerRadius * 2.0f / node->innerRadius;

		for (i=SEGMENTS_PER_CIRCLE,j=0,k=0; i >= 0; i--,j+=2,k+=8) {
			(*fp).c[0] = node->innerRadius * (float) sinf(((float)PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE));
			(*fp).c[1] = node->innerRadius * (float) cosf(((float)PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE));	
			fp++;
			(*fp).c[0] = node->outerRadius * (float) sinf(((float)PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE));
			(*fp).c[1] = node->outerRadius * (float) cosf(((float)PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE));	
			fp++;

			lindex[k + 0] = j;
			lindex[k + 1] = j+1;
			lindex[k + 2] = j+1;
			lindex[k + 3] = j+2;
			lindex[k + 4] = j+2;
			lindex[k + 5] = j;
			lindex[k + 6] = j+1;
			lindex[k + 7] = j+3;

			(*tp).c[0] = 0.5f + ((float)sinf(((float)PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE))/id);
			(*tp).c[1] = 0.5f + ((float)cosf(((float)PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE))/id);	
			tp++;
			(*tp).c[0] = 0.5f + ((float)sinf(((float)PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE))/od);
			(*tp).c[1] = 0.5f + ((float)cosf(((float)PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE))/od);
			tp++;
		}
		node->__wireindices = lindex;
	}


	/* compiling done, set up for rendering. thread safe */
	node->__numPoints = 0;
	ofp = node->__points.p;
	otp = node->__texCoords.p;
	node->__points.p = sfp;
	node->__texCoords.p = stp;
	node->__simpleDisk = simpleDisc;
	node->__numPoints = tmpint;
	FREE_IF_NZ (ofp);
	FREE_IF_NZ (otp);

	/* we can set the extents here... */
	node->EXTENT_MAX_X = node->outerRadius;
	node->EXTENT_MIN_X = -node->outerRadius;
	node->EXTENT_MAX_Y = node->outerRadius;
	node->EXTENT_MIN_Y = -node->outerRadius;
}

void render_Disk2D (struct X3D_Disk2D *node){
	COMPILE_IF_REQUIRED
	if (node->__numPoints>0) {	
		struct textureVertexInfo mtf = {(GLfloat *)node->__texCoords.p,2,GL_FLOAT,0,NULL,NULL};
		/* for BoundingBox calculations */
		setExtent( node->EXTENT_MAX_X, node->EXTENT_MIN_X, 
			node->EXTENT_MAX_Y, node->EXTENT_MIN_Y, 0.0f,0.0f,X3D_NODE(node));

		CULL_FACE(node->solid)

		textureCoord_send(&mtf);
		FW_GL_VERTEX_POINTER (2,GL_FLOAT,0,(GLfloat *)node->__points.p);


		/* do the array drawing; sides are simple 0-1-2-3, 4-5-6-7, etc quads */
		if (node->__simpleDisk) {
			if(DESIRE(getShaderFlags().base,SHADINGSTYLE_WIRE)){
				//wireframe triangles
				sendElementsToGPU(GL_LINES,((node->__numPoints-1)*4 -1 ),node->__wireindices); //should be segs x 2 lines/seg = (pts-1) x 2 lines / pt
			}else{
				sendArraysToGPU (GL_TRIANGLE_FAN, 0, node->__numPoints);
			}
		}
		else{
			if(DESIRE(getShaderFlags().base,SHADINGSTYLE_WIRE)){
				//wireframe triangles
				sendElementsToGPU(GL_LINES,(node->__numPoints*4 -4 -1),node->__wireindices); //(nseg -1)*4 = (npts-2)*2 = npts*2 -4
			}else{
				sendArraysToGPU (GL_TRIANGLE_STRIP, 0, node->__numPoints);
			}
		}

		gglobal()->Mainloop.trisThisLoop += node->__numPoints;
	}
}
void rendray_Disk2D(struct X3D_Disk2D* node) {
	//copy from rendray_Cylinder and hack
	float ri,ro, z;
	struct point_XYZ t_r1, t_r2;
	get_current_ray(&t_r1, &t_r2);

	ri = node->innerRadius;
	ro = node->outerRadius;
	z = 0.0f;
	/* Caps */
	if (!ZEQ) {
		float zrat0 = (float)ZRAT(z);
		if (TRAT(zrat0)) {
			float cx = (float)MRATX(zrat0);
			float cy = (float)MRATY(zrat0);
			float rhit2 = cx * cx + cy * cy;
			if (ro * ro > rhit2 && ri * ri < rhit2) {
				rayhit(zrat0, cx, cy, z, 0, 0, 1, -1, -1, "disk2d");
			}
		}
	}
}

/***********************************************************************************/

void compile_TriangleSet2D (struct X3D_TriangleSet2D *node){
        /*  have to regen the shape*/
	GLfloat maxX, minX;
	GLfloat maxY, minY;
	GLfloat Ssize, Tsize;
	int i,j;
	ushort *lindex;
	struct SFVec2f *fp; //GLfloat *fp;
	int tmpint;

	MARK_NODE_COMPILED

	/* do we have vertex counts in sets of 3? */
	if ((node->vertices.n %3) != 0) {
		printf ("TriangleSet2D, have incorrect vertex count, %d\n",node->vertices.n);
		node->vertices.n -= node->vertices.n % 3;
	}

	/* save this, and tell renderer that this has 0 vertices (threading stuff) */
	tmpint = node->vertices.n;
	node->vertices.n = 0;

	/* ok, now if renderer renders (threading) it'll see zero, so we are safe */
	FREE_IF_NZ (node->__texCoords.p);
	node->__texCoords.p = fp = MALLOC (struct SFVec2f *, sizeof(struct SFVec2f) * (tmpint)); //MALLOC (GLfloat *, sizeof (GLfloat) * tmpint * 2);
	node->__texCoords.n = tmpint;
	node->__wireindices = lindex = MALLOC (ushort *, sizeof(ushort)*(tmpint+1)*2); //over malloc a bit, should be: pts = lines, lines * 2 ends/line
	/* find min/max values for X and Y axes */
	minY = minX = FLT_MAX;
	maxY = maxX = -FLT_MAX;
	for (i=0; i<tmpint; i++) {
		if (node->vertices.p[i].c[0] < minX) minX = node->vertices.p[i].c[0];
		if (node->vertices.p[i].c[1] < minY) minY = node->vertices.p[i].c[1];
		if (node->vertices.p[i].c[0] > maxX) maxX = node->vertices.p[i].c[0];
		if (node->vertices.p[i].c[1] > maxY) maxY = node->vertices.p[i].c[1];
	}

	/* save these numbers for extents */
	node->EXTENT_MAX_X = maxX;
	node->EXTENT_MIN_X = minX;
	node->EXTENT_MAX_Y = maxY;
	node->EXTENT_MIN_Y = minY;

	/* printf ("minX %f maxX %f minY %f maxY %f\n",minX, maxX, minY, maxY); */
	Ssize = maxX - minX;
	Tsize = maxY - minY;
	/* printf ("ssize %f tsize %f\n",Ssize, Tsize); */

	for (i=0,j=0; i<tmpint/3; i++,j+=6) {
		//wireframe indices
		int i3 = i*3;
		lindex[j + 0] = i3;
		lindex[j + 1] = i3+1;
		lindex[j + 2] = i3+1;
		lindex[j + 3] = i3+2;
		lindex[j + 4] = i3+2;
		lindex[j + 5] = i3;
	}

	for (i=0; i<tmpint; i++) {
		(*fp).c[0] = (node->vertices.p[i].c[0] - minX) / Ssize;
		(*fp).c[1] = (node->vertices.p[i].c[1] - minY) / Tsize; 
		fp++;
	}

	/* restore, so we know how many tris there are */
	node->vertices.n = tmpint;
}

void render_TriangleSet2D (struct X3D_TriangleSet2D *node){
	COMPILE_IF_REQUIRED
	if (node->vertices.n>0) {	
		struct textureVertexInfo mtf = {(GLfloat *)node->__texCoords.p,2,GL_FLOAT,0,NULL,NULL};
		/* for BoundingBox calculations */
		setExtent( node->EXTENT_MAX_X, node->EXTENT_MIN_X, 
			node->EXTENT_MAX_Y, node->EXTENT_MIN_Y, 0.0f,0.0f,X3D_NODE(node));

		CULL_FACE(node->solid)

		textureCoord_send(&mtf);
		FW_GL_VERTEX_POINTER (2,GL_FLOAT,0,(GLfloat *)node->vertices.p);


		if(DESIRE(getShaderFlags().base,SHADINGSTYLE_WIRE)){
			//wireframe triangles
			sendElementsToGPU(GL_LINES,(node->vertices.n*2),node->__wireindices); //(nseg -1)*4 = (npts-2)*2 = npts*2 -4
		}else{
			sendArraysToGPU (GL_TRIANGLES, 0, node->vertices.n);
		}

		gglobal()->Mainloop.trisThisLoop += node->vertices.n;
	}
}
//rendray_TriangleSet2D
void rendray_TriangleSet2D(struct X3D_TriangleSet2D* node) {
	//copy from rendray_Cylinder and hack
	float r, a0, a1, z;
	struct point_XYZ t_r1, t_r2;
	get_current_ray(&t_r1, &t_r2);

	z = 0.0f;
	if (!ZEQ) {
		float zrat0 = (float)ZRAT(z);
		if (TRAT(zrat0)) {
			float cx = (float)MRATX(zrat0);
			float cy = (float)MRATY(zrat0);
			float px[2];
			px[0] = cx;
			px[1] = cy;
			int iside[3];
			struct SFVec2f* pp = node->vertices.p;
			for (int i = 0; i < node->vertices.n; i += 3) {
				//assuming clockwise vertices around triangle,
				//if hitpoint is to the right of all 3 triangle sides, its inside
				iside[0] = isLeftSide2f(pp[i].c, pp[i + 1].c, px);
				iside[1] = isLeftSide2f(pp[i + 1].c, pp[i + 2].c, px);
				iside[2] = isLeftSide2f(pp[i + 2].c, pp[i].c, px);
				//printf("i %d isides %d %d %d\n", i, iside[0], iside[1], iside[2]);
				if (iside[0] <= 0 && iside[1] <=0 && iside[2] <= 0) {
					rayhit(zrat0, cx, cy, z, 0, 0, 1, -1, -1, "triangleset2d");
					break;
				}
				//assuming counter-clockwise vertices around triangle,
				//if hitpoint is to the left of all 3 triangle sides, its inside
				if (iside[0] >= 0 && iside[1] >= 0 && iside[2] >= 0) {
					rayhit(zrat0, cx, cy, z, 0, 0, 1, -1, -1, "triangleset2d");
					break;
				}

			}
		}
	}
}


/***********************************************************************************/


/* this code is remarkably like Box, but with a zero z axis. */
void compile_Rectangle2D (struct X3D_Rectangle2D *node) {
	float *pt;
	struct SFVec3f *ptr;
	float x = ((node->size).c[0])/2;
	float y = ((node->size).c[1])/2;

	MARK_NODE_COMPILED

	/*  MALLOC memory (if possible)*/
	if (!node->__points.p) ptr = MALLOC (struct SFVec3f *,sizeof(struct SFVec3f)*(6));
	else ptr = node->__points.p;

	/*  now, create points; 6 points per face.*/
	pt = (float *) ptr;
	#define PTF0 *pt++ =  x; *pt++ =  y; *pt++ =  0.0f;
	#define PTF1 *pt++ = -x; *pt++ =  y; *pt++ =  0.0f;
	#define PTF2 *pt++ = -x; *pt++ = -y; *pt++ =  0.0f;
	#define PTF3 *pt++ =  x; *pt++ = -y; *pt++ =  0.0f;

	PTF0 PTF1 PTF2  PTF0 PTF2 PTF3 /* front */
	/* finished, and have good data */
	node->__points.p = (struct SFVec3f*) ptr;

	#undef PTF0
	#undef PTF1
	#undef PTF2
	#undef PTF3
}


void render_Rectangle2D (struct X3D_Rectangle2D *node) {
	extern GLfloat boxtex[];		/*  in CFuncs/statics.c*/
	extern GLfloat boxnorms[];		/*  in CFuncs/statics.c*/
	struct textureVertexInfo mtf = {boxtex,2,GL_FLOAT,0,NULL,NULL};
	
	float x = ((node->size).c[0])/2;
	float y = ((node->size).c[1])/2;

	/* test for <0 of sides */
	if ((x < 0) || (y < 0)) return;

	COMPILE_IF_REQUIRED
	if (!node->__points.p) return; /* still compiling */

	/* for BoundingBox calculations */
	setExtent(x,-x,y,-y,0.0f,0.0f,X3D_NODE(node));

	CULL_FACE(node->solid)

	/*  Draw it; assume VERTEX and NORMALS already defined.*/
	textureCoord_send(&mtf);
	FW_GL_VERTEX_POINTER (3,GL_FLOAT,0,(GLfloat *)node->__points.p);
	FW_GL_NORMAL_POINTER (GL_FLOAT,0,boxnorms);

	/* do the array drawing; sides are simple 0-1-2-3, 4-5-6-7, etc quads */
	if(DESIRE(getShaderFlags().base,SHADINGSTYLE_WIRE)){
		//wireframe triangles
		static ushort wireindices [] = { 0, 1, 1, 2, 2, 0, 3, 4, 4, 5, 5, 3 };
		sendElementsToGPU(GL_LINES,6*2,wireindices); //(nseg -1)*4 = (npts-2)*2 = npts*2 -4
	}else{
		sendArraysToGPU (GL_TRIANGLES, 0, 6);
	}
	gglobal()->Mainloop.trisThisLoop += 2;
}

void rendray_Rectangle2D(struct X3D_Rectangle2D* node) {
	//copy from rendray_Cylinder and hack
	float sx,sy, z;
	struct point_XYZ t_r1, t_r2;
	get_current_ray(&t_r1, &t_r2);

	sx = node->size.c[0];
	sy = node->size.c[1];
	z = 0.0f;
	if (!ZEQ) {
		float zrat0 = (float)ZRAT(z);
		if (TRAT(zrat0)) {
			float cx = (float)MRATX(zrat0);
			float cy = (float)MRATY(zrat0);
			if (fabs(cx) < fabs(sx) && fabs(cy) < fabs(sy)) {
				rayhit(zrat0, cx, cy, z, 0, 0, 1, -1, -1, "disk2d");
			}
		}
	}
}
/***********************************************************************************/
//http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/geometry2D.html#ArcClose2D
// "the angle starts at +x and goes toward +y"
// y^
//  |  /
//  | / ) angle
//  |_____> x
static void *createLines (float start, float end, float radius, int closed, int *size, float *_extent) {
	int i;
	int isCircle;
	int numPoints;
	GLfloat tmp;
	GLfloat *points;
	GLfloat *fp;
	int arcpoints;

        float myminx = FLT_MAX;
        float mymaxx = -FLT_MAX;
        float myminy = FLT_MAX;
        float mymaxy = -FLT_MAX;

	*size = 0;

	/* is this a circle? */
	isCircle =  APPROX(start,end);

	/* bounds check, and sort values */
	if ((start < PI*2.0) || (start > PI*2.0)) start = 0.0f;
	if ((end < PI*2.0) || (end > PI*2.0)) end = (float) (PI/2.0);
	if (radius<0.0) radius = 1.0f;

	if (end > start) {
		tmp = start;
		start = end;
		end = tmp;
	}
		

	if (isCircle) {
		numPoints = SEGMENTS_PER_CIRCLE;
		closed = NONE; /* this is a circle, CHORD, PIE dont mean anything now */
	} else {
		numPoints = (int) ((float)(SEGMENTS_PER_CIRCLE * (start-end))/(PI*2.0f));
		if (numPoints>SEGMENTS_PER_CIRCLE) numPoints=SEGMENTS_PER_CIRCLE;
	}

	/* we always have to draw the line - we have a line strip, and we calculate
	   the beginning points; we have also to calculate the ending point. */
	numPoints++;
	arcpoints = numPoints;

	/* closure type */
	if (closed == CHORD) numPoints+=2;
	if (closed == PIE) numPoints+=2;

	points = MALLOC (float *, sizeof(float)*numPoints*2);
	fp = points;

	for (i=0; i<arcpoints; i++) {
		*fp = radius * cosf(((float)PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE));	
		fp++;
		*fp = radius * sinf(((float)PI * 2.0f * (float)i)/((float)SEGMENTS_PER_CIRCLE));	
		fp++;
	}

	/* do we have to draw any pies, cords, etc, etc? */
	if (closed == CHORD) {
		/* go to mid-chord */
		*fp = .5f*(points[0] + points[(arcpoints-1)*2]); 
		fp++;
		*fp = .5f*(points[1] + points[(arcpoints-1)*2 +1]);
		fp++; 

		/* loop back to first point */
		*fp = radius * cosf(0.0f/((float)SEGMENTS_PER_CIRCLE));	
		fp++;
		*fp = radius * sinf(0.0f/((float)SEGMENTS_PER_CIRCLE));	
		fp++;
	} else if (closed == PIE) {
		/* go to origin */
		*fp = 0.0f; fp++; *fp=0.0f; fp++; 
		/* go back to first point */
		*fp = radius * cosf(0.0f/((float)SEGMENTS_PER_CIRCLE));	
		fp++;
		*fp = radius * sinf(0.0f/((float)SEGMENTS_PER_CIRCLE));	
		fp++;
	}

		
	/* find extents */
	*size = numPoints;
	if (numPoints==0) {
                EXTENT_MAX_X = 0.0f;
                EXTENT_MIN_X = 0.0f;
                EXTENT_MAX_Y = 0.0f;
                EXTENT_MIN_Y = 0.0f;
        } else { 
		/* find min/max for setExtent for these points */
		fp = points;
		for (i=0; i<numPoints; i++) {
			/* do X first */
                        if (*fp > mymaxx) mymaxx = *fp;
                        if (*fp < myminx) myminx = *fp;
			fp++;
			/* do Y second */
                        if (*fp > mymaxy) mymaxy = *fp;
                        if (*fp < myminy) myminy = *fp;
			fp++;
		}
		EXTENT_MIN_X = myminx;
		EXTENT_MAX_X = mymaxx;
		EXTENT_MIN_Y = myminy;
		EXTENT_MAX_Y = mymaxy;
	}

	return (void *)points;
}





void collide_Disk2D (struct X3D_Disk2D *node) {
	UNUSED (node);
}

void collide_Rectangle2D (struct X3D_Rectangle2D *node) {
	/* Modified Box code. */
	struct sNaviInfo *naviinfo;
	GLDOUBLE awidth, atop, abottom, astep, modelMatrix[16];
	struct point_XYZ iv = {0,0,0};
	struct point_XYZ jv = {0,0,0};
	struct point_XYZ kv = {0,0,0};
	struct point_XYZ ov = {0,0,0};
	struct point_XYZ delta;

	ttglobal tg = gglobal();
	/*easy access, naviinfo.step unused for sphere collisions */
	naviinfo = (struct sNaviInfo*)tg->Bindable.naviinfo;
	awidth = naviinfo->width; /*avatar width*/
	atop = naviinfo->width; /*top of avatar (relative to eyepoint)*/
	abottom = -naviinfo->height; /*bottom of avatar (relative to eyepoint)*/
	astep = -naviinfo->height+naviinfo->step;


	iv.x = node->size.c[0];
	jv.y = node->size.c[1]; 
	kv.z = 0.0;
	ov.x = -(node->size.c[0])/2; ov.y = -(node->size.c[1])/2; ov.z = 0.0;

	/* get the transformed position of the Box, and the scale-corrected radius. */
	FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelMatrix);

	matmultiplyAFFINE(modelMatrix,modelMatrix,FallInfo()->avatar2collision); 
	//dug9july2011 matmultiply(modelMatrix,FallInfo()->avatar2collision,modelMatrix); 

	{
		/*  minimum bounding box MBB test in avatar/collision space */
		double shapeMBBmin[3], shapeMBBmax[3], dsize[3];
		//int i;
		float2double(dsize,node->size.c,3);
		vecscaled(shapeMBBmax,dsize,.5);
		vecscaled(shapeMBBmax,dsize,-.5);
		//for(i=0;i<3;i++)
		//{
		//	shapeMBBmin[i] = DOUBLE_MIN(-(node->size.c[i])*.5,node->size.c[i]*.5);
		//	shapeMBBmax[i] = DOUBLE_MAX(-(node->size.c[i])*.5,node->size.c[i]*.5);
		//}
		if(!avatarCollisionVolumeIntersectMBB(modelMatrix, shapeMBBmin, shapeMBBmax))return;
	}
	/* get transformed box edges and position */
	transform(&ov,&ov,modelMatrix);
	transform3x3(&iv,&iv,modelMatrix);
	transform3x3(&jv,&jv,modelMatrix);
	transform3x3(&kv,&kv,modelMatrix);

	delta = box_disp(abottom,atop,astep,awidth,ov,iv,jv,kv);

	vecscale(&delta,&delta,-1);

	accumulate_disp(CollisionInfo(),delta);


	#ifdef COLLISIONVERBOSE
	if((fabs(delta.x) != 0. || fabs(delta.y) != 0. || fabs(delta.z) != 0.))
		printf("COLLISION_BOX: (%f %f %f) (%f %f %f)\n",
		ov.x, ov.y, ov.z,
		delta.x, delta.y, delta.z
		);
	if((fabs(delta.x != 0.) || fabs(delta.y != 0.) || fabs(delta.z) != 0.))
		printf("iv=(%f %f %f) jv=(%f %f %f) kv=(%f %f %f)\n",
		iv.x, iv.y, iv.z,
		jv.x, jv.y, jv.z,
		kv.x, kv.y, kv.z
		);
	#endif
}

void collide_TriangleSet2D(struct X3D_TriangleSet2D* node) {
	UNUSED(node);
}
struct point_XYZ get_poly_disp_2(struct point_XYZ* p, int num, struct point_XYZ n);
#define FLOAT_TOLERANCE 0.00000001
/*
void collide_TriangleSet2D(struct X3D_TriangleSet2D* node) {
	GLDOUBLE modelMatrix[16];

	ttglobal tg = gglobal();
	struct point_XYZ maxdispv = { 0,0,0 };
	double maxdisp = 0.0;

	// get the transformed position of the Box, and the scale-corrected radius. 
	FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelMatrix);

	matmultiplyAFFINE(modelMatrix, modelMatrix, FallInfo()->avatar2collision);
	{
		// minimum bounding box MBB test in avatar/collision space
		float center[3], size[3], bboxmin[3], bboxmax[3];
		extent6f2bbox(node->_extent, center, size);
		vecdif3f(bboxmin, center, size);
		vecadd3f(bboxmax, center, size);
		double shapeMBBmin[3], shapeMBBmax[3];
		float2double(shapeMBBmin, bboxmin, 3);
		float2double(shapeMBBmax, bboxmax, 3);
		if (!avatarCollisionVolumeIntersectMBB(modelMatrix, shapeMBBmin, shapeMBBmax))return;
	}
	for(int i=0;i<node->vertices.n;i+=3){
		double pts[3][3], nn[3], v1[3], v2[3], disp;
		for (int j = 0; j < 3; j++) {
			float2double(pts[j], node->vertices.p[i + j].c, 2);
			pts[j][2] = 0.0;
			transform(pts[j], pts[j], modelMatrix);
		}
		vecdifd(v1, pts[1], pts[0]);
		vecdifd(v2, pts[2], pts[0]);
		veccrossd(nn, v2, v1);
		struct point_XYZ dispv = get_poly_disp_2(pts, 3, nn);
		disp = vecdot(&dispv, &dispv);

		//keep result only if:
		// displacement is positive
		// displacement is smaller than minimum displacement up to date
		if ((disp > FLOAT_TOLERANCE) && (disp > maxdisp)) {
			maxdisp = disp;
			maxdispv = dispv;
		}
	}
	vecscale(&maxdispv, &maxdispv, -1);

	accumulate_disp(CollisionInfo(), maxdispv);

}
*/