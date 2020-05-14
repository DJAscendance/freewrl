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


*/


/* last HAnimHumanoid skinCoord and skinNormals */
typedef struct pComponent_HAnim{
	double HHMatrix[16];
	Stack *humanoid_stack;
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
	}
}
void Component_HAnim_clear(struct tComponent_HAnim *t){
	//public
	//private
	{
		ppComponent_HAnim p = (ppComponent_HAnim)t->prv;
		deleteStack(struct X3D_HAnimHumanoid*,p->humanoid_stack);
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
				for(int i=0;i<HH->motions.n;i++){
					if(HH->motionsEnabled.p[i]){
						//printmatrix(jointMatrix.mat);
						FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelviewMatrix);
						update_jointMatrixFromMotion(HH->motions.p[i],node->name->strptr,modelviewMatrix);
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

		RECORD_DISTANCE

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

		RECORD_DISTANCE

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
void render_HAnimHumanoid (struct X3D_HAnimHumanoid *node) {
	/* save the skinCoords and skinNormals for use in following HAnimJoints */
	/* printf ("rendering HAnimHumanoid\n"); */
}

void render_HAnimJoint (struct X3D_HAnimJoint * node) {
	int i,j, jointTransformIndex;
	double modelviewMatrix[16]; //, mvmInverse[16];
	struct X3D_HAnimHumanoid *HH;
	JMATRIX jointMatrix;
	Stack *JT;
	float *PVW, *PVI;

	ppComponent_HAnim p = (ppComponent_HAnim)gglobal()->Component_HAnim.prv;
	//printf ("rendering HAnimJoint %d\n",node); 
	
	HH = peek_humanoid();
	if(HH){
		JT = HH->_JT;

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


void compile_HAnimHumanoid(struct X3D_HAnimHumanoid *node){
	//printf("compile_HAnimHumanoid\n");
	//check if the coordinate count is the same
	INITIALIZE_EXTENT

	push_humanoid(node);
	if(node->motions.n){
		if(node->motions.n > node->motionsEnabled.n){
			// the default is to enable all motions
			int *moe = MALLOC(int*,node->motions.n * sizeof(int));
			memset(moe,0,node->motions.n * sizeof(int));
			memcpy(moe,node->motionsEnabled.p,node->motionsEnabled.n*sizeof(int));
			for(int i=node->motionsEnabled.n;i<node->motions.n;i++)
				moe[i] = TRUE; //FALSE //not sure - specs don't say default, just empty [], I'll use TRUE while developing/debugging
			FREE_IF_NZ(node->motionsEnabled.p);
			node->motionsEnabled.p = moe;
			node->motionsEnabled.n = node->motions.n;
		}
		for(int i=0;i<node->motions.n;i++){
			check_compile(node->motions.p[i]);
		}
	}

	int nsc = 0, nsn = 0;
	float *psc = NULL, *psn = NULL;
	if(node->skinCoord && node->skinCoord->_nodeType == NODE_Coordinate){
		float ee[6];
		struct X3D_Coordinate * nc = (struct X3D_Coordinate * )node->skinCoord;
		nsc = nc->point.n;
		psc = (float*)nc->point.p;
		node->_origCoords = realloc(node->_origCoords,nsc*3*sizeof(float));
		memcpy(node->_origCoords,psc,nsc*3*sizeof(float));
		if(0){
			//find a few coordinates in skinCoord I hacked, by xyz, and give me their index, for making a displacer
			float myfind[9] = {-0.030000f, -0.070000f, 1.777000f,  -0.070000f, 1.777000f, 0.130000f,  1.777000f, 0.130000f, 0.070000f };
			int i,j;
			for(i=0;i<nsc;i++){
				for(j=0;j<3;j++)
					if(vecsametol3f(&psc[i*3],&myfind[j*3],.001f)){
						printf("%d %f %f %f\n",i,myfind[j*3 + 0],myfind[j*3 +1],myfind[j*3 +2]);
					}
			}
		}
		extent6f_from_box3fn(ee,nc->point.p->c, nc->point.n);
		setExtent(ee[0],ee[1],ee[2],ee[3],ee[4],ee[5],X3D_NODE(node));
	}
	if(node->skinNormal && node->skinNormal->_nodeType == NODE_Normal){
		struct X3D_Normal * nn = (struct X3D_Normal * )node->skinNormal;
		//Assuming 1 normal per coord, coord 1:1 normal
		nsn = nn->vector.n;
		psn = (float*)nn->vector.p;
		node->_origNorms = realloc(node->_origNorms,nsn*3*sizeof(float));
		memcpy(node->_origNorms,psn,nsn*3*sizeof(float));
	}
	
	//allocate the joint-transform_index and joint-weight arrays
	//Nov 2016: max 4: meaning each skinCoord can have up to 4 joints referencing/influencing it
	//4 chosen so it's easier to port to GPU method with vec4
	if(node->_NV == 0 || node->_NV != nsc){
		node->_PVI = realloc(node->_PVI,nsc*4*sizeof(float)); //indexes, up to 4 joints per skinCoord
		node->_PVW = realloc(node->_PVW,nsc*4*sizeof(float)); //weights, up to 4 joints per skinCoord
		node->_NV = nsc;
	}
	//allocate the transform array
	if(node->_JT == NULL) {
		if(vertexTransformMethod == VERTEXTRANSFORMMETHOD_GPU){
			//new stack quat + position
		}else if(vertexTransformMethod == VERTEXTRANSFORMMETHOD_CPU){
			node->_JT = newStack(JMATRIX); //we don't know how many joints there are - need to count as we go
		}
	}
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
		for(int i=0;i<node->motions.n;i++){
			if(node->motionsEnabled.p[i])
				render_node(X3D_NODE(node->motions.p[i]));
		}
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



	if(1) normalChildren(node->skeleton); //render_HAnimJoint happens here


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

			}
		}else if(vertexTransformMethod == VERTEXTRANSFORMMETHOD_GPU){
			//push shader flaga with += SKELETAL
		}

		if(1) normalChildren(node->skin);
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

	fin_BBox((struct X3D_Node*)node,(struct BBoxFields*)&node->bboxCenter,FALSE);
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

	/* now, just render the non-directionalLight children */
	normalChildren(node->children);

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
struct chan_name {
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
//struct channellist {
//	int count;
//	int channel[6];
//};
struct joint_frame_motion {
	char *jname;
	int nchan;
	int ichan[6];
	float *values;
};
int chan_lookup(char *cname){
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
	node->startFrame = 0;
	node->endFrame = node->frameCount -1;
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

struct joint_frame_motion * jointFrameMotion(struct X3D_HAnimMotion* HM, char *jname){
	struct joint_frame_motion * jm = NULL;
	if(HM){
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
	}
	return jm;
}
void update_jointMatrixFromMotion(struct X3D_Node* HMnode, char *jname, double *jmatrix0){
	struct X3D_HAnimMotion* HM = (struct X3D_HAnimMotion*) HMnode;
	if(HM && HM->_nodeType == NODE_HAnimMotion){
		struct joint_frame_motion *jm = jointFrameMotion(HM,jname);
		int debug = 0;
		if(jm){ // && strcmp(jname,"HumanoidRoot")){
			double mat1[16],jmatrix[16],xyz[3];
			if(debug) printf("in update_jointMatrix\n");
			if(debug) printmatrix(jmatrix0);
			matidentity4d(jmatrix);
			//if(debug) 
			//printf("%s ",jname);
			for(int i=0;i<jm->nchan;i++){
				float value = jm->values[i];
				//if(debug) 
				//printf("%d %4.2f ",jm->ichan[i],value);
				// Q. what kind of angles are those 
				// https://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToMatrix/index.htm
				matidentity4d(mat1);
				switch(jm->ichan[i]){
					case 1:
						matrixFromAxisAngle4d(mat1, (double)value, -1.0, 0.0,0.0);
						matmultiplyAFFINE(jmatrix,mat1,jmatrix);
						if(debug){
						printf("case 1 mat1\n");
						printmatrix(mat1);
						printf("case 1 jmatrix\n");
						printmatrix(jmatrix);
						}
						break;
					case 2: 
					break;
						matrixFromAxisAngle4d(mat1, (double)value, 0.0, -1.0, 0.0);
						matmultiplyAFFINE(jmatrix,mat1,jmatrix);
						if(debug){
						printf("case 2 mat1\n");
						printmatrix(mat1);
						printf("case 2 jmatrix\n");
						printmatrix(jmatrix);
						}
						break;
						break;
					case 3:
						matrixFromAxisAngle4d(mat1, (double)value, 0.0, 0.0, -1.0);
						matmultiplyAFFINE(jmatrix,mat1,jmatrix);
						if(debug){
						printf("case 3 mat1\n");
						printmatrix(mat1);
						printf("case 3 jmatrix\n");
						printmatrix(jmatrix);
						}
						break;
					case 4:
						mattranslate4d(mat1,vecsetd(xyz,(double)value,0.0,0.0));
						matmultiplyAFFINE(jmatrix,mat1,jmatrix);
						if(debug){
						printf("case 4 mat1\n");
						printmatrix(mat1);
						printf("case 4 jmatrix\n");
						printmatrix(jmatrix);
						}
						break;
					case 5:
						mattranslate4d(mat1,vecsetd(xyz,0.0,(double)value,0.0));
						matmultiplyAFFINE(jmatrix,mat1,jmatrix);
						if(debug){
						printf("case 5 mat1\n");
						printmatrix(mat1);
						printf("case 5 jmatrix\n");
						printmatrix(jmatrix);
						}
						break;
					case 6:
						mattranslate4d(mat1,vecsetd(xyz,0.0,0.0,(double)value));
						matmultiplyAFFINE(jmatrix,mat1,jmatrix);
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
			//matinverseAFFINE(mat1,jmatrix);
			matmultiplyAFFINE(jmatrix0,jmatrix,jmatrix0);
			//if(debug)
			//printf("\n");
		}
	}
}
// <<<<<<<<< HAnimMotion ======================

// >>>>> read BVH motion file

#ifdef _MSC_VER
#define strcasecmp stricmp
#endif //_MSC_VER
#define RADIANS_PER_DEGREE (double)0.0174532925199432957692
#define DEGREES_PER_RADIAN (double)57.2957795130823208768


#define TRUE 1
#define FALSE 0
struct eul_order {
	int order[3];
	char *str;
} eul_orders [] = {
	{-1,-1,-1,"XYZ"},  // XXX Dummy one, no rotation anyway!
    {0, 1, 2, "XYZ"},
    {0, 2, 1, "XZY"},
    {1, 0, 2, "YXZ"},
    {1, 2, 0, "YZX"},
    {2, 0, 1, "ZXY"},
    {2, 1, 0, "ZYX"},
};
char * eul_order_lookup (int *order) {
	for(int i=0;i<7;i++){
		int match = TRUE;
		for(int j=0;j<3;j++)
			match = match && order[j] == eul_orders[i].order[j];
		if(match){
			return eul_orders[i].str;
		}
	}
	return eul_orders[0].str;
}

struct anim_record{
	float lxyz[3];
	float rxyz[3];
};


struct BVH_Node {
	char *name; // bvh joint name
	struct BVH_Node *parent;  // BVH_Node type or None for no parent
	Stack * children;  // a list of children of this type.
    float rest_head_world[3];  // worldspace rest location for the head of this node
    float rest_head_local[3];   // localspace rest location for the head of this node
    float *rest_tail_world;  // worldspace rest location for the tail of this node
    float *rest_tail_local;  // worldspace rest location for the tail of this node
	float rest_tail_local_store[3];
	int channels[7];  // list of 6 ints, -1 for an unused channel, otherwise an index for the BVH motion data lines, loc triple then rot triple
	int rot_order[3];  // a triple of indices as to the order rotation is applied. [0,1,2] is x/y/z - [None, None, None] if no rotation.
    char * rot_order_str; // same as above but a string 'XYZ' format.
    Stack *anim_data; //[6];  // a list one tuple's one for each frame. (locx, locy, locz, rotx, roty, rotz), euler rotation ALWAYS stored xyz order, even when native used.
    BOOL has_loc;  // Convenience function, bool, same as (channels[0]!=-1 or channels[1]!=-1 or channels[2]!=-1)
    BOOL has_rot;  // Convenience function, bool, same as (channels[3]!=-1 or channels[4]!=-1 or channels[5]!=-1)
    int index;  // index from the file, not strictly needed but nice to maintain order
    float *temp;  // use this for whatever you want
};

struct BVH_Node * init_BVH_Node( char *name, float * rest_head_world, float * rest_head_local, 
	struct BVH_Node *parent, int *channels, int *rot_order, int index){
	struct BVH_Node *self = (struct BVH_Node*)malloc(sizeof(struct BVH_Node));
	memset(self,0,sizeof(struct BVH_Node));

	self->name = name;
	veccopy3f(self->rest_head_world,rest_head_world);
	veccopy3f(self->rest_head_local,rest_head_local);
	self->rest_tail_world = NULL; //,-1.0f,-1.0f,-1.0f);
	vecset3f(self->rest_tail_local,-1.0f,-1.0f,-1.0f);
	self->parent = parent;
	memcpy(self->channels,channels,3*sizeof(int));
	memcpy(self->rot_order,rot_order,3*sizeof(int));
	self->rot_order_str = eul_order_lookup(self->rot_order);
	self->index = index;

	// convenience functions
	self->has_loc = channels[0] != -1 || channels[1] != -1 || channels[2] != -1;
	self->has_rot = channels[3] != -1 || channels[4] != -1 || channels[5] != -1;

	self->children = newStack(struct BVH_Node*);

	// list of 6 length tuples: (lx,ly,lz, rx,ry,rz)
	// even if the channels aren't used they will just be zero
	//
	self->anim_data = newStack(struct anim_record); // [(0, 0, 0, 0, 0, 0)]
	return self;
}

    //def __repr__(self):
    //    return ("BVH name: '%s', rest_loc:(%.3f,%.3f,%.3f), rest_tail:(%.3f,%.3f,%.3f)" %
    //            (self.name,
    //             self.rest_head_world.x, self.rest_head_world.y, self.rest_head_world.z,
    //             self.rest_head_world.x, self.rest_head_world.y, self.rest_head_world.z))


//void sorted_nodes(struct BVH_Nodes *bvh_nodes){
//    bvh_nodes_list = list(bvh_nodes.values())
//    bvh_nodes_list.sort(key=lambda bvh_node: bvh_node.index)
//}
//

void read_bvh(char *file_path, char *rotate_mode, float global_scale,
	Stack *bvh_nodes, float *bvh_frame_time, int *bvh_frame_count)
{
    // File loading stuff
    // Open the file for importing
    FILE *fp = fopen(file_path, "r");

    // Seperate into a list of lists, each line a list of words.
	char *rv; 
	char line [2048];
	char *token, *delims;
	rv = fgets(line,2048,fp);

    //char * file_lines = fread(file,file.readlines()
    //// Non standard carrage returns?
    //if len(file_lines) == 1:
    //    file_lines = file_lines[0].split('\r')

    // Split by whitespace.
    //file_lines = [ll for ll in [l.split() for l in file_lines] if ll]
	delims = " ,\r\n\\t\"";
	token = strtok(line,delims);
    // Create hierarchy as empties
    //if file_lines[0][0].lower() == 'hierarchy':
	if( strcasecmp(token,"hierarchy")){
		printf("not a BVH file \n");
		return;
	}

    bvh_nodes = NULL;
   // struct BVH_Nodes *bvh_nodes_serial = NULL;
	Stack *bvh_nodes_serial = newStack(struct BVH_Nodes *);
    *bvh_frame_count = 0;
    *bvh_frame_time = 0.0;

	int channelIndex = -1;

    int lineIdx = 0;  // An index for the file.
    //while(lineIdx < len(file_lines) - 1){
	while( fgets(line,2048,fp)){
        //...
		token = strtok(line,delims);
        if(!strcasecmp(token,"root") || !strcasecmp(token,"joint")){
			char *nametokens[4];
			char name[100];
			int len=0;
			memset(nametokens,0,4*sizeof(void*));
			while(nametokens[len] = strtok(NULL,delims)) len++;
            // Join spaces into 1 word with underscores joining it.
			strcpy(name,nametokens[0]);
            // Make sure the names are unique - Object names will match joint names exactly and both will be unique.
			for(int i=1;i<len-1;i++) {
				strcat(name,"_");
				strcat(name,nametokens[i]);
			}
            // MAY NEED TO SUPPORT MULTIPLE ROOTS HERE! Still unsure weather multiple roots are possible?
            //print '%snode: %s, parent: %s' % (len(bvh_nodes_serial) * '  ', name,  bvh_nodes_serial[-1])
			fgets(line,2048,fp); // {
			fgets(line,2048,fp); // OFFSET 8.77824 4.35084 1.2192
            //lineIdx += 2  // Increment to the next line (Offset)
			token = strtok(line,delims);
			float rest_head_local[3];
			for(int i=0;i<3;i++){
				token = strtok(NULL,delims);
				sscanf(token,"%f",&rest_head_local[i]);
			}
            //rest_head_local = Vector((float(file_lines[lineIdx][1]), float(file_lines[lineIdx][2]), float(file_lines[lineIdx][3]))) * global_scale
            //lineIdx += 1  // Increment to the next line (Channels)
			fgets(line,2048,fp); //     CHANNELS 3 Zrotation Xrotation Yrotation
            // newChannel[Xposition, Yposition, Zposition, Xrotation, Yrotation, Zrotation]
            // newChannel references indices to the motiondata,
            // if not assigned then -1 refers to the last value that will be added on loading at a value of zero, this is appended
            // We'll add a zero value onto the end of the MotionDATA so this always refers to a value.
			int my_channel[6];
			for(int i=0;i<6;i++)
				my_channel[i] = -1;
            //my_channel = [-1, -1, -1, -1, -1, -1]
			int my_rot_order[3];
			for(int i=0;i<3;i++)
				my_rot_order[i] = -1;
            //my_rot_order = [None, None, None]
            int rot_count = 0;
			token = strtok(line,delims); //CHANNELS
			token = strtok(NULL,delims); //3
			int channels;
			sscanf(token,"%d",&channels);
            //for channel in file_lines[lineIdx][2:]:
			for(int i=0;i<channels;i++){
				char *channel = strtok(NULL,delims); //Zrotation
                //channel = channel.lower()
                int channelIndex = i+1; // += 1  // So the index points to the right channel
                if(!strcasecmp(channel,"xposition") )
                    my_channel[0] = channelIndex;
                else if(!strcasecmp(channel,"yposition") )
                    my_channel[1] = channelIndex;
                else if(!strcasecmp(channel,"zposition") )
                    my_channel[2] = channelIndex;

                else if(!strcasecmp(channel,"xrotation")){
                    my_channel[3] = channelIndex;
                    my_rot_order[rot_count] = 0;
                    rot_count += 1;
                }else if(!strcasecmp(channel,"yrotation")){
                    my_channel[4] = channelIndex;
                    my_rot_order[rot_count] = 1;
                    rot_count += 1;
                }else if(!strcasecmp(channel,"zrotation")){
                    my_channel[5] = channelIndex;
                    my_rot_order[rot_count] = 2;
                    rot_count += 1;
				}
			}
            //channels = file_lines[lineIdx][2:]

            struct BVH_Node *my_parent = stack_top(struct BVH_Node*,bvh_nodes_serial); //[-1];  // account for none
			float rest_head_world[3];
            // Apply the parents offset accumulatively
            if( my_parent == NULL)
                veccopy3f(rest_head_world,rest_head_local);
            else
                vecadd3f(rest_head_world,my_parent->rest_head_world, rest_head_local);

			struct BVH_Node *bvh_node;
			int index = vectorSize(bvh_nodes_serial) -1;
            bvh_node = init_BVH_Node(name, rest_head_world, rest_head_local, my_parent, my_channel, my_rot_order, index);
			//bvh_nodes[name] = bvh_node
            // If we have another child then we can call ourselves a parent, else
            stack_push(struct BVH_Node*,bvh_nodes_serial,bvh_node);
		}	
        // Account for an end node
        //if file_lines[lineIdx][0].lower() == 'end' and file_lines[lineIdx][1].lower() == 'site':  // There is sometimes a name after 'End Site' but we will ignore it.
        if(!strcasecmp(token,"end") || !strcasecmp(strtok(NULL,delims),"joint")){  //End Site
			fgets(line,2048,fp); // {
			fgets(line,2048,fp); // OFFSET 8.77824 4.35084 1.2192
            //lineIdx += 2  // Increment to the next line (Offset)
			token = strtok(line,delims);
			float rest_tail[3];
			for(int i=0;i<3;i++){
				token = strtok(NULL,delims);
				sscanf(token,"%f",&rest_tail[i]);
			}
			vecscale3f(rest_tail,rest_tail,global_scale);
            //lineIdx += 2  // Increment to the next line (Offset)
            //rest_tail = Vector((float(file_lines[lineIdx][1]), float(file_lines[lineIdx][2]), float(file_lines[lineIdx][3]))) * global_scale

			struct BVH_Node *bvh_node;
			int index = vectorSize(bvh_nodes_serial) -1;
			bvh_node = vector_get(struct BVH_Node*,bvh_nodes_serial,index);
			vecadd3f(bvh_node->rest_tail_world,bvh_node->rest_head_world,rest_tail);
            //bvh_nodes_serial[-1].rest_tail_world = bvh_nodes_serial[-1].rest_head_world + rest_tail
			vecadd3f(bvh_node->rest_tail_local,bvh_node->rest_head_local,rest_tail);
            //bvh_nodes_serial[-1].rest_tail_local = bvh_nodes_serial[-1].rest_head_local + rest_tail

            // Just so we can remove the Parents in a uniform way - End has kids
            // so this is a placeholder
            //bvh_nodes_serial.append(None)
            stack_push(struct BVH_Node*,bvh_nodes_serial,NULL);
		}
        //if len(file_lines[lineIdx]) == 1 and file_lines[lineIdx][0] == '}':  // == ['}']
		if(!strcmp(token,"}")){ //}
            //bvh_nodes_serial.pop()  // Remove the last item
			stack_pop(struct BVH_Nodes*,bvh_nodes_serial);
		}
        // End of the hierarchy. Begin the animation section of the file with
        // the following header.
        //  MOTION
        //  Frames: n
        //  Frame Time: dt
        //if len(file_lines[lineIdx]) == 1 and file_lines[lineIdx][0].lower() == 'motion':
        if(!strcasecmp(token,"motion") ){ //MOTION
            //lineIdx += 1  // Read frame count.
			fgets(line,2048,fp); //Frames:	2752
			token = strtok(line,delims); //frames:
			if(!strcasecmp(token,"frames:")){
				token = strtok(NULL,delims); //2752
				sscanf(token,"%d",bvh_frame_count);
			}
            //if (len(file_lines[lineIdx]) == 2 and
            //    file_lines[lineIdx][0].lower() == 'frames:'):

            //    bvh_frame_count = int(file_lines[lineIdx][1])

            //lineIdx += 1  // Read frame rate. 
			fgets(line,2048,fp); //Frame Time:	0.00833333
			token = strtok(line,delims); //frame
			if(!strcasecmp(token,"frame")){
				token = strtok(line,delims); //time
				if(!strcasecmp(token,"time")){
					token = strtok(line,delims); //0.00833333
					sscanf(token,"%f",bvh_frame_time);
				}
			}
            //if (len(file_lines[lineIdx]) == 3 and
            //    file_lines[lineIdx][0].lower() == 'frame' and
            //    file_lines[lineIdx][1].lower() == 'time:'):

            //    bvh_frame_time = float(file_lines[lineIdx][2])

            //lineIdx += 1  // Set the cursor to the first frame
			//fgets(line,2048,fp); // Set the cursor to the first frame

            break;
		}
		fgets(line,2048,fp); // Set the cursor to the first frame
        //lineIdx += 1
	} //end while lines
 // Remove the None value used for easy parent reference
 // substitute needed

    //del bvh_nodes[None]
    // Dont use anymore
    //del bvh_nodes_serial

    // importing world with any order but nicer to maintain order
    // second life expects it, which isn't to spec.
    //bvh_nodes_list = sorted_nodes(bvh_nodes)

	Stack *bvh_nodes_list = bvh_nodes_serial;
	int nodecount = vectorSize(bvh_nodes_list);
    //while lineIdx < len(file_lines):
	while(fgets(line,2048,fp)){
        //line = file_lines[lineIdx]
        for(int i=0;i<nodecount;i++){ // bvh_node in bvh_nodes_list:
            //for bvh_node in bvh_nodes_serial:
			struct BVH_Node* bvh_node = vector_get(struct BVH_Node*,bvh_nodes_list,i);
            float lx, ly, lz, rx, ry, rz;
			struct anim_record record;
			lx=ly=lz=rx=ry=rz = 0.0f;
            int * channels = bvh_node->channels;
            Stack *anim_data = bvh_node->anim_data;
            //if( channels[0] != -1){
			if(channels[0] != -1){
				sscanf(strtok(line,delims),"%f",&lx);
				lx *= global_scale;
                //lx = global_scale * float(line[channels[0]])
			}
            //if channels[1] != -1:
			if(channels[1] != -1){
				sscanf(strtok(NULL,delims),"%f",&ly);
				ly *= global_scale;
                //ly = global_scale * float(line[channels[1]])
			}
            //if channels[2] != -1:
			if(channels[2] != -1){
				sscanf(strtok(NULL,delims),"%f",&lz);
				lz *= global_scale;
                //lz = global_scale * float(line[channels[2]])
			}

            //if channels[3] != -1 or channels[4] != -1 or channels[5] != -1:
			if(channels[3] != -1 || channels[4] != -1 || channels[5] != -1){
				//rx = radians(float(line[channels[3]]))
				sscanf(strtok(NULL,delims),"%f",&rx);
				rx *= (float)RADIANS_PER_DEGREE;
				//ry = radians(float(line[channels[4]]))
				sscanf(strtok(NULL,delims),"%f",&ry);
				ry *= (float)RADIANS_PER_DEGREE;
				//rz = radians(float(line[channels[5]]))
				sscanf(strtok(NULL,delims),"%f",&rz);
				rz *= (float)RADIANS_PER_DEGREE;
			}
            // Done importing motion data //
			memset(&record,0,sizeof(struct anim_record));
			record.lxyz[0] = lx;
			record.lxyz[1] = ly;
			record.lxyz[2] = lz;
			record.rxyz[0] = rx;
			record.rxyz[1] = ry;
			record.rxyz[2] = rz;
            //anim_data.append((lx, ly, lz, rx, ry, rz))
			stack_push(struct anim_record,anim_data,record);
		}
        //lineIdx += 1
	}

    // Assign children
    //for bvh_node in bvh_nodes_list:
	for(int i=0;i<vectorSize(bvh_nodes_serial);i++){
		struct BVH_Node *bvh_node = vector_get(struct BVH_Node*,bvh_nodes_serial,i);
        struct BVH_Node *bvh_node_parent = bvh_node->parent;
        if(bvh_node_parent)
            stack_push(struct BVH_Node*,bvh_node_parent->children,bvh_node);
	}
    // Now set the tip of each bvh_node
    //for bvh_node in bvh_nodes_list:
	for(int i=0;i<vectorSize(bvh_nodes_serial);i++){
		struct BVH_Node *bvh_node = vector_get(struct BVH_Node*,bvh_nodes_serial,i);

        //if not bvh_node.rest_tail_world:
		if(!bvh_node->rest_tail_world){
            //if len(bvh_node.children) == 0:
			int nchildren = vectorSize(bvh_node->children);
			if(nchildren == 0){
                // could just fail here, but rare BVH files have childless nodes
                //bvh_node.rest_tail_world = Vector(bvh_node.rest_head_world)
				bvh_node->rest_tail_world = bvh_node->rest_head_world;
                bvh_node->rest_tail_local = bvh_node->rest_head_local;
            //elif len(bvh_node.children) == 1:
			}else if(nchildren == 1){
				struct BVH_Node *bvh_child0 = vector_get(struct BVH_Node*,bvh_node->children,0);
                bvh_node->rest_tail_world = bvh_child0->rest_head_world; //[0]->rest_head_world;
                //bvh_node->rest_tail_local = bvh_node.rest_head_local + bvh_node.children[0].rest_head_local

                bvh_node->rest_tail_local = vecadd3f(bvh_node->rest_tail_local_store,bvh_node->rest_head_local,bvh_child0->rest_head_local);
            }else{
                // allow this, see above
                //if not bvh_node.children:
                //	raise Exception("bvh node has no end and no children. bad file")

                // Removed temp for now
                float rest_tail_world[3]; // = Vector((0.0, 0.0, 0.0))
				vecset3f(rest_tail_world,0.0f,0.0f,0.0f);
                float rest_tail_local[3]; // = Vector((0.0, 0.0, 0.0))
				vecset3f(rest_tail_local,0.0f,0.0f,0.0f);
                //for bvh_node_child in bvh_node.children:
				for(int j=0;j<nchildren;j++){
					struct BVH_Node *bvh_child = vector_get(struct BVH_Node*,bvh_node->children,j);
                    //rest_tail_world += bvh_node_child.rest_head_world
					vecadd3f(rest_tail_world,rest_tail_world,bvh_child->rest_head_world);
                    //rest_tail_local += bvh_node_child.rest_head_local
					vecadd3f(rest_tail_local,rest_tail_local,bvh_child->rest_head_local);
				}
				vecscale3f(bvh_node->rest_tail_world,rest_tail_world,1.0f/(float)nchildren);
                //bvh_node.rest_tail_world = rest_tail_world * (1.0 / len(bvh_node.children))
				vecscale3f(bvh_node->rest_tail_local,rest_tail_local,1.0f/(float)nchildren);
                //bvh_node.rest_tail_local = rest_tail_local * (1.0 / len(bvh_node.children))
			}
		}
        // Make sure tail isn't the same location as the head.
		if( vecapprox3f(bvh_node->rest_tail_local,bvh_node->rest_head_world,.001f*global_scale)){
			//if (bvh_node.rest_tail_local - bvh_node.rest_head_local).length <= 0.001 * global_scale:
            printf("\tzero length node found:", bvh_node->name);
            bvh_node->rest_tail_local[1] += global_scale / 10.0f;
            bvh_node->rest_tail_world[1] += global_scale / 10.0f;
		}
	}
	bvh_nodes = bvh_nodes_serial;
    //return bvh_nodes, bvh_frame_time, bvh_frame_count
}



// <<<< read BVH motion file