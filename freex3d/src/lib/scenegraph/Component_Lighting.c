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
void lightTable_clear();
void lightTable_push(usehit tuple);
void lightTable_pop();
int lightTable_count();
usehit* lightTable_item(int i);
int new_lightway();
static int lightway = 0; //0 pre-May 22, 2022 1 post-May 22, 2022
int new_lightway() {
	return lightway;
}

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

// a specialization of InternalRep - see PolyRep.h
struct X3D_LightRep {
	int itype; //=5, 0 PointRep 1 LineRep 2 PolyRep 3 MeshRep 4 TextureRep 5 LightRep
	struct X3D_Node* depthTexture;
	int size;
	int idepthtexture;
};

void* set_LightRep(void* _lightrep)
{
	struct X3D_LightRep* lightrep = _lightrep;
	if (!_lightrep) {
		_lightrep = MALLOC(struct X3D_LightRep*, sizeof(struct X3D_LightRep));
		memset(_lightrep, 0, sizeof(struct X3D_LightRep));
		lightrep = (struct X3D_LightRep*)_lightrep;
		lightrep->itype = 5;
		lightrep->size = 1024; //size of shadow image, or for pointlight, size of each of 6 sides of cubemap
		lightrep->idepthtexture = -1;
	}
	return lightrep;
}



#define RETURN_IF_LIGHT_STATE_NOT_US \
		if (renderstate()->render_light== VF_globalLight) { \
			if (!node->global) return;\
			/* printf ("and this is a global light\n"); */\
		} else if (node->global) return; \
		/* else printf ("and this is a local light\n"); */

void compile_shadowMap(struct X3D_Node* node);
void compile_DirectionalLight (struct X3D_DirectionalLight *node) {
    struct point_XYZ vec;

	if (node->shadows) compile_shadowMap(X3D_NODE(node));

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
void render_DirectionalLight (struct X3D_DirectionalLight *node) {
	/* if we are doing global lighting, is this one for us? */
	RETURN_IF_LIGHT_STATE_NOT_US
	/*
		if (renderstate()->render_light== VF_globalLight) { 
			if (!node->global){ 
				printf("x local dir,we want global %u\n",node);
				return;
			}
			 printf ("* global dir, we want global %u\n",node); 
		} else {
			if (node->global){
			  printf("x global dir, we want local %u\n",node);
			  return; 
			}
			else {
			   printf ("* local dir, we want local %u\n",node); 
			}
		}
	*/
    COMPILE_IF_REQUIRED;

	if(node->on) {
		if (new_lightway()) {
			//both global on VF_GlobalLight on children->render, and local on prep_sibAffectors come in here
			usehit uhit;
			uhit.node = X3D_NODE(node);
			fw_glGetDoublev(GL_MODELVIEW_MATRIX, uhit.mvm);
			lightTable_push(uhit);
		}
		else {

			int light = nextlight();
			if (light >= 0) {
				float pos[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
				setLightState(light, TRUE);
				setLightType(light, 2);
				FW_GL_LIGHTFV(light, LIGHT_DIRECTION, node->direction.c);
				FW_GL_LIGHTFV(light, LIGHT_POSITION, pos); //direction lights don't have a postion
				FW_GL_LIGHTFV(light, LIGHT_COLOR, node->color.c);
				FW_GL_LIGHTF(light, LIGHT_INTENSITY, node->intensity);
				FW_GL_LIGHTF(light, LIGHT_AMBIENT, node->ambientIntensity);
				FW_GL_LIGHTI(light, LIGHT_SHADOWS, node->shadows);
				FW_GL_LIGHTF(light, LIGHT_SHADOWINTENSITY, node->shadowIntensity);
				FW_GL_LIGHTI(light, LIGHT_DEPTHMAP, -1); //WHERE DO WE GET NUMBER

			/* used to test if a PointLight, SpotLight or DirectionalLight in shader  */
				setLightChangedFlag(light);
			}
		}
	}
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
		if (new_lightway()) {
			//both global on VF_GlobalLight on children->render, and local on prep_sibAffectors come in here
			usehit uhit;
			uhit.node = X3D_NODE(node);
			fw_glGetDoublev(GL_MODELVIEW_MATRIX, uhit.mvm);
			lightTable_push(uhit);
		}
		else {

			int light = nextlight();
			if (light >= 0) {
				float vec[4] = { 0.0f, 0.0f, -1.0f, 1.0f };
				if (node->global && node->shadows) render_PointLight_shadowMap(node);
				setLightState(light, TRUE);
				setLightType(light, 0);
				FW_GL_LIGHTFV(light, LIGHT_DIRECTION, vec);
				FW_GL_LIGHTFV(light, LIGHT_POSITION, node->location.c); //node->_loc.c);

				FW_GL_LIGHTFV(light, LIGHT_ATTENUATION, node->attenuation.c);

				FW_GL_LIGHTFV(light, LIGHT_COLOR, node->color.c);
				FW_GL_LIGHTF(light, LIGHT_INTENSITY, node->intensity);
				FW_GL_LIGHTF(light, LIGHT_AMBIENT, node->ambientIntensity);
				FW_GL_LIGHTF(light, GL_LIGHT_RADIUS, node->radius);
				FW_GL_LIGHTI(light, LIGHT_SHADOWS, node->shadows);
				FW_GL_LIGHTF(light, LIGHT_SHADOWINTENSITY, node->shadowIntensity);
				FW_GL_LIGHTI(light, LIGHT_DEPTHMAP, -1); //WHERE DO WE GET NUMBER
				setLightChangedFlag(light);
			}
		}
	}
}

/* pointLights are done before the rendering of geometry */
void prep_PointLight (struct X3D_PointLight *node) {

	if (!renderstate()->render_light) return;
	/* this will be a global light here... */
	render_PointLight(node);
}

void compile_SpotLight (struct X3D_SpotLight *node) {
    struct point_XYZ vec;
	float dlen;
    int i;

 //   for (i=0; i<3; i++) node->_loc.c[i] = node->location.c[i];
 //   node->_loc.c[3] = 1.0f;/* 1 == this is a position, not a vector */

 //   vec.x = (double) node->direction.c[0];
 //   vec.y = (double) node->direction.c[1];
 //   vec.z = (double) node->direction.c[2];
	//dlen = veclength(vec);
	//if(dlen < .1f) {
	//	vec.x = 0.0; vec.y = 0.0, vec.z = -1.0;
	//}
 //   normalize_vector(&vec);
 //   node->_dir.c[0] = (float) vec.x;
 //   node->_dir.c[1] = (float) vec.y;
 //   node->_dir.c[2] = (float) vec.z;
 //   node->_dir.c[3] = 1.0f;/* 1.0 = SpotLight */
	if (node->shadows) compile_shadowMap(X3D_NODE(node));

    MARK_NODE_COMPILED;
}

void render_SpotLight(struct X3D_SpotLight *node) {
	float ft;

	/* if we are doing global lighting, is this one for us? */
	RETURN_IF_LIGHT_STATE_NOT_US

    COMPILE_IF_REQUIRED;

	if(node->on) {
		if (new_lightway()) {
			//both global on VF_GlobalLight on children->render, and local on prep_sibAffectors come in here
			usehit uhit;
			uhit.node = X3D_NODE(node);
			fw_glGetDoublev(GL_MODELVIEW_MATRIX, uhit.mvm);
			lightTable_push(uhit);
		}else{
			int light = nextlight();
			if (light >= 0) {
				setLightState(light, TRUE);
				setLightType(light, 1);
				FW_GL_LIGHTFV(light, LIGHT_DIRECTION, node->direction.c); //_dir.c);
				FW_GL_LIGHTFV(light, LIGHT_POSITION, node->location.c); //_loc.c);

				FW_GL_LIGHTFV(light, LIGHT_ATTENUATION, node->attenuation.c);
				FW_GL_LIGHTFV(light, LIGHT_COLOR, node->color.c);
				FW_GL_LIGHTF(light, LIGHT_INTENSITY, node->intensity);
				FW_GL_LIGHTF(light, LIGHT_AMBIENT, node->ambientIntensity);

				FW_GL_LIGHTF(light, GL_SPOT_BEAMWIDTH, node->beamWidth); // ft);
				//ConsoleMessage ("spotLight, bw %f, cuta %f, PI/4 %f", node->beamWidth,node->cutOffAngle, PI/4.0);

				/* create a ratio of light in relation to PI/4.0 */
				FW_GL_LIGHTF(light, GL_SPOT_CUTOFF, node->cutOffAngle); // ft);
				FW_GL_LIGHTF(light, GL_LIGHT_RADIUS, node->radius);
				FW_GL_LIGHTI(light, LIGHT_SHADOWS, node->shadows);
				FW_GL_LIGHTF(light, LIGHT_SHADOWINTENSITY, node->shadowIntensity);
				FW_GL_LIGHTI(light, LIGHT_DEPTHMAP, -1); //WHERE DO WE GET NUMBER

				setLightChangedFlag(light);
			}
		}
	}
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

int getLocalLight();
void pushLocalLight(int lastlight);
void popLocalLight();


void sib_prep_DirectionalLight(struct X3D_Node *parent, struct X3D_Node *sibAffector){
	int lastlight;
	//if ((parent->_renderFlags & VF_localLight)==VF_localLight && renderstate()->render_light != VF_globalLight){
	if ( renderstate()->render_light != VF_globalLight){
	  saveLightState2(&lastlight);
	  pushLocalLight(lastlight);
	  render_DirectionalLight((struct X3D_DirectionalLight*)sibAffector);
	}
}
void sib_prep_SpotlLight(struct X3D_Node *parent, struct X3D_Node *sibAffector){
	int lastlight;
	if ( renderstate()->render_light != VF_globalLight){
	  saveLightState2(&lastlight);
	  pushLocalLight(lastlight);
	  render_SpotLight((struct X3D_SpotLight*)sibAffector);
	}

}
void sib_prep_PointLight(struct X3D_Node *parent, struct X3D_Node *sibAffector){
	int lastlight;
	if ( renderstate()->render_light != VF_globalLight){
	  saveLightState2(&lastlight);
	  pushLocalLight(lastlight);
	  render_PointLight((struct X3D_PointLight*)sibAffector);
	}
}


void sib_fin_DirectionalLight(struct X3D_Node *parent, struct X3D_Node *sibAffector){
	int lastlight;
	if ( renderstate()->render_light != VF_globalLight) {
		lastlight = getLocalLight();
		if(numberOfLights() > lastlight) {
			setLightChangedFlag(numberOfLights()-1);
			refreshLightUniforms();
		}
		restoreLightState2(lastlight);
		popLocalLight();
	}
}
void sib_fin_SpotlLight(struct X3D_Node *parent, struct X3D_Node *sibAffector){
	int lastlight;
	if (renderstate()->render_light != VF_globalLight) {
		lastlight = getLocalLight();
		if(numberOfLights() > lastlight) {
			setLightChangedFlag(numberOfLights()-1);
			refreshLightUniforms();
		}
		popLocalLight();
		restoreLightState2(lastlight);
	}
}
void sib_fin_PointLight(struct X3D_Node *parent, struct X3D_Node *sibAffector){
	int lastlight;
	if (renderstate()->render_light != VF_globalLight) {
		lastlight = getLocalLight();
		if(numberOfLights() > lastlight) {
			setLightChangedFlag(numberOfLights()-1);
			refreshLightUniforms();
		}
		popLocalLight();
		restoreLightState2(lastlight);
	}
}
void sib_prep_Light(struct X3D_Node* parent, struct X3D_Node* sibAffector);
void sib_fin_Light(struct X3D_Node* parent, struct X3D_Node* sibAffector);
void sib_prep_Light(struct X3D_Node* parent, struct X3D_Node* sibAffector) {
	if (renderstate()->render_light != VF_globalLight) {
		struct X3D_PointLight* light = X3D_POINTLIGHT(sibAffector);
		if (light->global == FALSE && light->on == TRUE) {
			//should lightTable_push(usehit):
			switch (light->_nodeType) {
			case NODE_SpotLight:
				render_SpotLight((struct X3D_SpotLight*)sibAffector);
				break;
			case NODE_PointLight:
				render_PointLight((struct X3D_PointLight*)sibAffector);
				break;
			case NODE_DirectionalLight:
				render_DirectionalLight((struct X3D_DirectionalLight*)sibAffector);
				break;
			default:
				break;
			}
		}
	}
}
void sib_fin_Light(struct X3D_Node* parent, struct X3D_Node* sibAffector) {
	if (renderstate()->render_light != VF_globalLight) {
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


void get_view_matrix(double* savePosOri, double* saveView);
void freeASCIIString(struct Uni_String* us);


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
		tti->status = TEX_NEEDSBINDING; //I found I didn't need - yet
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
	{
		struct X3D_LightRep* lightrep = (struct X3D_LightRep*)node->_intern;
		struct X3D_PixelTexture* tex = (struct X3D_PixelTexture*)lightrep->depthTexture;
		render_node(X3D_NODE(tex));
	}
	/* Finished rendering CubeMap, set it back for normal textures */
	PRINT_GL_ERROR_IF_ANY("render_PointLight_shadowMaps END");

}




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
void fw_gluPerspective_2(GLDOUBLE xcenter, GLDOUBLE fovy, GLDOUBLE aspect, GLDOUBLE zNear, GLDOUBLE zFar);
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

		clearLightTable();//turns all lights off- will turn them on for VF_globalLight and scope-wise for non-global in VF_geom

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

void generate_shadowmap_2D(usehit uhit) {
	//call from mainloop once per frame:
	//foreach shadow depth texture location in genshadow list
	// set viewpoint pose
	// render scene to fbo
	// convert fbo to regular texture
	//clear genshadow list
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
	struct X3D_PixelTexture* tex = (struct X3D_PixelTexture*)lightrep->depthTexture;
	memcpy(modelviewmatrix, uhit.mvm, 16 * sizeof(double));

	//compile_generatedcubemap - creates framebufferobject fbo
	tti = getTableIndex(tex->__textureTableIndex);
	PRINT_GL_ERROR_IF_ANY("generate_shadowMaps_2D before");

	isize = lightrep->size; //set in compile_
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
	//for (int j = 0; j < cubetex->__subTextures.n; j++) //should be 6
	{
		textureTableIndexStruct_s* ttip;
		struct X3D_PixelTexture* nodep;
		GLuint pixelType;
		int bytesPerPixel;
		int j = 0;
		nodep = tex;
		ttip = tti;

		//we won't directly generate cubemap textures here, but looks interesting as possible 
		//  shotcut to skip readpixels below
		if (1) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ttip->OpenGLTexture);
			//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ttip->OpenGLTexture, 0);
		}
		PRINT_GL_ERROR_IF_ANY("generate_shadowMaps before GL calls");

		//glClearColor(1.0f, 0.0f, 0.0f, 1.0f); //red, for diagnostics during debugging
		//FW_GL_CLEAR(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		FW_GL_CLEAR(GL_DEPTH_BUFFER_BIT);
		//glClear(GL_DEPTH_BUFFER_BIT);
		PRINT_GL_ERROR_IF_ANY("generate_shadowMaps GL calls 1");

		//set viewpoint matrix for side
		//setup_projection(); 
		if(0) {
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
		}
		else {
			fw_glSetDoublev(GL_PROJECTION_MATRIX, uhit.proj);
			fw_glSetDoublev(GL_MODELVIEW_MATRIX, uhit.mvm);
		}
		clearLightTable();//turns all lights off- will turn them on for VF_globalLight and scope-wise for non-global in VF_geom

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
		//render_hier(rootNode(), VF_Geom | VF_Blend | VF_Depth);
		//	PRINT_GL_ERROR_IF_ANY("XEvents::render, render_hier(VF_Geom)");
		//}

		//if you can figure out how to use regular texture in cubemap, then there may be a shortcut
		//for now, we'll pull the fbo pixels back into cpu space and put them in pixeltexture
		if (0) {
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
			ttip->channels = 1; // 4;
			ttip->idepthbuffer = 1;
			ttip->status = TEX_NEEDSBINDING;
			if (1) {
				//write out tti as web3dit image files for diagnostic viewing, can use for BackGround node
				//void saveImage_web3dit(struct textureTableIndexStruct *tti, char *fname)
				static int iframe = 0;
				iframe++;
				if (iframe == 50) {
					char namebuf[100];
					sprintf(namebuf, "%s%d.web3dit", "depth_", j);
					saveImage_web3dit(ttip, namebuf);
				}
			}
		}
	}
	popnset_viewport();
	popnset_framebuffer();
	//compile_generatedcubemaptexture // convert to opengl
	memcpy(bstack->backgroundmatrix, savebackmat, 16 * sizeof(double));
}

void generate_GlobalShadowMaps() {
	Stack* genshadow_stack;
	ttglobal tg = gglobal();
	ppComponent_Lighting p = (ppComponent_Lighting)tg->Component_Lighting.prv;
	genshadow_stack = p->genshadow_stack;
	if (vectorSize(genshadow_stack)) {
		int i, j, n;

		n = vectorSize(genshadow_stack);
		for (i = 0; i < n; i++) {
			usehit uhit;

			uhit = vector_get(usehit, genshadow_stack, i);
			switch (uhit.node->_nodeType) {
				case NODE_PointLight:
					generate_shadowmap_cube(uhit);
					break;
				case NODE_DirectionalLight:
				case NODE_SpotLight:
				case NODE_TextureProjector:
				case NODE_TextureProjectorParallel:
					generate_shadowmap_2D(uhit);
				default:
					break;
			}
		}
	}
	genshadow_stack->n = 0;
	PRINT_GL_ERROR_IF_ANY("generate_GlobalShadowMaps END");

}
#else //SHADOWMAPS
void generate_GlobalShadowMaps() {} //stub
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
void sendLightInfo2(s_shader_capabilities_t* me) {
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
		if (lightType < 2) { //not direction
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
		if (lightType == 2) {
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
			GLUNIFORM1I(me->lightdepthmap[j], lightrep->idepthtexture);
		}
	}
	GLUNIFORM1I(me->lightcount, lightcount);

	PRINT_GL_ERROR_IF_ANY("END sendLightInfo");
}

