/*


X3D Lighting Component

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
#include "../opengl/OpenGL_Utils.h"
#include "../opengl/Frustum.h"
#include "RenderFuncs.h"
//#include "../opengl/OpenGL_Utils.h"
#include "LinearAlgebra.h"
#include "Polyrep.h"
#include "Component_Shape.h"

typedef struct pComponent_Lighting {
	Stack* genshadow_stack;
	Stack* light_stack;
}*ppComponent_Lighting;

static void* Component_Lighting_constructor() {
	void* v = MALLOCV(sizeof(struct pComponent_Lighting));
	memset(v, 0, sizeof(struct pComponent_Lighting));
	return v;
}

// iglobal.c loves to call this one.
void Component_Lighting_init(struct tComponent_Lighting* t) {
	//public
	//private
	t->prv = Component_Lighting_constructor();
	{
		ppComponent_Lighting p = (ppComponent_Lighting)t->prv;
		p->genshadow_stack = newStack(usehit);
		p->light_stack = newStack(usehit);
	}
}

// iglobal.c loves to call this one.
void Component_Lighting_clear(struct tComponent_Lighting* t) {
	//public
	//private
	{
		ppComponent_Lighting p = (ppComponent_Lighting)t->prv;
		deleteVector(usehit, p->genshadow_stack);
		deleteVector(usehit, p->light_stack);
	}
}

//ppComponent_Lighting p = (ppComponent_Lighting)gglobal()->Component_Lighting.prv;
void get_view_matrix(double* savePosOri, double* saveView);
void set_debug_quad_near_farplane(float nearplane, float farplane);
void lightTable_clear();
void lightTable_push(usehit tuple);
void lightTable_pop();
int lightTable_count();
usehit* lightTable_item(int i);
void generate_shadowmap_2D(usehit uhit, int index);

//LIGHT TABLE
void lightTable_clear() {
	//called once per frame, before the search for global=true projectors
	//will clear any global=true projectors from last frame
	ppComponent_Lighting p = (ppComponent_Lighting)gglobal()->Component_Lighting.prv;
	clearStack(p->light_stack);
}
void lightTable_push(usehit tuple) {
	//called when we find a global=true, on=true light and
	//called in sib_prep for a global=false, on=true light
	ppComponent_Lighting p = (ppComponent_Lighting)gglobal()->Component_Lighting.prv;
	stack_push(usehit, p->light_stack, tuple);

}
void lightTable_pop() {
	//called in sib_fin for a global=false, on=true light
	ppComponent_Lighting p = (ppComponent_Lighting)gglobal()->Component_Lighting.prv;
	if (p->light_stack->n < 1)
		printf("ouch from lightTable_pop()\n");
	stack_pop(usehit, p->light_stack);
}
int lightTable_count() {
	ppComponent_Lighting p = (ppComponent_Lighting)gglobal()->Component_Lighting.prv;
	return p->light_stack->n;
}
usehit *lightTable_item(int i) {
	ppComponent_Lighting p = (ppComponent_Lighting)gglobal()->Component_Lighting.prv;
	return vector_get_ptr(usehit,p->light_stack,i);
}
int lightTable_node_use_count(struct X3D_Node *node) {
	int count = 0;
	for (int i = 0; i < lightTable_count(); i++) {
		if (lightTable_item(i)->node == node) count++;
	}
	return count;
}


// a specialization of InternalRep - see PolyRep.h
struct X3D_LightRep {
	int itype; //=5, 0 PointRep 1 LineRep 2 PolyRep 3 MeshRep 4 TextureRep 5 LightRep 6 ProjectorRep
	//depth section
	Stack* depth_buffer_stack;
	int size;
	double matproj[16];
	double matview[16];
	//light section
	void* lightbuf;
	int ilightbuf; //opengl uniform buffer index

};

void* set_LightRep(void* _lightrep)
{
	struct X3D_LightRep* lightrep = _lightrep;
	if (!_lightrep) {
		_lightrep = MALLOC(struct X3D_LightRep*, sizeof(struct X3D_LightRep));
		memset(_lightrep, 0, sizeof(struct X3D_LightRep));
		lightrep = (struct X3D_LightRep*)_lightrep;
		lightrep->itype = 5;
		lightrep->ilightbuf = -1;
		lightrep->size = 1024; //size of shadow image, or for pointlight, size of each of 6 sides of cubemap
		//lightrep->idepthtexture = -1;
	}
	return lightrep;
}
void delete_LightRep(void* _lightrep) {
	//call during node deletion > unRegisterX3DAnyNode > delete_geomrep
	if (_lightrep) {
		struct X3D_LightRep* lightrep = _lightrep;
		if (lightrep->depth_buffer_stack) {
			//Q. are the texture nodes it points to registered and deleted separately?
			// here we assume so
			deleteStack(struct X3D_Node*, lightrep->depth_buffer_stack);
		}
		FREE_IF_NZ(_lightrep);
	}
}

// PointLight Shadowmap phases/stages
// cubemap Stage I - creating empty cubemap and 6 side textures and fbo
// cubemap Stage II - rendering the 6 sides of cubemap to fill with images
// cubemap Stage III - sending to shader for sampling cubemap
//GCM generated cube map texture method:
// - Stage II uses 6 individual textures
//non-GCM method:
// - uses cubemap in Stage II, specificie which side is attached to FBO for rendering
static int gcm_method_used = 0;
int gcm_method() {
	return gcm_method_used;
}

#define RETURN_IF_LIGHT_STATE_NOT_US \
		if (renderstate()->render_light== VF_globalLight) { \
			if (!node->global) return;\
			/* printf ("and this is a global light\n"); */\
		} else if (node->global || renderstate()->render_depth || !renderstate()->render_geom ) return; \
		/* else printf ("and this is a local light\n"); */


void projPerspective(double fovy, double aspect, double zNear, double zFar, double* matrix);
void projOrtho(double left, double right, double bottom, double top,
	double nearZ, double farZ, double* matrix);
void projLookAt(double eyex, double eyey, double eyez,
	double centerx, double centery, double centerz,
	double upx, double upy, double upz, double* matrix);
double* matrix_lookAtd(double* eye3, double* center3, double* up3, double* matrix) {
	//gluLookAt convention:
	// eye - the viewpoint
	// center - any point along the ray to the scene, typically a point on the geometry in the scene to look at
	// up - which way is up in the viewing volume
	projLookAt(eye3[0], eye3[1], eye3[2], center3[0], center3[1], center3[2], up3[0], up3[1], up3[2], matrix);
	return matrix;
}
double* matrix_lookAtfd(float* eye3, float* center3, float* up3, double* matrix) {
	double eyed[3], centerd[3], upd[3];
	float2double(eyed, eye3, 3);
	float2double(centerd, center3, 3);
	float2double(upd, up3, 3);
	matrix_lookAtd(eyed, centerd, upd, matrix);
	return matrix;
}
void compile_DirectionalLight (struct X3D_DirectionalLight *node) {
    struct point_XYZ vec;


    MARK_NODE_COMPILED;
}
void mesa_Ortho(GLDOUBLE left, GLDOUBLE right, GLDOUBLE bottom, GLDOUBLE top, GLDOUBLE nearZ, GLDOUBLE farZ, GLDOUBLE* m);
void render_DirectionalLight0(struct X3D_Node* parent, struct X3D_DirectionalLight* node) {

	// if we are doing global lighting, is this one for us? 
	RETURN_IF_LIGHT_STATE_NOT_US

	COMPILE_IF_REQUIRED;

	if (node->on) {
		//both global on VF_GlobalLight on children->render, and local on prep_sibAffectors come in here
		usehit uhit;
		uhit.node = X3D_NODE(node);
		uhit.userdata = parent;
		fw_glGetDoublev(GL_MODELVIEW_MATRIX, uhit.mvm);
		if (node->shadows) {
			//prepare local view matrix (from node.location, node.direction which aren't included in modelview matrix)
			// and projection matrix, both of which are stable / same between DEF and USE instances of a directionallight
			node->_intern = set_LightRep(node->_intern);
			struct X3D_LightRep* lightrep = (struct X3D_LightRep*)node->_intern;
			//lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
			//for up vector in theory we need a few cross products to ensure its at least orthogonal to direction
			//- up is somewhat arbitrary -spotlight is symmetrical about direction vector-
			//  but must be consistent between depth texture rendering and shader sampling
			float up[3], center[3], location[3];
			vecset3f(up, 0.0f, 1.0f, 0.0f);
			vecset3f(location, 0.0f, 0.0f, 0.0f);
			vecadd3f(center, location, node->direction.c);
			matrix_lookAtfd(location, center, up, lightrep->matview);

			//for ortho, we will scale in render_directionalLight to parent extent 
			//so shadow map covers siblings affected
			float eout6[6], ein6[6];
			double ed[6];
			if (node->global) {
				extent6f_copy(ein6, rootNode()->_extent);
			}
			else {
				extent6f_copy(ein6, parent->_extent);
			}
			if (!extent6f_isSet(ein6) ) {
				float e6[6], scale3[3];
				extent6f_constructor(e6, -1.0f, 1.0f, -1.0f, 1.0f, .1f, 15.0f); //something for the first frame
				extent6f_scale3f(eout6, e6, vecset3f(scale3, 8.0f, 4.0f, 1.0f));
			}
			else {
				float e6[6], scale3[3];
				extent6f_mattransform4d(e6, ein6, lightrep->matview);
				extent6f_scale3f(eout6, e6, vecset3f(scale3, 1.01f, 1.01f, 1.01f));

			}
			int method = 1;
			if (method == 0) {
				//extent6f_printf(eout6); printf("e6\n");
				float2double(ed, eout6, 6);
				//projOrtho(ed[1],ed[0],ed[3],ed[2], .1, ed[4], lightrep->matproj); only half or 1/4 the area
				mesa_Ortho(ed[1], ed[0], ed[3], ed[2], .1, ed[4], lightrep->matproj);

				set_debug_quad_near_farplane(eout6[4], eout6[5]);
			}
			else if (method == 1) {
				//convert extent in lightview space into an equivalent transform
				//and transform it back to light space, to append to mvm
				float size[3];
				double mate[16], matel[16], dcenter[3], dsize[3], matviewInv[16], mvm2[16], matev[16], matevinv[16];
				extent6f2bbox(eout6, center, size);
				//extent6f_printf(eout6); printf("extent in lightview space\n");
				//vecprint3fb("center", center, "\n");
				//vecprint3fb("size", size, "\n");
				matidentity4d(mate);
				mattranslate4d(mate, float2double(dcenter, center, 3));
				matscale4d(mate, float2double(dsize, size, 3));
				//printmatrix2(mate, "center and size mat in lightview space");

				//printmatrix2(matevinv, "center and size in light space");
				matcopy(uhit.extra, mate);
				//do the following in generate_shadowmap_2D and shadow section of sendLightInfo2, with mate = uhit.extra
				//	matmultiplyAFFINE(mvm2, uhit.extra, uhit.mvm);
				//matcopy(uhit.mvm, mvm2);
				mesa_Ortho(-.5,.5,-.5,.5, .001,.5, lightrep->matproj);

				set_debug_quad_near_farplane(.0, .5);

			}
			int nuse = lightTable_node_use_count(X3D_NODE(node));
			uhit.ivalue = make_or_get_depth_buffer(nuse, X3D_NODE(node));
			generate_shadowmap_2D(uhit, 0);

		}
		lightTable_push(uhit);
	}
}
void render_DirectionalLight(struct X3D_DirectionalLight* node) {
	render_DirectionalLight0(NULL, node);
}
/* global lights  are done before the rendering of geometry */
void prep_DirectionalLight (struct X3D_DirectionalLight *node) {
	if (!renderstate()->render_light) return;
	render_DirectionalLight(node);
}

static struct X3D_DirectionalLight* headlight = NULL;
void push_headlight() {
	if (!headlight) {
		headlight = createNewX3DNode(NODE_DirectionalLight);
		compile_DirectionalLight(headlight);
		vecset3f(headlight->direction.c, 0.0f, 0.0f, -1.0f);
		headlight->global = TRUE;
		headlight->on = TRUE;
	}
	{
		// like render_DirectionalLight(headlight);
		// except identity mvm (headlight pose is identity in avatar coordinates)
		// and no compile_shadowMap / no shadowmaps for headlight (shadows would be obscured anyway)
		struct X3D_DirectionalLight* node =headlight;
		RETURN_IF_LIGHT_STATE_NOT_US
		usehit uhit;
		uhit.node = X3D_NODE(headlight);
		matidentity4d(uhit.mvm);
		lightTable_push(uhit);
	}
}
void render_headlight() {
	//how set renderflags so it knows its VF_GlobalLight?
	//call from render_hier on VF_globalLight pass, before scenegraph render
	if (fwl_get_headlight()) //checks if Viewer requests headlight
		push_headlight();
}


void compile_PointLight(struct X3D_PointLight* node) {
	if (node->shadows) {
		//prepare local view matrix (from node.location, which isn't included in modelview matrix)
		// and projection matrix, both of which are stable / same between DEF and USE instances of a Pointlight
		node->_intern = set_LightRep(node->_intern);
		struct X3D_LightRep* lightrep = (struct X3D_LightRep*)node->_intern;
		set_debug_quad_near_farplane(.5f, node->radius);

		//lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		//for up vector in theory we need a few cross products to ensure its at least orthogonal to direction
		//- up is somewhat arbitrary -spotlight is symmetrical about direction vector-
		//  but must be consistent between depth texture rendering and shader sampling
		float up[3], direction[3], center[3];
		vecset3f(up, 0.0f, 1.0f, 0.0f);
		vecset3f(direction, 0.0f, 0.0f, 1.0f);
		vecadd3f(center, node->location.c, direction);
		matrix_lookAtfd(node->location.c, center, up, lightrep->matview);
		matidentity4d(lightrep->matproj);
	}
	MARK_NODE_COMPILED;
}

void generate_shadowmap_cube(usehit uhit, int index);
void render_PointLight0(struct X3D_Node* parent, struct X3D_PointLight* node) {
	float ft;

	/* if we are doing global lighting, is this one for us? */
	RETURN_IF_LIGHT_STATE_NOT_US

	COMPILE_IF_REQUIRED;

	if (node->on) {
		//both global on VF_GlobalLight on children->render, and local on prep_sibAffectors come in here
		usehit uhit;
		uhit.node = X3D_NODE(node);
		uhit.userdata = parent;
		fw_glGetDoublev(GL_MODELVIEW_MATRIX, uhit.mvm);
		if (node->shadows) {
			int nuse = lightTable_node_use_count(X3D_NODE(node));
			uhit.ivalue = make_or_get_depth_buffer(nuse, X3D_NODE(node));
			generate_shadowmap_cube(uhit, uhit.ivalue);
			/* Finished rendering CubeMap, set it back for normal textures */
			getAppearanceProperties()->cubeFace = 0;


		}
		lightTable_push(uhit);
	}
}
void render_PointLight(struct X3D_PointLight* node) {
	render_PointLight0(NULL, node);
}
/* pointLights are done before the rendering of geometry */
void prep_PointLight (struct X3D_PointLight *node) {

	if (!renderstate()->render_light) return;
	/* this will be a global light here... */
	render_PointLight(node);
}


/* 
// NOT IMPLEMENTED as of June 2022, would buffer lights so don't send every child_shape
//UNIFORM BUFFER
//https ://www.khronos.org/opengl/wiki/Interface_Block_(GLSL) 
//-about ¾ down Buffer backed
//https ://learnopengl.com/Advanced-OpenGL/Advanced-GLSL 
//-25 % down Interface blocks > Uniform blocks, Lights mentioned.

static int light_buf = 0;
int light_buffering() {
	return light_buf;
}
struct Lightbuf {
	//std140 padding required so GPU padding using std140 is aligned
	//static part of light, stored in gl buffer
	float color[4];			//padded so consistently ends on vec4 boundary on CPU side, can read as vec3 on GPU side
	float location[4]; 
	float halfVector[4];
	float direction[4]; 
	float attenuations[4]; 
	float matview[16];		//contains .location and .direction +up please transpose for shader
	float matproj[16];		//shadow map > contains frustum projection, please transpose for shader
	float ambient;			//the rest take up 4 bytes each on GPU and CPU, no padding needed except at end
	float intensity;
	float spotBeamWidth;
	float spotCutoff;
	float lightRadius;
	float shadowIntensity;
	bool shadows;
	int depthmap;
	int lighttype;
	int pad1;
	int pad2;
	int pad3;
};
static int lightpose_buf = 0;
int lightpose_buffering() {
	return lightpose_buf;
}
struct LightPose {
	//std140 padding required so GPU padding using std140 is aligned
	//dynamic part of light, specifically modelview matrix aka visit transform 
	// can be different for each render_ visit
	// --can have multiple DEF/USE references to a light, or parent can be DEF/USEd--
	// and typically re-sent to shader every child_shape affected, unless buffered
	// lifespan of a visit transform is maximum 1 frame, so when lightTable cleared, should be cleared.
	float modelview[16];	//visit transform: to transform .location .direction into viewpoint / eye space
	float eye2frustum[16];	//for shadow maps, transforms from viewpoint to shadow map frustum
	int lightbuf;			//opengl uniform buffer index pointing to a struct Lightbuf
	int lightbufIndex;		//index of bound LightBufs in shader.
	int pad2;
};
*/
void compile_SpotLight (struct X3D_SpotLight *node) {
	if (node->shadows) {
		//prepare local view matrix (from node.location, node.direction which aren't included in modelview matrix)
		// and projection matrix, both of which are stable / same between DEF and USE instances of a spotlight
		node->_intern = set_LightRep(node->_intern);
		struct X3D_LightRep* lightrep = (struct X3D_LightRep*)node->_intern;
		projPerspective(node->cutOffAngle * (180.0 / PI) * 2.0, 1.0, .5, (double)node->radius, lightrep->matproj);
		set_debug_quad_near_farplane(.5f, node->radius);

		//for up vector in theory we need a few cross products to ensure its at least orthogonal to direction
		//- up is somewhat arbitrary -spotlight is symmetrical about direction vector-
		//  but must be consistent between depth texture rendering and shader sampling
		float up[3], center[3];
		vecset3f(up, 0.0f, 1.0f, 0.0f);
		vecadd3f(center, node->location.c, node->direction.c);
		matrix_lookAtfd(node->location.c, center, up, lightrep->matview);
	}
	/* NOT IMPLEMENTED as of June 2022
	if (light_buffering()) {
		//static / infrequently changing part of light
		//.on and .global aren't included, they are implied in the list sent in sendLightInfo() to shader
		//modelview matrix isn't included: DEF/USE instances each have a different LightPose
		node->_intern = set_LightRep(node->_intern);
		struct X3D_LightRep* lightrep = (struct X3D_LightRep*)node->_intern;
		if (!lightrep->lightbuf) {
			lightrep->lightbuf = malloc(sizeof(struct Lightbuf));
			memset(lightrep->lightbuf, 0, sizeof(struct Lightbuf));
		}
		struct Lightbuf* lightbuf = (struct Lightbuf*)lightrep->lightbuf;
		//fill out lightbuf from node fields
		veccopy3f(lightbuf->color, node->color.c);
		veccopy3f(lightbuf->location, node->location.c);
		veccopy3f(lightbuf->direction, node->direction.c);
		veccopy3f(lightbuf->attenuations, node->attenuation.c);
		lightbuf->ambient = node->ambientIntensity;
		lightbuf->intensity = node->intensity;
		lightbuf->spotBeamWidth = node->beamWidth;
		lightbuf->spotCutoff = node->cutOffAngle;
		lightbuf->lightRadius = node->radius;
		lightbuf->shadows = node->shadows;
		lightbuf->lighttype = 1; //0 point 1 spot 2 direction
		//if shadows, update proj and view matx
		if (lightbuf->shadows) {
			double mtrans[16];
			mattranspose(mtrans, lightrep->matview);
			double2float(lightbuf->matview, mtrans,16);
			mattranspose(mtrans, lightrep->matproj);
			double2float(lightbuf->matproj, mtrans,16);
		}
		if (lightrep->ilightbuf < 0)
			glGenBuffers(1, &lightrep->ilightbuf);
		glBindBuffer(GL_UNIFORM_BUFFER, lightrep->ilightbuf);
		int bufsize = sizeof(struct Lightbuf); 
		glBufferData(GL_UNIFORM_BUFFER, bufsize, lightbuf, GL_STATIC_DRAW); // allocate 152 bytes of memory
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

	}
	*/
    MARK_NODE_COMPILED;
}


void render_SpotLight0(struct X3D_Node *parent, struct X3D_SpotLight *node) {
	float ft;

	/* if we are doing global lighting, is this one for us? */
	RETURN_IF_LIGHT_STATE_NOT_US

    COMPILE_IF_REQUIRED;

	if(node->on) {
		//both global on VF_GlobalLight on children->render, and local on prep_sibAffectors come in here
		usehit uhit;
		uhit.node = X3D_NODE(node);
		uhit.userdata = parent;
		fw_glGetDoublev(GL_MODELVIEW_MATRIX, uhit.mvm);
		if (node->shadows) {
			int nuse = lightTable_node_use_count(X3D_NODE(node));
			//shadowTable_push(uhit);
			//render_shadowMap(X3D_NODE(node));
			uhit.ivalue = make_or_get_depth_buffer(nuse,X3D_NODE(node));
			generate_shadowmap_2D(uhit, 0);

		}
		lightTable_push(uhit);
		/* NOT IMPLEMENTED as of June 2022
		if (lightpose_buffering()) {
			//dynamic maximum lifespan 1 frame buffering of light visit transform
			//so (sibling or global) affected shapes can share common LightPose rather than resending on every child_shape
			struct X3D_LightRep* lightrep = (struct X3D_LightRep*)node->_intern;
			float w2l[16];
			//following textureProjector
			double modelviewinv[16], eye2projector[16], matfull[16], mtrans[16];
			matinverse(modelviewinv, uhit.mvm);
			matmultiplyAFFINE(eye2projector, modelviewinv, lightrep->matview);
			matmultiplyFULL(matfull, eye2projector, lightrep->matproj);
			double2float(w2l, matfull, 16);
			struct LightPose pose;
			mattranspose(mtrans, matfull);
			double2float(pose.eye2frustum, mtrans, 16);
			mattranspose(mtrans, uhit.mvm);
			double2float(pose.modelview, mtrans, 16);
			pose.lightbuf = lightrep->ilightbuf;
			//push_heavy should manage a reusable list of uniform buffers
			// - the size of maximum light visits per frame
			// - and refresh buffer contents during a push (and ignoring old contents on pop or lightTable_clear())
		//	lightTable_push_heavy(uhit, &pose); //not yet implemented
		}
		*/
	}
}
void render_SpotLight(struct X3D_SpotLight* node) {
	render_SpotLight0(NULL, node);
}
/* SpotLights are done before the rendering of geometry */
void prep_SpotLight (struct X3D_SpotLight *node) {
	if (!renderstate()->render_light) return;
	render_SpotLight(node);
}

void compile_EnvironmentLight(struct X3D_EnvironmentLight * node){
}
void render_EnvironmentLight(struct X3D_EnvironmentLight * node){
}
void prep_EnvironmentLight(struct X3D_EnvironmentLight * node){
}


void sib_prep_Light(struct X3D_Node* parent, struct X3D_Node* sibAffector);
void sib_fin_Light(struct X3D_Node* parent, struct X3D_Node* sibAffector);
void sib_prep_Light(struct X3D_Node* parent, struct X3D_Node* sibAffector) {
	struct X3D_PointLight* light = X3D_POINTLIGHT(sibAffector);
	if (renderstate()->render_light != VF_globalLight && !renderstate()->render_depth && renderstate()->render_geom) {
		if (light->global == FALSE && light->on == TRUE) {
			//should lightTable_push(usehit):
			switch (light->_nodeType) {
			case NODE_SpotLight:
				render_SpotLight0(parent,(struct X3D_SpotLight*)sibAffector);
				break;
			case NODE_PointLight:
				render_PointLight0(parent,(struct X3D_PointLight*)sibAffector);
				break;
			case NODE_DirectionalLight:
				render_DirectionalLight0(parent,(struct X3D_DirectionalLight*)sibAffector);
				break;
			default:
				break;
			}
		}
	}
}
void sib_fin_Light(struct X3D_Node* parent, struct X3D_Node* sibAffector) {
	if (renderstate()->render_light != VF_globalLight && !renderstate()->render_depth && renderstate()->render_geom) {
		struct X3D_PointLight* light = X3D_POINTLIGHT(sibAffector);
		if(light->global == FALSE && light->on == TRUE)
			lightTable_pop();
	}
}

// BORROWED FROM GENERATEDCUBEMAPTEXTURE AND MODIFIED FOR DEPTH CUBEMAP FOR POINTLIGHT >>>
//
// see https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping 
// for Phase I generate shadow map and Phase II use shadow map 
//

#include "Component_Shape.h"
#include "../opengl/Textures.h"
void pushnset_framebuffer(int ibuffer);
void popnset_framebuffer();

#ifdef GL_DEPTH_COMPONENT32
#define FW_GL_DEPTH_COMPONENT GL_DEPTH_COMPONENT32
#else
#define FW_GL_DEPTH_COMPONENT GL_DEPTH_COMPONENT16
#endif
int haveFrameBufferObject();



void freeASCIIString(struct Uni_String* us);


struct X3D_Node * make_depth_buffer(int width, int height) {
	struct X3D_PixelTexture* tex = (struct X3D_PixelTexture*)createNewX3DNode(NODE_PixelTexture);;
	PRINT_GL_ERROR_IF_ANY("make_depth_buffer START");
	//if (tex->__subTextures.n == 0) 
	{

		int i;
		struct textureTableIndexStruct* tti;

		tti = getTableIndex(tex->__textureTableIndex);
		tti->status = TEX_LOADED; // TEX_NEEDSBINDING; //I found I didn't need - yet
		tti->idepthbuffer = 1;
		tti->x = width;
		tti->y = height;
		//tti->z = 6;
//		loadTextureNode(X3D_NODE(tex), NULL);
		if (tti->ifbobuffer == 0 && haveFrameBufferObject()) {
			int j;
			tti->x = width; //by storing and retrieving initial size from here
			tti->y = height;
			tti->z = 1;
			// https://www.opengl.org/wiki/Framebuffer_Object

			glGenFramebuffers(1, &tti->ifbobuffer);
			pushnset_framebuffer(tti->ifbobuffer); //binds framebuffer. we push here, in case higher up we are already rendering the whole scene to an fbo

			glGenTextures(1, &tti->OpenGLTexture);
			glBindTexture(GL_TEXTURE_2D, tti->OpenGLTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tti->OpenGLTexture, 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
			popnset_framebuffer(); //tti->ifbobuffer);
		}

	}
	PRINT_GL_ERROR_IF_ANY("make_depth_buffer END");


	return X3D_NODE(tex);

}


struct X3D_Node* make_depth_buffer_cube(int width, int height) {
	//called once per program run for each PointLight scenegraph USE / visit
	//its a 'dynamic cubemap' like GeneratedCubeMapTexture
	// that means we'll be re-rendering and re-submitting each of the 6 textures once per frame
	// so need access.
	// cubemap Stage I
	struct X3D_GeneratedCubeMapTexture *cubetex = createNewX3DNode(NODE_GeneratedCubeMapTexture);
	PRINT_GL_ERROR_IF_ANY("make_depth_buffer_cube START");
	//if (cubetex->__subTextures.n == 0) 
	{

		int i;
		struct textureTableIndexStruct* tti;
		tti = getTableIndex(cubetex->__textureTableIndex);
		tti->status = TEX_LOADED; // I found I didn't need - yet TEX_NEEDSBINDING; //
		tti->x = width;
		tti->y = height;
		tti->z = 1;
		tti->idepthbuffer = 1;
		GLuint status;
		if (tti->ifbobuffer == 0 && haveFrameBufferObject()) {
			int j;
			// https://www.opengl.org/wiki/Framebuffer_Object
			// https://learnopengl.com/Advanced-OpenGL/Cubemaps - doesn't show 'dynamic' cubemaps, but create with images, not renderbuffers, so can sample in shader
			// https://learnopengl.com/Advanced-OpenGL/Framebuffers 

			{
				glGenTextures(1, &tti->OpenGLTexture);
				glBindTexture(GL_TEXTURE_CUBE_MAP, tti->OpenGLTexture);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexParameter.xhtml 
				for (size_t i = 0; i < 6; ++i) {
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
				}
				glGenFramebuffers(1, &tti->ifbobuffer);
				pushnset_framebuffer(tti->ifbobuffer); //binds framebuffer. we push here, in case higher up we are already rendering the whole scene to an fbo
				PRINT_GL_ERROR_IF_ANY("make_depth_buffer_cube 1");
				glDrawBuffer(GL_NONE);
				glViewport(0, 0, width, height);

				//bind one tex now for fun, and to check FBO completeness, but will bind in iteration loop during depth rendering generate_shadowmap_cube
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + 0, tti->OpenGLTexture, 0);
				status = glCheckNamedFramebufferStatus(tti->ifbobuffer, GL_FRAMEBUFFER);

				popnset_framebuffer(); //tti->ifbobuffer);
				glBindTexture(GL_TEXTURE_CUBE_MAP, 0);


			}

			// https://www.khronos.org/opengl/wiki/Framebuffer_Object#Framebuffer_Completeness
			// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glCheckFramebufferStatus.xhtml 
			status = glCheckNamedFramebufferStatus(tti->ifbobuffer, GL_FRAMEBUFFER);
			if (status != GL_FRAMEBUFFER_COMPLETE) {
				printf("make_depth_buffer_cube: framebuffer not complete\n");
				switch (status) {
				case GL_FRAMEBUFFER_UNDEFINED:
					printf("GL_FRAMEBUFFER_UNDEFINED\n"); break;
				case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
					printf("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n"); break;
				case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
					printf("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n"); break;
				case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
					printf("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER\n"); break;
				case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
					printf("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER\n"); break;
				case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
					printf("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE\n"); break;
				case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
					printf("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS\n"); break;
				case GL_FRAMEBUFFER_UNSUPPORTED:
					printf("GL_FRAMEBUFFER_UNSUPPORTED\n"); break;
				default:
					printf("unknown GL error %u\n", (unsigned int)status); break;
				}
			}
		}
	}
	PRINT_GL_ERROR_IF_ANY("make_depth_buffer_cube END");

	// tell the whole system to re-create the data for these sub-children 
	//node->__regenSubTextures = TRUE;

	return X3D_NODE(cubetex);

}
int make_or_get_depth_buffer(int index, struct X3D_Node* node) {
	node->_intern = set_LightRep(node->_intern);
	struct X3D_LightRep* lightrep = (struct X3D_LightRep* )node->_intern;
	if (!lightrep->depth_buffer_stack) {
		lightrep->depth_buffer_stack = newStack(struct X3D_Node*);
	}
	if (index > -1 && index < vectorSize(lightrep->depth_buffer_stack)) return index;
	struct X3D_Node* depth_buffer_texture = NULL;
	if(node->_nodeType == NODE_PointLight)
		depth_buffer_texture = make_depth_buffer_cube(lightrep->size, lightrep->size);
	else
		depth_buffer_texture = make_depth_buffer(lightrep->size, lightrep->size);
	stack_push(struct X3D_Node*, lightrep->depth_buffer_stack, X3D_NODE(depth_buffer_texture));
	return vectorSize(lightrep->depth_buffer_stack) - 1;
}




// https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping  
// shows rendering of shadow maps, and debug quad rendering
// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
static unsigned int quadVAO = 0;
static unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-.8f,  .8f, 0.0f, 0.0f, 1.0f,
			-.8f, -.8f, 0.0f, 0.0f, 0.0f,
			 .8f,  .8f, 0.0f, 1.0f, 1.0f,
			 .8f, -.8f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	PRINT_GL_ERROR_IF_ANY("render_quad before glDrawArrays");
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	PRINT_GL_ERROR_IF_ANY("render_quad after glDrawArrays");
	glBindVertexArray(0);
}
static struct debug_quad {
	int textureID; // opengl texture, -1 for no texture
	int which_debug_shader;
	float near_plane, far_plane;
} debug_quad = { -1,0,1.0f,15.0f };
void set_debug_quad(int which_debug_shader, int textureID) {
	// which_debug_shader - flag to indicate which quad shader 
	//  0=turn off debug quad 
	//  1-normal texture 
	//  2=ortho depth 
	//  3=perspective depth
	//  4=cubemap color
	//  5=cubemap depth latitude, longitude method
	//  6=cubemap depth Tee method
	// textureID - opengl texture number
	debug_quad.textureID = textureID;
	debug_quad.which_debug_shader = which_debug_shader;
}
void set_debug_quad_near_farplane(float nearplane, float farplane) {
	debug_quad.near_plane = nearplane;
	debug_quad.far_plane = farplane;
}
void render_debug_quad() {
	//call this routinely at the end of main scene render() before swapBuffers
	// if no debug_request just returns, else renders a quad over any rendered scene
	int ia;
	if (debug_quad.textureID < 0)return;
	PRINT_GL_ERROR_IF_ANY("render_debug_quad START");
	s_shader_capabilities_t* scap;
	shaderflagsstruct shader_requirements;
	memset(&shader_requirements, 0, sizeof(shaderflagsstruct));
	shader_requirements.debug = debug_quad.which_debug_shader;
	scap = getMyShaders(shader_requirements);
	enableGlobalShader(scap);
	if (debug_quad.which_debug_shader > 2 && debug_quad.which_debug_shader < 5 || debug_quad.which_debug_shader == 6) {
		ia = glGetUniformLocation(scap->myShaderProgram, "near_plane");
		glUniform1f(ia, debug_quad.near_plane);
		ia = glGetUniformLocation(scap->myShaderProgram, "far_plane");
		glUniform1f(ia, debug_quad.far_plane);
	}
	ia = glGetUniformLocation(scap->myShaderProgram, "textureUnit");
	glUniform1i(ia, 0);
	PRINT_GL_ERROR_IF_ANY("render_debug_quad before ActiveTexture");

	glActiveTexture(GL_TEXTURE0);
	PRINT_GL_ERROR_IF_ANY("render_debug_quad before enable CUBE_MAP");

	if (debug_quad.which_debug_shader > 3) {
		PRINT_GL_ERROR_IF_ANY("render_debug_quad before bind CUBE_MAP");
		glBindTexture(GL_TEXTURE_CUBE_MAP, debug_quad.textureID);
	}
	else
		glBindTexture(GL_TEXTURE_2D, debug_quad.textureID);
	PRINT_GL_ERROR_IF_ANY("render_debug_quad before renderQuad");

	renderQuad();
	PRINT_GL_ERROR_IF_ANY("render_debug_quad after renderQuad");

	if (debug_quad.which_debug_shader > 3) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
	PRINT_GL_ERROR_IF_ANY("render_debug_quad END");
}
//we'll do a different matrix rotation for each face, using sideangle struct:
static struct {
	double angle;
	double x;
	double y;
	double z;
} sideangle[6] = {
//{ 90.0,0.0,1.0,0.0}, //+x
//{-90.0,0.0,1.0,0.0}, //-x
//{-90.0,1.0,0.0,0.0}, //+y  weird but works
//{ 90.0,1.0,0.0,0.0}, //-y  "
//{  0.0,0.0,1.0,0.0}, //+z (lhs)
//{180.0,0.0,1.0,0.0}, //-z

{ 90.0,0.0,1.0,0.0}, //+x
{-90.0,0.0,1.0,0.0}, //-x
{ 90.0,1.0,0.0,0.0}, //+y  weird but works
{-90.0,1.0,0.0,0.0}, //-y  "
{  0.0,0.0,1.0,0.0}, //+z (lhs)
{180.0,0.0,1.0,0.0}, //-z
};
#define RADIANS_PER_DEGREE (double)0.0174532925199432957692
#define DEGREES_PER_RADIAN (double)57.2957795130823208768

void saveImage_web3dit(struct textureTableIndexStruct* tti, char* fname);
void fw_gluPerspective_2(double xcenter, double fovy, double aspect, double zNear, double zFar);
void pushnset_viewport(float* vpFraction);
void popnset_viewport();
void render_bound_background();

// called from MainLoop.c
#include "../x3d_parser/Bindable.h"
void generate_shadowmap_cube(usehit uhit, int index) {
	//call from render_PointLight or render_TextureProjectorPoint once per frame (its a so-called 'dynamic cubemap', like GeneratedCubeMapTexture)
	// usehit - same as for lightTable / shared with lightTable which holds one usehit per scenegraph node visit
	// -- so DEF/USE of light node will have 2+ entries in lightTable and 2+ shadowmaps
	// -- currently fbo textures are persisted in Light node > LightRep > depth_buffer_stack
	// -- index is index in that stack, 
	//    and corresponds to scenegraph sequential visit number to light node on render pass
	//assumes depth fbo buffer / texture already exists / created elsewhere / persistent storage
	// set viewpoint pose at light node
	// render parent>children sub-scene to fbo texture via render_hier2(parent,VF_Geom | VF_Depth)
	// cubemap Stage II
	double savebackmat[16];

	int isize;
	double modelviewmatrix[16];
	textureTableIndexStruct_s* tti;
	float vp[4] = { 0.0f,1.0f,0.0f,1.0f }; //arbitrary
	struct X3D_PointLight* node;
	struct X3D_LightRep* lightrep;
	bindablestack* bstack;
	ttglobal tg = gglobal();
	bstack = getActiveBindableStacks(tg);

	//set_viewmatrix();
	//this function tampers with the normal background matrix, which has already been prepped for the mainloop rendering
	//so save it, and restore after gencubemap loop of 6
	memcpy(savebackmat, bstack->backgroundmatrix, 16 * sizeof(double));

	node = (struct X3D_Nodet*)uhit.node;
	float radius = 10.0f;
	if (node->_nodeType == NODE_PointLight)
		radius = ((struct X3D_PointLight*)node)->radius;
	else if (node->_nodeType == NODE_TextureProjectorPoint)
		radius = ((struct X3D_TextureProjectorPoint*)node)->farDistance;
	lightrep = (struct X3D_LightRep*)node->_intern;
	struct X3D_GeneratedCubeMapTexture* cubetex = (struct X3D_GeneratedCubeMapTexture*)vector_get(struct X3D_Node*, lightrep->depth_buffer_stack, uhit.ivalue);;
	memcpy(modelviewmatrix, uhit.mvm, 16 * sizeof(double));

	//compile_generatedcubemap - creates framebufferobject fbo
	tti = getTableIndex(cubetex->__textureTableIndex);
	PRINT_GL_ERROR_IF_ANY("generate_shadowMaps before cube 6 loop");

	pushnset_framebuffer(tti->ifbobuffer); //binds framebuffer. we push here, in case higher up we are already rendering the whole scene to an fbo
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	PRINT_GL_ERROR_IF_ANY("generate_shadowMaps after drawBuffers");

	pushnset_viewport(vp); //something to push so we can pop-and-set below, so any mainloop GL_BACK viewport is restored
	glViewport(0, 0, tti->x, tti->y); //viewport we want 
	FW_GL_MATRIX_MODE(GL_PROJECTION);
	FW_GL_PUSH_MATRIX();
	FW_GL_LOAD_IDENTITY();
	FW_GL_MATRIX_MODE(GL_MODELVIEW);
	FW_GL_PUSH_MATRIX();
	FW_GL_LOAD_IDENTITY();

	PRINT_GL_ERROR_IF_ANY("generate_shadowMaps before loop 2");
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		printf("generate_shadowmap_cube: framebuffer not complete\n");
	//create fbo or fbo tiles collection for generatedcubemap
	//method: we draw each face to a single framebuffer texture, 
	// and readpixels back into 6 PixelTexture tti->texdata, so its a bit like ImageCubeMap except 
	// we skip the steps of creating and reading back PixelTexture->image.p into texdata
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);

	glBindTexture(GL_TEXTURE_CUBE_MAP, tti->OpenGLTexture);

	for (int j = 0; j < 6; j++) {  //should be 6 cubetex->__subTextures.n
		textureTableIndexStruct_s* ttip;
		struct X3D_PixelTexture* nodep;
		GLuint pixelType;
		int bytesPerPixel;
		PRINT_GL_ERROR_IF_ANY("generate_cube shadow before glFramebufferTexture2D");
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, tti->OpenGLTexture, 0);
		// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glFramebufferTexture.xhtml
		PRINT_GL_ERROR_IF_ANY("generate_cube shadow after glFramebufferTexture2D");
		FW_GL_CLEAR(GL_DEPTH_BUFFER_BIT);
		PRINT_GL_ERROR_IF_ANY("generate_shadowMaps GL calls 1");

		//set viewpoint matrix for side
		{
			double world2light[16], world2lightview[16], mvm[16];
			double savePosOri[16], saveView[16], viewmatrix[16], mvmInverse[16], matrotside[16], world2lightviewside[16];
			double matproj[16];
			get_view_matrix(savePosOri, saveView);
			matmultiplyAFFINE(viewmatrix, saveView, savePosOri);
			//printmatrix2(viewmatrix, "vp view matrix");

			matcopy(mvm, uhit.mvm);
			matinverseAFFINE(mvmInverse, mvm);
			//printmatrix2(uhit.mvm, "uhit.mvm");

			matmultiplyAFFINE(world2light, viewmatrix, mvmInverse); // = world2light[16]
			//printmatrix2(world2light, "world2light = viewmatrix x mvmInverse");

			matmultiplyAFFINE(world2lightview, world2light, lightrep->matview);
			//printmatrix2(lightrep->matview, "lighrep.matview");
			matrotate(matrotside,RADIANS_PER_DEGREE * sideangle[j].angle, sideangle[j].x, sideangle[j].y, sideangle[j].z);
			matmultiplyAFFINE(world2lightviewside, world2lightview, matrotside);
			//printmatrix2(world2lightview, "world2lightview = lighrep.matview x world2light");

			//fw_glSetDoublev(GL_PROJECTION_MATRIX, lightrep->matproj); //identity
			projPerspective(90.0, 1.0, .1, radius, matproj);
			fw_glSetDoublev(GL_PROJECTION_MATRIX, matproj);
			//printmatrix2(lightrep->matproj, "matproj");

			fw_glSetDoublev(GL_MODELVIEW_MATRIX, world2lightviewside);
		}
		/*  4. Nodes (not the blended ones)*/
		PRINT_GL_ERROR_IF_ANY("generate_shadowMaps before render_hier");

		profile_start("hier_geom");
		render_hier2(rootNode(), VF_Geom | VF_Depth);
		profile_end("hier_geom");
		PRINT_GL_ERROR_IF_ANY("generate_shadowMaps after render_hier");

		PRINT_GL_ERROR_IF_ANY("generate_shadowMaps after GL calls");
		if (j == 5) {
			if (0) {
				//console printf of last rendered side depth map
				static char* texdata = NULL;
				if (!texdata) {
					texdata = malloc(tti->x * tti->y * sizeof(float));
					memset(texdata, 0, tti->x * tti->y * sizeof(float));
				}
				PRINT_GL_ERROR_IF_ANY("generate_shadowMaps cube in quad prep 0");
				glReadPixels(0, 0, tti->x, tti->y, GL_DEPTH_COMPONENT, GL_FLOAT, texdata);
				float* ftex = (float*)texdata;
				for (int ik = 0; ik < tti->y; ik += 128) {
					for (int jk = 0; jk < tti->x; jk += 128)
						printf("%4f ", ftex[ik * tti->x + jk]);
					printf("\n");
				}
			}
			if (0) {
				PRINT_GL_ERROR_IF_ANY("generate_shadowMaps cube in quad prep 2");
				set_debug_quad_near_farplane(.1f, radius);

				set_debug_quad(6, tti->OpenGLTexture);
				PRINT_GL_ERROR_IF_ANY("generate_shadowMaps cube in quad prep 3");

			}
		}
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	FW_GL_MATRIX_MODE(GL_PROJECTION);
	FW_GL_POP_MATRIX();
	FW_GL_MATRIX_MODE(GL_MODELVIEW);
	FW_GL_POP_MATRIX();

	popnset_viewport();
	popnset_framebuffer();

	memcpy(bstack->backgroundmatrix, savebackmat, 16 * sizeof(double));
}

void PRINT_GL_ERROR(GLenum _global_gl_err) {
	if (_global_gl_err == GL_INVALID_ENUM) {printf ("GL_INVALID_ENUM"); }
	else if (_global_gl_err == GL_INVALID_VALUE) {printf ("GL_INVALID_VALUE"); }
	else if (_global_gl_err == GL_INVALID_OPERATION) {printf ("GL_INVALID_OPERATION"); }
	else if (_global_gl_err == GL_STACK_OVERFLOW) {printf ("GL_STACK_UNDERFLOW"); }
	else if (_global_gl_err == GL_STACK_UNDERFLOW) {printf ("GL_STACK_UNDERFLOW"); }
	else if (_global_gl_err == GL_OUT_OF_MEMORY) {printf ("GL_OUT_OF_MEMORY"); }
	else if (_global_gl_err == GL_INVALID_FRAMEBUFFER_OPERATION) {printf ("GL_INVALID_FRAMEBUFFER_OPERATION"); }
	else if (_global_gl_err == GL_CONTEXT_LOST) {printf ("GL_CONTEXT_LOST"); }
	else if (_global_gl_err == GL_TABLE_TOO_LARGE) {printf ("GL_TABLE_TOO_LARGE"); }
	else printf ("unknown error %d ",_global_gl_err);
}
void generate_shadowmap_2D(usehit uhit, int index) {
	//call from render_xxxLight or render_TextureProjectorxxx once per frame
	// usehit - same as for lightTable/projectorTable / shared with lightTable which holds one usehit per scenegraph node visit
	// -- so DEF/USE of light node will have 2+ entries in lightTable and 2+ shadowmaps
	// -- currently fbo textures are persisted in Light node > LightRep > depth_buffer_stack
	// -- index is index in that stack, 
	//    and corresponds to scenegraph sequential visit number to light node on render pass
	//assumes depth fbo buffer / texture already exists / created elsewhere / persistent storage
	// set viewpoint pose at light node
	// render parent>children sub-scene to fbo texture via render_hier2(parent,VF_Geom | VF_Depth)
	double savebackmat[16];

	int isize;
	double modelviewmatrix[16];
	textureTableIndexStruct_s* tti;
	float vp[4] = { 0.0f,1.0f,0.0f,1.0f }; //arbitrary
	struct X3D_Node* node;
	struct X3D_LightRep* lightrep;
	bindablestack* bstack;
	ttglobal tg = gglobal();
	bstack = getActiveBindableStacks(tg);

	//this function tampers with the normal background matrix, which has already been prepped for the mainloop rendering
	//so save it, and restore after gencubemap loop of 6
	memcpy(savebackmat, bstack->backgroundmatrix, 16 * sizeof(double));

	node = (struct X3D_Node*)uhit.node;
	lightrep = (struct X3D_LightRep*)node->_intern; //X3D_LightRep and X3D_ProjectorRep are same order for the depth fields
	struct X3D_PixelTexture* tex = (struct X3D_PixelTexture*)vector_get(struct X3D_Node*, lightrep->depth_buffer_stack,uhit.ivalue);
	memcpy(modelviewmatrix, uhit.mvm, 16 * sizeof(double));

	//compile_generatedcubemap - creates framebufferobject fbo
	tti = getTableIndex(tex->__textureTableIndex);
	PRINT_GL_ERROR_IF_ANY("generate_shadowMaps_2D before");

	//isize = lightrep->size; //set in compile_
	pushnset_framebuffer(tti->ifbobuffer); //binds framebuffer. we push here, in case higher up we are already rendering the whole scene to an fbo
	pushnset_viewport(vp); //something to push so we can pop-and-set below, so any mainloop GL_BACK viewport is restored
	glViewport(0, 0, tti->x, tti->y); //viewport we want 
	FW_GL_MATRIX_MODE(GL_PROJECTION);
	FW_GL_PUSH_MATRIX();
	FW_GL_LOAD_IDENTITY();
	FW_GL_MATRIX_MODE(GL_MODELVIEW);
	FW_GL_PUSH_MATRIX();
	FW_GL_LOAD_IDENTITY();

	{
		textureTableIndexStruct_s* ttip;
		struct X3D_PixelTexture* nodep;
		GLuint pixelType;
		int bytesPerPixel;
		int j = 0;
		nodep = tex;
		ttip = tti;

		PRINT_GL_ERROR_IF_ANY("generate_shadowMaps before GL calls");

		FW_GL_CLEAR(GL_DEPTH_BUFFER_BIT);
		//GLenum _global_gl_err = glGetError(); 
		//while (_global_gl_err != GL_NONE) {
		//	PRINT_GL_ERROR(_global_gl_err);
		//	printf(" here: %s (%s:%d)\n", "generate_shadowMaps clear depth buffer", __FILE__, __LINE__);
		//	_global_gl_err = glGetError();
		//}
		PRINT_GL_ERROR_IF_ANY("generate_shadowMaps GL calls 1");

		//set viewpoint matrix 
		{
			double world2light[16], world2lightview[16], mvm[16];
			double savePosOri[16], saveView[16], viewmatrix[16], mvmInverse[16];
			get_view_matrix(savePosOri, saveView);
			matmultiplyAFFINE(viewmatrix, saveView, savePosOri);
			//printmatrix2(viewmatrix, "vp view matrix");

			if (uhit.node->_nodeType == NODE_DirectionalLight)
				matmultiplyAFFINE(mvm, uhit.extra, uhit.mvm);
			else
				matcopy(mvm, uhit.mvm);
			matinverseAFFINE(mvmInverse, mvm);
			//printmatrix2(uhit.mvm, "uhit.mvm");

			matmultiplyAFFINE(world2light, viewmatrix, mvmInverse); // = world2light[16]
			//printmatrix2(world2light, "world2light = viewmatrix x mvmInverse");

			matmultiplyAFFINE(world2lightview, world2light, lightrep->matview);
			//printmatrix2(lightrep->matview, "lighrep.matview");

			//printmatrix2(world2lightview, "world2lightview = lighrep.matview x world2light");

			fw_glSetDoublev(GL_PROJECTION_MATRIX, lightrep->matproj);
			//printmatrix2(lightrep->matproj, "matproj");

			fw_glSetDoublev(GL_MODELVIEW_MATRIX, world2lightview);
		}
		/*  4. Nodes (not the blended ones)*/
		PRINT_GL_ERROR_IF_ANY("generate_shadowMaps before render_hier");

		profile_start("hier_geom");
		struct X3D_Node* root = uhit.userdata ? uhit.userdata : rootNode();
		render_hier2(root, VF_Geom | VF_Depth);
		profile_end("hier_geom");
		PRINT_GL_ERROR_IF_ANY("generate_shadowMaps after render_hier");

	}
	//set index to 0 to debug (or 1 or which of the light visit shadow maps you want to see at end of frame)
	if (index == -1) set_debug_quad(2, tti->OpenGLTexture); // lightrep->idepthtexture);
	FW_GL_MATRIX_MODE(GL_PROJECTION);
	FW_GL_POP_MATRIX();
	FW_GL_MATRIX_MODE(GL_MODELVIEW);
	FW_GL_POP_MATRIX();

	popnset_viewport();
	popnset_framebuffer();
	memcpy(bstack->backgroundmatrix, savebackmat, 16 * sizeof(double));
}





void transformPositionToEye0(double *modelMatrix, float* pos)
{
	float aux[4];
	 // assumes pos[3] = 0.0; only use first 3 of these numbers
	transformf(aux, pos, modelMatrix);
	veccopy3f(pos, aux);
}

void transformDirectionToEye0(double *modelMatrix, float* dir)
{
	float* a;
	double *b;
	float aux[4];
	b = modelMatrix;
	a = dir;
	//should this be an inverse transpose? Is it? I have no idea.
	aux[0] = (float)(b[0] * a[0] + b[4] * a[1] + b[8] * a[2]);
	aux[1] = (float)(b[1] * a[0] + b[5] * a[1] + b[9] * a[2]);
	aux[2] = (float)(b[2] * a[0] + b[6] * a[1] + b[10] * a[2]);
	veccopy3f(dir, aux);

}


GLint tunit(int index);
void sendLightInfo2(s_shader_capabilities_t* me) {
	// in case we are trying to render a node that has just been killed...
	if (me == NULL) return;
	/*
	if (light_buffering()) {
		sendLightInfo3(me);
		return;
	}
	*/
	PRINT_GL_ERROR_IF_ANY("BEGIN sendLightInfo2");
	int lightcount = lightTable_count();

	for (int j = 0; j < lightcount; j++) {
		usehit* uhit = lightTable_item(j);
		struct X3D_Node* node = uhit->node;
		struct X3D_PointLight* plight = X3D_POINTLIGHT(node);
		struct X3D_SpotLight* slight = X3D_SPOTLIGHT(node);
		struct X3D_DirectionalLight* dlight = X3D_DIRECTIONALLIGHT(node);
		struct X3D_LightRep* lightrep = (struct X3D_LightRep*)node->_intern;
		int lightType = 0;
		//0 - pointlight
		//1 - spotlight
		//2 - directionlight
		switch (plight->_nodeType) {
			case NODE_PointLight: lightType = 0; break;
			case NODE_SpotLight: lightType = 1; break;
			case NODE_DirectionalLight: lightType = 2; break;
			default: break;
		}
		//save a bit of bandwidth by not sending unused parameters for a light type
		if (lightType < 2) { //not directional
			GLUNIFORM3FV(me->lightAtten[j], 1, plight->attenuation.c); //.light_Attenuations);
			GLUNIFORM1F(me->lightRadius[j], plight->radius);
			float location[4];
			veccopy3f(location, plight->location.c);
			transformPositionToEye0(uhit->mvm, location);
			GLUNIFORM3FV(me->lightLocation[j], 1, location);
		}
		if (lightType == 1) { //spot
			GLUNIFORM1F(me->lightSpotCutoffAngle[j], slight->cutOffAngle);
			GLUNIFORM1F(me->lightSpotBeamWidth[j], slight->beamWidth);
			float direction[4];
			veccopy3f(direction, slight->direction.c);
			transformDirectionToEye0(uhit->mvm, direction);
			GLUNIFORM3FV(me->lightDirection[j], 1, direction);
		}
		if (lightType == 2) { //directional
			float direction[4];
			veccopy3f(direction, dlight->direction.c);
			transformDirectionToEye0(uhit->mvm, direction);
			GLUNIFORM3FV(me->lightDirection[j], 1, direction);
		}
		GLUNIFORM1F(me->lightAmbientIntensity[j], plight->ambientIntensity);
		GLUNIFORM3FV(me->lightColor[j], 1, plight->color.c);
		GLUNIFORM1F(me->lightIntensity[j], plight->intensity);
		GLUNIFORM1I(me->lightType[j], lightType);
		GLUNIFORM1I(me->lightshadows[j], plight->shadows);
		GLUNIFORM1F(me->lightshadowIntensity[j], plight->shadowIntensity);
		if (plight->shadows) {
			//lookup a textureUnit[index] index to use on this pass
			//process the uhit->mvm matrix for shadows
			struct X3D_Node* texnode = (struct X3D_Node*)vector_get(struct X3D_Node*, lightrep->depth_buffer_stack, uhit->ivalue);
			int itexunit, iunit;
			textureTableIndexStruct_s* tti;
			if (texnode->_nodeType == NODE_GeneratedCubeMapTexture && !gcm_method()) {
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
				PRINT_GL_ERROR_IF_ANY("sendLightInfo after bind_or_share");
				glUniform1i(me->textureUnitCube[iunit], itexunit); // iunit);
			}
			else {
				struct X3D_PixelTexture* tex = (struct X3D_PixelTexture*)texnode;
				tti = getTableIndex(tex->__textureTableIndex);
				PRINT_GL_ERROR_IF_ANY("sendLightInfo before bind_or_share");
				itexunit = share_or_next_material_sampler_index_2D(tti->OpenGLTexture); // returns i as in GL_TEXTUREi, next available
				iunit = tunit2D(itexunit); //returns index into shader sampler2D textureUnit[iunit]
				PRINT_GL_ERROR_IF_ANY("sendLightInfo after bind_or_share");
				glUniform1i(me->textureUnit[iunit], itexunit);
			}
			GLUNIFORM1I(me->lightdepthmap[j], iunit);
			float w2l[16];
			{
				//following textureProjector
				double modelviewinv[16], eye2projector[16], matfull[16], mvm[16];
				if (uhit->node->_nodeType == NODE_DirectionalLight)
					matmultiplyAFFINE(mvm, uhit->extra, uhit->mvm);
				else
					matcopy(mvm, uhit->mvm);

				matinverse(modelviewinv, mvm);
				matmultiplyAFFINE(eye2projector, modelviewinv, lightrep->matview);
				matmultiplyFULL(matfull, eye2projector, lightrep->matproj);
				double2float(w2l, matfull, 16);
			}
			//printf("w2l\n");
			//for (int ii = 0; ii < 4; ii++){
			//	for (int jj = 0; jj < 4; jj++) printf("%f ", w2l[ii * 4 + jj]);
			//	printf("\n");
			//}
			GLUNIFORMMATRIX4FV(me->lightMat[j], 1, GL_FALSE, w2l);
		}
	}
	GLUNIFORM1I(me->lightcount, lightcount);

	PRINT_GL_ERROR_IF_ANY("END sendLightInfo");
}

