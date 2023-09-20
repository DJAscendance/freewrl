/*


X3D H-Anim Component

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
#include "../opengl/Material.h"
#include "../opengl/OpenGL_Utils.h"
#include "Children.h"
#include "../scenegraph/RenderFuncs.h"
#include "../opengl/Frustum.h"
#include "LinearAlgebra.h"

/* #include "OpenFW_GL_Utils.h" */

/*
HAnim examples:
http://www.web3d.org/x3d/content/examples/Basic/HumanoidAnimation/
								Octaga				InstantReality
- BoxMan.x3d					good				doesn't animate
- AllenDutton.x3d				skin stuck			good
- NancyStandShootRifleM24.x3d	good				anim good, skin bad
- (KelpForest) NancyDiving.x3d  good				good
HAnim Prototypes:
http://www.web3d.org/x3d/content/examples/Basic/HumanoidAnimation/_pages/page11.html
HAnim examples:
http://www.web3d.org/x3d/content/examples/Basic/#HumanoidAnimation
Specs:
http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/hanim.html
These specs don't articulate the field meanings, instead point to an ISO doc:
http://www.iso.org/iso/home/store/catalogue_tc/catalogue_detail.htm?csnumber=33912
I19774 about $80
Here's some free online docs:
http://www.h-anim.org/
http://h-anim.org/Specifications/H-Anim200x/ISO_IEC_FCD_19774/
http://h-anim.org/Specifications/H-Anim200x/ISO_IEC_FCD_19774/ObjectInterfaces.html
- Humanoid	has transform, skeleton, skin, (flat lists of) skinCoord, skinNormal, segments,joints,sites,viewpoints
- Joint		has transform, children, displacers, skinCoord indx,wt
- Segment	has            children, displacers, mass, coord
- Site		has transform, children 
- Displacer has                      displacements, coord, wt, coord index, 
All the fields are discussed.

July 2016 
- where we left off years ago?
- define HANIMHANIM below to compile - lots of errors
Nov 2016
- stack errors fixed by removing return; at top of some functions
x but doesn't render/animate correctly: whole body frozen, various limbs moving independently/dismembered

Related Links on HAnim
http://www.web3d.org/working-groups/humanoid-animation-h-anim
- The tutorial
Links on character animation for real-time graphics:
http://www.cescg.org/CESCG-2002/MPutzKHufnagl/paper.pdf
- Character Animation for Real-time Applications
http://apc.dcti.iscte.pt/praticas/Real-Time%20Character%20Animation%20for%20Computer%20Games.pdf
- (Anderson) Real-Time Character Animation for Computer Games
https://books.google.ca/books?id=O0sxtDeT5PEC&pg=PA125&lpg=PA125&dq=Animating+single+mesh+character+in+real+time&source=bl&ots=u8gETEoSM0&sig=AW7nQB25K4IxjZp_7xSlLJRxiCQ&hl=en&sa=X&ved=0ahUKEwiQ6NHdoarOAhVX2mMKHVrdDR8Q6AEIJjAB#v=onepage&q=Animating%20single%20mesh%20character%20in%20real%20time&f=false
- book: Real-time Character Animation with Visual C++ 
- shows additive approach to vertex blending:
	vertex {
		xyz original;
		xyz world;
		int number_of_limbs_influencing;
	}
	set number_of_limbs influencing vertex;
	on each frame, 
		zero worlds
		Iterate over limbs, transforming originals influenced and adding/summing onto worlds.
		divide each world by number_of_limbs
http://www.nvidia.com/object/doc_characters.html
- Nvidia link page for various game programming algos with gpu acceleration
http://ruh.li/AnimationVertexSkinning.html
- Skinning shader
http://http.developer.nvidia.com/GPUGems/gpugems_ch04.html
- Bind pose, dawn
http://www.3dgep.com/gpu-skinning-of-md5-models-in-opengl-and-cg/
- Glsl impl of skinning
http://tech-artists.org/wiki/Vertex_Skinning
- ogre sample shader
https://www.cs.tcd.ie/publications/tech-reports/reports.06/TCD-CS-2006-46.pdf
- dual quat skinning


Q if the skeleton isnt rendered, when would you traverse the skeleton
And whats the output?
Q isnt there some kind of ik solver that could/should be exposed to sai as a node?
Otherwise you will need compiled/fast scrpt engine if its all in js, and
scene authors will need to know all that stuff.
https://en.wikipedia.org/wiki/Inverse_kinematics
https://en.wikipedia.org/wiki/Moment_of_inertia
dug9 july 2016: What's Weird about HAnim: 
a) looks like there's very little animation work we have to do in our browser
	the scene author has to do it all in js / via SAI
b) the skeleton isn't rendered (if no geom on sites/segments?)
	so when do you traverse the joints and what's the output?

dug9 aug 2016: would this work: 
Just render_HAnimHumanoid (as un-shared/opaque private scenegraph?)
- traverse the joints and segments privately from HanimHumanoid to call their render_ functions
- no HanimHumanoid? then don't render any joints or segments - don't list virtual functions for them

Design Options:
1. 2-step
a) interpolate the pose of joints and segments relative to HanimHumanoid
	i) start with Identity transform at HanimHumanoid
	ii) traverse down segments and joints, and sites, pushing, multiplying and saving 
		the cumulative transform in each segment/joint/site
b) for each segment/joint/site (done at the HanimHumanoid level)
	i) push cumulative pose for segment/joint/site onto transform stack
	ii) render any attached geometry via noralchildren
2. Combined step
a) traverse down segments pushing and multiplying pose
b) when visiting a segment/joint/site render its children geometry
	x this wont work with skin/'skinning', just attached solid geometry ie scuba tank
	* so instead of the 'render' step for skin, there would be vertex-update step
3. Best Guess
a) traverse skeleton joints rendering attached solid geometry, and updating influenced skin vertices
b) divide skin vertices by number of influencers
c) render skin

Requirements:
- single deformable mesh should be 'easy' / possible / efficient to update / interpolate 

How to update single deformable mesh? 
Please research, as its a common thing in game programming.
Guesses:
- do we have/could we have a system for interpolating the compiled polyrep?
- for example if during compiling the polyrep we kept an expanded index, 
  so a vertex index stored in a joint could find all the triangle vertices in the compiled / tesselated / triangulated mesh?
	array[original index] [list of compiled polyrep vertex indexes]
- or when compiling the HAnimHumanoid, would/could we break the polyrep into chunks associated with segments
	based on 3D proximity
- or would we transform the whole mesh for each segment, except weight the points differently,
	so that the final mesh is a per-vertex-weighted sum of all segment meshes

Nov 2016
state Nov 3, 2016: 
NancyDiving.x3d (scene :HanimHumanoid(HH) with 
	HH->Skeleton children[ RootJoint, Joints, Segments] and 
	HH->joints, HH->segments
	no HH->skin,->viewpoints
x I don't see a single skin mesh/vertices being updated by weighted transforms
x I don't see any mention of Displacer node type
H: its a LOA 0 (or lowest level, with separate segments for each limb)

freewrl 
x I don't see a single skin mesh vertices being updated by weighted transforms in code below
x I don't see any mention of Displacer node type below, although its in perl/structs.h
H: 
it was put together quickly using boilerplate scenegraph calls, for LOA 0,
but without detailed custom code or testing to make it work for LOA 1+

freewrl rendering of NancyDiving.x3d
x skin frozen, while indvidual body segments are transformed separately / dismembered, 
x seem to be missing rotations on the segments
FIXED for LOA-0 ie NancyDiving.x3d

Nov 5, 2016
Need to add .skin weighted vertex blending for LOA-1 ie BoxMan.x3d
2 methods:
CPU - transform coords on CPU side, try to do without re-compiling shape node as a result of change
	a) stream_polyrep actualcoord update based on tesselation indexes, for glDrawArray
	b) replace stream_polyrep with new stream_indexed so child_shape renders via glDrawElements 
		then less processing to update vertices which are kept 1:1 with original
GPU - as with CPU a) or b), except:
	jointTransforms JT[] are sent to gpu, with index and weight as vertex attributes
	https://www.opengl.org/wiki/Skeletal_Animation

How it would all work
on child_humanoid rendering call:
- clear a list of per-skinCoord-vertex indexs PSVI[] and weights PSVW[] size = skinCoord.n
- clear a flat list of transforms, [] size ~= number of joints
- clear a stack of humanoid transforms
- render skeleton
	for each Joint 
		aggregate humanoid transform like we normally do
		GPU method: 
			parse xyz and quaternion from the matrix (is that possible/easy? do I have code?)
			add {xyz,quaternion} joint transform to joint transform list JT[], get its new index JI
		CPU method
			add cumulative humanoid joint transform to joint transform list JT[], get its new index JI
		iterate over joint's list of coordIndices, and for each index
			add the jointTransformIndex JI to PSVI[index]
			add the jointWeight to go with the index PSVW[index] = wt
	for each sites or segment - render as normal scenegraph content
- render skin
	GPU method: set skeletal animation vertex blending shader flag SKELETAL on shaderflags stack
	CPU method: copy and do weighted transform of coords here using PSVI, PSVW
	for each shape in skin:
		draw as normal shape, almost, except: 
		GPU method: send (untransformed) coordinates to shader as vertices as usual
		CPU method: if SKELETAL && CPU substitute transformed coords, and send to shader
		for drawElements, send indices
		if SKELETAL && GPU, then, after compiling/fetching shader:
			send list of jointtransforms to shader as array(s) of vec4 
			send indexes to JTs to shader as vertex attributes
			send weights to shader as vertex attributes
			in shader if SKELETAL && GPU
				apply weighted transforms

June 24, 2020 Note: glTF skinning shows vertex shader formula
- you pass shader a skin mesh in standing pose
- and (an array of) matrices and skin vertex weights
- and vertex shader does the math
https://www.khronos.org/gltf/ 
https://www.khronos.org/files/gltf20-reference-guide.pdf
- reference guide shows vertex shader code for weighted matrix blending for skinning
- freewrl needs this 
x currenlty we are transforming mesh vertices in CPU on each frame - all CPU

*/


/* last HAnimHumanoid skinCoord and skinNormals */
typedef struct pComponent_HAnim{
	double HHMatrix[16];
	Stack *humanoid_stack;
	Stack* joint_center;
}* ppComponent_HAnim;
void *Component_HAnim_constructor(){
	void *v = MALLOCV(sizeof(struct pComponent_HAnim));
	memset(v,0,sizeof(struct pComponent_HAnim));
	return v;
}
void Component_HAnim_init(struct tComponent_HAnim *t){
	//public
	//private
	t->prv = Component_HAnim_constructor();
	{
		ppComponent_HAnim p = (ppComponent_HAnim)t->prv;
		p->humanoid_stack = newStack(struct X3D_HAnimHumanoid*);
		p->joint_center = newStack(float*);
	}
}
void Component_HAnim_clear(struct tComponent_HAnim *t){
	//public
	//private
	{
		ppComponent_HAnim p = (ppComponent_HAnim)t->prv;
		deleteStack(struct X3D_HAnimHumanoid*,p->humanoid_stack);
		deleteStack(float*, p->joint_center);
	}
}
//ppComponent_HAnim p = (ppComponent_HAnim)gglobal()->Component_HAnim.prv;


// compile_HAnimHumanoid and render_ push and pop 
// so accessory nodes when rendered can refer to HH = peek_humanoid() without passing down call stack
void push_humanoid(struct X3D_HAnimHumanoid *HH){
	ppComponent_HAnim p = (ppComponent_HAnim)gglobal()->Component_HAnim.prv;
	stack_push(struct X3D_HAnimHumanoid*,p->humanoid_stack,HH);
}
void pop_humanoid(){
	ppComponent_HAnim p = (ppComponent_HAnim)gglobal()->Component_HAnim.prv;
	stack_pop(struct X3D_HAnimHumanoid *,p->humanoid_stack);
}
struct X3D_HAnimHumanoid * peek_humanoid(){
	ppComponent_HAnim p = (ppComponent_HAnim)gglobal()->Component_HAnim.prv;
	return stack_top(struct X3D_HAnimHumanoid *, p->humanoid_stack);
}
void push_joint_center(float *center) {
	ppComponent_HAnim p = (ppComponent_HAnim)gglobal()->Component_HAnim.prv;
	stack_push(float*, p->joint_center, center);
}
void pop_joint_center() {
	ppComponent_HAnim p = (ppComponent_HAnim)gglobal()->Component_HAnim.prv;
	stack_pop(float*, p->joint_center);
}
float* peek_joint_center() {
	ppComponent_HAnim p = (ppComponent_HAnim)gglobal()->Component_HAnim.prv;
	return stack_top(float*, p->joint_center);
}


void update_jointMatrixFromMotion(struct X3D_Node* HM, char *jname, double *jmatrix);


void compile_HAnimJoint (struct X3D_HAnimJoint *node){

	INITIALIZE_EXTENT;

	/* printf ("changed Transform for node %u\n",node); */
	node->__do_center = verify_translate ((GLfloat *)node->center.c);
	node->__do_trans = verify_translate ((GLfloat *)node->translation.c);
	node->__do_scale = verify_scale ((GLfloat *)node->scale.c);
	node->__do_rotation = verify_rotate ((GLfloat *)node->rotation.c);
	node->__do_scaleO = verify_rotate ((GLfloat *)node->scaleOrientation.c);

	node->__do_anything = (node->__do_center ||
			node->__do_trans ||
			node->__do_scale ||
			node->__do_rotation ||
			node->__do_scaleO);

	//REINITIALIZE_SORTED_NODES_FIELD(node->children,node->_sortedChildren);
	INITIALIZE_EXTENT
	MARK_NODE_COMPILED

}
void prep_HAnimJoint (struct X3D_HAnimJoint *node) {



	COMPILE_IF_REQUIRED

	/* rendering the viewpoint means doing the inverse transformations in reverse order (while poping stack),
		* so we do nothing here in that case -ncoder */

	/* printf ("prep_Transform, render_hier vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */

	/* do we have any geometry visible, and are we doing anything with geometry? */
	//OCCLUSIONTEST

	if(!renderstate()->render_vp) {
		push_transform_local_identity();

		/* do we actually have any thing to rotate/translate/scale?? */
		if (node->__do_anything) {

			FW_GL_PUSH_MATRIX();
			FW_GL_PUSH_MATRIX(); //this is to get us a separate 4x4 matrix just for the stuff here
			FW_GL_LOAD_IDENTITY(); // .. wehich we will save for child_Transform to propagate its bbox up to its extent

			/* TRANSLATION */
			if (node->__do_trans)
				FW_GL_TRANSLATE_F(node->translation.c[0],node->translation.c[1],node->translation.c[2]);

			/* CENTER */
			if (node->__do_center)
				FW_GL_TRANSLATE_F(node->center.c[0],node->center.c[1],node->center.c[2]);
		//any motion nodes enabled? if so apply current frame transform
		if(1){
			struct X3D_HAnimHumanoid *HH = peek_humanoid();
			if(HH->motions.n){
				double modelviewMatrix[16];
				for (int i = 0; i < HH->motions.n; i++) {
					//if(HH->motionsEnabled.p[i]){
					struct X3D_HAnimMotion* HM = (struct X3D_HAnimMotion*)HH->motions.p[i];
					if(HM->transitionWeight > 0.0){
						//printmatrix(jointMatrix.mat);
						FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelviewMatrix);
						update_jointMatrixFromMotion(X3D_NODE(HM),node->name->strptr,modelviewMatrix);
						FW_GL_SETDOUBLEV(GL_MODELVIEW_MATRIX, modelviewMatrix);
						//printmatrix(jointMatrix.mat);
					}
				}
			}
		}


			/* ROTATION */
			if (node->__do_rotation) {
				FW_GL_ROTATE_RADIANS(node->rotation.c[3], node->rotation.c[0],node->rotation.c[1],node->rotation.c[2]);
			}

			/* SCALEORIENTATION */
			if (node->__do_scaleO) {
				FW_GL_ROTATE_RADIANS(node->scaleOrientation.c[3], node->scaleOrientation.c[0], node->scaleOrientation.c[1],node->scaleOrientation.c[2]);
			}


			/* SCALE */
			if (node->__do_scale)
				FW_GL_SCALE_F(node->scale.c[0],node->scale.c[1],node->scale.c[2]);

			/* REVERSE SCALE ORIENTATION */
			if (node->__do_scaleO)
				FW_GL_ROTATE_RADIANS(-node->scaleOrientation.c[3], node->scaleOrientation.c[0], node->scaleOrientation.c[1],node->scaleOrientation.c[2]);

			/* REVERSE CENTER */
			if (node->__do_center)
				FW_GL_TRANSLATE_F(-node->center.c[0],-node->center.c[1],-node->center.c[2]);

			{
				double mat[16];

				FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX,mat); //we got our local transform saved
				FW_GL_POP_MATRIX();
				FW_GL_TRANSFORM_D(mat); //now apply the above to prep for child_Tranform
				reset_transform_local(mat);
			}

		} 

		//RECORD_DISTANCE

	}

}


void fin_HAnimJoint (struct X3D_HAnimJoint *node) {

	OCCLUSIONTEST

	if(!renderstate()->render_vp) {
		pop_transform_local();
		if (node->__do_anything) {
			FW_GL_POP_MATRIX();
		}
	} else {
		/*Rendering the viewpoint only means finding it, and calculating the reverse WorldView matrix.*/
		if((node->_renderFlags & VF_Viewpoint) == VF_Viewpoint) {
			FW_GL_TRANSLATE_F(((node->center).c[0]),((node->center).c[1]),((node->center).c[2])
			);
			FW_GL_ROTATE_RADIANS(((node->scaleOrientation).c[3]),((node->scaleOrientation).c[0]),((node->scaleOrientation).c[1]),((node->scaleOrientation).c[2])
			);
			FW_GL_SCALE_F((float)1.0/(((node->scale).c[0])),(float)1.0/(((node->scale).c[1])),(float)1.0/(((node->scale).c[2]))
			);
			FW_GL_ROTATE_RADIANS(-(((node->scaleOrientation).c[3])),((node->scaleOrientation).c[0]),((node->scaleOrientation).c[1]),((node->scaleOrientation).c[2])
			);
			FW_GL_ROTATE_RADIANS(-(((node->rotation).c[3])),((node->rotation).c[0]),((node->rotation).c[1]),((node->rotation).c[2])
			);
			FW_GL_TRANSLATE_F(-(((node->center).c[0])),-(((node->center).c[1])),-(((node->center).c[2]))
			);
			FW_GL_TRANSLATE_F(-(((node->translation).c[0])),-(((node->translation).c[1])),-(((node->translation).c[2]))
			);
		}
	}

} 

void compile_HAnimSite (struct X3D_HAnimSite *node){

	INITIALIZE_EXTENT;

	/* printf ("changed Transform for node %u\n",node); */
	node->__do_center = verify_translate ((GLfloat *)node->center.c);
	node->__do_trans = verify_translate ((GLfloat *)node->translation.c);
	node->__do_scale = verify_scale ((GLfloat *)node->scale.c);
	node->__do_rotation = verify_rotate ((GLfloat *)node->rotation.c);
	node->__do_scaleO = verify_rotate ((GLfloat *)node->scaleOrientation.c);

	node->__do_anything = (node->__do_center ||
			node->__do_trans ||
			node->__do_scale ||
			node->__do_rotation ||
			node->__do_scaleO);

	//REINITIALIZE_SORTED_NODES_FIELD(node->children,node->_sortedChildren);
	INITIALIZE_EXTENT
	MARK_NODE_COMPILED

}
void prep_HAnimSite (struct X3D_HAnimSite *node) {



	COMPILE_IF_REQUIRED

	/* rendering the viewpoint means doing the inverse transformations in reverse order (while poping stack),
		* so we do nothing here in that case -ncoder */

	/* printf ("prep_Transform, render_hier vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */

	/* do we have any geometry visible, and are we doing anything with geometry? */
	//OCCLUSIONTEST

	if(!renderstate()->render_vp) {
		/* do we actually have any thing to rotate/translate/scale?? */
		push_transform_local_identity();

		if (node->__do_anything) {

			FW_GL_PUSH_MATRIX();
			FW_GL_PUSH_MATRIX(); //this is to get us a separate 4x4 matrix just for the stuff here
			FW_GL_LOAD_IDENTITY(); // .. wehich we will save for child_Transform to propagate its bbox up to its extent

			/* TRANSLATION */
			if (node->__do_trans)
				FW_GL_TRANSLATE_F(node->translation.c[0],node->translation.c[1],node->translation.c[2]);

			/* CENTER */
			if (node->__do_center)
				FW_GL_TRANSLATE_F(node->center.c[0],node->center.c[1],node->center.c[2]);

			/* ROTATION */
			if (node->__do_rotation) {
				FW_GL_ROTATE_RADIANS(node->rotation.c[3], node->rotation.c[0],node->rotation.c[1],node->rotation.c[2]);
			}

			/* SCALEORIENTATION */
			if (node->__do_scaleO) {
				FW_GL_ROTATE_RADIANS(node->scaleOrientation.c[3], node->scaleOrientation.c[0], node->scaleOrientation.c[1],node->scaleOrientation.c[2]);
			}


			/* SCALE */
			if (node->__do_scale)
				FW_GL_SCALE_F(node->scale.c[0],node->scale.c[1],node->scale.c[2]);

			/* REVERSE SCALE ORIENTATION */
			if (node->__do_scaleO)
				FW_GL_ROTATE_RADIANS(-node->scaleOrientation.c[3], node->scaleOrientation.c[0], node->scaleOrientation.c[1],node->scaleOrientation.c[2]);

			/* REVERSE CENTER */
			if (node->__do_center)
				FW_GL_TRANSLATE_F(-node->center.c[0],-node->center.c[1],-node->center.c[2]);
			{
				double mat[16];

				FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX,mat); //we got our local transform saved
				FW_GL_POP_MATRIX();
				FW_GL_TRANSFORM_D(mat); //now apply the above to prep for child_Tranform
				reset_transform_local(mat);
			}

		} 

		//RECORD_DISTANCE

	}

}


void fin_HAnimSite (struct X3D_HAnimSite *node) {

	OCCLUSIONTEST

	if(!renderstate()->render_vp) {
		pop_transform_local();
		if (node->__do_anything) {
			FW_GL_POP_MATRIX();
		}
	} else {
		/*Rendering the viewpoint only means finding it, and calculating the reverse WorldView matrix.*/
		if((node->_renderFlags & VF_Viewpoint) == VF_Viewpoint) {
			FW_GL_TRANSLATE_F(((node->center).c[0]),((node->center).c[1]),((node->center).c[2])
			);
			FW_GL_ROTATE_RADIANS(((node->scaleOrientation).c[3]),((node->scaleOrientation).c[0]),((node->scaleOrientation).c[1]),((node->scaleOrientation).c[2])
			);
			FW_GL_SCALE_F((float)1.0/(((node->scale).c[0])),(float)1.0/(((node->scale).c[1])),(float)1.0/(((node->scale).c[2]))
			);
			FW_GL_ROTATE_RADIANS(-(((node->scaleOrientation).c[3])),((node->scaleOrientation).c[0]),((node->scaleOrientation).c[1]),((node->scaleOrientation).c[2])
			);
			FW_GL_ROTATE_RADIANS(-(((node->rotation).c[3])),((node->rotation).c[0]),((node->rotation).c[1]),((node->rotation).c[2])
			);
			FW_GL_TRANSLATE_F(-(((node->center).c[0])),-(((node->center).c[1])),-(((node->center).c[2]))
			);
			FW_GL_TRANSLATE_F(-(((node->translation).c[0])),-(((node->translation).c[1])),-(((node->translation).c[2]))
			);
		}
	}

} 

typedef  struct {
	double mat [16];
	float normat[9];
}  JMATRIX;
enum {
	VERTEXTRANSFORMMETHOD_CPU = 1,
	VERTEXTRANSFORMMETHOD_GPU = 2,
};
static int vertexTransformMethod = VERTEXTRANSFORMMETHOD_CPU;
char* lookup_brotoDefname(struct X3D_Proto* ec, struct X3D_Node* node);

void render_HAnimHumanoid (struct X3D_HAnimHumanoid *node) {
	/* save the skinCoords and skinNormals for use in following HAnimJoints */
	//printf ("rendering HAnimHumanoid DEF %s type %s\n", lookup_brotoDefname(X3D_PROTO(node->_executionContext), X3D_NODE(node)), stringNodeType(node->_nodeType));

}
void render_rig_segment(float* jcenter) {
	//needs work, idea is to render rig, based on absolute centers, 
	// and rely on parent transform stack to orient.
	// works a bit but lots of misses, don't know what to conclude or what to fix.

	float extent[6], scale, size[3], center[3], diff[3], add[3];
	vecdif3f(diff, peek_joint_center(), jcenter);
	scale = veclength3f(diff);
	vecadd3f(add, peek_joint_center(), jcenter);
	vecscale3f(center, add, .5f);
	size[0] = size[2] = scale*.1;
	size[1] = scale;
	bbox2extent6f(center, size, extent);
	extent6f_draw(extent);
}

void render_HAnimJoint (struct X3D_HAnimJoint * node) {
	int i,j, jointTransformIndex;
	double modelviewMatrix[16]; //, mvmInverse[16];
	struct X3D_HAnimHumanoid *HH;
	JMATRIX jointMatrix;
	Stack *JT;
	float *PVW, *PVI;


	ppComponent_HAnim p = (ppComponent_HAnim)gglobal()->Component_HAnim.prv;
	//printf ("rendering HAnimJoint DEF %s type %s\n", lookup_brotoDefname(X3D_PROTO(node->_executionContext), X3D_NODE(node)), stringNodeType(node->_nodeType));
	
	HH = peek_humanoid();
	if(HH){
		JT = HH->_JT;
		// needs work, needs a launch parameter or HAnim field flag
		if(0) render_rig_segment(node->center.c);

		//step 1, generate transform
		FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelviewMatrix);
		matmultiplyAFFINE(jointMatrix.mat,modelviewMatrix,p->HHMatrix);

		//any motion nodes enabled? if so apply current frame transform
		if(0) if(HH->motions.n){
			for(int i=0;i<HH->motions.n;i++){
				if(HH->motionsEnabled.p[i]){
					//printmatrix(jointMatrix.mat);
					update_jointMatrixFromMotion(HH->motions.p[i],node->name->strptr,jointMatrix.mat);
					//printmatrix(jointMatrix.mat);
				}
			}
		}
		if(HH->skinNormal){
			//want 'inverse-transpose' 3x3 float for transforming normals
			//(its almost the same as jointMatrix.mat except when shear due to assymetric scales)
			float fmat4[16], fmat3[9],fmat3i[9]; //,fmat3it[9];
			matdouble2float4(fmat4,jointMatrix.mat);
			mat423f(fmat3,fmat4);
			matinverse3f(fmat3i,fmat3);
			mattranspose3f(jointMatrix.normat,fmat3i);
			//printf("jm.normat[1] %f\n",jointMatrix.normat[1]);
		}

		if(vertexTransformMethod == VERTEXTRANSFORMMETHOD_GPU){
			//convert to quaternion + position
			//add to HH transform list
		}else if(vertexTransformMethod == VERTEXTRANSFORMMETHOD_CPU){
			//step 2, add transform to HH transform list, get its index in list
			stack_push(JMATRIX,JT,jointMatrix);
		}
		//I'll let this index start at 1, and subtract 1 when retrieving with vector_get, 
		//so I can use jointTransformIndex==0 as a sentinal value for 'no transform stored'
		//to save me from having an extra .n transforms variable
		jointTransformIndex = vectorSize(JT); 
	
		//step 3, add transform index and weight to each skin vertex
		PVW = (float*)HH->_PVW;
		PVI = (float*)HH->_PVI;
		for(i=0;i<node->skinCoordIndex.n;i++){
			int idx = node->skinCoordIndex.p[i];
			float wt = node->skinCoordWeight.p[min(i,node->skinCoordWeight.n -1)];
			for(j=0;j<4;j++){
				if(PVI[idx*4 + j] == 0.0f){
					PVI[idx*4 +j] = (float)jointTransformIndex;
					PVW[idx*4 +j] = wt;
				}
			}
		}
		//step 4: add on any Displacer displacements
		if(HH->skinCoord && node->displacers.n ){
			int ni, i;
			float *psc, *pdp;
			int *ci;
			struct X3D_Coordinate *nc = (struct X3D_Coordinate*)HH->skinCoord;
			psc = (float*)nc->point.p;
			// nsc = nc->point.n;
			for(i=0;i<node->displacers.n;i++){
				int index, j;
				float *point, weight, wdisp[3];
				struct X3D_HAnimDisplacer *dp = (struct X3D_HAnimDisplacer *)node->displacers.p[i];
				
				weight = dp->weight;
				//printf(" %f ",weight);
				pdp = (float*)dp->displacements.p;
				// ndp = dp->displacements.n;

				ni = dp->coordIndex.n;
				ci = dp->coordIndex.p;
				for(j=0;j<ni;j++){
					index = ci[j];
					point = &psc[index*3];
					vecscale3f(wdisp,&pdp[j*3],weight);
					vecadd3f(point,point,wdisp);
				}
			}
			if(0){ //this is done in child_HAnimHumanoid for the skinCoord parents
				//force HAnimSegment.children[] shape nodes using segment->coord to recompile
				int k;
				Stack *parents;
				HH->skinCoord->_change++;
				parents = HH->skinCoord->_parentVector;
				for(k=0;k<vectorSize(parents);k++){
					struct X3D_Node *parent = vector_get(struct X3D_Node*,parents,k);
					parent->_change++;
				}
			}

		}
	} //if HH

}
int vecsametol3f(float *a, float *b, float tol){
	int i,isame = TRUE;
	for(i=0;i<3;i++)
		if(fabsf(a[i] - b[i]) > tol) isame = FALSE;
	return isame;
}


void compile_HAnimHumanoid(struct X3D_HAnimHumanoid* node) {
	//printf("compile_HAnimHumanoid\n");
	//check if the coordinate count is the same
	INITIALIZE_EXTENT

		push_humanoid(node);
	if (node->motions.n) {
		if (node->motions.n > node->motionsEnabled.n) {
			// the default is to enable all motions
			int* moe = MALLOC(int*, node->motions.n * sizeof(int));
			memset(moe, 0, node->motions.n * sizeof(int));
			memcpy(moe, node->motionsEnabled.p, node->motionsEnabled.n * sizeof(int));

			for (int i = node->motionsEnabled.n; i < node->motions.n; i++) {
				moe[i] = TRUE; //FALSE //not sure - specs don't say default, just empty [], I'll use TRUE while developing/debugging
	
			}
			FREE_IF_NZ(node->motionsEnabled.p);
			node->motionsEnabled.p = moe;
			node->motionsEnabled.n = node->motions.n;

		}
		if (node->motions.n > node->_lastMotionsEnabled.n) {
			node->_lastMotionsEnabled.p = MALLOC(int*, node->motions.n * sizeof(int));
			node->_lastMotionsEnabled.n = node->motions.n;
			for (int i = 0; i < node->_lastMotionsEnabled.n; i++)
				node->_lastMotionsEnabled.p[i] = 0;
		}
		for (int i = 0; i < node->motions.n; i++) {
			check_compile(node->motions.p[i]);
		}
	}

	int nsc = 0, nsn = 0;
	float* psc = NULL, * psn = NULL;
	if (node->skinCoord && node->skinCoord->_nodeType == NODE_Coordinate) {
		float ee[6];
		struct X3D_Coordinate* nc = (struct X3D_Coordinate*)node->skinCoord;
		nsc = nc->point.n;
		psc = (float*)nc->point.p;
		node->_origCoords = realloc(node->_origCoords, nsc * 3 * sizeof(float));
		memcpy(node->_origCoords, psc, nsc * 3 * sizeof(float));
		if (0) {
			//find a few coordinates in skinCoord I hacked, by xyz, and give me their index, for making a displacer
			float myfind[9] = { -0.030000f, -0.070000f, 1.777000f,  -0.070000f, 1.777000f, 0.130000f,  1.777000f, 0.130000f, 0.070000f };
			int i, j;
			for (i = 0; i < nsc; i++) {
				for (j = 0; j < 3; j++)
					if (vecsametol3f(&psc[i * 3], &myfind[j * 3], .001f)) {
						printf("%d %f %f %f\n", i, myfind[j * 3 + 0], myfind[j * 3 + 1], myfind[j * 3 + 2]);
					}
			}
		}
		//extent6f_from_box3fn(ee,nc->point.p->c, nc->point.n);
		//setExtent(ee[0],ee[1],ee[2],ee[3],ee[4],ee[5],X3D_NODE(node));
	}
	if (node->skinNormal && node->skinNormal->_nodeType == NODE_Normal) {
		struct X3D_Normal* nn = (struct X3D_Normal*)node->skinNormal;
		//Assuming 1 normal per coord, coord 1:1 normal
		nsn = nn->vector.n;
		psn = (float*)nn->vector.p;
		node->_origNorms = realloc(node->_origNorms, nsn * 3 * sizeof(float));
		memcpy(node->_origNorms, psn, nsn * 3 * sizeof(float));
	}

	//allocate the joint-transform_index and joint-weight arrays
	//Nov 2016: max 4: meaning each skinCoord can have up to 4 joints referencing/influencing it
	//4 chosen so it's easier to port to GPU method with vec4
	if (node->_NV == 0 || node->_NV != nsc) {
		node->_PVI = realloc(node->_PVI, nsc * 4 * sizeof(float)); //indexes, up to 4 joints per skinCoord
		node->_PVW = realloc(node->_PVW, nsc * 4 * sizeof(float)); //weights, up to 4 joints per skinCoord
		node->_NV = nsc;
	}
	//allocate the transform array
	if (node->_JT == NULL) {
		if (vertexTransformMethod == VERTEXTRANSFORMMETHOD_GPU) {
			//new stack quat + position
		}
		else if (vertexTransformMethod == VERTEXTRANSFORMMETHOD_CPU) {
			node->_JT = newStack(JMATRIX); //we don't know how many joints there are - need to count as we go
		}
	}
	node->_renderFlags |= VF_Geom; //a HAnimHumanoid is a child but also skin is geom
	MARK_NODE_COMPILED
	pop_humanoid();

}

void child_HAnimHumanoid(struct X3D_HAnimHumanoid *node) {
	int nc;
	//float *originalCoords;
	struct X3D_HAnimHumanoid *HH;
	Stack *JT;
	ppComponent_HAnim p = (ppComponent_HAnim)gglobal()->Component_HAnim.prv;
	COMPILE_IF_REQUIRED
	//LOCAL_LIGHT_SAVE

	/* any segments at all? */
/*
printf ("hanimHumanoid, segment counts joints %d segs %d sites %d skeleton %d skin %d vps %d\n",
		node->joints.n,
		node->segments.n,
		node->sites.n,
		node->skeleton.n,
		node->skin.n,
		node->viewpoints.n);
*/

	nc = node->joints.n + node->segments.n + node->viewpoints.n + node->sites.n +
		node->skeleton.n + node->skin.n;

	RETURN_FROM_CHILD_IF_NOT_FOR_ME 
	push_humanoid(node);

	if(renderstate()->render_vp){
		/* Lets do viewpoints */
		normalChildren(node->viewpoints);
		return;
	}
	
	if(node->motions.n){
		int nkept = 0;
		for(int i=0;i<node->motions.n;i++){
			struct X3D_HAnimMotion* HM = (struct X3D_HAnimMotion*)node->motions.p[i];
			int keep = node->motionsEnabled.p[i];
			HM->transitionWeight = 1.0f;
			if (HM->transitionStart == 0.0) HM->transitionStart = TickTime() - node->transitionTime;
			if (node->transitionTime > 0.0) {
				if (node->motionsEnabled.p[i] != node->_lastMotionsEnabled.p[i]) {
					HM->transitionStart = TickTime();
				}
				double dtime = TickTime() - HM->transitionStart;
				float weight = dtime / node->transitionTime;
				//printf("%f ", weight);
				weight = min(1.0f, weight);
				if (node->motionsEnabled.p[i]) 
					HM->transitionWeight = weight;
				else HM->transitionWeight = 1.0f - weight;
				if (HM->transitionWeight > 0.0f) keep = TRUE;
				node->_lastMotionsEnabled.p[i] = node->motionsEnabled.p[i];
			}
			if (keep) {
				//if(HM->transitionWeight < 1.0f)
				//  printf("%d %f  ", i, HM->transitionWeight);
				render_node(X3D_NODE(node->motions.p[i]));
				nkept++;
			}
		}
		//printf("%d", nkept);
	}

	// segments, joints, sites are flat-lists for convenience
	// skeleton is the scenegraph-like transform hierarchy of joints and segments and sites
	// skin relies on something updating its vertices based on skeleton transforms
	/* Lets do segments first */
	/* now, just render the non-directionalLight segments */
	if(0) normalChildren(node->segments);


	/* Lets do joints second */
	/* do we have to sort this node? */
	/* now, just render the non-directionalLight joints */
	if(0) normalChildren(node->joints);


	/* Lets do sites third */
	/* do we have to sort this node? */
	/* do we have a local light for a child? */
	//LOCAL_LIGHT_CHILDREN(node->sites);
	/* now, just render the non-directionalLight sites */
	if(0) normalChildren(node->sites);

	prep_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);

	prep_BBox((struct BBoxFields*)&node->bboxCenter);


	/* Lets do skeleton fourth */
	/* do we have to sort this node? */
	/* now, just render the non-directionalLight skeleton */
	//skeleton is the basic thing to render for LOA 0
	memset(node->_PVI,0,4*node->_NV*sizeof(float));
	memset(node->_PVW,0,4*node->_NV*sizeof(float));
	JT = node->_JT; 
	JT->n = 0;

	//in theory, HH, HHMatrix could be a stack, so you could have an hanimhumaoid within an hanimhunaniod
	HH = node;
	{
		double modelviewMatrix[16];
		FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelviewMatrix);
		matinverseAFFINE(p->HHMatrix,modelviewMatrix);
	}
	if(node->skin.n){
		if(vertexTransformMethod == VERTEXTRANSFORMMETHOD_CPU){
			//save original coordinates before rendering skeleton
			// - HAnimJoint may have displacers that change the Coords
			//transform each vertex and its normal using weighted transform
			int nsc = 0, nsn = 0;
			float *psc = NULL, *psn = NULL;
			if(node->skinCoord && node->skinCoord->_nodeType == NODE_Coordinate){
				struct X3D_Coordinate * nc = (struct X3D_Coordinate * )node->skinCoord;
				struct X3D_Normal *nn = (struct X3D_Normal *)node->skinNormal; //might be NULL 
				nsc = nc->point.n;
				psc = (float*)nc->point.p;
				memcpy(psc,node->_origCoords,3*nsc*sizeof(float));
				if(nn){
					nsn = nn->vector.n;
					psn = (float *)nn->vector.p;
					memcpy(psn,node->_origNorms,3*nsn*sizeof(float));
				}
			}
		}

	}

	float zerocenter[3];
	push_joint_center(vecset3f(zerocenter, 0.0f, 0.0f, 0.0f));
	if(1) normalChildren(node->skeleton); //render_HAnimJoint happens here
	pop_joint_center();

	if(node->skin.n){
		if(vertexTransformMethod == VERTEXTRANSFORMMETHOD_CPU){
			//save original coordinates
			//transform each vertex and its normal using weighted transform
			int i,j,nsc = 0;
			// int  nsn = 0;
			float *psc = NULL, *psn = NULL;
			if(node->skinCoord && node->skinCoord->_nodeType == NODE_Coordinate){
				float ee[6];
				struct X3D_Coordinate * nc = (struct X3D_Coordinate * )node->skinCoord;
				struct X3D_Normal *nn = (struct X3D_Normal *)node->skinNormal; //might be NULL 
				nsc = nc->point.n;
				psc = (float*)nc->point.p[0].c;
				//memcpy(psc,node->_origCoords,3*nsc*sizeof(float));
				if(nn){
					// nsn = nn->vector.n;
					psn = (float *)nn->vector.p;
					//memcpy(psn,node->_origNorms,3*nsn*sizeof(float));
				}

				for(i=0;i<nsc;i++){
					float totalWeight;
					float *point, *norm; 
					float newpoint[3], newnorm[3];
					float *PVW, *PVI;

					point = &psc[i*3];
					norm = NULL;
					if(nn) norm = &psn[i*3];
					PVW = node->_PVW;
					PVI = node->_PVI;

					memset(newpoint,0,3*sizeof(float));
					memset(newnorm,0,3*sizeof(float));
					totalWeight = 0.0f;
					for(j=0;j<4;j++){
						int jointTransformIndex = (int)PVI[i*4 + j];
						float wt = PVW[i*4 + j];
						if(jointTransformIndex > 0){
							float tpoint[3], tnorm[3];
							JMATRIX jointMatrix;
							jointMatrix = vector_get(JMATRIX,node->_JT,jointTransformIndex -1);
							transformf(tpoint,point,jointMatrix.mat);
							vecscale3f(tpoint,tpoint,wt);
							vecadd3f(newpoint,newpoint,tpoint);
							if(nn){
								transform3x3f(tnorm,norm,jointMatrix.normat);
								vecnormalize3f(tnorm,tnorm); 
								vecscale3f(tnorm,tnorm,wt);
								vecadd3f(newnorm,newnorm,tnorm);
							}
							totalWeight += wt;
						}
					}
					if(totalWeight > 0.0f){
						vecscale3f(newpoint,newpoint,1.0f/totalWeight);
						veccopy3f(point,newpoint);
						if(nn){
							vecscale3f(newnorm,newnorm,1.0f/totalWeight);
							vecnormalize3f(norm,newnorm);
						}
					}
				}
				if(0){
					//print out before and after coords
					float *osc = node->_origCoords;
					for(i=0;i<nsc;i++){
						printf("%d ",i);
						for(j=0;j<3;j++) printf("%f ",psc[i*3 +j]);
						printf("/ ");
						for(j=0;j<3;j++) printf("%f ",osc[i*3 +j]);
						printf("\n");
					}
					printf("\n");
				}

				//trigger recompile of skin->shapes when rendering skin
				//Nov 6, 2016: recompiling a shape / polyrep on each frame eats memory 
				//NODE_NEEDS_COMPILING
				if(1){
					int k;
					Stack *parents;
					node->skinCoord->_change++;
					parents = node->skinCoord->_parentVector;
					for(k=0;k<vectorSize(parents);k++){
						struct X3D_Node *parent = vector_get(struct X3D_Node*,parents,k);
						parent->_change++;
					}
				}
				//extent6f_from_box3fn(ee, psc, nsc);
				//setExtent(ee[0], ee[1], ee[2], ee[3], ee[4], ee[5], X3D_NODE(node));

			}
		}else if(vertexTransformMethod == VERTEXTRANSFORMMETHOD_GPU){
			//push shader flaga with += SKELETAL
		}
		if(1) normalChildren(node->skin);
		if(0) for (int j = 0; j < node->skin.n; j++) {
			printf("skin[%d] extent: ", j);
			for (int i = 0; i < 6; i++) printf("%4.3f ", node->skin.p[j]->_extent[i]);
			printf("\n");
		}

		if(vertexTransformMethod == VERTEXTRANSFORMMETHOD_GPU){
			//pop shader flags
		} else if(vertexTransformMethod == VERTEXTRANSFORMMETHOD_CPU){
			//restore original coordinates 
			int nsc, nsn;
			float *psc, *psn;
			struct X3D_Coordinate * nc = (struct X3D_Coordinate * )node->skinCoord;
			struct X3D_Normal * nn = (struct X3D_Normal * )node->skinNormal;
			nsc = nc->point.n;
			psc = (float*)nc->point.p;
			memcpy(psc,node->_origCoords,3*nsc*sizeof(float));
			if(nn){
				nsn = nn->vector.n;
				psn = (float*)nn->vector.p;
				memcpy(psn,node->_origNorms,3*nsn*sizeof(float));
			}
		}
	} //if skin
	//if (renderstate()->render_geom) printf("humanoid gets geom and other=%d\n",renderstate()->render_other);
	fin_BBox((struct X3D_Node*)node,(struct BBoxFields*)&node->bboxCenter,FALSE);
	//printf("bboxCenter %f %f %f size %f %f %f\n", node->bboxCenter.c[0], node->bboxCenter.c[1], node->bboxCenter.c[2],
	//	node->bboxSize.c[0], node->bboxSize.c[1], node->bboxSize.c[2]);
	fin_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);


	/* did we have that directionalLight? */
	//LOCAL_LIGHT_OFF
	pop_humanoid();
}


void child_HAnimJoint(struct X3D_HAnimJoint *node) {

	//CHILDREN_COUNT
	/* any children at all? */
	//if (nc==0) return;

	/* should we go down here? */
	//RETURN_FROM_CHILD_IF_NOT_FOR_ME

	/* do we have to sort this node? */

	/* just render the non-directionalLight children */
	prep_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);
	prep_BBox((struct BBoxFields*)&node->bboxCenter);

	push_joint_center(node->center.c); //joint center for drawing armature
	/* now, just render the non-directionalLight children */
	normalChildren(node->children);
	pop_joint_center();

	fin_BBox((struct X3D_Node*)node,(struct BBoxFields*)&node->bboxCenter,TRUE);
	fin_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);
}
float *vecmix3f(float *out3, float* a3, float *b3, float fraction){
	int i;
	for(i=0;i<3;i++){
		out3[i] = (1.0f - fraction)*a3[i] + fraction*b3[i];
	}
	return out3;
}
void child_HAnimSegment(struct X3D_HAnimSegment *node) {

	//CHILDREN_COUNT


//note to implementer: have to POSSIBLE_PROTO_EXPANSION(node->coord, tmpN)

	/* any children at all? */
	//if (nc==0) return;

	/* should we go down here? */
	//RETURN_FROM_CHILD_IF_NOT_FOR_ME

	/* do we have to sort this node? Only if not a proto - only first node has visible children. */

	/* now, just render the non-directionalLight children */
	if(node->coord && node->displacers.n){

		int nsc, ni, i;
		float *psc, *pdp;
		int *ci;
		struct X3D_Coordinate *nc = (struct X3D_Coordinate*)node->coord;
		psc = (float*)nc->point.p;
		nsc = nc->point.n;
		if(!node->_origCoords)
			node->_origCoords = malloc(3*nsc*sizeof(float));
		memcpy(node->_origCoords,psc,3*nsc*sizeof(float));
		for(i=0;i<node->displacers.n;i++){
			int index, j;
			float *point, weight, wdisp[3];
			struct X3D_HAnimDisplacer *dp = (struct X3D_HAnimDisplacer *)node->displacers.p[i];
				
			weight = dp->weight;
			//printf(" %f ",weight);
			pdp = (float*)dp->displacements.p;
			// ndp = dp->displacements.n;

			ni = dp->coordIndex.n;
			ci = dp->coordIndex.p;
			for(j=0;j<ni;j++){
				index = ci[j];
				point = &psc[index*3];
				vecscale3f(wdisp,&pdp[j*3],weight);
				vecadd3f(point,point,wdisp);
			}
		}
		if(1){
			//force HAnimSegment.children[] shape nodes using segment->coord to recompile
			Stack *parents;
			int k;
			node->coord->_change++;
			parents = node->coord->_parentVector;
			for(k=0;k<vectorSize(parents);k++){
				struct X3D_Node *parent = vector_get(struct X3D_Node*,parents,k);
				parent->_change++;
			}
		}

		if(0){
			//find a few coordinates in segment->coord I hacked, by xyz, and give me their index, 
			// for making a displacer
			float myfind[9] = {-0.029100f, 1.603000f, 0.042740f,    -0.045570f, 1.601000f, 0.036520f,    -0.018560f, 1.600000f, 0.043490f };
			int j,found = FALSE;
			printf("\n");
			for(i=0;i<nsc;i++){
				for(j=0;j<3;j++)
					if(vecsametol3f(&psc[i*3],&myfind[j*3],.0001f)){
						printf("%d %f %f %f\n",i,myfind[j*3 + 0],myfind[j*3 +1],myfind[j*3 +2]);
						found = TRUE;
					}
			}
			if(found)
				printf("\n");
		}
	}
	prep_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);
	prep_BBox((struct BBoxFields*)&node->bboxCenter);

	normalChildren(node->children);

	fin_BBox((struct X3D_Node*)node,(struct BBoxFields*)&node->bboxCenter,FALSE);
	fin_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);

	if(node->coord && node->displacers.n){
		int nsc;
		float *psc;
		struct X3D_Coordinate *nc = (struct X3D_Coordinate*)node->coord;
		psc = (float*)nc->point.p;
		nsc = nc->point.n;
		memcpy(psc,node->_origCoords,3*nsc*sizeof(float));
	}
}


void child_HAnimSite(struct X3D_HAnimSite *node) {

	//CHILDREN_COUNT
	//RETURN_FROM_CHILD_IF_NOT_FOR_ME

	/* do we have to sort this node? */

	/* do we have a local light for a child? */
	prep_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);
	prep_BBox((struct BBoxFields*)&node->bboxCenter);

	/* now, just render the non-directionalLight children */
	normalChildren(node->children);

	fin_BBox((struct X3D_Node*)node,(struct BBoxFields*)&node->bboxCenter,TRUE);
	fin_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);
}


// ======== HAnimMotion >>>>>>>>>>>>>>
int char_is_separator(char c, char *separators){
	int is_sep = FALSE;
	char *s = separators;
	while(*s != 0){
		if(c == *s){
			is_sep = TRUE; break;
		}
		s++;
	}
	return is_sep;
}
//adapted from cson
static int next_token( char const ** inp, char *separators, char const ** end )
{
    char const * pos = NULL;
	if(!(inp && end && *inp))
		printf("ouch\n");
    assert( inp && end && *inp );
    if( *inp == *end ) return 0;
    pos = *inp;
    if( !*pos )
    {
        *end = pos;
        return 0;
    }
    for( ; *pos && ( char_is_separator(*pos,separators)); ++pos) { /* skip preceeding splitters */ }
    *inp = pos;
    for( ; *pos && ( !char_is_separator(*pos,separators)); ++pos) { /* find next splitter */ }
    *end = pos;
    return (pos > *inp) ? 1 : 0;
}
Stack* parse_joint_names(struct X3D_Node* node, char *joint_names){
	char *sep = " \n\r\t,";
	Stack* jnames = newStack(char*);
	//adapted from cson >>
	int len, rc;
	char *beg, *end;
    beg = joint_names;
    end = NULL;
    for(int i=0;; ++i, beg=end, end=NULL )
    {
        rc = next_token( &beg, sep, &end );
        if(!rc) break;
        assert( beg != end );
        assert( end > beg );
		//*end = '\0';
        len = (unsigned int)(end - beg);
		char *name = malloc(len+1);
		register_node_gc(node,name);
        //if( len > (BufSize-1) ) return cson_rc.RangeError;
        //memset( buf, 0, len + 1 );
        memcpy(name, beg, len );
        name[len] = 0;
		stack_push(char*,jnames,name);
    }
	//<< adapted from cson
	return jnames;
}
enum {
CHAN_RX = 1,
CHAN_RY = 2,
CHAN_RZ = 3,
CHAN_TX = 4,
CHAN_TY = 5,
CHAN_TZ = 6,
CHAN_NONE = 0,
};
static struct chan_name {
int iname;
char *cname;
} chan_names [] = {
{CHAN_RX, "Xrotation"},
{CHAN_RY, "Yrotation"},
{CHAN_RZ, "Zrotation"},
{CHAN_TX, "Xposition"},
{CHAN_TY, "Yposition"},
{CHAN_TZ, "Zposition"},
{CHAN_NONE,NULL},
};
static int chan_lookup(char *cname){
	int i, iname;
	struct chan_name *cn;
	i = 0;
	iname = 0;
	do{
		cn = &chan_names[i];
		if(!strcmp(cn->cname,cname)){
			iname = cn->iname;
			break;
		}
		i++;
	}while(cn->cname != NULL);
	return iname;
	
}
struct joint_frame_motion {
	char *jname;
	char *mocap_name;
	int nchan;
	int ichan[6];
	int level;
	float *values;
};
char *channame_lookup(int ichan){
	int i;
	struct chan_name *cn;
	i = 0;
	char * cname = NULL;
	do{
		cn = &chan_names[i];
		if(cn->iname == ichan){
			cname = cn->cname;
			break;
		}
		i++;
	}while(cn->iname != CHAN_NONE);
	return cname;
}
char *next_buffer_token(char **beg, char* sep, char **end){
	static char buffer[128];
	int len, rc;
	buffer[0] = '\0';
    rc = next_token( beg, sep, end );
    if(rc){
		assert( *beg != *end );
		assert( *end > *beg );
		//*end = '\0';
		len = (unsigned int)(*end - *beg);
		len = min(len,127);
		memcpy(buffer, *beg, len );
		buffer[len] = 0;
	}
	return buffer;
}
int parse_channels(char *channelstring, int nentries, struct joint_frame_motion * chan){
	char *sep = " \n\r\t,";
	//adapted from cson >>
	int len, rc, count, totalcount;
	char *beg, *end, *token;
	totalcount = 0;
    beg = channelstring;
    end = NULL;
    for(int i=0;i<nentries; ++i, beg=end, end=NULL )
    {
        token = next_buffer_token( &beg, sep, &end );
		len = strlen(token);
        if(!len) break;
		sscanf(token,"%d",&count);
		totalcount += count;
		chan[i].nchan = count;
		for(int j=0;j<count;j++){
			beg=end; end=NULL;
	        token = next_buffer_token( &beg, sep, &end );
			int ichan = chan_lookup(token);
			chan[i].ichan[j] = ichan;
		}
    }
	return totalcount;
}
//struct mojoint {
//	float v[6];
//};
//struct moframe {
//	struct mojoint * mj;
//};
float *parse_float_values(int n, char *str){
	char *beg, *end, *token;
	int len;
	char *sep = " \n\r\t,";
	float *fv = malloc(n*sizeof(float));
    beg = str;
    end = NULL;
    for(int i=0;i<n; ++i, beg=end, end=NULL )
    {
        token = next_buffer_token( &beg, sep, &end );
		len = (unsigned int)(*end - *beg);
        if(!len) break;
		sscanf(token,"%f",&fv[i]);
    }
	return fv;
}
//void parse_values(struct moframe *moframes,int framecount, int jointcount, int channelcount, struct joint_frame_motion* chan, char *values){
//	float *fvalues = parse_float_values(framecount * channelcount, values);
//	float *fv = fvalues;
//	for(int i=0;i<framecount;i++){
//		struct moframe *mof = &moframes[i];
//		for(int j=0;j<jointcount;j++){
//			struct mojoint *moj = &mof->mj[j];
//			for(int k=0;k<chan[j].count;k++){
//				moj->v[k] = *fv;
//				if(chan[j].channel[k] < 4)
//					moj->v[k] *= PI/180.0;
//				fv++;
//			}
//		}
//	}
//}
#define RADIANS_PER_DEGREE (double)0.0174532925199432957692
#define DEGREES_PER_RADIAN (double)57.2957795130823208768
void compile_HAnimMotion(struct X3D_HAnimMotion *node) {
	//motion data

	//parse jouint names
	struct Vector *jnames = parse_joint_names(X3D_NODE(node),node->joints->strptr);
	printf("\n");
	for(int i=0;i<jnames->n;i++)
		printf("%d %s\n",i,vector_get(char*,jnames,i));
	int njoints = jnames->n;

	//parse channels
	struct joint_frame_motion *chan = malloc(njoints * sizeof(struct joint_frame_motion));
	int channelcount = parse_channels(node->channels->strptr,njoints,chan);
	//in theory channelcount is how many floats to advance in fvalues to get the next frame pointer.


	for(int i=0;i<njoints;i++){
		chan[i].jname = vector_get(char*,jnames,i);
		printf("joint %d nchan %d ",i, chan[i].nchan);
		for(int j=0;j<chan[i].nchan;j++){
			printf("%s ",channame_lookup(chan[i].ichan[j]));
		}
		printf("\n");
	}
	//parse float frame data
	float *fvalues = parse_float_values(node->frameCount * channelcount, node->values->strptr);

	//convert degrees to radians
	for(int iframe=0;iframe<node->frameCount;iframe++){
		float *fv = &fvalues[iframe * channelcount];
		int kchan = 0;
		for(int j=0;j<njoints;j++){
			//printf("%s %d \n",vector_get(char*,jnames,j),chan[j].nchan);
			for(int k=0;k<chan[j].nchan;k++){
				if(chan[j].ichan[k] < 4)
					fv[kchan] *= RADIANS_PER_DEGREE; //PI / 180.0; //
				//printf("%d %5.2f ",chan[j].ichan[k],chan[j].ichan[k] < 4 ? fv[kchan]*180.0/PI : fv[kchan]);
				kchan++;
			}
			//printf("\n");
		}
	}

	//we won't 'map' to parent during compile - we'll find the motion joint -if any- on the fly in HAnimJoint function(s)

	//frame state
	//?? anything to do?
	node->_njoints = njoints;
	node->_channelcount = channelcount;
	node->_fvalues = fvalues;
	node->_channels = chan;
	node->_framevalues = fvalues;
	//node->startFrame = 0;
	if(node->endFrame == 0) node->endFrame = node->frameCount -1;
	MARK_EVENT(X3D_NODE(node), offsetof(struct X3D_HAnimMotion, frameCount));
	MARK_NODE_COMPILED
}
void render_HAnimMotion(struct X3D_HAnimMotion *node) {
	//main job: set the frame pointer for the current time, increment, enabled state
	COMPILE_IF_REQUIRED
	int index = 0;
	float *fvalues = (float*)node->_fvalues;
	int channelcount = (int)node->_channelcount;
	float *frame_values;
	int isActive = FALSE;

	int increment = node->frameIncrement;
	if(increment == 0) return; //the official way to pause
	index = node->frameIndex;
	int fcount = node->frameCount;
	index = max(0,min(index,fcount-1)); //iclamp

	int starting = 0;
	int stopping = 0;
	isActive = node->enabled && ((node->loop && increment != 0) || (increment > 0 && index < fcount -1) || (increment < 0 && index > 0) );
	if(node->enabled && !node->_lastenabled){
		starting = TRUE;
		node->_lastenabled = node->enabled;
	}else if(!node->enabled && node->_lastenabled){
		stopping = TRUE;
		node->_lastenabled = node->enabled;
	}
	if(starting){
		node->_startTime = TickTime();
	}


	if(node->next){
		index = index + increment;
		node->next = FALSE;
	} else if(node->previous){
		index = index - increment;
		node->previous = FALSE;
	} else if(node->enabled){
		double dtime = TickTime() - node->_startTime;
		index = node->frameIncrement * (int)( dtime / node->frameDuration);
	}
	int startingloop = 0;
	if(node->loop){
		int lindex = index % fcount;
		startingloop = lindex != index;
		index = lindex;
	}
	index = max(0,min(index,fcount-1)); //iclamp
	if(starting && index == fcount -1 && increment > 0) index = 0;
	if(starting && index == 0 && increment < 0) index = fcount -1;
	if(starting || startingloop ){
		node->cycleTime = TickTime();
		MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_HAnimMotion, cycleTime));
	}
	if(isActive){
		node->elapsedTime = TickTime();
		MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_HAnimMotion, elapsedTime));
	}
	node->frameIndex = index;
	frame_values = &fvalues[node->frameIndex * channelcount];
	node->_framevalues = frame_values; //frame pointer into big array of floats, good for current frame only
}
enum{
	LOADER_INITIAL_STATE=0,
	LOADER_REQUEST_RESOURCE,
	LOADER_FETCHING_RESOURCE,
	LOADER_PROCESSING,
	LOADER_LOADED,
	LOADER_COMPILED,
	LOADER_STABLE,
};

struct joint_frame_motion * jointFrameMotion(struct X3D_HAnimMotion *node, char *jname){
	struct joint_frame_motion * jm = NULL;
	if(node){
		if(node->_nodeType == NODE_HAnimMotion){
			struct X3D_HAnimMotion* HM = (struct X3D_HAnimMotion*) node;
			if(HM->enabled){
				//see if we have the joint
				int njoints = (int)HM->_njoints;
				struct joint_frame_motion * chan = HM->_channels;
				float *frame_values = (float*)HM->_framevalues;  //render_HAnimMotion should have run this frame to set the frame pointer
				int kchan = 0;
				for(int i=0;i<njoints;i++){
					if(!strcmp(chan[i].jname,jname)){
						//if so return the channel mapping and fvalue pointer
						jm = &chan[i];
						jm->values = &frame_values[kchan];
						break;
					}
					kchan += chan[i].nchan;
				}
			}
		}else if(node->_nodeType == NODE_HAnimMotionPlay){
			struct X3D_HAnimMotionPlay* HM = (struct X3D_HAnimMotionPlay*) node;
			struct X3D_HAnimMotionData *HD = (struct X3D_HAnimMotionData*) HM->data;
			if(HD->__loadstatus != LOADER_LOADED) return jm; //return NULL
			if(HM->enabled && HD){
				//see if we have the joint
				int njoints = (int)HD->_njoints;
				struct joint_frame_motion * chan = HD->_channels;
				float *frame_values = (float*)HM->_framevalues;  //render_HAnimMotion should have run this frame to set the frame pointer
				if (!frame_values) return NULL; //but with multiple motions, and changing motion on the fly, sometimes it needs another frame
				int kchan = 0;
				for(int i=0;i<njoints;i++){
					if(!strcmp(chan[i].jname,jname)){
						//if so return the channel mapping and fvalue pointer
						//printf("%s ",jname);
						jm = &chan[i];
						jm->values = &frame_values[kchan];
						//if(!strcmp(jname,"humanoid_root")){
						//	printf("humanoid_root vals=");
						//	for(int m=0;m<chan[i].nchan;m++) printf("%f ",jm->values[m]);
						//	printf("\n");
						//}
						break;
					}
					kchan += chan[i].nchan;
				}
			}
		}
	}
	return jm;
}

void update_jointMatrixFromMotion(struct X3D_Node* HMnode, char* jname, double* jmatrix0) {
	struct X3D_HAnimMotion* HM = (struct X3D_HAnimMotion*)HMnode;
	if (HM && (HM->_nodeType == NODE_HAnimMotion || HM->_nodeType == NODE_HAnimMotionPlay)) {
		float weight = HM->transitionWeight;
		struct joint_frame_motion* jm = jointFrameMotion(HM, jname);
		int debug, debug2;
		debug = debug2 = FALSE;
		//if(!strcmp(jname,"l_shoulder")) debug = TRUE;
		if(jm){ // && strcmp(jname,"HumanoidRoot")){
			double mat1[16],jmatrix[16],xyz[3];
			if(debug) printf("in update_jointMatrix\n");
			if(debug) printmatrix(jmatrix0);
			matidentity4d(jmatrix);
			int igl = FALSE;
			if (igl) glPushMatrix();
			if (igl) glLoadIdentity();
			if(debug || debug2) 
				printf("%s ",jname);
			
			for(int ii=0;ii<jm->nchan;ii++){
				int i = ii; // jm->nchan - 1 - ii;
				float value = jm->values[i] * weight;
				if(debug)
				printf("%d %4.2f ",jm->ichan[i],value);
				// Q. what kind of angles are those 
				// https://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToMatrix/index.htm
				int ir = 0;
				matidentity4d(mat1);
				switch(jm->ichan[i]){
					case 1:
						matrixFromAxisAngle4d(mat1, -(double)value, 1.0, 0.0,0.0);
						if (igl) glRotatef(value*DEGREES_PER_RADIAN, 1, 0, 0);
						if (debug2) printf("xr %f ", value*DEGREES_PER_RADIAN);
						if(ir) matmultiplyAFFINE(jmatrix,jmatrix, mat1);
						else matmultiplyAFFINE(jmatrix, mat1, jmatrix);
						if(debug){
						printf("case 1 mat1\n");
						printmatrix(mat1);
						printf("case 1 jmatrix\n");
						printmatrix(jmatrix);
						}
						break;
					case 2: 
						matrixFromAxisAngle4d(mat1, -(double)value, 0.0, 1.0, 0.0);
						if (igl) glRotatef(value * DEGREES_PER_RADIAN, 0, 1, 0);
						if (debug2) printf("yr %f ", value * DEGREES_PER_RADIAN);
						if (ir) matmultiplyAFFINE(jmatrix, jmatrix, mat1);
						else matmultiplyAFFINE(jmatrix, mat1, jmatrix);
						if(debug){
						printf("case 2 mat1\n");
						printmatrix(mat1);
						printf("case 2 jmatrix\n");
						printmatrix(jmatrix);
						}
						break;
					case 3:
						matrixFromAxisAngle4d(mat1, -(double)value, 0.0, 0.0, 1.0);
						if (igl) glRotatef(value * DEGREES_PER_RADIAN, 0, 0, 1);
						if (debug2) printf("zr %f ", value * DEGREES_PER_RADIAN);
						if (ir) matmultiplyAFFINE(jmatrix, jmatrix, mat1);
						else matmultiplyAFFINE(jmatrix, mat1, jmatrix);
						if(debug){
						printf("case 3 mat1\n");
						printmatrix(mat1);
						printf("case 3 jmatrix\n");
						printmatrix(jmatrix);
						}
						break;
					case 4:
						mattranslate4d(mat1,vecsetd(xyz,(double)value,0.0,0.0));
						if (igl) glTranslatef(value,0,0);
						if (ir) matmultiplyAFFINE(jmatrix, jmatrix, mat1);
						else matmultiplyAFFINE(jmatrix, mat1, jmatrix);
						if(debug){
						printf("case 4 mat1\n");
						printmatrix(mat1);
						printf("case 4 jmatrix\n");
						printmatrix(jmatrix);
						}
						break;
					case 5:
						mattranslate4d(mat1,vecsetd(xyz,0.0,(double)value,0.0));
						if (igl) glTranslatef(0, value, 0);
						if (ir) matmultiplyAFFINE(jmatrix, jmatrix, mat1);
						else matmultiplyAFFINE(jmatrix, mat1, jmatrix);
						if(debug){
						printf("case 5 mat1\n");
						printmatrix(mat1);
						printf("case 5 jmatrix\n");
						printmatrix(jmatrix);
						}
						break;
					case 6:
						mattranslate4d(mat1,vecsetd(xyz,0.0,0.0,(double)value));
						if (igl) glTranslatef(0, 0, value);
						if (ir) matmultiplyAFFINE(jmatrix, jmatrix, mat1);
						else matmultiplyAFFINE(jmatrix, mat1, jmatrix);
						if(debug){
						printf("case 6 mat1\n");
						printmatrix(mat1);
						printf("case 6 jmatrix\n");
						printmatrix(jmatrix);
						}
						break;
					default:
						if(debug) printf("OUCH DEFAULT\n");
						break;
				}
			}
			if (debug2)printf("\n");
			if(0) if (jm->level == 1) {
				double toYup[] = {1,0,0,0, 0,0,-1,0, 0,1,0,0, 0,0,0,1};
				matmultiplyAFFINE(jmatrix, toYup, jmatrix);
			}
			if(debug) if (!strcmp(jm->jname, "l_shoulder")) {
				double tmatrix[16];
				glGetDoublev(GL_MODELVIEW_MATRIX, tmatrix);

				printf("\n");
				if (igl) {
					printf("opengl matrix multiply\n");
						printf("%lf %lf %lf %lf\n  %lf %lf %lf %lf\n  %lf %lf %lf %lf\n  %lf %lf %lf %lf\n",
							tmatrix[0], tmatrix[1], tmatrix[2], tmatrix[3],
							tmatrix[4], tmatrix[5], tmatrix[6], tmatrix[7],
							tmatrix[8], tmatrix[9], tmatrix[10], tmatrix[11],
							tmatrix[12], tmatrix[13], tmatrix[14], tmatrix[15]);
					printf("\n");
				}
				printf("fw matrix multiply\n");
				printf("%lf %lf %lf %lf\n  %lf %lf %lf %lf\n  %lf %lf %lf %lf\n  %lf %lf %lf %lf\n",
					jmatrix[0], jmatrix[1], jmatrix[2], jmatrix[3],
					jmatrix[4], jmatrix[5], jmatrix[6], jmatrix[7],
					jmatrix[8], jmatrix[9], jmatrix[10], jmatrix[11],
					jmatrix[12], jmatrix[13], jmatrix[14], jmatrix[15]);
				printf("\n");
				if (igl) memcpy(jmatrix, tmatrix, 16 * sizeof(double));
				printf("fw matrix multiply\n");
				printf("%lf %lf %lf %lf\n  %lf %lf %lf %lf\n  %lf %lf %lf %lf\n  %lf %lf %lf %lf\n",
					jmatrix[0], jmatrix[1], jmatrix[2], jmatrix[3],
					jmatrix[4], jmatrix[5], jmatrix[6], jmatrix[7],
					jmatrix[8], jmatrix[9], jmatrix[10], jmatrix[11],
					jmatrix[12], jmatrix[13], jmatrix[14], jmatrix[15]);
				printf("\n");

			}
			//matinverseAFFINE(mat1,jmatrix);
			matmultiplyAFFINE(jmatrix0,jmatrix,jmatrix0);
			if (igl) glPopMatrix();
			//if(debug)
			//printf("\n");
		}
	}
}
// <<<<<<<<< HAnimMotion ======================


//exprimental nodes not in specs: 
// Motion = MotionPlay + (MotionData or MotionDataFile)
// we still have v4 Motion, but also a MotionPlay:Motion which 
// allows MotionData part to be DEF/USEd aka shared among charagers in a scene.
// MotionPlay will have a frame index and timing info, so can stay 1:1 with HAnimHumanoid character
// MotionData can be DEF/USED by multiple MotionPlay nodes
// MotionDataFile - allows reading popular mocap/MotionCapture file formats .bvh, .c3d ...
void map_mocap_to_hanim_loa( struct joint_frame_motion *chan, int mjoint, int loa);
void bvh_set_mapping(char** mapping, int n);
void read_bvh_blob(char *blob, int ignorePosition, int yUp, int teePose, 
	int flipZ, float armAngle, float legAngle, float scale,  
	struct joint_frame_motion **chan, int *njoint, int *channel_count, float **values, 
	float *bvh_frame_time, int *bvh_frame_count);
void read_bvh_blob_to_node(struct X3D_HAnimMotionDataFile * node, char *blob, int len){
	//Stack *bvh_nodes = NULL;
	float bvh_frame_time;
	int bvh_frame_count;
	//float global_scale = 1.0f;
	struct joint_frame_motion * chan = NULL;
	float *fvalues = NULL;
	int channel_count;
	int njoint;
	if (node->mapping.n) {
		char** mapp = malloc(2 * sizeof(char*) * node->mapping.n);
		for (int i = 0; i < node->mapping.n; i++)
			mapp[i] = node->mapping.p[i]->strptr;
		bvh_set_mapping(mapp, node->mapping.n / 2);
	}
	else {
		bvh_set_mapping(NULL, 0); //will use internal mapping
	}
	read_bvh_blob(blob, node->ignorePosition, node->yUp, node->teePose, 
		node->flipZ, node->armAngle, node->legAngle, node->scale,
		&chan, &njoint, &channel_count, &fvalues, &bvh_frame_time,&bvh_frame_count);
	map_mocap_to_hanim_loa(chan,njoint,node->loa);
	node->frameCount = bvh_frame_count;
	MARK_EVENT(X3D_NODE(node), offsetof(struct X3D_HAnimMotionDataFile, frameCount));
	node->frameDuration = bvh_frame_time;
	node->_njoints = njoint;
	node->_channels = chan;
	node->_channelcount = channel_count;
	node->_fvalues = fvalues;
}
void process_mocap(resource_item_t *res){
	//a chance to do a bit of out-of-render-thread processing.
	openned_file_t *of;
	of = res->openned_files;
	if (!of) {
		/* error */
		return;
	}

	char *blob = of->fileData;
	int len = of->fileDataSize;

	struct X3D_HAnimMotionDataFile * node = (struct X3D_HAnimMotionDataFile *) res->whereToPlaceData;

	printf("process mocap\n");
	read_bvh_blob_to_node(node,blob,len);
	res->complete = TRUE;
	res->status = ress_parsed;
}
void compile_HAnimMotionData(struct X3D_HAnimMotionData *node){
	//motion data

	//parse jouint names
	struct Vector *jnames = parse_joint_names(X3D_NODE(node),node->joints->strptr);
	printf("\n");
	for(int i=0;i<jnames->n;i++)
		printf("%d %s\n",i,vector_get(char*,jnames,i));
	int njoints = jnames->n;

	//parse channels
	struct joint_frame_motion *chan = malloc(njoints * sizeof(struct joint_frame_motion));
	int channelcount = parse_channels(node->channels->strptr,njoints,chan);
	//in theory channelcount is how many floats to advance in fvalues to get the next frame pointer.


	for(int i=0;i<njoints;i++){
		chan[i].jname = vector_get(char*,jnames,i);
		printf("joint %d nchan %d ",i, chan[i].nchan);
		for(int j=0;j<chan[i].nchan;j++){
			printf("%s ",channame_lookup(chan[i].ichan[j]));
		}
		printf("\n");
	}
	//parse float frame data
	float *fvalues = parse_float_values(node->frameCount * channelcount, node->values->strptr);

	//convert degrees to radians
	for(int iframe=0;iframe<node->frameCount;iframe++){
		float *fv = &fvalues[iframe * channelcount];
		int kchan = 0;
		for(int j=0;j<njoints;j++){
			//printf("%s %d \n",vector_get(char*,jnames,j),chan[j].nchan);
			for(int k=0;k<chan[j].nchan;k++){
				if(chan[j].ichan[k] < 4)
					fv[kchan] *= RADIANS_PER_DEGREE; //PI / 180.0; //
				//printf("%d %5.2f ",chan[j].ichan[k],chan[j].ichan[k] < 4 ? fv[kchan]*180.0/PI : fv[kchan]);
				kchan++;
			}
			//printf("\n");
		}
	}

	//we won't 'map' to parent during compile - we'll find the motion joint -if any- on the fly in HAnimJoint function(s)

	//frame state
	//?? anything to do?
	node->_njoints = njoints;
	node->_channelcount = channelcount;
	node->_fvalues = fvalues;
	node->_channels = chan;
	node->__loadstatus = LOADER_LOADED;
	MARK_NODE_COMPILED
}
void render_HAnimMotionData(struct X3D_HAnimMotionData *node){
	COMPILE_IF_REQUIRED
}


//enum{
//	LOADER_INITIAL_STATE=0,
//	LOADER_REQUEST_RESOURCE,
//	LOADER_FETCHING_RESOURCE,
//	LOADER_PROCESSING,
//	LOADER_LOADED,
//	LOADER_COMPILED,
//	LOADER_STABLE,
//};
void compile_HAnimMotionDataFile(struct X3D_HAnimMotionDataFile *node){
	resource_item_t *res;
	int retval = FALSE;
	switch (node->__loadstatus) {
		case LOADER_INITIAL_STATE: /* nothing happened yet */

		if (node->url.n == 0) {
			node->__loadstatus = LOADER_STABLE; /* a "do-nothing" approach */
		} else {
			res = resource_create_multi(&(node->url));
			res->media_type = resm_mocap; //resm_fshader;
			node->__loadstatus = LOADER_REQUEST_RESOURCE;
			node->__loadResource = res;
		}
		break;

		case LOADER_REQUEST_RESOURCE:
		res = node->__loadResource;
		resource_identify(node->_parentResource, res);
		/* printf ("load_Inline, we have type  %s  status %s\n",
			resourceTypeToString(res->type), resourceStatusToString(res->status)); */
		res->actions = resa_download | resa_load; //not resa_parse which we do below
		resitem_enqueue(ml_new(res)); 
		//frontenditem_enqueue(ml_new(res));
		node->__loadstatus = LOADER_FETCHING_RESOURCE;
		break;

		case LOADER_FETCHING_RESOURCE:
		res = node->__loadResource;
		/* printf ("load_Inline, we have type  %s  status %s\n",
			resourceTypeToString(res->type), resourceStatusToString(res->status)); */
		// do we try the next url in the multi-url? 
		if(res->complete){
			if (res->status == ress_loaded) {
				//determined during load process by resource_identify_type(): res->media_type = resm_vrml; //resm_unknown;
				if(1){
					//send it for out-of-display-thread-processing
					res->whereToPlaceData = X3D_NODE(node);
					//res->offsetFromWhereToPlaceData = 0; 
					res->actions = resa_process;
					node->__loadstatus = LOADER_PROCESSING; // a "do-nothing" approach 
					res->complete = FALSE;
					//send_resource_to_parser(res);
					//send_resource_to_parser_if_available(res);
					resitem_enqueue(ml_new(res));
				}else{
					//in-display-thread procesing
					process_mocap(res);
					node->__loadstatus = LOADER_LOADED; // a "do-nothing" approach 
					res->complete = TRUE;

				}
			} else if ((res->status == ress_failed) || (res->status == ress_invalid)) {
				//no hope left
				printf ("resource failed to load\n");
				node->__loadstatus = LOADER_STABLE; // a "do-nothing" approach 
			}
		}
		break;

		case LOADER_PROCESSING:
			res = node->__loadResource;

			//printf ("inline parsing.... %s\n",resourceStatusToString(res->status));
			//printf ("res complete %d\n",res->complete);
			if(res->complete){
				if (res->status == ress_parsed) {
					node->__loadstatus = LOADER_LOADED;
				}else{
					node->__loadstatus = LOADER_STABLE;
				}
			}

		break;
		case LOADER_STABLE:
		break;
		case LOADER_LOADED:
		case LOADER_COMPILED:
		retval = TRUE;
	}
	if(node->__loadstatus == LOADER_STABLE || node->__loadstatus == LOADER_LOADED)
		MARK_NODE_COMPILED
}
void render_HAnimMotionDataFile(struct X3D_HAnimMotionDataFile *node){
	COMPILE_IF_REQUIRED
}

void compile_HAnimMotionClip(struct X3D_HAnimMotionClip *node){
	int is_file = node->url.n;
	if(is_file){
	
		resource_item_t *res;
		int retval = FALSE;
		switch (node->__loadstatus) {
			case LOADER_INITIAL_STATE: /* nothing happened yet */

			if (node->url.n == 0) {
				node->__loadstatus = LOADER_STABLE; /* a "do-nothing" approach */
			} else {
				res = resource_create_multi(&(node->url));
				res->media_type = resm_mocap; //resm_fshader;
				node->__loadstatus = LOADER_REQUEST_RESOURCE;
				node->__loadResource = res;
			}
			break;

			case LOADER_REQUEST_RESOURCE:
			res = node->__loadResource;
			resource_identify(node->_parentResource, res);
			/* printf ("load_Inline, we have type  %s  status %s\n",
				resourceTypeToString(res->type), resourceStatusToString(res->status)); */
			res->actions = resa_download | resa_load; //not resa_parse which we do below
			resitem_enqueue(ml_new(res)); 
			//frontenditem_enqueue(ml_new(res));
			node->__loadstatus = LOADER_FETCHING_RESOURCE;
			break;

			case LOADER_FETCHING_RESOURCE:
			res = node->__loadResource;
			/* printf ("load_Inline, we have type  %s  status %s\n",
				resourceTypeToString(res->type), resourceStatusToString(res->status)); */
			// do we try the next url in the multi-url? 
			if(res->complete){
				if (res->status == ress_loaded) {
					//determined during load process by resource_identify_type(): res->media_type = resm_vrml; //resm_unknown;
					if(1){
						//send it for out-of-display-thread-processing
						res->whereToPlaceData = X3D_NODE(node);
						//res->offsetFromWhereToPlaceData = 0; 
						res->actions = resa_process;
						node->__loadstatus = LOADER_PROCESSING; // a "do-nothing" approach 
						res->complete = FALSE;
						//send_resource_to_parser(res);
						//send_resource_to_parser_if_available(res);
						resitem_enqueue(ml_new(res));
					}else{
						//in-display-thread procesing
						process_mocap(res);
						node->__loadstatus = LOADER_LOADED; // a "do-nothing" approach 
						res->complete = TRUE;

					}
				} else if ((res->status == ress_failed) || (res->status == ress_invalid)) {
					//no hope left
					printf ("resource failed to load\n");
					node->__loadstatus = LOADER_STABLE; // a "do-nothing" approach 
				}
			}
			break;

			case LOADER_PROCESSING:
				res = node->__loadResource;

				//printf ("inline parsing.... %s\n",resourceStatusToString(res->status));
				//printf ("res complete %d\n",res->complete);
				if(res->complete){
					if (res->status == ress_parsed) {
						node->__loadstatus = LOADER_LOADED;
					}else{
						node->__loadstatus = LOADER_STABLE;
					}
				}

			break;
			case LOADER_STABLE:
			break;
			case LOADER_LOADED:
			case LOADER_COMPILED:
			retval = TRUE;
		}
		if(node->__loadstatus == LOADER_STABLE || node->__loadstatus == LOADER_LOADED)
			MARK_NODE_COMPILED	

	}else{
		//field data
		//parse jouint names
		struct Vector *jnames = parse_joint_names(X3D_NODE(node),node->joints->strptr);
		printf("\n");
		for(int i=0;i<jnames->n;i++)
			printf("%d %s\n",i,vector_get(char*,jnames,i));
		int njoints = jnames->n;

		//parse channels
		struct joint_frame_motion *chan = malloc(njoints * sizeof(struct joint_frame_motion));
		int channelcount = parse_channels(node->channels->strptr,njoints,chan);
		//in theory channelcount is how many floats to advance in fvalues to get the next frame pointer.


		for(int i=0;i<njoints;i++){
			chan[i].jname = vector_get(char*,jnames,i);
			printf("joint %d nchan %d ",i, chan[i].nchan);
			for(int j=0;j<chan[i].nchan;j++){
				printf("%s ",channame_lookup(chan[i].ichan[j]));
			}
			printf("\n");
		}
		//parse float frame data
		float *fvalues = parse_float_values(node->frameCount * channelcount, node->values->strptr);

		//convert degrees to radians
		for(int iframe=0;iframe<node->frameCount;iframe++){
			float *fv = &fvalues[iframe * channelcount];
			int kchan = 0;
			for(int j=0;j<njoints;j++){
				//printf("%s %d \n",vector_get(char*,jnames,j),chan[j].nchan);
				for(int k=0;k<chan[j].nchan;k++){
					if(chan[j].ichan[k] < 4)
						fv[kchan] *= RADIANS_PER_DEGREE; //PI / 180.0; //
					//printf("%d %5.2f ",chan[j].ichan[k],chan[j].ichan[k] < 4 ? fv[kchan]*180.0/PI : fv[kchan]);
					kchan++;
				}
				//printf("\n");
			}
		}

		//we won't 'map' to parent during compile - we'll find the motion joint -if any- on the fly in HAnimJoint function(s)

		//frame state
		//?? anything to do?
		node->_njoints = njoints;
		node->_channelcount = channelcount;
		node->_fvalues = fvalues;
		node->_channels = chan;
		node->__loadstatus = LOADER_LOADED;
		MARK_NODE_COMPILED
	}

}
void render_HAnimMotionClip(struct X3D_HAnimMotionClip *node){
	COMPILE_IF_REQUIRED
}

void compile_HAnimMotionPlay(struct X3D_HAnimMotionPlay *node){

	struct X3D_HAnimMotionData *motiondata = (struct X3D_HAnimMotionData *)node->data;

	if(motiondata){
		if(node->data->_nodeType == NODE_HAnimMotionData || node->data->_nodeType == NODE_HAnimMotionDataFile || node->data->_nodeType == NODE_HAnimMotionClip){
			render_node(X3D_NODE(node->data));
			int fileclip = node->data->_nodeType == NODE_HAnimMotionClip && ((struct X3D_HAnimMotionClip*)(node->data))->url.n > 0;
			if(node->data->_nodeType == NODE_HAnimMotionDataFile || fileclip){
				struct X3D_HAnimMotionDataFile * motiondatafile = (struct X3D_HAnimMotionDataFile*)node->data;
				//node->startFrame = motiondatafile->ignoreFirstFrame ? 1 : 0;
				//if(node->endFrame == 0) node->endFrame = motiondatafile->frameCount -1;
				if (motiondatafile->__loadstatus != LOADER_LOADED) return;
				MARK_EVENT(X3D_NODE(motiondata), offsetof(struct X3D_HAnimMotionData, frameCount));
				MARK_NODE_COMPILED
			}else{
				//node->startFrame = 0;
				//if(node->endFrame == 0) node->endFrame = motiondata->frameCount -1;
				MARK_EVENT(X3D_NODE(motiondata), offsetof(struct X3D_HAnimMotionData, frameCount));
				MARK_NODE_COMPILED
			}
		}
	}
}
void updateMotionPlayFromData(struct X3D_HAnimMotionPlay* play, struct X3D_HAnimMotionData* data) {
	if (data && data->_nodeType == NODE_HAnimMotionData || data->_nodeType == NODE_HAnimMotionDataFile || data->_nodeType == NODE_HAnimMotionClip) {
		render_node(X3D_NODE(data));
		if (data->__loadstatus != LOADER_LOADED) return;
		int fileclip = data->_nodeType == NODE_HAnimMotionClip && ((struct X3D_HAnimMotionClip*)(data))->url.n > 0;
		if (data->_nodeType == NODE_HAnimMotionDataFile || fileclip) {
			struct X3D_HAnimMotionDataFile* motiondatafile = (struct X3D_HAnimMotionDataFile*)data;
			play->startFrame = motiondatafile->ignoreFirstFrame ? 1 : 0;
			play->endFrame = motiondatafile->frameCount - 1;
		}
		else {
			//node->startFrame = 0;
			//if (play->endFrame == 0) 
			play->startFrame = 0;
			play->endFrame = data->frameCount - 1;
		}
	}
}
void render_HAnimMotionPlay(struct X3D_HAnimMotionPlay *node){
	//main job: set the frame pointer for the current time, increment, enabled state
	COMPILE_IF_REQUIRED
	int index = 0;
	struct X3D_HAnimMotionData *motiondata = (struct X3D_HAnimMotionData *)node->data;
	if(motiondata && motiondata->_nodeType == NODE_HAnimMotionData || motiondata->_nodeType == NODE_HAnimMotionDataFile || motiondata->_nodeType == NODE_HAnimMotionClip ){
		render_node(X3D_NODE(motiondata));
		if(motiondata->__loadstatus != LOADER_LOADED) return;
	}
	updateMotionPlayFromData(node, motiondata);

	float *fvalues = (float*)motiondata->_fvalues;
	int channelcount = (int)motiondata->_channelcount;
	float *frame_values;
	int isActive = FALSE;

	int increment = node->frameIncrement;
//	if(increment == 0) return; //the official way to pause
	index = node->frameIndex;
	int fcount = node->endFrame - node->startFrame + 1; // motiondata->frameCount;
	index = max(0,min(index,node->endFrame)); //iclamp

	int starting = 0;
	int stopping = 0;
	isActive = node->enabled && ((node->loop && increment != 0) || (increment > 0 && index < node->endFrame) || (increment < 0 && index > 0) );
	if(node->enabled && !node->_lastenabled){
		starting = TRUE;
		node->_lastenabled = node->enabled;
	}else if(!node->enabled && node->_lastenabled){
		stopping = TRUE;
		node->_lastenabled = node->enabled;
	}
	if(starting){
		node->_startTime = TickTime();
	}


	if(node->next){
		index = index + 1;// increment;
		node->next = FALSE;
	} else if(node->previous){
		index = index - 1; // increment;
		node->previous = FALSE;
	} else if(node->enabled && increment){
		double dtime = TickTime() - node->_startTime;
		index = node->frameIncrement * (int)( dtime / motiondata->frameDuration);
	}
	int startingloop = 0;
	if(node->loop){
		int lindex = ((index - node->startFrame) % fcount) + node->startFrame;
		startingloop = lindex != index;
		index = lindex;
	}
	index = max(0,min(index,node->endFrame)); //iclamp
	if (increment) index = max(index, node->startFrame); //if playing, skip initial teePose
	if(starting && index == node->endFrame && increment > 0) index = node->startFrame;
	if(starting && index <= node->startFrame && increment < 0) index = node->endFrame;
	if(starting || startingloop ){
		node->cycleTime = TickTime();
		MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_HAnimMotion, cycleTime));
	}
	if(isActive){
		node->elapsedTime = TickTime();
		MARK_EVENT (X3D_NODE(node), offsetof(struct X3D_HAnimMotion, elapsedTime));
	}
	int last_index = node->frameIndex;
	node->frameIndex = index;
	if(last_index != index)
		MARK_EVENT(X3D_NODE(node), offsetof( struct X3D_HAnimMotionPlay, frameIndex));
	frame_values = &fvalues[node->frameIndex * channelcount];
	node->_framevalues = frame_values; //frame pointer into big array of floats, good for current frame only
	COMPILE_IF_REQUIRED
}

void compile_HAnimPermuter(struct X3D_HAnimPermuter* node){
	if (node->compute) {
		//generate random permutations of
		// HH HAnimHumanoid
		// HM HAnimMotion 
		// keep Stand motion the same for all
		unsigned int permutation;
		int HHindex, HMindex, np;
		int HHn, HMn;
		HHn = node->humanoids.n;
		HMn = node->motions.n; //first one is walk same for every humanoid
		np = 0;
		FREE_IF_NZ(node->permutations.p);
		node->permutations.p = malloc((HHn*HMn+2) * sizeof(int));
		for (int i = 0; i < HHn; i++) {
			for (int j = 1; j < HMn; j++) {
				node->permutations.p[np] = i * 1000 + j;
				np++;
			}
		}
		node->permutations.n = np;
	}
	MARK_NODE_COMPILED
}
void render_HAnimPermuter(struct X3D_HAnimPermuter* node){
	COMPILE_IF_REQUIRED
}
void child_HAnimPermuter(struct X3D_HAnimPermuter* node){
	//here we do the permutation you choose in the ParticleSystem
	int permutation = node->permutations.p[node->index];
	int HHindex = permutation / 1000;
	int HMindex = permutation - (HHindex*1000);
	struct X3D_HAnimHumanoid* HH = (struct X3D_HAnimHumanoid*)node->humanoids.p[HHindex];
	if (HH->motions.n == 0) {
		HH->motions.n = 2;
		HH->motions.p = malloc(2 * sizeof(void*));
	}
	struct X3D_HAnimMotion* HM = (struct X3D_HAnimMotion*)node->motions.p[HMindex];
	if (node->_play.n == 0) {
		struct X3D_HAnimMotionPlay* HMP0, * HMP1;
		node->_play.p = malloc(2 * sizeof(void*));
		node->_play.p[0] = HMP0 = createNewX3DNode(NODE_HAnimMotionPlay); //for standing motion
		node->_play.p[1] = HMP1 = createNewX3DNode(NODE_HAnimMotionPlay); //for walking motions
		node->_play.n = 2;
		//enabled='true' loop='true' frameIncrement='1' frameIndex='1'
		HMP0->enabled = TRUE;
		HMP1->enabled = TRUE;
		HMP0->loop = TRUE;
		HMP1->loop = TRUE;
		HMP0->frameIncrement = 1;
		HMP1->frameIncrement = 1;
		HMP0->frameIndex = 1;
		HMP1->frameIndex = 1;
	}
	if (HM->_nodeType == NODE_HAnimMotionData || HM->_nodeType == NODE_HAnimMotionDataFile) {
		//parent MotionData to MotionPlay
		struct X3D_HAnimMotionPlay* HMP = (struct X3D_HAnimMotionPlay*)node->_play.p[1];
		HMP->data = X3D_NODE(HM);
		HM = (struct X3D_HAnimMotion*)HMP;
	}
	HH->motions.p[1] = X3D_NODE(HM);
	//set HM-stand
	struct X3D_HAnimMotion* HMS = (struct X3D_HAnimMotion*)node->motions.p[0]; //assume stand is the first motion
	if (HMS->_nodeType == NODE_HAnimMotionData || HMS->_nodeType == NODE_HAnimMotionDataFile) {
		struct X3D_HAnimMotionPlay* HMP = (struct X3D_HAnimMotionPlay*)node->_play.p[0];
		HMP->data = X3D_NODE(HMS);
		HMS = (struct X3D_HAnimMotion*)HMP;
	}
	HH->motions.p[0] = X3D_NODE(HMS);
	node->humanoid = X3D_NODE(HH);
	//now draw
	//child_HAnimHumanoid(HH); // node->humanoid);

}