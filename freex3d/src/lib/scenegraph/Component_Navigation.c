/*


X3D Navigation Component

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

#include "../x3d_parser/Bindable.h"
#include "LinearAlgebra.h"
#include "Collision.h"
#include "quaternion.h"
#include "Viewer.h"
#include "../opengl/Frustum.h"
#include "Children.h"
#include "../opengl/OpenGL_Utils.h"
#include "../opengl/Frustum.h"
#include "../scenegraph/RenderFuncs.h"


struct X3D_Node *getActiveLayerBoundViewpoint();
void prep_Viewpoint (struct X3D_Viewpoint *node) {
	double a1;
	GLint viewPort[10];
	X3D_Viewer *viewer;
	if (!renderstate()->render_vp) return;
	viewer = Viewer();

	/* we will never get here unless we are told that we are active by the scene graph; actually
	   doing this test can screw us up, so DO NOT do this test!
			if(!node->isBound) return;
	*/
	if((struct X3D_Node*)node == getActiveLayerBoundViewpoint() && !node->_donethispass){
		node->_donethispass = 1; //if the vp id DEF/USED multiple places in the scengraph, 
								 // this test takes the first one (and helps exit render_node early around virt->children)


		{
			//dug9slerp  this fix works with a test file VP_set_orientation.x3d
			Quaternion q3;
			vrmlrot_to_quaternion(&q3,node->orientation.c[0],node->orientation.c[1],node->orientation.c[2],-node->orientation.c[3]);
			quaternion_togl(&q3);
		}
		FW_GL_TRANSLATE_D(-node->position.c[0],-node->position.c[1],-node->position.c[2]);

		/* now, lets work on the Viewpoint fieldOfView */
		FW_GL_GETINTEGERV(GL_VIEWPORT, viewPort);
		if(viewPort[2] > viewPort[3]) {
			a1=0;
			viewer->fieldofview = node->fieldOfView/3.1415926536*180;
		} else {
			a1 = node->fieldOfView;
			a1 = atan2(sin(a1),viewPort[2]/((float)viewPort[3]) * cos(a1));
			viewer->fieldofview = a1/3.1415926536*180;
		}
	}
	// printf ("render_Viewpoint, bound to %d, fieldOfView %f \n",node,node->fieldOfView); 
}
void draw_viewpoint(int type, float *fov, float aspect);
void render_Viewpoint (struct X3D_Viewpoint *node) {
	float center[3],size[3];
	if(node->_show_pin_point || fwl_getShowViewpoints())
		draw_bbox(double2float(center,node->_pin_point.c,3),vecset3f(size,.4f,.4f,.4f));
	if(fwl_getShowViewpoints()){
		FW_GL_PUSH_MATRIX();
		FW_GL_TRANSLATE_D(node->_position.c[0],node->_position.c[1],node->_position.c[2]);
		FW_GL_ROTATE_RADIANS( node->_orientation.c[3],node->_orientation.c[0],node->_orientation.c[1],
				node->_orientation.c[2]);
		//draw_bbox(vecset3f(center,0.0,0.0,0.0),vecset3f(size,.4f,.4f,1.2f));
		draw_viewpoint(node->_nodeType,&node->fieldOfView,node->aspectRatio);
		FW_GL_POP_MATRIX();
	}
}
void render_OrthoViewpoint (struct X3D_OrthoViewpoint *node) {
	float center[3],size[3];
	if(node->_show_pin_point || fwl_getShowViewpoints())
		draw_bbox(double2float(center,node->_pin_point.c,3),vecset3f(size,.4f,.4f,.4f));
	if(fwl_getShowViewpoints()){
		FW_GL_PUSH_MATRIX();
		FW_GL_TRANSLATE_D(node->_position.c[0],node->_position.c[1],node->_position.c[2]);
		FW_GL_ROTATE_RADIANS( node->_orientation.c[3],node->_orientation.c[0],node->_orientation.c[1],
				node->_orientation.c[2]);
		//draw_bbox(vecset3f(center,0.0,0.0,0.0),vecset3f(size,.4f,.4f,1.2f));
		draw_viewpoint(node->_nodeType,node->fieldOfView.p,1.0f);

		FW_GL_POP_MATRIX();
	}
}
void prep_OrthoViewpoint (struct X3D_OrthoViewpoint *node) {
	int ind;

	if (!renderstate()->render_vp) return;

	/* we will never get here unless we are told that we are active by the scene graph; actually
	   doing this test can screw us up, so DO NOT do this test!
			if(!node->isBound) return;
	*/
	
	if((struct X3D_Node*)node == getActiveLayerBoundViewpoint() && !node->_donethispass){
		node->_donethispass = 1; //if the vp id DEF/USED multiple places in the scengraph, 
	

		/* perform OrthoViewpoint translations */
		FW_GL_ROTATE_RADIANS(-node->orientation.c[3],node->orientation.c[0],node->orientation.c[1],
			node->orientation.c[2]);
		FW_GL_TRANSLATE_D(-node->position.c[0],-node->position.c[1],-node->position.c[2]);

		/* now, lets work on the OrthoViewpoint fieldOfView */
		if (node->fieldOfView.n == 4) {
			for (ind=0; ind<4; ind++) {
					Viewer()->orthoField[ind] = (double) node->fieldOfView.p[ind];
			}
		}
	}
}

/******************************************************************************************/

void proximity_Billboard (struct X3D_Billboard *node) {
	/* printf ("prox_billboard, do nothing\n"); */
}

void prep_Billboard (struct X3D_Billboard *node) {
	if(1){
		//Mar 14 2018 this works with geoViewpoint (GeoTouchSensorExampleB.x3d) and viewpoint (47.x3d)
		double mod[16], modi[16], matr[16], axis[3];
		int align;

		RECORD_DISTANCE
		push_transform_local_identity();

		FW_GL_PUSH_MATRIX();

		//to align with viewepoint, cancel/undo any rotations in modelview matrix
		FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, mod);
		float2double(axis,node->axisOfRotation.c,3);
		align = (APPROX(veclengthd(axis),0.0f));
		if(align){
			//axisOfRotation== (0,0,0) as per specs means full alignment with vp
			// cancel/undo rotations of modelview matrix:
			double modb[16], modbi[16];
			matrixAFFINE2RotationMatrix(modb,mod);
			matinverseAFFINE(matr,modb);
			FW_GL_TRANSFORM_D(matr);
		}else{
			// normal axisOfRotation
			//we calculate an additional swing matrix around the axisOfRotation
			//1. get the position of the vp in billboard-local-coords = vpos
			//2. cross axisOfRotation with vpos to get a perpendicular to both
			//3. cross axisOfRotation with zvec to get a perpendicular to both
			//4. get a rotation difference matrix between those 2 perrp vectors 
			//5. modify modelview by subtracting off the difference rotation
			double vpos[3], zvec[3], perpa[3], perpb[3];
			//calculate position of viewpoint vp in billboard-local coords: vpos
			vecsetd(vpos,0.0,0.0,0.0);
			matinverseAFFINE(modi,mod);
			transformAFFINEd(vpos,vpos,modi);
			vecnormald(vpos,vpos);
			//z axis in billboard-local system zvec
			vecsetd(zvec,0.0,0.0,1.0); 
			//2 cross products
			veccrossd(perpa,axis,vpos);
			veccrossd(perpb,axis,zvec);
			//swing matrix around axisOfRotation
			matrotate2vd(matr,perpa,perpb);
			FW_GL_TRANSFORM_D(matr);
		}
		reset_transform_local(matr);

	}else{
		// not sure why the old way looked at viewer Quat in case of axisOfRotation 0 0 0
		// x didn't work with geoViewpoint
		struct point_XYZ vpos, ax, cp, cp2, arcp;
		static const struct point_XYZ orig = {0.0, 0.0, 0.0};
		static const struct point_XYZ zvec = {0.0, 0.0, 1.0};
		struct orient_XYZA viewer_orient;
		GLDOUBLE mod[16];
		GLDOUBLE proj[16];
		int align;
		double len, len2, angle;
		int sign;

		RECORD_DISTANCE

		ax.x = node->axisOfRotation.c[0];
		ax.y = node->axisOfRotation.c[1];
		ax.z = node->axisOfRotation.c[2];
		align = (APPROX(VECSQ(ax),0));

		viewer_fetch_LCS(Viewer());
		quaternion_to_vrmlrot(&(Viewer()->Quat),
			&(viewer_orient.x), &(viewer_orient.y),
			&(viewer_orient.z), &(viewer_orient.a));

		FW_GL_PUSH_MATRIX();

		FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, mod);
		if(0){
			FW_GL_GETDOUBLEV(GL_PROJECTION_MATRIX, proj);
			FW_GLU_UNPROJECT(orig.x, orig.y, orig.z, mod, proj, viewport, &vpos.x, &vpos.y, &vpos.z);
		}
		if(1){
			//feature-AFFINE_GLU_UNPROJECT
			double modi[16];
			matinverseAFFINE(modi,mod);
			transform(&vpos,&orig,modi);
		}
		len = VECSQ(vpos);
		if (APPROX(len, 0)) { return; }
		VECSCALE(vpos, 1/sqrt(len));

		if (align) {
			ax.x = viewer_orient.x;
			ax.y = viewer_orient.y;
			ax.z = viewer_orient.z;
		}

		VECCP(ax, zvec, arcp);
		len = VECSQ(arcp);
		if (APPROX(len, 0)) { return; }

		len = VECSQ(ax);
		if (APPROX(len, 0)) { return; }
		VECSCALE(ax, 1/sqrt(len));

		VECCP(vpos, ax, cp); /* cp is now 90deg to both vector and axis */
		len = sqrt(VECSQ(cp));
		if (APPROX(len, 0)) {
			FW_GL_ROTATE_RADIANS(-viewer_orient.a, ax.x, ax.y, ax.z);
			return;
		}
		VECSCALE(cp, 1/len);

		/* Now, find out angle between this and z axis */
		VECCP(cp, zvec, cp2);

		len2 = VECPT(cp, zvec); /* cos(angle) */
		len = sqrt(VECSQ(cp2)); /* this is abs(sin(angle)) */

		/* Now we need to find the sign first */
		if (VECPT(cp, arcp) > 0) 
		{ 
			sign = -1; 
		} else { 
			sign = 1; 
		}
		angle = atan2(len2, sign*len);

		FW_GL_ROTATE_RADIANS(angle, ax.x, ax.y, ax.z);
	}
}

void fin_Billboard (struct X3D_Billboard *node) {
	UNUSED(node);
	pop_transform_local();

	FW_GL_POP_MATRIX();
}


void  child_Billboard (struct X3D_Billboard *node) {
    int nc = node->children.n;

	/* any children at all? */
	if (nc==0) return;

	/* do we have a local light for a child? */
	prep_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);
	/* now, just render the non-directionalLight children */
	prep_BBox((struct BBoxFields*)&node->bboxCenter);
	normalChildren(node->children);
	fin_BBox((struct X3D_Node*)node,(struct BBoxFields*)&node->bboxCenter,TRUE);
	fin_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);
}


/******************************************************************************************/

//
//void render_NavigationInfo (struct X3D_NavigationInfo *node) {
//	/* check the set_bind eventin to see if it is TRUE or FALSE */
//	ttglobal tg = gglobal();
//	if (node->set_bind < 100) {
//		if (node->set_bind == 1) set_naviinfo(node);
//		bind_node (X3D_NODE(node), getActiveBindableStacks(tg)->navigation);
//	}
//	if(!node->isBound) return;
//}


/*
Nov 28, 2016 we aren't doing collision->proxy
COLLISION_PROXY
http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/navigation.html#Collision
"""
The collision proxy, defined in the proxy field, is any legal children node as described in 10.2.1 Grouping and children node types that is used as a substitute for the Collision node's children during collision detection. The proxy is used strictly for collision detection; it is not drawn.
"""
http://www.web3d.org/x3d/content/examples/Basic/development/ProxyShapeExampleIndex.html
- example showing you can re-order / change the order / length of children field and proxy should still be proxy
*/

void child_Collision (struct X3D_Collision *node) {
    int nc = node->children.n;
	int i;
	struct X3D_Node *tmpN;

	if(renderstate()->render_collision) {
		/* test against the collide field (vrml) enabled (x3d) and that we actually have a proxy field */
		if((node->collide) && (node->enabled) && !(node->proxy)) {
			struct sCollisionInfo OldCollisionInfo;
			struct sCollisionInfo * ci = CollisionInfo();
			OldCollisionInfo = *ci;
			for(i=0; i<nc; i++) {
				void *p = ((node->children).p[i]);
				#ifdef CHILDVERBOSE
				printf("RENDER COLLISION %d CHILD %d\n",node, p);
				#endif
				render_node(p);
			}
			if((!APPROX(ci->Offset.x,
					OldCollisionInfo.Offset.x)) ||
			   (!APPROX(ci->Offset.y,
				   OldCollisionInfo.Offset.y)) ||
			   (!APPROX(ci->Offset.z,
				    OldCollisionInfo.Offset.z))) {
			/* old code was:
			if(CollisionInfo.Offset.x != OldCollisionInfo.Offset.x ||
			   CollisionInfo.Offset.y != OldCollisionInfo.Offset.y ||
			   CollisionInfo.Offset.z != OldCollisionInfo.Offset.z) { */
				/*collision occured
				 * bit 0 gives collision, bit 1 gives change */
				node->__hit = (node->__hit & 1) ? 1 : 3;
			} else
				node->__hit = (node->__hit & 1) ? 2 : 0;

		}
		if(node->proxy) {
			POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->proxy,tmpN)
			render_node(tmpN);
		}

	} else { /*standard group behaviour*/

		/* do we have a local light for a child? */
		prep_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);
		/* now, just render the non-directionalLight children */
		prep_BBox((struct BBoxFields*)&node->bboxCenter);
		normalChildren(node->children);
		fin_BBox((struct X3D_Node*)node,(struct BBoxFields*)&node->bboxCenter,FALSE);

		fin_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);
	}
}

/* LOD changes between X3D and VRML - level and children fields are "equivalent" */
void child_LOD (struct X3D_LOD *node) {

/*
if (node->_selected != NULL) {
struct X3D_Node *selno = X3D_NODE(node->_selected);
printf ("childLOD %p (root %p), flags %x ",selno,rootNode,selno->_renderFlags);
if ((selno->_renderFlags & VF_Viewpoint) == VF_Viewpoint) printf ("VF_Viewpoint ");
if ((selno->_renderFlags & VF_Geom) == VF_Geom) printf ("VF_Geom ");
if ((selno->_renderFlags & VF_localLight) == VF_localLight) printf ("VF_localLight ");
if ((selno->_renderFlags & VF_Sensitive) == VF_Sensitive) printf ("VF_Sensitive ");
if ((selno->_renderFlags & VF_Blend) == VF_Blend) printf ("VF_Blend ");
if ((selno->_renderFlags & VF_Proximity) == VF_Proximity) printf ("VF_Proximity ");
if ((selno->_renderFlags & VF_Collision) == VF_Collision) printf ("VF_Collision ");
if ((selno->_renderFlags & VF_globalLight) == VF_globalLight) printf ("VF_globalLight ");
if ((selno->_renderFlags & VF_hasVisibleChildren) == VF_hasVisibleChildren) printf ("VF_hasVisibleChildren ");
if ((selno->_renderFlags & VF_shouldSortChildren) == VF_shouldSortChildren) printf ("VF_shouldSortChildren ");
printf ("\n");
}
*/

	prep_BBox((struct BBoxFields*)&node->bboxCenter);
	render_node(node->_selected);
	fin_BBox((struct X3D_Node*)node,(struct BBoxFields*)&node->bboxCenter,FALSE);

}


/* calculate the LOD distance */
void proximity_LOD (struct X3D_LOD *node) {
	GLDOUBLE mod[16];
	GLDOUBLE proj[16];
	struct point_XYZ vec;
	double dist;
	int nran = (node->range).n;
	int nnod = (node->level).n;
	int xnod = (node->children).n;

	int i;

	/* no range, display the first node, if it exists */
	if(!nran) {
		if (node->__isX3D)  {
			if (nnod > 0) node->_selected = (node->children).p[0];
			else node->_selected = NULL;
		} else {
			if (xnod > 0) node->_selected = (node->level).p[0];
			else node->_selected = NULL;
		}
		return;
	}

	/* calculate which one to display */
	FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, mod);
	if(0){
		//this is centered on the front face of the frustum, about .1 away from avatar center (approximately correct)
		/* printf ("LOD, mat %f %f %f\n",mod[12],mod[13],mod[14]); */
		FW_GL_GETDOUBLEV(GL_PROJECTION_MATRIX, proj);
		FW_GLU_UNPROJECT(0,0,0,mod,proj,viewport, &vec.x,&vec.y,&vec.z);
		//printf("old vec= %f %f %f\n", vec.x,vec.y,vec.z);
	}
	if(1){
		//feature-AFFINE_GLU_UNPROJECT
		//this is centered on the avatar (correct)
		double modi[16];
		struct point_XYZ orig = {0.0,0.0,0.0};
		matinverseAFFINE(modi,mod);
		transform(&vec,&orig,modi);
		//printf("new vec= %f %f %f\n", vec.x,vec.y,vec.z);
		//printf("\n");
	}
	vec.x -= (node->center).c[0];
	vec.y -= (node->center).c[1];
	vec.z -= (node->center).c[2];

	dist = sqrt(VECSQ(vec));
	i = 0;

	while (i<nran) {
		if(dist < ((node->range).p[i])) { break; }
		i++;
	}

	/* is this VRML or X3D? */
	if (node->__isX3D) {
		if (xnod > 0) {
			/* X3D "children" field */
			if(i >= xnod) i = xnod-1;
				node->_selected = (node->children).p[i];
				/* printf ("selecting X3D nod %d \n",i); */
		} else node->_selected = NULL;
		
	} else {
		if (nnod > 0) {
			/* VRML "range" field */
			if(i >= nnod) i = nnod-1;
			node->_selected = (node->level).p[i];
			/* printf ("selecting vrml nod\n"); */
		} else { node->_selected = NULL; }
	}
	if(i != node->level_changed){
		node->level_changed = i;
		MARK_EVENT(X3D_NODE(node),offsetof(struct X3D_LOD,level_changed));
	}
}



/************************************************************************
 *
 * ViewpointGroup Node 
 *
 ************************************************************************/
 void add_node_to_broto_context(struct X3D_Proto *currentContext,struct X3D_Node *node);

void compile_ViewpointGroup (struct X3D_ViewpointGroup *node) {
	struct X3D_ProximitySensor *pn;

	/* check if we need to create the proximity node */
	if (node->__proxNode == NULL) {
		/* create proximity */
		pn = (struct X3D_ProximitySensor *) createNewX3DNode(NODE_ProximitySensor);
		if(node->_executionContext)
			add_node_to_broto_context(X3D_PROTO(node->_executionContext),X3D_NODE(pn));

		/* any changes needed here?? */
		node->__proxNode = (void *)pn;

		/* link this in so the VF_Proximity flag will propagate */
		ADD_PARENT(X3D_NODE(pn),X3D_NODE(node));
	}

	/* get the Proximity Node */
	pn = X3D_PROXIMITYSENSOR(node->__proxNode);

	/* copy size, center over */
	memcpy (&pn->center, &node->center, sizeof (float)*3);
	memcpy (&pn->size, &node->size, sizeof (float)*3);

	/* enable it */
	pn->enabled=TRUE;

	/* tell the proximity that it has changed */
	pn->_change++;

	MARK_NODE_COMPILED
}


void child_ViewpointGroup (struct X3D_ViewpointGroup *node) {
        int i;

	/* do we have an attached proximity node? If so, we'll be flagged to do
	   the sensitive pass */

	/* printf ("child_ViewpointGroup, this %u rf %x \n",node,node->_renderFlags);
	  printf ("       ..., render_hier vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
          render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */

	if (renderstate()->render_proximity) {
		if (node->__proxNode != NULL) {
			/* printf ("have prox, rendering it\n"); */
			render_node(X3D_NODE(node->__proxNode));

			/* printf ("prox active %d\n",X3D_PROXIMITYSENSOR(node->__proxNode)->isActive); */
		}

	}

	if (!renderstate()->render_vp) return;

	/* render the viewpoints - one of these will be active */
        for(i=0; i<node->children.n; i++) {
                struct X3D_Node *p = X3D_NODE(node->children.p[i]);
                if (p != NULL) {
                        render_node(p);
                }
        }

}
#ifdef _MSC_VER
#define strcasecmp stricmp
#endif //_MSC_VER

static double screespace_allowed_error = 5.0; //pixles?
void compile_Tile(struct X3D_Tile *node){

}
void prep_Tile(struct X3D_Tile *node){

}
enum {
	TILE_REFINE_DEFAULT = 0,
	TILE_REFINE_REPLACE = 1,
	TILE_REFINE_ADD = 2,
};
void child_Tile(struct X3D_Tile *node){
//
// similar to Tiles3D?
// https://github.com/CesiumGS/3d-tiles/blob/master/3d-tiles-overview.pdf
//
	double screenspace_error = 1.e+06;
	
	int refine = TILE_REFINE_DEFAULT; //we should get it from a stack, so top one dominates.
	if(!strcasecmp(node->refine->strptr,"REPLACE")) refine = TILE_REFINE_REPLACE;
	else if(!strcasecmp(node->refine->strptr,"ADD")) refine = TILE_REFINE_ADD;

	//adapted from proximit_LOD
	{
		double mod[16],modi[16], proj[16], orig[3], origb[3], vec[3],vecb[3], vec4[4], range, viewspace_error, nearplane_error;
		int viewPort[10];
		/* calculate which one to display */
		FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, mod);
		//feature-AFFINE_GLU_UNPROJECT
		//this is centered on the avatar (correct)
		vecsetd(orig,0.0,0.0,0.0); //viewpoint
		matinverseAFFINE(modi,mod);
		transformAFFINEd(vec,orig,modi);
		//printf("new vec= %f %f %f\n", vec.x,vec.y,vec.z);
		//printf("\n");
		range = veclengthd(vec);
		//printf("range= %lf\n",range);


		// Tiles3D S.1 screen space error:
		// sse = (geometricError * screenHeight) / (tileDistance* 2*tan(fovy/2))
		// our method: transform 2 points from tile space to screen space
		// - in tile space they are geometricError distance apart
		// - in screen space they will be SSE apart
		// - should work for orthoViewpoint as well as perspective
		vecsetd(vec,node->geometricError,0.0,-range);
		transformAFFINEd(orig,vec,mod);
		vecsetd(vec,0.0,0.0,-range);
		transformAFFINEd(origb,vec,mod);
		vecdifd(vec,orig,origb);
		viewspace_error = veclengthd(vec);
		FW_GL_GETDOUBLEV(GL_PROJECTION_MATRIX, proj);
		veccopyd(vec4,orig);
		vec4[3] = 1.0;
		transformFULL4d(vec4,vec4,proj);
		vecscaled(orig,vec4,1.0/vec4[3]);
		veccopyd(vec4,origb);
		vec4[3] = 1.0;
		transformFULL4d(vec4,vec4,proj);
		vecscaled(origb,vec4,1.0/vec4[3]);
		vecdifd(vec,orig,origb);
		nearplane_error = veclengthd(vec); 
		FW_GL_GETINTEGERV(GL_VIEWPORT, viewPort);
		screenspace_error = nearplane_error / 2.0 * (double) viewPort[2];
		//printf("screen %lf near %lf view %lf\n",screenspace_error,nearplane_error,viewspace_error);
	}
	if(screenspace_error <= screespace_allowed_error || node->children.n == 0 || refine == TILE_REFINE_ADD){
		render_node(node->content);
		if(node->showContent == FALSE){
			node->showContent = TRUE;
			MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_Tile, showContent));
		}
	}else{
		if(node->showContent == TRUE){
			node->showContent = FALSE;
			MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_Tile, showContent));
		}
	}
	if(screenspace_error > screespace_allowed_error && node->children.n > 0 ){
		//adapted from child_Group:
		prep_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);
		prep_BBox((struct BBoxFields*)&node->bboxCenter);
		normalChildren(node->children);
		fin_BBox((struct X3D_Node*)node,(struct BBoxFields*)&node->bboxCenter,FALSE);
		fin_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);
	}
}
void proximity_Tile(struct X3D_Tile *node){
	//double mod[16],modi[16], orig[3], vec[3];

	///* calculate which one to display */
	//FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, mod);
	////feature-AFFINE_GLU_UNPROJECT
	////this is centered on the avatar (correct)
	//vecsetd(orig,0,.0,0.0,0.0);
	//matinverseAFFINE(modi,mod);
	//transformAFFINEd(vec,orig,modi);
	////printf("new vec= %f %f %f\n", vec.x,vec.y,vec.z);
	////printf("\n");


}
