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

/*
//SHADOW TABLE
void shadowTable_clear() {
	//called once per frame, before the search for global=true projectors
	//will clear any global=true shadow lights from last frame
	ppComponent_Lighting p = (ppComponent_Lighting)gglobal()->Component_Lighting.prv;
	clearStack(p->genshadow_stack);
}
void shadowTable_push(usehit ptuple) {
	//called when we find a global=true, on=true shadow light, and
	//called in sib_prep for a global=false, on=false shadow light
	ppComponent_Lighting p = (ppComponent_Lighting)gglobal()->Component_Lighting.prv;
	//we need a deep copy because the light node can't hold it
	// because it can be DEF/USED with different transform each use
	stack_push(usehit, p->genshadow_stack, ptuple);

}
void shadowTable_pop() {
	//called in sib_fin for a global=false, on=true shadow light
	ppComponent_Lighting p = (ppComponent_Lighting)gglobal()->Component_Lighting.prv;
	if (p->genshadow_stack->n < 1)
		printf("ouch from shadowTable_pop()\n");
	stack_pop(usehit, p->genshadow_stack);

}
int shadowTable_count() {
	ppComponent_Lighting p = (ppComponent_Lighting)gglobal()->Component_Lighting.prv;
	return p->genshadow_stack->n;
}
usehit* shadowTable_item(int i) {
	ppComponent_Lighting p = (ppComponent_Lighting)gglobal()->Component_Lighting.prv;
	return vector_get_ptr(usehit, p->genshadow_stack, i);
}
*/
// a specialization of InternalRep - see PolyRep.h
struct X3D_LightRep {
	int itype; //=5, 0 PointRep 1 LineRep 2 PolyRep 3 MeshRep 4 TextureRep 5 LightRep
	//light section
	void* lightbuf;
	int ilightbuf; //opengl uniform buffer index
	//depth section
	struct X3D_Node* depthTexture;
	Stack* depth_buffer_stack;
	int size;
	//int idepthtexture;
	double matproj[16];
	double matview[16];
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
/*
enum {
	LIGHT_DIRECTION = 1,
	LIGHT_POSITION = 2,
	LIGHT_COLOR = 3,
	LIGHT_INTENSITY = 4,
	LIGHT_AMBIENT = 5,
};
*/
//void compile_shadowMap(struct X3D_Node* node);
//void render_shadowMap(struct X3D_Node* node);
/*
void render_DirectionalLight (struct X3D_DirectionalLight *node) {
	// if we are doing global lighting, is this one for us?
	RETURN_IF_LIGHT_STATE_NOT_US
    COMPILE_IF_REQUIRED;

	if(node->on) {
		//both global on VF_GlobalLight on children->render, and local on prep_sibAffectors come in here
		usehit uhit;
		uhit.node = X3D_NODE(node);
		fw_glGetDoublev(GL_MODELVIEW_MATRIX, uhit.mvm);
		lightTable_push(uhit);
	}
}
*/
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
				//matinverseAFFINE(matviewInv, lightrep->matview);
				//matmultiplyAFFINE(matev, matviewInv, mate);
				//printmatrix2(matev, "invcenter and size in lightview space");
				//matinverseAFFINE(matevinv, matev);

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
		//fw_glGetDoublev(GL_MODELVIEW_MATRIX, uhit.mvm);
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


void compile_PointLight_shadowMap(struct X3D_PointLight* node);
void render_PointLight_shadowMap(struct X3D_PointLight* node);

void compile_PointLight (struct X3D_PointLight *node) {
    int i;

	if (node->global && node->shadows) compile_PointLight_shadowMap(node);


    //ConsoleMessage("compile_PointLight, loc %f %f %f %f",node->_loc.c[0],node->_loc.c[1],node->_loc.c[2],node->_loc.c[3]);

    
    MARK_NODE_COMPILED;
    
}


void render_PointLight (struct X3D_PointLight *node) {

	/* if we are doing global lighting, is this one for us? */
	RETURN_IF_LIGHT_STATE_NOT_US
	/*
		if (renderstate()->render_light== VF_globalLight) { 
			if (!node->global) {
				printf("x local point, we want global %u\n", node);
				return;
			}
			 printf ("* global point,  we want global %u\n", node);
		} else {
			if (node->global){ 
				printf("x global point, we want local %u\n", node);
			  return; 
			}else {
			   printf ("* local point, we want local %u\n", node);
			}
		}
	*/
    COMPILE_IF_REQUIRED;

	if(node->on) {
		//both global on VF_GlobalLight on children->render, and local on prep_sibAffectors come in here
		usehit uhit;
		uhit.node = X3D_NODE(node);
		fw_glGetDoublev(GL_MODELVIEW_MATRIX, uhit.mvm);
		lightTable_push(uhit);
	}
}

/* pointLights are done before the rendering of geometry */
void prep_PointLight (struct X3D_PointLight *node) {

	if (!renderstate()->render_light) return;
	/* this will be a global light here... */
	render_PointLight(node);
}

//void mesa_Frustum(double left, double right, double bottom, double top, double nearZ, double farZ, double* m);
//double * perspective_projection_matrix(double fovy_radians, double aspect, double zNear, double zFar, double* matrix) {
//	double xmin, xmax, ymin, ymax;
//
//	ymax = zNear * tan(fovy_radians);
//	ymin = -ymax;
//	xmin = ymin * aspect;
//	xmax = ymax * aspect;
//
//	mesa_Frustum(xmin, xmax, ymin, ymax, zNear, zFar, matrix);
//	return matrix;
//}



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
void compile_SpotLight (struct X3D_SpotLight *node) {
	if (node->shadows) {
		//compile_shadowMap(X3D_NODE(node)); //prepares fbo buffer and texture
		//prepare local view matrix (from node.location, node.direction which aren't included in modelview matrix)
		// and projection matrix, both of which are stable / same between DEF and USE instances of a spotlight
		node->_intern = set_LightRep(node->_intern);
		struct X3D_LightRep* lightrep = (struct X3D_LightRep*)node->_intern;
		//glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		//perspective_projection_matrix(node->cutOffAngle*2.0, 1.0, .1, 10000.0, lightrep->matproj);
		projPerspective(node->cutOffAngle * (180.0 / PI) * 2.0, 1.0, .5, (double)node->radius, lightrep->matproj);
		set_debug_quad_near_farplane(.5f, node->radius);

		//lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		//for up vector in theory we need a few cross products to ensure its at least orthogonal to direction
		//- up is somewhat arbitrary -spotlight is symmetrical about direction vector-
		//  but must be consistent between depth texture rendering and shader sampling
		float up[3], center[3];
		vecset3f(up, 0.0f, 1.0f, 0.0f);
		vecadd3f(center, node->location.c, node->direction.c);
		matrix_lookAtfd(node->location.c, center, up, lightrep->matview);
	}

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

    MARK_NODE_COMPILED;
}
//void shadow_Light(struct X3D_Node* parent, struct X3D_Node* node) {
//	usehit uhit;
//	COMPILE_IF_REQUIRED;
//
//	uhit.node = X3D_NODE(node);
//	uhit.userdata = parent; //will render_hier(parent,..) in generate_globalShadowMaps()
//	fw_glGetDoublev(GL_MODELVIEW_MATRIX, uhit.mvm);
//	//lightTable_push(uhit);
//	if (node->_nodeType == NODE_SpotLight) {
//		struct X3D_SpotLight* light = X3D_SPOTLIGHT(node);
//		if (light->shadows) {
//			//shadowTable_push(uhit);
//			generate_shadowmap_2D(uhit, 0);
//			//render_shadowMap(X3D_NODE(node));
//		}
//	}
//}

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
				render_PointLight((struct X3D_PointLight*)sibAffector);
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
#define SHADOWMAPS 1
#ifdef SHADOWMAPS
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
	PRINT_GL_ERROR_IF_ANY("compile_shadowMap START");
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

			//glGenRenderbuffers(1, &tti->idepthbuffer);
			//glBindRenderbuffer(GL_RENDERBUFFER, tti->idepthbuffer);
			//glRenderbufferStorage(GL_RENDERBUFFER, FW_GL_DEPTH_COMPONENT, isize, isize);
			//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, tti->idepthbuffer);

			glGenTextures(1, &tti->OpenGLTexture);
			glBindTexture(GL_TEXTURE_2D, tti->OpenGLTexture);
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, isize, isize, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

			//glBindFramebuffer(GL_FRAMEBUFFER, tti->ifbobuffer); already bound with pushnset_framebuffer
			//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tti->OpenGLTexture, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tti->OpenGLTexture, 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
			popnset_framebuffer(); //tti->ifbobuffer);
		}

	}
	PRINT_GL_ERROR_IF_ANY("compile_PointLight_shadowMaps END");


	return X3D_NODE(tex);

}
int make_or_get_depth_buffer(int index, struct X3D_Node* node) {
	node->_intern = set_LightRep(node->_intern);
	struct X3D_LightRep* lightrep = (struct X3D_LightRep* )node->_intern;
	if (!lightrep->depth_buffer_stack) {
		lightrep->depth_buffer_stack = newStack(struct X3D_Node*);
	}
	if (index > -1 && index < vectorSize(lightrep->depth_buffer_stack)) return index;
	struct X3D_Node* depth_buffer_texture = make_depth_buffer(lightrep->size, lightrep->size);
	stack_push(struct X3D_Node*, lightrep->depth_buffer_stack, X3D_NODE(depth_buffer_texture));
	return vectorSize(lightrep->depth_buffer_stack) - 1;
}
//void compile_shadowMap(struct X3D_Node* node) {}
//void render_shadowMap(struct X3D_Node* node) {}
/*
void compile_shadowMap(struct X3D_Node* node) {
	node->_intern = set_LightRep(node->_intern);
	struct X3D_LightRep* lightrep = (struct X3D_LightRep*)node->_intern;
	//need a generic texture to hold results
	if (!lightrep->depthTexture)
		lightrep->depthTexture = createNewX3DNode(NODE_PixelTexture);
	struct X3D_PixelTexture* tex = (struct X3D_PixelTexture*)lightrep->depthTexture;
	PRINT_GL_ERROR_IF_ANY("compile_shadowMap START");
	//if (tex->__subTextures.n == 0) 
	{

		int i;
		struct textureTableIndexStruct* tti;

		tti = getTableIndex(tex->__textureTableIndex);
		tti->status = TEX_LOADED; // TEX_NEEDSBINDING; //I found I didn't need - yet
		tti->idepthbuffer = 1;
		tti->x = tti->y = lightrep->size;
		//tti->z = 6;
//		loadTextureNode(X3D_NODE(tex), NULL);
		if (tti->ifbobuffer == 0 && haveFrameBufferObject()) {
			int j, isize;
			isize = lightrep->size; //node->size is initializeOnly, we will ignore any change during run
			tti->x = isize; //by storing and retrieving initial size from here
			tti->y = isize;
			// https://www.opengl.org/wiki/Framebuffer_Object

			glGenFramebuffers(1, &tti->ifbobuffer);
			pushnset_framebuffer(tti->ifbobuffer); //binds framebuffer. we push here, in case higher up we are already rendering the whole scene to an fbo

			//glGenRenderbuffers(1, &tti->idepthbuffer);
			//glBindRenderbuffer(GL_RENDERBUFFER, tti->idepthbuffer);
			//glRenderbufferStorage(GL_RENDERBUFFER, FW_GL_DEPTH_COMPONENT, isize, isize);
			//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, tti->idepthbuffer);

			glGenTextures(1, &tti->OpenGLTexture);
			glBindTexture(GL_TEXTURE_2D, tti->OpenGLTexture);
			lightrep->idepthtexture = tti->OpenGLTexture;
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, isize, isize, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, isize, isize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

			//glBindFramebuffer(GL_FRAMEBUFFER, tti->ifbobuffer); already bound with pushnset_framebuffer
			//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tti->OpenGLTexture, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tti->OpenGLTexture, 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
			popnset_framebuffer(); //tti->ifbobuffer);
		}

	}
	PRINT_GL_ERROR_IF_ANY("compile_PointLight_shadowMaps END");


	MARK_NODE_COMPILED

}

void render_shadowMap(struct X3D_Node* node) {
	int count, iface;

	COMPILE_IF_REQUIRED
		PRINT_GL_ERROR_IF_ANY("render_PointLight_shadowMaps START");

	//if (!strcmp(node->update->strptr, "ALWAYS") || !strcmp(node->update->strptr, "NEXT_FRAME_ONLY")) {
	{
		ttrenderstate rs;
		rs = renderstate();
		//if (rs->render_geom && !rs->render_depth) 
		{
			//add (node,modelviewmatrix) for next frame
			//programmer: please clear the genshadow stack once per frame
			int i, isAdded;
			usehit uhit;
			ppComponent_Lighting p = (ppComponent_Lighting)gglobal()->Component_Lighting.prv;
			//check if already added, only add once for simplification
			isAdded = FALSE;
			for (i = 0; i < vectorSize(p->genshadow_stack); i++) {
				uhit = vector_get(usehit, p->genshadow_stack, i);
				if (uhit.node == X3D_NODE(node)) {
					isAdded = TRUE;
					break;
				}
			}
			if (!isAdded) {
				double modelviewMatrix[16], mvmInverse[16];
				double worldmatrix[16], viewmatrix[16], saveView[16], savePosOri[16]; //bothinverse[16], 
				usehit uhit;
				//GL_GET_MODELVIEWMATRIX
				FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelviewMatrix);
				get_view_matrix(savePosOri, saveView);
				matmultiplyAFFINE(viewmatrix, saveView, savePosOri);
				//matinverseAFFINE(bothinverse,viewmatrix);
				matinverseAFFINE(mvmInverse, modelviewMatrix);

				//matmultiplyAFFINE(worldmatrix,bothinverse,modelviewMatrix);
				//matmultiplyAFFINE(worldmatrix,modelviewMatrix,bothinverse);

				matmultiplyAFFINE(worldmatrix, viewmatrix, mvmInverse);

				//strip viewmatrix - will happen when we invert one of the USEUSE pair, and multiply
				uhit.node = X3D_NODE(node);
				//memcpy(uhit.mvm,modelviewMatrix,16*sizeof(double)); //deep copy
				memcpy(uhit.mvm, worldmatrix, 16 * sizeof(double)); //deep copy
				vector_pushBack(usehit, p->genshadow_stack, uhit);  //fat elements do another deep copy
				//if (!strcmp(node->update->strptr, "NEXT_FRAME_ONLY")) {
				//	//set back to NONE
				//	freeASCIIString(node->update);
				//	node->update = newASCIIString("NONE");
				//	//not sure why, but I don't seem to need to mark event
				//	//MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_PointLight, update));
				//	//printf("MARK_EVENT\n");
				//}
			}

		}
	}
	//render what we have now for debugging?
	if(0) {
		struct X3D_LightRep* lightrep = (struct X3D_LightRep*)node->_intern;
		struct X3D_PixelTexture* tex = (struct X3D_PixelTexture*)lightrep->depthTexture;
		render_node(X3D_NODE(tex));
	}
	// Finished rendering CubeMap, set it back for normal textures 
	PRINT_GL_ERROR_IF_ANY("render_PointLight_shadowMaps END");

}

*/


// called from the scene traversal, linked in GeneratedCode.c
void compile_PointLight_shadowMap(struct X3D_PointLight* node) {
	node->_intern = set_LightRep(node->_intern);
	struct X3D_LightRep* lightrep = (struct X3D_LightRep*)node->_intern;
	//need a generic cubemap texture to hold results
	if (!lightrep->depthTexture)
		lightrep->depthTexture = createNewX3DNode(NODE_GeneratedCubeMapTexture); 
	struct X3D_GeneratedCubeMapTexture* cubetex = (struct X3D_GeneratedCubeMapTexture*)lightrep->depthTexture;
	PRINT_GL_ERROR_IF_ANY("compile_PointLight_shadowMaps START");
	if (cubetex->__subTextures.n == 0) {

		int i;
		struct textureTableIndexStruct* tti;

		FREE_IF_NZ(cubetex->__subTextures.p); /* should be NULL, checking */
		cubetex->__subTextures.p = MALLOC(struct X3D_Node**, 6 * sizeof(struct X3D_PixelTexture*));
		for (i = 0; i < 6; i++) {
			struct X3D_PixelTexture* pt;
			pt = (struct X3D_PixelTexture*)createNewX3DNode(NODE_PixelTexture);
			cubetex->__subTextures.p[i] = X3D_NODE(pt);
			if (node->_executionContext)
				add_node_to_broto_context(X3D_PROTO(node->_executionContext), X3D_NODE(cubetex->__subTextures.p[i]));
			//tti = getTableIndex(pt->__textureTableIndex);
			//tti->status = TEX_NEEDSBINDING; //I found I didn't need - yet
			//tti->z = 6;

		}
		cubetex->__subTextures.n = 6;
		tti = getTableIndex(cubetex->__textureTableIndex);
		tti->status = TEX_NEEDSBINDING; //I found I didn't need - yet
		tti->x = tti->y = lightrep->size;
		//tti->z = 6;
		loadTextureNode(X3D_NODE(cubetex), NULL);
		if (tti->ifbobuffer == 0 && haveFrameBufferObject()) {
			int j, isize;
			isize = lightrep->size; //node->size is initializeOnly, we will ignore any change during run
			tti->x = isize; //by storing and retrieving initial size from here
			// https://www.opengl.org/wiki/Framebuffer_Object
			glGenFramebuffers(1, &tti->ifbobuffer);
			pushnset_framebuffer(tti->ifbobuffer); //binds framebuffer. we push here, in case higher up we are already rendering the whole scene to an fbo

			glGenRenderbuffers(1, &tti->idepthbuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, tti->idepthbuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, FW_GL_DEPTH_COMPONENT, isize, isize);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, tti->idepthbuffer);

			for (j = 0; j < cubetex->__subTextures.n; j++) {  //should be 6
				//textureTableIndexStruct_s* ttip;
				//struct X3D_PixelTexture * nodep;
				//nodep = (struct X3D_PixelTexture *)node->__subTextures.p[j];
				//ttip = getTableIndex(nodep->__textureTableIndex);
				//glGenTextures(1,&ttip->OpenGLTexture);
				//glBindTexture(GL_TEXTURE_2D, ttip->OpenGLTexture);

				//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, isize, isize, 0, GL_RGBA , GL_UNSIGNED_BYTE, 0);
				//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+j, GL_TEXTURE_2D, ttip->OpenGLTexture, 0);
			}
			glGenTextures(1, &tti->OpenGLTexture);
			glBindTexture(GL_TEXTURE_2D, tti->OpenGLTexture);
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, isize, isize, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, isize, isize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			//glBindFramebuffer(GL_FRAMEBUFFER, tti->ifbobuffer); already bound with pushnset_framebuffer
			//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tti->OpenGLTexture, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tti->OpenGLTexture, 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
			popnset_framebuffer(); //tti->ifbobuffer);
		}

	}
	PRINT_GL_ERROR_IF_ANY("compile_PointLight_shadowMaps END");

	/* tell the whole system to re-create the data for these sub-children */
	//node->__regenSubTextures = TRUE;

	MARK_NODE_COMPILED
	
}


// called from the scene traversal render_PointLight
void render_PointLight_shadowMap(struct X3D_PointLight* node) {
	int count, iface;

	COMPILE_IF_REQUIRED
	PRINT_GL_ERROR_IF_ANY("render_PointLight_shadowMaps START");

	//if (!strcmp(node->update->strptr, "ALWAYS") || !strcmp(node->update->strptr, "NEXT_FRAME_ONLY")) {
	{
		ttrenderstate rs;
		rs = renderstate();
		//if (rs->render_geom && !rs->render_depth) 
		{
			//add (node,modelviewmatrix) for next frame
			//programmer: please clear the genshadow stack once per frame
			int i, isAdded;
			usehit uhit;
			ppComponent_Lighting p = (ppComponent_Lighting)gglobal()->Component_Lighting.prv;
			//check if already added, only add once for simplification
			isAdded = FALSE;
			for (i = 0; i < vectorSize(p->genshadow_stack); i++) {
				uhit = vector_get(usehit, p->genshadow_stack, i);
				if (uhit.node == X3D_NODE(node)) {
					isAdded = TRUE;
					break;
				}
			}
			if (!isAdded) {
				double modelviewMatrix[16], mvmInverse[16];
				double worldmatrix[16], viewmatrix[16], saveView[16], savePosOri[16]; //bothinverse[16], 
				usehit uhit;
				//GL_GET_MODELVIEWMATRIX
				FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelviewMatrix);
				get_view_matrix(savePosOri, saveView);
				matmultiplyAFFINE(viewmatrix, saveView, savePosOri);
				//matinverseAFFINE(bothinverse,viewmatrix);
				matinverseAFFINE(mvmInverse, modelviewMatrix);

				//matmultiplyAFFINE(worldmatrix,bothinverse,modelviewMatrix);
				//matmultiplyAFFINE(worldmatrix,modelviewMatrix,bothinverse);

				matmultiplyAFFINE(worldmatrix, viewmatrix, mvmInverse);

				//strip viewmatrix - will happen when we invert one of the USEUSE pair, and multiply
				uhit.node = X3D_NODE(node);
				//memcpy(uhit.mvm,modelviewMatrix,16*sizeof(double)); //deep copy
				memcpy(uhit.mvm, worldmatrix, 16 * sizeof(double)); //deep copy
				vector_pushBack(usehit, p->genshadow_stack, uhit);  //fat elements do another deep copy
				//if (!strcmp(node->update->strptr, "NEXT_FRAME_ONLY")) {
				//	//set back to NONE
				//	freeASCIIString(node->update);
				//	node->update = newASCIIString("NONE");
				//	//not sure why, but I don't seem to need to mark event
				//	//MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_PointLight, update));
				//	//printf("MARK_EVENT\n");
				//}
			}

		}
	}
	//render what we have now for debugging?
	{
		struct X3D_LightRep* lightrep = (struct X3D_LightRep*)node->_intern;
		struct X3D_GeneratedCubeMapTexture* cubetex = (struct X3D_GeneratedCubeMapTexture*)lightrep->depthTexture;
		/* we have the 6 faces from the image, just go through and render them as a cube */
		if (cubetex->__subTextures.n == 0) return; /* not generated yet  */

		for (count = 0; count < 6; count++) {

			/* set up the appearanceProperties to indicate a CubeMap */
			getAppearanceProperties()->cubeFace = GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT + count;

			/* go through these, back, front, top, bottom, right left */
			iface = count;
			render_node(cubetex->__subTextures.p[iface]);
		}
	}
	/* Finished rendering CubeMap, set it back for normal textures */
	getAppearanceProperties()->cubeFace = 0;
	PRINT_GL_ERROR_IF_ANY("render_PointLight_shadowMaps END");

}


//we'll do a different matrix rotation for each face, using sideangle struct:
static struct {
	double angle;
	double x;
	double y;
	double z;
} sideangle[6] = {
{ 90.0,0.0,1.0,0.0}, //+x
{-90.0,0.0,1.0,0.0}, //-x
{-90.0,1.0,0.0,0.0}, //+y  weird but works
{ 90.0,1.0,0.0,0.0}, //-y  "
{  0.0,0.0,1.0,0.0}, //+z (lhs)
{180.0,0.0,1.0,0.0}, //-z
};

void saveImage_web3dit(struct textureTableIndexStruct* tti, char* fname);
void fw_gluPerspective_2(double xcenter, double fovy, double aspect, double zNear, double zFar);
void pushnset_viewport(float* vpFraction);
void popnset_viewport();
void render_bound_background();

// called from MainLoop.c
#include "../x3d_parser/Bindable.h"
void generate_shadowmap_cube(usehit uhit) {
	//call from mainloop once per frame:
	//foreach cubemaptexture location in cubgen list
	//  foreach 6 sides
	//    set viewpoint pose
	//    render scene to fbo
	//  convert fbo to regular cubemap texture
	//clear cubegen list
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

	node = (struct X3D_PointLight*)uhit.node;
	lightrep = (struct X3D_LightRep*)node->_intern;
	struct X3D_GeneratedCubeMapTexture* cubetex = (struct X3D_GeneratedCubeMapTexture*)lightrep->depthTexture;
	memcpy(modelviewmatrix, uhit.mvm, 16 * sizeof(double));

	//compile_generatedcubemap - creates framebufferobject fbo
	tti = getTableIndex(cubetex->__textureTableIndex);
	PRINT_GL_ERROR_IF_ANY("generate_shadowMaps before cube 6 loop");

	isize = tti->x; //set in compile_
	pushnset_framebuffer(tti->ifbobuffer); //binds framebuffer. we push here, in case higher up we are already rendering the whole scene to an fbo
	//GLuint attachments [1] = {GL_COLOR_ATTACHMENT0};
	//glDrawBuffers(1,attachments); //'draw' is implied in GL_RENDERBUFFER above
	//glReadBuffer(GL_COLOR_ATTACHMENT0); //'read' is implied in GL_RENDERBUFFER
	pushnset_viewport(vp); //something to push so we can pop-and-set below, so any mainloop GL_BACK viewport is restored
	glViewport(0, 0, isize, isize); //viewport we want 

	//create fbo or fbo tiles collection for generatedcubemap
	//method: we draw each face to a single framebuffer texture, 
	// and readpixels back into 6 PixelTexture tti->texdata, so its a bit like ImageCubeMap except 
	// we skip the steps of creating and reading back PixelTexture->image.p into texdata
	for (int j = 0; j < cubetex->__subTextures.n; j++) {  //should be 6
		textureTableIndexStruct_s* ttip;
		struct X3D_PixelTexture* nodep;
		GLuint pixelType;
		int bytesPerPixel;

		nodep = (struct X3D_PixelTexture*)cubetex->__subTextures.p[j];
		ttip = getTableIndex(nodep->__textureTableIndex);
		//we won't directly generate cubemap textures here, but looks interesting as possible 
		//  shotcut to skip readpixels below
		// glBindTexture(GL_TEXTURE_2D, ttip->OpenGLTexture);
		// glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ttip->OpenGLTexture, 0);
		PRINT_GL_ERROR_IF_ANY("generate_shadowMaps before GL calls");

		//glClearColor(1.0f, 0.0f, 0.0f, 1.0f); //red, for diagnostics during debugging
		//FW_GL_CLEAR(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		FW_GL_CLEAR(GL_DEPTH_BUFFER_BIT);
		//glClear(GL_DEPTH_BUFFER_BIT);
		PRINT_GL_ERROR_IF_ANY("generate_shadowMaps GL calls 1");

		//set viewpoint matrix for side
		//setup_projection(); 
		FW_GL_MATRIX_MODE(GL_PROJECTION);
		FW_GL_LOAD_IDENTITY();
		//fw_gluPerspective(90.0, 1.0, .1,10000.0);
		fw_gluPerspective_2(0.0, 90.0, 1.0, .1, 10000.0);
		PRINT_GL_ERROR_IF_ANY("generate_shadowMaps GL calls 3");

		FW_GL_MATRIX_MODE(GL_MODELVIEW);
		FW_GL_LOAD_IDENTITY();
		PRINT_GL_ERROR_IF_ANY("generate_shadowMaps GL calls 5");

		fw_glSetDoublev(GL_MODELVIEW_MATRIX, modelviewmatrix);
		fw_glRotated(sideangle[j].angle, sideangle[j].x, sideangle[j].y, sideangle[j].z);
		fw_glGetDoublev(GL_MODELVIEW_MATRIX, bstack->viewmatrix);

		//clearLightTable();//turns all lights off- will turn them on for VF_globalLight and scope-wise for non-global in VF_geom

		//render_bound_background();

		///*  turn light #0 off only if it is not a headlight.*/
		//if (!fwl_get_headlight()) {
		//	setLightState(HEADLIGHT_LIGHT, FALSE);
		//	setLightType(HEADLIGHT_LIGHT, 2); // DirectionalLight
		//}

		///*  Other lights*/
		//PRINT_GL_ERROR_IF_ANY("XEvents::render, before render_hier");

		//render_hier(rootNode(), VF_globalLight);
		//PRINT_GL_ERROR_IF_ANY("XEvents::render, render_hier(VF_globalLight)");
		//render_hier(rootNode(), VF_Other);

		/*  4. Nodes (not the blended ones)*/
		PRINT_GL_ERROR_IF_ANY("generate_shadowMaps before render_hier");

		profile_start("hier_geom");
		render_hier(rootNode(), VF_Geom | VF_Depth);
		profile_end("hier_geom");
		PRINT_GL_ERROR_IF_ANY("generate_shadowMaps after render_hier");


		///*  5. Blended Nodes*/
		//if (tg->RenderFuncs.have_transparency) {
		//	/*  render the blended nodes*/
		//	render_hier(rootNode(), VF_Geom | VF_Blend | VF_Cube);
		//	PRINT_GL_ERROR_IF_ANY("XEvents::render, render_hier(VF_Geom)");
		//}

		//if you can figure out how to use regular texture in cubemap, then there may be a shortcut
		//for now, we'll pull the fbo pixels back into cpu space and put them in pixeltexture
		pixelType = GL_DEPTH_COMPONENT; // GL_RGBA;
		bytesPerPixel = sizeof(float); // 4;
		if (!ttip->texdata || ttip->x != isize) {
			FREE_IF_NZ(ttip->texdata);
			ttip->texdata = MALLOC(GLvoid*, bytesPerPixel * isize * isize);
		}

		/* grab the data */
		//FW_GL_PIXELSTOREI (GL_UNPACK_ALIGNMENT, 1);
		//FW_GL_PIXELSTOREI (GL_PACK_ALIGNMENT, 1);

		//FW_GL_READPIXELS(0, 0, isize, isize, pixelType, GL_UNSIGNED_BYTE, ttip->texdata);
		FW_GL_READPIXELS(0, 0, isize, isize, GL_DEPTH_COMPONENT, GL_FLOAT, ttip->texdata);
		PRINT_GL_ERROR_IF_ANY("generate_shadowMaps after glReadPixels");

		ttip->x = isize;
		ttip->y = isize;
		ttip->z = 1;
		ttip->hasAlpha = 0; // 1;
		ttip->channels = 0; // 4;
		ttip->idepthbuffer = 1;
		ttip->status = TEX_NEEDSBINDING;
		if (0) {
			//write out tti as web3dit image files for diagnostic viewing, can use for BackGround node
			//void saveImage_web3dit(struct textureTableIndexStruct *tti, char *fname)
			static int iframe = 0;
			iframe++;
			if (iframe == 50) {
				char namebuf[100];
				sprintf(namebuf, "%s%d.web3dit", "cubemapface_", j);
				saveImage_web3dit(ttip, namebuf);
			}
		}
	}
	popnset_viewport();
	popnset_framebuffer();
	//compile_generatedcubemaptexture // convert to opengl
	memcpy(bstack->backgroundmatrix, savebackmat, 16 * sizeof(double));
}
// https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping  
// shows rendering of shadow maps, and debug quad rendering
// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
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
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
static struct debug_quad {
	int textureID; // opengl texture, -1 for no texture
	int which_debug_shader; 
	float near_plane, far_plane;
} debug_quad = { -1,0,1.0f,15.0f};
void set_debug_quad(int which_debug_shader, int textureID) {
	// which_debug_shader - flag to indicate which quad shader 0=turn off debug quad 1-normal texture 2=ortho depth 3=perspective depth
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
	s_shader_capabilities_t* scap;
	shaderflagsstruct shader_requirements;
	memset(&shader_requirements, 0, sizeof(shaderflagsstruct));
	shader_requirements.debug = debug_quad.which_debug_shader;
	scap = getMyShaders(shader_requirements);
	enableGlobalShader(scap);
	if (debug_quad.which_debug_shader == 3) {
		ia = glGetUniformLocation(scap->myShaderProgram, "near_plane");
		glUniform1f(ia,debug_quad.near_plane);
		ia = glGetUniformLocation(scap->myShaderProgram, "far_plane");
		glUniform1f(ia, debug_quad.far_plane);
	}
	ia = glGetUniformLocation(scap->myShaderProgram, "textureUnit");
	glUniform1i(ia, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, debug_quad.textureID);
	renderQuad();

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
	//call from render_xxxLight once per frame
	// usehit - same as for lightTable / shared with lightTable which holds one usehit per scenegraph node visit
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
	struct X3D_SpotLight* node;
	struct X3D_LightRep* lightrep;
	bindablestack* bstack;
	ttglobal tg = gglobal();
	bstack = getActiveBindableStacks(tg);

	//set_viewmatrix();
	//this function tampers with the normal background matrix, which has already been prepped for the mainloop rendering
	//so save it, and restore after gencubemap loop of 6
	memcpy(savebackmat, bstack->backgroundmatrix, 16 * sizeof(double));

	node = (struct X3D_SpotLight*)uhit.node;
	lightrep = (struct X3D_LightRep*)node->_intern;
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
/*
void generate_GlobalShadowMaps() {
	//Stack* genshadow_stack;
	//ttglobal tg = gglobal();
	//ppComponent_Lighting p = (ppComponent_Lighting)tg->Component_Lighting.prv;
	//genshadow_stack = p->genshadow_stack;
	//if (vectorSize(genshadow_stack)) {
	if(shadowTable_count()){
		int i, j, n;

		n = shadowTable_count(); // vectorSize(genshadow_stack);
		for (i = 0; i < n; i++) {
			usehit *uhit;

			uhit = shadowTable_item(i); // vector_get(usehit, genshadow_stack, i);
			switch (uhit->node->_nodeType) {
				case NODE_PointLight:
					generate_shadowmap_cube(*uhit);
					break;
				case NODE_DirectionalLight:
				case NODE_SpotLight:
				case NODE_TextureProjector:
				case NODE_TextureProjectorParallel:
					generate_shadowmap_2D(*uhit, i);
				default:
					break;
			}
		}
	}
	shadowTable_clear(); //genshadow_stack->n = 0;
	PRINT_GL_ERROR_IF_ANY("generate_GlobalShadowMaps END");

}
#else //SHADOWMAPS
void generate_GlobalShadowMaps() {} //stub
*/
#endif //SHADOWMAPS



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

/*

void clear_uniform_buffers_used() {
	//call this in child_shape just before you start sending data / textures to the shader program
	ppComponent_Lighting p = (ppComponent_Lighting)gglobal()->Component_Lighting.prv;
	p->uniformbufs.n = 0;
}
int next_uniform_buffer() {
	ppComponent_Lighting p = (ppComponent_Lighting)gglobal()->Component_Lighting.prv;
	p->tuniformbufs.n++;
	return p->uniformbufs.n - 1;
}

int uniform_buffers_used() {
	ppComponent_Lighting p = (ppComponent_Lighting)gglobal()->Component_Lighting.prv;
	return p->uniformbufs.n;
}
int bind_or_share_next_uniform_buffer(char *uniform_block_name, GLint ubuffer) {
	// call this when sending uniform buffers to the shader 
	// benefits 
	// this one automatically
	// a) checks if this uniform buffer is already bound
	//   and if so return the buffer unit OR
	// b) if not already bound, increments the buffer unit, binds (and returns its  unit index

	ppComponent_Lighting p = (ppComponent_Lighting)gglobal()->Component_Lighting.prv;

	//check if sharable
	int unit = -1;
	for (int i = 0; i < p->uniformbufs.n; i++) {
		if (p->uniformbufs.p[i] == ubuffer) {
			unit = i;
			break;
		}
	}
	if (unit == -1) {
		unit = next_textureUnit();
		p->uniformbufs.p[unit] = ubuffer;
		//glActiveTexture(GL_TEXTURE0 + unit);
		//glBindTexture(samplerType, ubuffer);
	}
	return unit;
}
*/
void sendLightInfo3(s_shader_capabilities_t* me) {
	// in case we are trying to render a node that has just been killed...
	if (me == NULL) return;

	PRINT_GL_ERROR_IF_ANY("BEGIN sendLightInfo2");
	int lightcount = lightTable_count();

	for (int j = 0; j < lightcount; j++) {
		usehit* uhit = lightTable_item(j);
		struct X3D_Node* node = uhit->node;
		struct X3D_PointLight* plight = X3D_POINTLIGHT(node);
		struct X3D_SpotLight* slight = X3D_SPOTLIGHT(node);
		struct X3D_DirectionalLight* dlight = X3D_DIRECTIONALLIGHT(node);
		struct X3D_LightRep* lightrep = (struct X3D_LightRep*)node->_intern;
		//int light_index = bind_or_share_next_uniform_buffer("lightbuf",pose->lightbufIndex);
		//GLUNIFORM1I(me->lightbuf[j], light_index)
		//if (lightpose_buffering()) {
		//	struct LightPose* pose = lightTable_item_heavy(j);
		//	me->lightpose[j] = glGetUniformBlockIndex(myProg, "lightPose");
		//	//int pose_index = bind_or_share_next_uniform_buffer(pose->lightposebuf);
		//	//GLUNIFORM1I(me->lightpose[j], pose_index);
		//}
	}
	GLUNIFORM1I(me->lightcount, lightcount);

	PRINT_GL_ERROR_IF_ANY("END sendLightInfo");

}
GLint tunit(int index);
void sendLightInfo2(s_shader_capabilities_t* me) {
	// in case we are trying to render a node that has just been killed...
	if (me == NULL) return;
	if (light_buffering()) {
		sendLightInfo3(me);
		return;
	}
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
			struct X3D_PixelTexture* tex = (struct X3D_PixelTexture*)vector_get(struct X3D_Node*, lightrep->depth_buffer_stack, uhit->ivalue);
			textureTableIndexStruct_s* tti = getTableIndex(tex->__textureTableIndex);
			int itexunit = bind_or_share_next_textureUnit(GL_TEXTURE_2D, tti->OpenGLTexture); // lightrep->idepthtexture);
			//int iunit = tunit(itexunit);
			glUniform1i(me->textureUnit[itexunit], itexunit); // iunit);
			GLUNIFORM1I(me->lightdepthmap[j], itexunit);
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

			GLUNIFORMMATRIX4FV(me->lightMat[j], 1, GL_FALSE, w2l);
		}
	}
	GLUNIFORM1I(me->lightcount, lightcount);

	PRINT_GL_ERROR_IF_ANY("END sendLightInfo");
}

