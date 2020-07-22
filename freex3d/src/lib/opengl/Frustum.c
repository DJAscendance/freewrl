/*


???

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
#include "../scenegraph/quaternion.h"
#include "../scenegraph/Viewer.h"
#include "Frustum.h"
#include "../opengl/OpenGL_Utils.h"
#include "../scenegraph/LinearAlgebra.h"


#include "Textures.h"
#include <float.h>

//#define FRUSTUMVERBOSE

static void quaternion_multi_rotation(struct point_XYZ *ret, const Quaternion *quat, const struct point_XYZ * v, int count);
static void add_translation (struct point_XYZ *arr,  float x, float y, float z, int count);
static void multiply_in_scale(struct point_XYZ *arr, float x, float y, float z, int count);



/*********************************************************************
 * OLD - NOW USE Occlusion tests
 * Frustum calculations. Definitive work (at least IMHO) is thanks to
 * Steven Baker - look at http://sjbaker.org/steve/omniv/frustcull.html/
 *
 * Thanks Steve!
 *
 */


#undef  OCCLUSIONVERBOSE

#ifdef OCCLUSION

	/* if we have a visible Shape node, how long should we wait until we try to determine
	   if it is still visible? */
	#define OCCWAIT 		20 

	/* we have a visibility sensor, we want to really see when it becomes invis. */
	#define OCCCHECKNEXTLOOP	1

	/* we are invisible - don't let it go too long before we try to see visibility */
	#define OCCCHECKSOON		4

	/* how many samples of a Shape are needed before it becomes visible? If it is too
	   small, don't worry about displaying it If this number is too large, "flashing" 
	   will occur, as the shape is dropped, while still displaying (the number) of pixels
	   on the screen */
	#define OCCSHAPESAMPLESIZE	1	
#endif //OCCLUSION

typedef struct pFrustum{
	/* Occlusion VisibilitySensor code */
	GLuint *OccQueries;// = NULL;

	/* newer occluder code */
	GLuint potentialOccluderCount;// = 0;
	void ** occluderNodePointer;// = NULL;

	/* older occluder code */
	#ifdef OCCLUSION 
	int maxOccludersFound;// = 0;
	int QueryCount;// = 0;
	int OccInitialized;// = FALSE;
	#endif

	GLuint OccQuerySize;//=0;

	//	#ifdef OCCLUSIONVERBOSE
	//		GLint queryCounterBits;
	//	#endif

	GLuint OccResultsAvailable;// = FALSE;

}* ppFrustum;
void *Frustum_constructor(){
	void *v = MALLOCV(sizeof(struct pFrustum));
	memset(v,0,sizeof(struct pFrustum));
	return v;
}
void Frustum_init(struct tFrustum *t){
	//public
	t->OccFailed = FALSE;
	//private
	t->prv = Frustum_constructor();
	{
		ppFrustum p = (ppFrustum)t->prv;
		/* Occlusion VisibilitySensor code */
		p->OccQueries = NULL;

		/* newer occluder code */
		p->potentialOccluderCount = 0;
		p->occluderNodePointer = NULL;

		/* older occluder code */
		#ifdef OCCLUSION 
		p->maxOccludersFound = 0;
		p->QueryCount = 0;
		p->OccInitialized = FALSE;
		#endif

		p->OccQuerySize=0;

		p->OccResultsAvailable = FALSE;
	}
}

void beginOcclusionQuery(struct X3D_VisibilitySensor* node, int render_geometry)
{
	ppFrustum p = (ppFrustum)gglobal()->Frustum.prv;
	if (render_geometry) { 
		if (p->potentialOccluderCount < p->OccQuerySize) { 
            TRACE_MSG ("beginOcclusionQuery, potoc %d occQ %d\n",p->potentialOccluderCount, p->OccQuerySize); 
			if (node->__occludeCheckCount < 0) { 
				 TRACE_MSG ("beginOcclusionQuery, query %u, node %s\n",p->potentialOccluderCount, stringNodeType(node->_nodeType)); 
#if !defined(GL_ES_VERSION_2_0)
//void glBeginQuery(GLenum, GLuint);

				FW_GL_BEGIN_QUERY(GL_SAMPLES_PASSED, p->OccQueries[p->potentialOccluderCount]); 
#endif
				p->occluderNodePointer[p->potentialOccluderCount] = (void *)node; 
			} 
		}
	} 
}

void endOcclusionQuery(struct X3D_VisibilitySensor* node, int render_geometry)
{
	ppFrustum p = (ppFrustum)gglobal()->Frustum.prv;
	if (render_geometry) { 
		if (p->potentialOccluderCount < p->OccQuerySize) { 
			if (node->__occludeCheckCount < 0) { 
				TRACE_MSG ("glEndQuery node %p\n",node); 
#if !defined( GL_ES_VERSION_2_0 )
				FW_GL_END_QUERY(GL_SAMPLES_PASSED); 
#endif
				p->potentialOccluderCount++; 
			} 
		} 
	} 
}


//extent6f {xmax,xmin,ymax,ymin,zmax,zmin}
float *extent6f_constructor(float *extent6, float xmin,float xmax,  float ymin,float ymax, float zmin,float zmax){
	float *e = extent6;
	e[0]=xmax; e[1] = xmin; e[2]=ymax; e[3]=ymin;  e[4]=zmax; e[5]=zmin; 
	return e;
}
float *extent6f_clear(float *extent6){
	float *e = extent6;
	//s max,min y max,min, z max,min
	e[0]=-10000.0; e[1]=10000.0; e[2]=-10000.0; e[3]=10000.0; e[4]=-10000.0; e[5]=10000.0;
	return e;
}
int extent6f_isSet(float *extent6){
	//extents are set with min > max, so a way to tell
	// if they are set is to check if min <= max or max >= min
	int iret;
	float *e = extent6;
	//is max >= min for any dimensions? if so, then is set.
	//iret = (e[0] >= e[1] && e[2] >= e[3] && e[4] >= e[5]) ? TRUE : FALSE;
	//iret = (e[0] >= e[1] || e[2] >= e[3] || e[4] >= e[5]) ? TRUE : FALSE;
	iret = (e[0] >= e[1] && e[2] >= e[3] && e[4] >= e[5]) ? TRUE : FALSE;
	return iret;
}
float *extent6f_copy(float *eout6, float *ein6){
	memcpy(eout6,ein6,6*sizeof(float));
	return eout6;
}
void extent6f_to_vec3f(float *extent6, float *pmin, float *pmax){
	int i;
	for(i=0;i<3;i++){
		pmin[i] = extent6[i*2 + 1];
		pmax[i] = extent6[i*2 + 0];
	}
}
void extent6f_from_vec3f2(float *extent6, float *pmin, float *pmax){
	int i;
	for(i=0;i<3;i++){
		extent6[i*2 + 1] = pmin[i];
		extent6[i*2 + 0] = pmax[i];
	}
}


float *extent6f_union_extent6f(float *extent6, float *ein6){
	int i,isa,isb;
	isa = extent6f_isSet(extent6);
	isb = extent6f_isSet(ein6);
	if(isa && isb)
	for(i=0;i<3;i++){
		extent6[i*2 + 1] = min(extent6[i*2 + 1], ein6[i*2 + 1]);
		extent6[i*2 + 0] = max(extent6[i*2 + 0], ein6[i*2 + 0]);
	}
	else if(isb) extent6f_copy(extent6,ein6);
	return extent6;
}
float *extent6f_intersect_extent6f(float *extent6, float *eina, float *einb){
	int i,isa,isb;
	extent6f_clear(extent6);
	isa = extent6f_isSet(eina);
	isb = extent6f_isSet(einb);
	if(isa && isb)
	for(i=0;i<3;i++){
		extent6[i*2 + 1] = max(eina[i*2 + 1], einb[i*2 + 1]);
		extent6[i*2 + 0] = min(eina[i*2 + 0], einb[i*2 + 0]);
	}
	return extent6;
}
float *extent6f_union_vec3f(float *extent6, float *p3){
	int i,isa,isb;
	isa = extent6f_isSet(extent6);
	if(!isa)
	for(i=0;i<3;i++){
		extent6[i*2 + 1] = p3[i];
		extent6[i*2 + 0] = p3[i];
	}
	for(i=0;i<3;i++){
		extent6[i*2 + 1] = min(extent6[i*2 + 1], p3[i]);
		extent6[i*2 + 0] = max(extent6[i*2 + 0], p3[i]);
	}
	return extent6;
}
int extent6f_point_inside(float *extent6, float *pd){
	int inside = TRUE;
	for(int i=0;i<3;i++){
		inside = inside && extent6[i*2 + 1] < pd[i];
		inside = inside && pd[i] < extent6[i*2 + 0];
	}
	return inside;
}
float *extent6f_union_vec2f(float *extent6, float *p2){
	int i,isa,isb;
	isa = extent6f_isSet(extent6);
	if(!isa)
	for(i=0;i<2;i++){
		extent6[i*2 + 1] = p2[i];
		extent6[i*2 + 0] = p2[i];
	}
	for(i=0;i<2;i++){
		extent6[i*2 + 1] = min(extent6[i*2 + 1], p2[i]);
		extent6[i*2 + 0] = max(extent6[i*2 + 0], p2[i]);
	}
	return extent6;
}
void extent6f_to_box3f8(float *extent6, float *p3f8){
	//generate 8 points from extent
	int i,j,k,n;
	n = 0;
	//extent6f_printf(extent6);printf(" extent\n box:\n");
	for(k=0;k<2;k++)
		for(j=0;j<2;j++)
			for(i=0;i<2;i++){
				p3f8[n*3 + 0] = extent6[0 + i];
				p3f8[n*3 + 1] = extent6[2 + j];
				p3f8[n*3 + 2] = extent6[4 + k];
				//printf("%d %f %f %f\n",n,p3f8[n*3 + 0],p3f8[n*3 + 1],p3f8[n*3 + 2]);
				n++;
			}
	//printf("\n");
}
float * extent6f_from_box3fn(float *extent6,float *p, int n){
	int i,j;
	extent6f_clear(extent6);
	for(i=0;i<n;i++)
		extent6f_union_vec3f(extent6,&p[i*3]);
	return extent6;
}
float * extent6f_from_box2fn(float *extent6,float *p, int n){
	int i,j;
	extent6f_clear(extent6);
	for(i=0;i<n;i++)
		extent6f_union_vec2f(extent6,&p[i*2]);
	return extent6;
}
float *extent6f_scale3f(float *eout6, float *ein6, float *s3){
	int i;
	for(i=0;i<3;i++){
		eout6[i*2 + 0] = ein6[i*2 + 0] * s3[i];
		eout6[i*2 + 1] = ein6[i*2 + 1] * s3[i];
	}
	return eout6;
}
float *extent6f_translate3f(float *eout6, float *ein6, float *p3){
	int i;
	for(i=0;i<3;i++){
		eout6[i*2 + 0] = ein6[i*2 + 0] + p3[i];
		eout6[i*2 + 1] = ein6[i*2 + 1] + p3[i];
	}
	return eout6;
}
float *extent6f_translate3d(float *eout6, float *ein6, double *p3){
	int i;
	for(i=0;i<3;i++){
		eout6[i*2 + 0] = ein6[i*2 + 0] + p3[i];
		eout6[i*2 + 1] = ein6[i*2 + 1] + p3[i];
	}
	return eout6;
}
float *extent6f_get_center3f(float *extent6, float *center3){
	int i;
	for(i=0;i<3;i++){
		center3[i] = .5f*(extent6[i*2 + 0] + extent6[i*2 + 1]);
	}
	return center3;
}
float extent6f_get_maxsize(float *extent6){
	float msize;
	int i;
	msize = 0.0f;
	for(i=0;i<3;i++){
		msize = max(msize,extent6[i*2 + 0] - extent6[i*2 + 1]);
	}
	return msize;
}
void extent6f2bbox(float *extent6, float* center, float *size){
	//extent6: xmax,xmin,ymax,ymin,zmax,zmin
	for(int i=0;i<3;i++){
		if(extent6[2*i] >= extent6[2*i+1]){
			center[i] = .5f*extent6[2*i] + .5f*extent6[2*i+1];
			size[i] = extent6[2*i+0] - extent6[2*i+1];
		}else{
			center[i] = 0.0f;
			size[i] = -1.0f;
		}
	}
}
void bbox2extent6f(float* center, float *size, float *extent6){
	
	for(int i=0;i<3;i++){
		if(size[i] >= 0.0f){
			extent6[2*i +0] = center[i] + .5f*size[i]; //max
			extent6[2*i +1] = center[i] - .5f*size[i]; //min
		}else{
			extent6[2*i +0] = -10000.0f; //max
			extent6[2*i +1] =  10000.0f; //min
		}
	}
}

float extent6f_get_maxradius(float *extent6){
	
	float radius, p3f8[8][3], pc[3], pd[3];
	int i;
	radius = 0.0f;
	extent6f_get_center3f(extent6,pc);
	extent6f_to_box3f8(extent6, p3f8[0]);
	for(i=0;i<8;i++){
		vecdif3f(pd,p3f8[i],pc);
		radius = max(radius, veclength3f(pd));
	}
	return radius;
}
float *extent6f_rotate4f(float *eout6, float *ein6, float *vrot4){
	int i;
	float p3f[8][3];
	double p3d[8][3];
	Quaternion rq;

	extent6f_to_box3f8(ein6,p3f[0]);
	float2double(p3d[0],p3f[0],24);
	vrmlrot_to_quaternion(&rq,vrot4[0],vrot4[1], vrot4[2], vrot4[3]); 
	for(i=0;i<8;i++){
		quaternion_rotationd(p3d[i],&rq,p3d[i]); 
	}
	double2float(p3f[0],p3d[0],24);
	extent6f_from_box3fn(eout6,p3f[0],8);
	return eout6;
}

float *extent6f_rotate4d(float *eout6, float *ein6, double *vrot4){
	int i;
	float p3f[8][3];
	double p3d[8][3];
	Quaternion rq;

	extent6f_to_box3f8(ein6,p3f[0]);
	float2double(p3d[0],p3f[0],24);
	vrmlrot_to_quaternion(&rq,vrot4[0],vrot4[1], vrot4[2], vrot4[3]); 
	for(i=0;i<8;i++){
		quaternion_rotationd(p3d[i],&rq,p3d[i]); 
	}
	double2float(p3f[0],p3d[0],24);
	extent6f_from_box3fn(eout6,p3f[0],8);
	return eout6;
}
float *extent6f_mattransform4d(float *eout6,float *ein6, double *mat4){
	int i;
	float p3f[8][3];
	double p3d[8][3];
	Quaternion rq;
	if(extent6f_isSet(ein6)){
		extent6f_to_box3f8(ein6,p3f[0]);
		float2double(p3d[0],p3f[0],24);
		for(i=0;i<8;i++){
			transformAFFINEd(p3d[i],p3d[i],mat4); 
		}
		double2float(p3f[0],p3d[0],24);
		extent6f_from_box3fn(eout6,p3f[0],8);
	}else{
		extent6f_clear(eout6);
	}
	return eout6;
	
}
float *orientedBBox2extent6f(float *extent6, float *obb12){
	// Tiles3D section 3. https://github.com/CesiumGS/3d-tiles/blob/master/3d-tiles-overview.pdf
	float *center = &obb12[0];
	float *hx = &obb12[3];
	float *hy = &obb12[6];
	float *hz = &obb12[9];
	float p3f[8][3], temp[3], temp2[3];
	int ijk = 0;
	for(int i=0;i<2;i++)
		for(int j=0;j<2;j++)
			for(int k=0;k<2;k++){
				veccopy3f(temp,center);
				vecadd3f(temp,temp,vecscale3f(temp2,hx,i?1.0f:-1.0f));
				vecadd3f(temp,temp,vecscale3f(temp2,hy,j?1.0f:-1.0f));
				vecadd3f(temp,temp,vecscale3f(temp2,hz,k?1.0f:-1.0f));
				veccopy3f(p3f[ijk], temp);
				ijk++;
			}
	extent6f_from_box3fn(extent6,p3f[0], 8);
	return extent6;
}
float *orientedBBox2vec3fn(float *p3fn24, float *obb12){
	// Tiles3D section 3. https://github.com/CesiumGS/3d-tiles/blob/master/3d-tiles-overview.pdf
	float *center = &obb12[0];
	float *hx = &obb12[3];
	float *hy = &obb12[6];
	float *hz = &obb12[9];
	float *p3f[8], temp[3], temp2[3];
	for(int i=0;i<8;i++) p3f[i] = &p3fn24[3*i];
	int ijk = 0;
	for(int i=0;i<2;i++)
		for(int j=0;j<2;j++)
			for(int k=0;k<2;k++){
				veccopy3f(temp,center);
				vecadd3f(temp,temp,vecscale3f(temp2,hx,i?1.0f:-1.0f));
				vecadd3f(temp,temp,vecscale3f(temp2,hy,j?1.0f:-1.0f));
				vecadd3f(temp,temp,vecscale3f(temp2,hz,k?1.0f:-1.0f));
				veccopy3f(p3f[ijk], temp);
				ijk++;
			}
	//for(int i=0;i<8;i++)
	//	printf("bvcoord %d %f %f %f\n",i,p3fn24[i*3],p3fn24[i*3+1],p3fn24[i*3+2]);
	return p3fn24;
}
float *orientedBBox_mattransform4d(float *out12, float *obb12, double *mat4){
	// Tiles3D section 3. https://github.com/CesiumGS/3d-tiles/blob/master/3d-tiles-overview.pdf
	//H when transforming an OBB, the half- vector parts shall be transformed like normals are:
	// using the transpose inverse
	float fmat4[16], fmat3[9],fmat3i[9],normat[9];
	matdouble2float4(fmat4,mat4);
	mat423f(fmat3,fmat4);
	matinverse3f(fmat3i,fmat3);
	mattranspose3f(normat,fmat3i);
	//transform half-vectors
	for(int i=0;i<3;i++)
		transform3x3f(&out12[(i+1)*3],&obb12[(i+1)*3],normat);
	//transform the center
	transformf(out12,obb12,mat4);
	return out12;
	
}
float *orientedBBox_mattransformAFFINE4d(float *p3fn24, float *obb12, double *mat4){
	// Tiles3D section 3. https://github.com/CesiumGS/3d-tiles/blob/master/3d-tiles-overview.pdf
	//goal: transform into cuboid space using (modelview x projction) but don't divide by perspectives
	// I think that's coboid space -1 to 1 on 3 axes
	// then its easier to do extent checks.
	float *p3f[8];
	double d1[3],d2[3];
	for(int i=0;i<8;i++) p3f[i] = &p3fn24[3*i];

	orientedBBox2vec3fn(p3f[0],obb12);
	for(int i=0;i<8;i++){
		float2double(d1,p3f[i],3);
		transformAFFINEd(d2,d1,mat4);
		double2float(p3f[i],d2,3);
	}
	return p3fn24;
}
void extent6f_printf(float *extent6){
	float *e = extent6;
	printf("min,max x:%8.1f,%8.1f y:%8.1f,%8.1f z:%8.1f,%8.1f ",e[1],e[0],e[3],e[2],e[5],e[4]);
}
void union_group_extent(float *e6);
void extent6f_setNodeExtentB(float *extent6, struct X3D_Node *node){
	extent6f_copy(node->_extent,extent6);
	union_group_extent(extent6);
}
void extent6f_setParentExtentB(float *extent6, struct X3D_Node *me){
	int i,j;
	struct X3D_Node *shapeParent;
	struct X3D_Node *groupParent;
	float *e = extent6;
    
	#ifdef FRUSTUMVERBOSE
	extent6f_printf(e);
	printf(" extent6f_setNodeExtentB me %p nt %s\n",me,stringNodeType(me->_nodeType));
	#endif

	/* record this for ME for sorting purposes for sorting children fields */

	if (me->_parentVector == NULL) {
		#ifdef FRUSTUMVERBOSE
		printf ("setExtent, parentVector NULL for node %p type %s\n",
			me,stringNodeType(me->_nodeType));
		#endif
		return;
	}

	for (i=0; i<vectorSize(me->_parentVector); i++) {
		shapeParent = vector_get(struct X3D_Node *, me->_parentVector,i);
		extent6f_copy(shapeParent->_extent,e);
		for (j=0; j<vectorSize(shapeParent->_parentVector); j++) {
			groupParent = vector_get(struct X3D_Node *, shapeParent->_parentVector,j);
			
			//extent6f_printf(e); printf(" e\n");
			//extent6f_printf(groupParent->_extent); printf(" gp before\n");
			extent6f_union_extent6f(groupParent->_extent,e);
			//extent6f_printf(groupParent->_extent); printf(" gp after union\n");
		}
	}
}
//struct Planed {
//	double normal[3];
//	double d;
//};
//enum {
//	NEARP =0,
//	FARP,
//	BOTTOM,
//	TOP,
//	LEFT,
//	RIGHT,
//};
//static struct Planed pl[6];
void planed_setCoefficients(struct Planed* p, double a, double b, double c, double d) {

	// set the normal vector
	vecsetd(p->normal,a,b,c);
	//compute the lenght of the vector
	double length = veclengthd(p->normal);
	// normalize the vector
	vecscaled(p->normal,p->normal,1.0/length);
	// and divide d by th length as well
	p->d = d/length;
	vecscaled(p->p,p->normal,p->d);
}
int imat(int irow, int icol){
	//int index = (irow-1)*4 + (icol-1);
	int index = (icol-1)*4 + (irow-1);
	return index;
}
void setFrustumPlanes(double *mvpMatrix, struct Planed *pl) {
	double *m = mvpMatrix;
	planed_setCoefficients(&pl[NEARP],
				 m[imat(3,1)] + m[imat(4,1)],
				 m[imat(3,2)] + m[imat(4,2)],
				 m[imat(3,3)] + m[imat(4,3)],
				 m[imat(3,4)] + m[imat(4,4)]);
	planed_setCoefficients(&pl[FARP],
				-m[imat(3,1)] + m[imat(4,1)],
				-m[imat(3,2)] + m[imat(4,2)],
				-m[imat(3,3)] + m[imat(4,3)],
				-m[imat(3,4)] + m[imat(4,4)]);
	planed_setCoefficients(&pl[BOTTOM],
				 m[imat(2,1)] + m[imat(4,1)],
				 m[imat(2,2)] + m[imat(4,2)],
				 m[imat(2,3)] + m[imat(4,3)],
				 m[imat(2,4)] + m[imat(4,4)]);
	planed_setCoefficients(&pl[TOP],
				-m[imat(2,1)] + m[imat(4,1)],
				-m[imat(2,2)] + m[imat(4,2)],
				-m[imat(2,3)] + m[imat(4,3)],
				-m[imat(2,4)] + m[imat(4,4)]);
	planed_setCoefficients(&pl[LEFT],
				 m[imat(1,1)] + m[imat(4,1)],
				 m[imat(1,2)] + m[imat(4,2)],
				 m[imat(1,3)] + m[imat(4,3)],
				 m[imat(1,4)] + m[imat(4,4)]);
	planed_setCoefficients(&pl[RIGHT],
				-m[imat(1,1)] + m[imat(4,1)],
				-m[imat(1,2)] + m[imat(4,2)],
				-m[imat(1,3)] + m[imat(4,3)],
				-m[imat(1,4)] + m[imat(4,4)]);
	//for(int i=0;i<6;i++){
	//	printf("plane[%d]= %lf %lf %lf, %lf\n",i,pl[i].normal[0],pl[i].normal[1],pl[i].normal[2],pl[i].d);
	//}
}
enum {
	OUTSIDE = 0,
	INSIDE = 1,
	INTERSECT = 2,
};
/*
float *getVertexP(float *p, float *v, float *normal){
	p = (xmin,ymin,zmin)
	if (normal.x >= 0)
		p.x = xmax;
	if (normal.y >=0))
		p.y = ymax;
	if (normal.z >= 0)
		p.z = zmax:
}
int frustum_boxInFrustum(struct Planed *frustum, float *abb) {
	// http://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-testing-boxes-ii/
	int result = INSIDE;
	//for each plane do ...
	for(int i=0; i < 6; i++) {

		// is the positive vertex outside?
		if (pl[i].distance(b.getVertexP(pl[i].normal)) < 0)
			return OUTSIDE;
		// is the negative vertex outside?
		else if (pl[i].distance(b.getVertexN(pl[i].normal)) < 0)
			result =  INTERSECT;
	}
	return(result);
}
*/
int plane_intersect_plane_intersect_plane(struct Planed *p1, struct Planed *p2, struct Planed *p3, double *point){
	//Granphics Gems I p.305
	// computes point of intersection of 3 planes, if it exists returns TRUE and point, else FALSE
	int intersection = FALSE;
	double pi[3], ptemp1[3], ptemp2[3], detval;
	vecsetd(pi,0.0,0.0,0.0);
	vecscaled(ptemp2,veccrossd(ptemp1,p2->normal,p3->normal),vecdotd(p1->p,p1->normal));
	vecaddd(pi,pi,ptemp2);
	vecscaled(ptemp2,veccrossd(ptemp1,p3->normal,p1->normal),vecdotd(p2->p,p2->normal));
	vecaddd(pi,pi,ptemp2);
	vecscaled(ptemp2,veccrossd(ptemp1,p1->normal,p2->normal),vecdotd(p3->p,p3->normal));
	vecaddd(pi,pi,ptemp2);
	detval = det3d(p1->normal,p2->normal,p3->normal);
	if( detval != 0.0){
		intersection = TRUE;
		vecscaled(point,pi,-1.0/detval);
		//printf("> %lf %lf %lf\n",pi[0],pi[1],pi[2]);
	}
	return intersection;
}

double plane_distance_to_point(struct Planed *plane, double *p){
	//assumes plane is normalized
	double dist= vecdotd(plane->normal,p);
	dist += plane->d;
	return dist;
}
int frustum_point_inside(struct Planed *frustum_planes, double *p) {
	//assumes 6 planes around frustum
	int result = INSIDE;

	for(int i=0; i < 6; i++) {
		//if(i==1) continue; //H: far plane not far enough
		if(plane_distance_to_point(&frustum_planes[i],p) < 0.0)
			return OUTSIDE;
	}
	return(result);
}
int frustum_generate_corner_points(struct Planed *frustum_planes, float *pf24n){
	//generates frusum corner points from planes,
	//near plane clockwise starting wtih upper left, then far plane same order
	//returns TRUE
	int n=0;
	int order [] = {LEFT,TOP,RIGHT,BOTTOM};
	for(int i=0;i<2;i++){
		for(int j=0;j<4;j++){
			double pi[3];
			int jj,kk,k;
			jj = order[j];
			k = j+1;
			kk = order[k % 4];
			if(!plane_intersect_plane_intersect_plane(&frustum_planes[i],&frustum_planes[jj],&frustum_planes[kk],pi)) 
				return FALSE;
			double2float(&pf24n[n*3],pi,3);
			n++;
		}
	}
	//for(int i=0;i<n;i++)
	//	printf("fc[%d] %f %f %f\n",i,pf24n[i*3],pf24n[i*3+1],pf24n[i*3+2]);
	return TRUE;
}
void FRUSTUM_GEOELEVATIONGRID(struct X3D_Node *me){
	int i;
	if (me->_nodeType == NODE_GeoElevationGrid) { 
		if( extent6f_isSet(me->_extent)) {
			float ef6[6];
			struct X3D_GeoElevationGrid *node = (struct X3D_GeoElevationGrid *)me; 
			extent6f_rotate4d(ef6, me->_extent, node->__localOrient.c);
			extent6f_translate3d(ef6,ef6,node->__autoOffset.c);
			extent6f_setNodeExtentB(ef6,me);
		} 
	} 
}



/* does this current node actually fit in the Switch rendering scheme? */
int is_Switchchild_inrange(struct X3D_Switch *node, struct X3D_Node *me) {
        int wc = node->whichChoice;

        /* is this VRML, or X3D?? */
        if (node->__isX3D == 0) {
                if(wc >= 0 && wc < ((node->choice).n)) {
                        void *p = ((node->choice).p[wc]);
                        return (X3D_NODE(p)==me);
                }
        } else {
                if(wc >= 0 && wc < ((node->children).n)) {
                        void *p = ((node->children).p[wc]);
                        return (X3D_NODE(p)==me);
                }
        }
	return FALSE;
}


/* does this current node actually display, according to the CADLayer scheme? */
//int is_CADLayerchild_inrange(struct X3D_CADLayer *node, struct X3D_Node *me) {
//    int i;
//    for (i=0; i<node->children.n; i++) {
//        
//        /* if we have more children than we have indexes into visible field, just return TRUE */
//        if ((i >= node->visible.n) && (node->children.p[i] == me)) return TRUE;
//        
//        /* if not, if it is in the visible field, return true */
//        else if ((node->visible.p[i]) && (node->children.p[i] == me)) return TRUE;
//        }
//    /* not visible, so return false */
//    return FALSE;
//}

/* does this current node actually fit in the GeoLOD rendering scheme? */
int is_GeoLODchild_inrange (struct X3D_GeoLOD* gpnode, struct X3D_Node *me) {
	/* is this node part of the active path for rendering? */
	int x,y;
	y = FALSE;

	for (x=0; x<gpnode->rootNode.n; x++) {
		/* printf ("comparing %u:%u %d of %d, types me %s rootNodeField: %s\n", 
			me, X3D_NODE(gpnode->rootNode.p[x]),
			x, gpnode->rootNode.n,
			stringNodeType (me->_nodeType),
			stringNodeType( X3D_NODE(gpnode->rootNode.p[x])->_nodeType)
			);
		*/

		if (me == X3D_NODE(gpnode->rootNode.p[x])) {
			y=TRUE;
			break;
		}
	}

/*
	if (y) printf ("GeoLOD, found child in rootNode "); else printf ("GeoLOD, child NOT part of ROOT ");
	if (X3D_GEOLOD(geomParent)->__inRange) printf ("INRANGE "); else printf ("NOT inrange ");
*/
	/* is this one actually being rendered? */
	return (y ^ gpnode->__inRange);
}


/* take the measurements of a geometry (eg, box), and save it. Note
 * that what is given is a Shape, the values get pushed up to the
 * Geometries grouping node parent. */




void setExtent(float maxx, float minx, float maxy, float miny, float maxz, float minz, struct X3D_Node *me) {
	float e[6];
	extent6f_constructor(e,minx,maxx,miny,maxy,minz,maxz);
	extent6f_setNodeExtentB(e,me);
}

static void quaternion_multi_rotation(struct point_XYZ *ret, const Quaternion *quat, const struct point_XYZ * v, int count){
	int i;
	for (i=0; i<count; i++) {
		quaternion_rotation(ret, quat, v);
		ret++; v++;
	}
}



static void add_translation (struct point_XYZ *arr,  float x, float y, float z, int count) {
	int i;
	for (i=0; i<count; i++) {
		arr->x += (double)x;
		arr->y += (double)y;
		arr->z += (double)z;
		arr++;
	}
}

static void multiply_in_scale(struct point_XYZ *arr, float x, float y, float z, int count) {
	int i;
	for (i=0; i<count; i++) {
		arr->x *= (double)x;
		arr->y *= (double)y;
		arr->z *= (double)z;
		arr++;
	}
}


void printmatrix(GLDOUBLE* mat) {
    int i;
    for(i = 0; i< 16; i++) {
	printf("mat[%d] = %4.3f%s",i,mat[i],i==3 ? "\n" : i==7? "\n" : i==11? "\n" : "");
    }
	printf ("\n");

}


/* for children nodes; set the parent grouping nodes extent - we expect the center
 * of the group to be passed in in the floats x,y,z */

void propagateExtent(struct X3D_Node *me) {
}

/* perform all the viewpoint rotations for a point */
/* send in a pointer for the result, the current bounding box point to rotate, and the current ModelView matrix */

void moveAndRotateThisPoint(struct point_XYZ *mypt, double x, double y, double z, double *MM) {
		float outF[3];
		float inF[3];
		inF[0] = (float) x; inF[1] = (float) y; inF[2] = (float) z;

		/* transform this vertex via the modelview matrix */
		transformf (outF,inF,MM);

		#ifdef VERBOSE
		printf ("transformed %4.2f %4.2f %4.2f, to %4.2f %4.2f %4.2f\n",inF[0], inF[1], inF[2],
			outF[0], outF[1], outF[2]);
		#endif
		mypt->x = outF[0]; mypt->y=outF[1],mypt->z = outF[2];
}


/**************************************************************************************/

/* get the center of the bounding box, rotate it, and find out how far it is Z distance from us.
*/


void record_ZBufferDistance(struct X3D_Node *node) {
	GLDOUBLE modelMatrix[16];
	double ex;
	double ey;
	double ez;
	struct point_XYZ movedPt;
	double minMovedDist;

	minMovedDist = -1000000000;

	#ifdef FRUSTUMVERBOSE
	printf ("\nrecordDistance for node %p nodeType %s size %4.2f %4.2f %4.2f ",node, stringNodeType (node->_nodeType),
	node->EXTENT_MAX_X - node->EXTENT_MIN_X,
	node->EXTENT_MAX_Y - node->EXTENT_MIN_Y,
	node->EXTENT_MAX_Z - node->EXTENT_MIN_Z
	); 

	if (APPROX(node->EXTENT_MAX_X,-10000.0)) printf ("EXTENT NOT INIT");

	printf ("\n");
	printf ("recordDistance, max,min %f:%f, %f:%f, %f:%f\n",
		node->EXTENT_MAX_X , node->EXTENT_MIN_X,
		node->EXTENT_MAX_Y , node->EXTENT_MIN_Y,
		node->EXTENT_MAX_Z , node->EXTENT_MIN_Z);
	#endif

	/* get the current pos in modelMatrix land */
	FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelMatrix);

#ifdef TRY_ONLY_ONE_POINT
#ifdef TRY_RADIUS
	/* get radius of bounding box around its origin */
	ex = (node->EXTENT_MAX_X - node->EXTENT_MIN_X) / 2.0;
	ey = (node->EXTENT_MAX_Y - node->EXTENT_MIN_Y) / 2.0;
	ez = (node->EXTENT_MAX_Z - node->EXTENT_MIN_Z) / 2.0;
	printf ("	ex %lf ey %lf ez %lf\n",ex,ey,ez);
#else
	/* get the center of the bounding box */
	ex = node->EXTENT_MAX_X + node->EXTENT_MIN_X;
	ey = node->EXTENT_MAX_Y + node->EXTENT_MIN_Y;
	ez = node->EXTENT_MAX_Z + node->EXTENT_MIN_Z;
#endif

	
	/* rotate the center of this point */
	moveAndRotateThisPoint (&movedPt, ex,ey,ez,modelMatrix);
	printf ("%lf %lf %lf centre is %lf %lf %lf\n",ex,ey,ez,movedPt.x, movedPt.y, movedPt.z);
	minMovedDist = movedPt.z;

#else
	
	/* printf ("moving all 8 points of this bounding box\n"); */
	ex = node->EXTENT_MIN_X;
	ey = node->EXTENT_MIN_Y;
	ez = node->EXTENT_MIN_Z;
	moveAndRotateThisPoint (&movedPt, ex,ey,ez,modelMatrix);
	if (movedPt.z > minMovedDist) minMovedDist = movedPt.z;
	/* printf ("%lf %lf %lf moved is %lf %lf %lf\n",ex,ey,ez,movedPt.x, movedPt.y, movedPt.z); */

	ex = node->EXTENT_MIN_X;
	ey = node->EXTENT_MIN_Y;
	ez = node->EXTENT_MAX_Z;
	moveAndRotateThisPoint (&movedPt, ex,ey,ez,modelMatrix);
	if (movedPt.z > minMovedDist) minMovedDist = movedPt.z;
	/* printf ("%lf %lf %lf moved is %lf %lf %lf\n",ex,ey,ez,movedPt.x, movedPt.y, movedPt.z); */

	ex = node->EXTENT_MIN_X;
	ey = node->EXTENT_MAX_Y;
	ez = node->EXTENT_MIN_Z;
	moveAndRotateThisPoint (&movedPt, ex,ey,ez,modelMatrix);
	if (movedPt.z > minMovedDist) minMovedDist = movedPt.z;
	/* printf ("%lf %lf %lf moved is %lf %lf %lf\n",ex,ey,ez,movedPt.x, movedPt.y, movedPt.z); */

	ex = node->EXTENT_MIN_X;
	ey = node->EXTENT_MAX_Y;
	ez = node->EXTENT_MAX_Z;
	moveAndRotateThisPoint (&movedPt, ex,ey,ez,modelMatrix);
	if (movedPt.z > minMovedDist) minMovedDist = movedPt.z;
	/* printf ("%lf %lf %lf moved is %lf %lf %lf\n",ex,ey,ez,movedPt.x, movedPt.y, movedPt.z); */

	ex = node->EXTENT_MAX_X;
	ey = node->EXTENT_MIN_Y;
	ez = node->EXTENT_MIN_Z;
	moveAndRotateThisPoint (&movedPt, ex,ey,ez,modelMatrix);
	if (movedPt.z > minMovedDist) minMovedDist = movedPt.z;
	/* printf ("%lf %lf %lf moved is %lf %lf %lf\n",ex,ey,ez,movedPt.x, movedPt.y, movedPt.z); */

	ex = node->EXTENT_MAX_X;
	ey = node->EXTENT_MIN_Y;
	ez = node->EXTENT_MAX_Z;
	moveAndRotateThisPoint (&movedPt, ex,ey,ez,modelMatrix);
	if (movedPt.z > minMovedDist) minMovedDist = movedPt.z;
	/* printf ("%lf %lf %lf moved is %lf %lf %lf\n",ex,ey,ez,movedPt.x, movedPt.y, movedPt.z); */

	ex = node->EXTENT_MAX_X;
	ey = node->EXTENT_MAX_Y;
	ez = node->EXTENT_MIN_Z;
	moveAndRotateThisPoint (&movedPt, ex,ey,ez,modelMatrix);
	if (movedPt.z > minMovedDist) minMovedDist = movedPt.z;
	/* printf ("%lf %lf %lf moved is %lf %lf %lf\n",ex,ey,ez,movedPt.x, movedPt.y, movedPt.z); */

	ex = node->EXTENT_MAX_X;
	ey = node->EXTENT_MAX_Y;
	ez = node->EXTENT_MAX_Z;
	moveAndRotateThisPoint (&movedPt, ex,ey,ez,modelMatrix);
	if (movedPt.z > minMovedDist) minMovedDist = movedPt.z;
	/* printf ("%lf %lf %lf moved is %lf %lf %lf\n",ex,ey,ez,movedPt.x, movedPt.y, movedPt.z); */
#endif

	node->_dist = minMovedDist;

#ifdef FRUSTUMVERBOSE
	printf ("I am at %lf %lf %lf\n",Viewer()->currentPosInModel.x, Viewer()->currentPosInModel.y, Viewer()->currentPosInModel.z);
	printf ("and distance to the nearest corner of the BB for this node is %lf\n", node->_dist);
#endif


 
#undef FRUSTUMVERBOSE

}

/***************************************************************************/

void OcclusionStartofRenderSceneUpdateScene() {

#ifdef OCCLUSION /* do we have hardware for occlusion culling? */
	int i;
	ppFrustum p;
	ttglobal tg = gglobal();
	p = (ppFrustum)gglobal()->Frustum.prv;
	/* each time through the event loop, we count the occluders. Note, that if, say, a 
	   shape was USED 100 times, that would be 100 occlude queries, BUT ONE SHAPE, thus
	   there is not an implicit 1:1 mapping between shapes and occlude queries */

	p->potentialOccluderCount = 0;

	/* did we have a failure here ? */
	if (tg->Frustum.OccFailed) return;

	/* have we been through this yet? */
	if (p->OccInitialized == FALSE) {
		#ifdef OCCLUSIONVERBOSE
		printf ("initializing OcclusionCulling...\n");
		#endif
		/* do we have an environment variable for this? */
		if (gglobal()->internalc.global_occlusion_disable) {
			tg->Frustum.OccFailed = TRUE;
		} else {
				s_renderer_capabilities_t *rdr_caps;
				rdr_caps = gglobal()->display.rdr_caps;
	        	if (rdr_caps->av_occlusion_q) {
		
				#ifdef OCCLUSIONVERBOSE
	        	        printf ("OcclusionStartofRenderSceneUpdateScene: have OcclusionQuery\n"); 
				#endif
	
				/* we make the OccQuerySize larger than the maximum number of occluders,
				   so we don't have to realloc too much */
				p->OccQuerySize = p->maxOccludersFound + 1000;

				p->occluderNodePointer = MALLOC (void **, sizeof (void *) * p->OccQuerySize);
				p->OccQueries = MALLOC (GLuint *, sizeof(GLuint) * p->OccQuerySize);
                FW_GL_GENQUERIES(p->OccQuerySize,p->OccQueries);
                //ConsoleMessage ("generated %d queries, pointer %p",p->OccQuerySize,p->OccQueries);
				p->OccInitialized = TRUE;
				for (i=0; i<p->OccQuerySize; i++) {
					p->occluderNodePointer[i] = 0;
				}
				p->QueryCount = p->maxOccludersFound; /* for queries - we can do this number */
				#ifdef OCCLUSIONVERBOSE
				printf ("QueryCount now %d\n",p->QueryCount);
				#endif

        		} else {
				#ifdef OCCLUSIONVERBOSE
        	       		 printf ("OcclusionStartofRenderSceneUpdateScene: DO NOT have OcclusionQuery\n"); 
				#endif

				/* we dont seem to have this extension here at runtime! */
				/* this happened, eg, on my Core4 AMD64 box with Mesa	*/
				tg->Frustum.OccFailed = TRUE;
				return;
			}
		}

	}


	/* did we find more shapes than before? */
	if (p->maxOccludersFound > p->QueryCount) {
        	if (p->maxOccludersFound > p->OccQuerySize) {
        	        /* printf ("have to regen queries\n"); */
			p->QueryCount = 0;

			/* possibly previous had zero occluders, lets just not bother deleting for zero */
			if (p->OccQuerySize > 0) {
				FW_GL_DELETEQUERIES (p->OccQuerySize, p->OccQueries);
				FW_GL_FLUSH();
			}

			p->OccQuerySize = p->maxOccludersFound + 1000;
			p->occluderNodePointer = REALLOC (p->occluderNodePointer,sizeof (void *) * p->OccQuerySize);
			p->OccQueries = REALLOC (p->OccQueries,sizeof (GLuint) * p->OccQuerySize);
            FW_GL_GENQUERIES(p->OccQuerySize,p->OccQueries);
                ConsoleMessage ("reinitialized queries... now %p",p->OccQueries);
			for (i=0; i<p->OccQuerySize; i++) {
				p->occluderNodePointer[i] = 0;
			}
		}
		p->QueryCount = p->maxOccludersFound; /* for queries - we can do this number */
		#ifdef OCCLUSIONVERBOSE
		printf ("QueryCount here is %d\n",p->QueryCount);
		#endif

       }


//	#ifdef OCCLUSIONVERBOSE
//        glGetQueryiv(GL_SAMPLES_PASSED, GL_QUERY_COUNTER_BITS, &p->queryCounterBits);
//        printf ("queryCounterBits %d\n",p->queryCounterBits);
//        #endif
#endif /* OCCLUSION */

}

void OcclusionCulling ()  {
//non-occlusion visibilitysensor method: __Samples = 0 in startofloopnodeupdates
#ifdef OCCLUSION /* do we have hardware for occlusion culling? */
	int i;
	struct X3D_Shape *shapePtr;
	struct X3D_VisibilitySensor *visSenPtr;
	int checkCount;
	GLuint samples;
	ppFrustum p;
	ttglobal tg = gglobal();
	p = (ppFrustum)tg->Frustum.prv;

//#ifdef OCCLUSIONVERBOSE
//	{
//	GLint query;
//	glGetQueryiv(GL_SAMPLES_PASSED, GL_CURRENT_QUERY, &query);
//	printf ("currentQuery is %d\n",query);
//	}
//#endif

	visSenPtr = NULL;
	shapePtr = NULL;

	/* Step 0. go through list of assigned nodes, and either:
		- if we have OcclusionQueries: REMOVE the VF_hasVisibleChildren flag;
		- else, set every node to VF_hasVisibleChildren */
	zeroVisibilityFlag();

	/* Step 1. did we have some problem with Occlusion ? */
	if (tg->Frustum.OccFailed) return;
	 
	/* Step 2. go through the list of "OccludeCount" nodes, and determine if they are visible. 
	   If they are not, then, we have to, at some point, make them visible, so that we can test again. */
	/* note that the potentialOccluderCount is only incremented if the __occludeCheckCount tells us
	   that it should be checked again - see the interplay between the eventLoop stuff in OpenGLUtils.c
 	   and the OCCLUSION* defines in headers.h - we DO NOT generate a query every time through the loop */
 
	//#ifdef OCCLUSIONVERBOSE
	//printf ("OcclusionCulling - potentialOccluderCount %d\n",p->potentialOccluderCount);
	//#endif

	for (i=0; i<p->potentialOccluderCount; i++) {
		#ifdef OCCLUSIONVERBOSE
		printf ("checking node %d of %d\n",i, p->potentialOccluderCount);
		#endif

		checkCount = 0;

		/* get the check count field for this node - see if we did a check of this */
		shapePtr = X3D_SHAPE(p->occluderNodePointer[i]);
		if (shapePtr != NULL) {
			if (shapePtr->_nodeType == NODE_Shape) {
				visSenPtr = NULL;
				checkCount = shapePtr->__occludeCheckCount;
			} else if (shapePtr->_nodeType == NODE_VisibilitySensor) {
				visSenPtr = X3D_VISIBILITYSENSOR(shapePtr);
				shapePtr = NULL;
				checkCount = visSenPtr->__occludeCheckCount;
			} else {
				printf ("OcclusionCulling on node type %s not allowed\n",stringNodeType(shapePtr->_nodeType));
				return;
			}
		}

		#ifdef OCCLUSIONVERBOSE
		if (shapePtr) printf ("OcclusionCulling, for a %s (index %d ptr %p) checkCount %d\n",stringNodeType(shapePtr->_nodeType),i,shapePtr,checkCount);
		else printf ("OcclusionCulling, for a %s (index %d) checkCount %d\n",stringNodeType(visSenPtr->_nodeType),i,checkCount);
		#endif

		/* an Occlusion test will have been run on this one */

		FW_GL_GETQUERYOBJECTUIV(p->OccQueries[i],GL_QUERY_RESULT_AVAILABLE,&p->OccResultsAvailable);
		PRINT_GL_ERROR_IF_ANY("FW_GL_GETQUERYOBJECTUIV::QUERY_RESULTS_AVAIL");

		#define SLEEP_FOR_QUERY_RESULTS
		#ifdef SLEEP_FOR_QUERY_RESULTS
		/* for now, lets loop to see when we get results */
		while (p->OccResultsAvailable == GL_FALSE) {
			usleep(100);
			FW_GL_GETQUERYOBJECTUIV(p->OccQueries[i],GL_QUERY_RESULT_AVAILABLE,&p->OccResultsAvailable);
			PRINT_GL_ERROR_IF_ANY("FW_GL_GETQUERYOBJECTUIV::QUERY_RESULTS_AVAIL");
		}
		#endif


		#ifdef OCCLUSIONVERBOSE
		if (p->OccResultsAvailable == GL_FALSE) printf ("results not ready for %d\n",i);
		#endif


		/* if we are NOT ready; we keep the count going, but we do NOT change the results of VisibilitySensors */
		if (p->OccResultsAvailable == GL_FALSE) samples = 10000;  
			
	        FW_GL_GETQUERYOBJECTUIV (p->OccQueries[i], GL_QUERY_RESULT, &samples);
		PRINT_GL_ERROR_IF_ANY("FW_GL_GETQUERYOBJECTUIV::QUERY");
				
		#ifdef OCCLUSIONVERBOSE
		printf ("i %d checkc %d samples %d\n",i,checkCount,samples);
		#endif
	
		if (p->occluderNodePointer[i] != 0) {
		
			/* if this is a VisibilitySensor, record the samples */
			if (visSenPtr != NULL) {

				#ifdef OCCLUSIONVERBOSE
				printf ("OcclusionCulling, found VisibilitySensor at %d, fragments %d active %d\n",i,samples,checkCount);
				#endif

                
				/* if this is a DEF/USE, we might already have done this one, as we have same
				   node pointer used in other places. */
				if (checkCount != OCCCHECKNEXTLOOP) {
	
					if (samples > 0) {
						visSenPtr->__visible  = TRUE;
						visSenPtr->__occludeCheckCount = OCCCHECKNEXTLOOP; /* look for this EVERY time through */
						visSenPtr->__Samples = samples;
					} else {
						visSenPtr->__occludeCheckCount = OCCCHECKSOON; /* check again soon */
						visSenPtr->__visible =FALSE;
						visSenPtr->__Samples = 0;
					}

				 /* } else {
					printf ("shape, already have checkCount == OCCCHECKNEXTLOOP, not changing visibility params\n");
				
			          */	
				}
			}
		
		
			/* is this is Shape? */
			else if (shapePtr != NULL) {
				#ifdef OCCLUSIONVERBOSE
				printf ("OcclusionCulling, found Shape %d, fragments %d active %d\n",i,samples,checkCount);
				#endif
                //if (samples == 0) ConsoleMessage ("invisible shape %d, fragments %d",i,samples);
                
				/* if this is a DEF/USE, we might already have done this one, as we have same
				   node pointer used in other places. */
				if (checkCount != OCCWAIT) {
	
					/* is this node visible? If so, tell the parents! */
					if (samples > OCCSHAPESAMPLESIZE) {
						TRACE_MSG ("Shape %p is VISIBLE\n",shapePtr);
						shapePtr->__visible = TRUE;
						shapePtr->__occludeCheckCount= OCCWAIT; /* wait a little while before checking again */
						shapePtr->__Samples = samples;
					} else {
						TRACE_MSG ("Shape %p is NOT VISIBLE\n",shapePtr);
						shapePtr->__visible=FALSE;
						shapePtr->__occludeCheckCount = OCCCHECKSOON; /* check again soon */
						shapePtr->__Samples = 0; 
					}
				/* } else {
					printf ("shape, already have checkCount == OCCWAIT, not changing visibility params\n");
				*/	
				}
			}
		}
	}
#endif /* OCCLUSION */
}

/* shut down the occlusion stuff */
void zeroOcclusion(void) {

#ifdef OCCLUSION /* do we have hardware for occlusion culling? */

	int i;
	ppFrustum p;
	ttglobal tg = gglobal();
	p= (ppFrustum)tg->Frustum.prv;

	if (tg->Frustum.OccFailed) return;

        #ifdef OCCLUSIONVERBOSE
        printf ("zeroOcclusion - potentialOccluderCount %d\n",p->potentialOccluderCount);
        #endif

        for (i=0; i<p->potentialOccluderCount; i++) {
#ifdef OCCLUSIONVERBOSE
                printf ("checking node %d of %d\n",i, p->potentialOccluderCount);
#endif

                FW_GL_GETQUERYOBJECTUIV(p->OccQueries[i],GL_QUERY_RESULT_AVAILABLE,&p->OccResultsAvailable);
                PRINT_GL_ERROR_IF_ANY("FW_GL_GETQUERYOBJECTUIV::QUERY_RESULTS_AVAIL");
#ifdef SLEEP_FOR_QUERY_RESULTS
                /* for now, lets loop to see when we get results */
                while (p->OccResultsAvailable == GL_FALSE) {
#ifdef OCCLUSIONVERBOSE
                        printf ("zero - waiting and looping for results\n"); 
#endif
                        usleep(1000);
                        FW_GL_GETQUERYOBJECTUIV(p->OccQueries[i],GL_QUERY_RESULT_AVAILABLE,&p->OccResultsAvailable);
                        PRINT_GL_ERROR_IF_ANY("FW_GL_GETQUERYOBJECTUIV::QUERY_RESULTS_AVAIL");
                }
#endif
	}
#ifdef OCCLUSIONVERBOSE
	printf ("zeroOcclusion - done waiting\n");
#endif

	p->QueryCount = 0;
    
    // debugging
    //if (p->OccQueries) {
      //  ConsoleMessage ("p->OccQueries exists, p->OccQuerySize %p, p->OccQueries %p",p->OccQuerySize, p->OccQueries);
    //}
	//if(p->OccQueries)
		//glDeleteQueries (p->OccQuerySize, p->OccQueries);
	//FW_GL_FLUSH();
	
	p->OccQuerySize=0;
	p->maxOccludersFound = 0;
	p->OccInitialized = FALSE;
	p->potentialOccluderCount = 0;
    //ConsoleMessage ("freeing OccQueries %p",p->OccQueries);
	FREE_IF_NZ(p->OccQueries);
	FREE_IF_NZ(p->occluderNodePointer);
#endif /* OCCLUSION */
}
