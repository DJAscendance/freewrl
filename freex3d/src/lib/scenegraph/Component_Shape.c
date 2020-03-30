/*


X3D Shape Component

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
#include "../opengl/Frustum.h"
#include "../opengl/Material.h"
#include "../opengl/OpenGL_Utils.h"
#include "../opengl/Textures.h"
#include "Component_ProgrammableShaders.h"
#include "Component_Shape.h"
#include "RenderFuncs.h"
#include "LinearAlgebra.h"

#define NOTHING 0

enum {
	MAT_NONE = 0,
	MAT_UNLIT = 1,
	MAT_REGULAR = 2,
	MAT_PHYSICAL = 3,
};

typedef struct pComponent_Shape{

	struct matpropstruct appearanceProperties;

	/* pointer for a TextureTransform type of node */
	struct X3D_Node *  this_textureTransform;  /* do we have some kind of textureTransform? */
	int isBackMaterial;
	/* for doing shader material properties */
	//struct X3D_TwoSidedMaterial *material_twoSided;
	//struct X3D_Material *material_oneSided;

	/* Any user defined shaders here? */
	struct X3D_Node * userShaderNode;
	int modulation;
    
}* ppComponent_Shape;

static void *Component_Shape_constructor(){
	void *v = MALLOCV(sizeof(struct pComponent_Shape));
	memset(v,0,sizeof(struct pComponent_Shape));
	return v;
}
void Component_Shape_init(struct tComponent_Shape *t){
	//public
	//private
	t->prv = Component_Shape_constructor();
	{
		ppComponent_Shape p = (ppComponent_Shape)t->prv;
		p->modulation = 0; //0 by scenefile spec version 1) v3.3(replace)- 2) v4.0+ (modulate everything)
		p->isBackMaterial = 0;
	}

}
void fwl_set_modulation(int modulation){
	ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;
	p->modulation = modulation; //0 per specs 1 blend texture and mat 2 blend mat x cpv x texture
}
int fwl_get_modulation(){
	ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;
	return p->modulation; //0 per specs 1 blend texture and mat 2 blend mat x cpv x texture
}
//getters
struct matpropstruct *getAppearanceProperties(){
	ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;

	return &p->appearanceProperties;
}

// see if the Appearance node has a valid Shader node as a child.
static int hasUserDefinedShader(struct X3D_Node *node) {
#define NO_SHADER 99999
	unsigned int rv = NO_SHADER;

	if (node==NULL) return 0;

	//ConsoleMessage ("hasUserDeginedShader, node ptr %p",node);
	//ConsoleMessage ("hasUserDefinedShader, nodeType %s",stringNodeType(node->_nodeType));
	if (node->_nodeType == NODE_Appearance) {
		struct X3D_Appearance *ap;
		POSSIBLE_PROTO_EXPANSION(struct X3D_Appearance *, node, ap);
		//ConsoleMessage ("appearance node shaders is %d %p",ap->shaders.n, ap->shaders.p);

		if (ap && ap->shaders.n != 0) {
			struct X3D_ComposedShader *cps;

			/* ok - what kind of node is here, and are there more than two? */
			if (ap->shaders.n > 1) {
				ConsoleMessage ("warning, Appearance->shaders has more than 1 node, using only first one");
			}
            
			POSSIBLE_PROTO_EXPANSION(struct X3D_ComposedShader *, ap->shaders.p[0], cps);

			//ConsoleMessage ("node type of shaders is :%s:",stringNodeType(cps->_nodeType));
			if(cps){
				if (cps->_nodeType == NODE_ComposedShader) {
					// might be set in compile_Shader already
					//ConsoleMessage ("cps->_initialized %d",cps->_initialized);
					//ConsoleMessage ("cps->_retrievedURLData %d",cps->_retrievedURLData);

					if (cps->_retrievedURLData) {
						if (cps->_shaderUserNumber == -1) cps->_shaderUserNumber = getNextFreeUserDefinedShaderSlot();
						rv = cps->_shaderUserNumber;
					}
				} else if (cps->_nodeType == NODE_PackagedShader) {
					// might be set in compile_Shader already
					if (X3D_PACKAGEDSHADER(cps)->_retrievedURLData) {
						if (X3D_PACKAGEDSHADER(cps)->_shaderUserNumber == -1) 
							X3D_PACKAGEDSHADER(cps)->_shaderUserNumber = getNextFreeUserDefinedShaderSlot();
						rv = X3D_PACKAGEDSHADER(cps)->_shaderUserNumber;
					}
				} else if (cps->_nodeType == NODE_ProgramShader) {
					// might be set in compile_Shader already
					if (X3D_PROGRAMSHADER(cps)->_retrievedURLData) {
					if (X3D_PROGRAMSHADER(cps)->_shaderUserNumber == -1) 
						X3D_PROGRAMSHADER(cps)->_shaderUserNumber = getNextFreeUserDefinedShaderSlot();

					rv = X3D_PROGRAMSHADER(cps)->_shaderUserNumber;
					}
				} else {
					ConsoleMessage ("shader field of Appearance is a %s, ignoring",stringNodeType(cps->_nodeType));
				}
			}

		} 
	} 

	//ConsoleMessage ("and ste is %d",ste);

	// ok - did we find an integer between 0 and some MAX_SUPPORTED_USER_SHADERS value?
	if (rv == NO_SHADER) rv = 0; 
	//else rv = USER_DEFINED_SHADER_START * (rv+1);

	//ConsoleMessage ("rv is going to be %x",rv);

	//ConsoleMessage ("hasUserDefinedShader, returning %p",*shaderTableEntry);
	return rv;
}
int hasGeneratedCubeMapTexture(struct X3D_Appearance *appearance){
	int ret = FALSE;
	struct X3D_Appearance *tmpN;   
	POSSIBLE_PROTO_EXPANSION(struct X3D_Appearance *, X3D_NODE(appearance),tmpN)
	if(tmpN && tmpN->texture)
		if(tmpN->texture->_nodeType == NODE_GeneratedCubeMapTexture) ret = TRUE;
	return ret;
}
// Save the shader node from the render_*Shader so that we can send in parameters when required
void setUserShaderNode(struct X3D_Node *me) {
	ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;
	p->userShaderNode = me;
}

struct X3D_Node *getThis_textureTransform(){
	ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;
	return p->this_textureTransform;
}
void clear_bound_textures();
void push_isBackMaterial(){
	ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;
	p->isBackMaterial += 1;
}
void pop_isBackMaterial(){
	ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;
	p->isBackMaterial -= 1;
}
int get_isBackMaterial(){
	ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;
	return p->isBackMaterial;

}
void child_Appearance (struct X3D_Appearance *node) {
	struct X3D_Node *tmpN;
	ttglobal tg = gglobal();
	
	/* printf ("in Appearance, this %d, nodeType %d\n",node, node->_nodeType);
	   printf (" vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	   render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */
	/* Render the material node... */
	RENDER_MATERIAL_SUBNODES(node->material);
	if(node->backMaterial){
		push_isBackMaterial();
		RENDER_MATERIAL_SUBNODES(node->backMaterial);
		pop_isBackMaterial();
	}
	
	if (node->fillProperties) {
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->fillProperties,tmpN);
		render_node(tmpN);
	}
	
	/* set line widths - if we have line a lineProperties node */
	if (node->lineProperties) {
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->lineProperties,tmpN);
		render_node(tmpN);
	}
	
	if(node->texture) {
		/* we have to do a glPush, then restore, later */
		/* glPushAttrib(GL_ENABLE_BIT); */
		
		ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;    


		/* is there a TextureTransform? if no texture, fugutaboutit */
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->textureTransform,p->this_textureTransform);
		
		/* now, render the texture */
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->texture,tmpN);
		tg->RenderFuncs.texturenode = (void*)tmpN;

		render_node(tmpN);
	}

	/* shaders here/supported?? */
	if (node->shaders.n !=0) {
		int count;
		int foundGoodShader = FALSE;
		
		for (count=0; count<node->shaders.n; count++) {
			POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->shaders.p[count], tmpN);
			
			/* have we found a valid shader yet? */
			if(tmpN){
				if (foundGoodShader) {
					/* printf ("skipping shader %d of %d\n",count, node->shaders.n); */
					/* yes, just tell other shaders that they are not selected */
					SET_SHADER_SELECTED_FALSE(tmpN);
				} else {
					/* render this node; if it is valid, then we call this one the selected one */
					SET_FOUND_GOOD_SHADER(tmpN);
					DEBUG_SHADER("running shader (%s) %d of %d\n",
					stringNodeType(X3D_NODE(tmpN)->_nodeType),count, node->shaders.n);
					render_node(tmpN);
				}
			}
		}
	}

	/* castle Effects here/supported?? */
	if (node->effects.n !=0) {
		//int count;
		//int foundGoodShader = FALSE;
		prep_sibAffectors(X3D_NODE(node),&node->effects);
		
		//for (count=0; count<node->effects.n; count++) {
		//	POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->effects.p[count], tmpN);
		//	
		//	/* have we found a valid shader yet? */
		//	if(tmpN){
		//		//if (foundGoodShader) {
		//		//	/* printf ("skipping shader %d of %d\n",count, node->shaders.n); */
		//		//	/* yes, just tell other shaders that they are not selected */
		//		//	SET_SHADER_SELECTED_FALSE(tmpN);
		//		//} else {
		//		//	/* render this node; if it is valid, then we call this one the selected one */
		//		//	SET_FOUND_GOOD_SHADER(tmpN);
		//			DEBUG_SHADER("running shader (%s) %d of %d\n",
		//			stringNodeType(X3D_NODE(tmpN)->_nodeType),count, node->effects.n);
		//			//render_node(tmpN);
		//			prep_sibAffectors(node,&node->effects);
		//		//}
		//	}
		//}
	}

}


void render_Material (struct X3D_Material *node) {
	COMPILE_IF_REQUIRED
	{
		ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;

		/* record this node for OpenGL-ES and OpenGL-3.1 operation */
		//p->material_oneSided = node;
		//if(get_isBackMaterial()){
		//	p->material_twoSided = node;
		//}
		if (node != NULL) {
			if(get_isBackMaterial()){
				memcpy (&p->appearanceProperties.fw_BackMaterial, node->_material, sizeof (struct fw_MaterialParameters));
			}else{
				memcpy (&p->appearanceProperties.fw_FrontMaterial, node->_material, sizeof (struct fw_MaterialParameters));
			}
		}
	}
}
//struct X3D_Material *get_material_oneSided(){
//	ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;
//	return p->material_oneSided;
//}
//struct X3D_TwoSidedMaterial *get_material_twoSided(){
//	ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;
//	return p->material_twoSided;
//}

/* bounds check the material node fields */
void compile_Material (struct X3D_Material *node) {
	struct X3D_Node **tnodes;
	struct fw_MaterialParameters *q;
	/* verify that the numbers are within range */
	node->ambientIntensity = fclamp(node->ambientIntensity,0.0f,1.0f);
	node->shininess = fclamp(node->shininess,0.0f,1.0f);
	node->transparency = fclamp(node->transparency,0.0f,1.0f);
	fvecclamp3f(node->diffuseColor.c,0.0f,1.0f);
	fvecclamp3f(node->emissiveColor.c,0.0f,1.0f);
	fvecclamp3f(node->specularColor.c,0.0f,1.0f);

	if(!node->_material){
		node->_material = malloc(sizeof(struct fw_MaterialParameters));
		register_node_gc(node,node->_material);
	}
	memset(node->_material,0,sizeof(struct fw_MaterialParameters));

	q = (struct fw_MaterialParameters *)node->_material;
	vecset3f(q->baseColor,1.0f,1.0f,1.0f); //saves boolean math in shader
	veccopy3f(q->diffuse,node->diffuseColor.c);
	veccopy3f(q->emissive,node->emissiveColor.c);
	veccopy3f(q->specular,node->specularColor.c);
	q->ambient = node->ambientIntensity;
	q->shininess = node->shininess;
	q->transparency = node->transparency;
	q->type = MAT_REGULAR;

	//new v4 textures
	// [0] normal [1] emissive [2] diffuse OR baseColor [3] specular/shiny OR metallic/roughness [4] ambient
	tnodes = q->textures;
	memset(tnodes,0,5*sizeof(void *));
	if(node->normalTexture)
	{
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->normalTexture,tnodes[0]);
	}
	if(node->emissiveTexture)
	{
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->emissiveTexture,tnodes[1]);
	}
	if(node->diffuseTexture)
	{
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->diffuseTexture,tnodes[2]);
	}
	if(node->specularShininessTexture)
	{
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->specularShininessTexture,tnodes[3]);
	}
	if(node->ambientTexture)
	{
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->ambientTexture,tnodes[4]);
	}
	int *cindex = q->cindex;
	cindex[0] = node->normalTextureChannel;
	cindex[1] = node->emissiveTextureChannel;
	cindex[2] = node->diffuseTextureChannel;
	cindex[3] = node->specularShininessTextureChannel;
	cindex[4] = node->ambientTextureChannel;
	q->nt = 0; //assume no material.texturexxx to start
	for(int i=0;i<5;i++){
		q->tcount[i] = 0; //default: no texture for this material function
		q->tstart[i] = q->nt; //shader: start looping over tindex where we left off, for tcount loops
		if(tnodes[i]){
			if(tnodes[i]->_nodeType == NODE_MultiTexture) {
				struct X3D_MultiTexture *mt = (struct X3D_MultiTexture*)tnodes[i];
				q->tcount[i] = mt->texture.n;
				q->nt += mt->texture.n;
				q->mt++;
			}else{
				//single texture
				q->nt++;
				q->tcount[i] = 1;
			}
		}
	}
	MARK_NODE_COMPILED
}



#define CHECK_COLOUR_FIELD(aaa) \
	case NODE_##aaa: { \
		struct X3D_##aaa *me = (struct X3D_##aaa *)realNode; \
		if (me->color == NULL) return NOTHING; /* do not add any properties here */\
		else return COLOUR_MATERIAL_SHADER; \
		break; \
	} 
#define CHECK_FOGCOORD_FIELD(aaa) \
	case NODE_##aaa: { \
		struct X3D_##aaa *me = (struct X3D_##aaa *)realNode; \
		if (me->fogCoord == NULL) return NOTHING; /* do not add any properties here */\
		else return HAVE_FOG_COORDS; \
		break; \
	} 


/* if this is a LineSet, PointSet, etc... */
// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/lighting.html#Lightingoff
// 'shapes that represent points or lines are unlit
static bool getIfLinePoints(struct X3D_Node *realNode) {
	if (realNode == NULL) return false;
	switch (realNode->_nodeType) {
		case NODE_IndexedLineSet:
		case NODE_LineSet:
		case NODE_PointSet:
		case NODE_Polyline2D:
		case NODE_Polypoint2D:
		case NODE_Circle2D:
		case NODE_Arc2D:
			return  true;
	}
	return false; // do not add any capabilites here.
}

/* is the texCoord field a TextureCoordinateGenerator or not? */
struct X3D_Node *getGeomTexCoordField(struct X3D_Node *realGeomNode){
	struct X3D_Node *tc = NULL;
	int *fieldOffsetsPtr = NULL;

	//ConsoleMessage ("getShapeTextureCoordGen, node type %s\n",stringNodeType(realNode->_nodeType));
	if (realGeomNode == NULL) return tc;

	fieldOffsetsPtr = (int *)NODE_OFFSETS[realGeomNode->_nodeType];
	/*go thru all field*/
	while (*fieldOffsetsPtr != -1) {
		if (*fieldOffsetsPtr == FIELDNAMES_texCoord) {
			// get the pointer stored here...
			memcpy(&tc,offsetPointer_deref(void*, realGeomNode,*(fieldOffsetsPtr+1)),sizeof(struct X3D_Node *));
			break;
		}
		fieldOffsetsPtr += FIELDOFFSET_LENGTH;
	}
	return tc;

}
static int getShapeTextureCoordGen(struct X3D_Node *realNode) {
	struct X3D_Node *tc = NULL;
	tc = getGeomTexCoordField(realNode);
	if (tc != NULL) {
		if (tc->_nodeType == NODE_TextureCoordinateGenerator) return HAVE_TEXTURECOORDINATEGENERATOR;
	}
	return 0;
}


/* Some shapes have Color nodes - if so, then we have other shaders */
static int getShapeColourShader (struct X3D_Node *myGeom) {
	struct X3D_Node *realNode;

	POSSIBLE_PROTO_EXPANSION(struct X3D_Node *,myGeom,realNode);

	if (realNode == NULL) return NOTHING;

	/* go through each node type that can have a Color node, and if it is not NULL
	   we know we have a Color node */

	switch (realNode->_nodeType) {
		CHECK_COLOUR_FIELD(IndexedFaceSet);
		CHECK_COLOUR_FIELD(IndexedLineSet);
		CHECK_COLOUR_FIELD(IndexedTriangleFanSet);
		CHECK_COLOUR_FIELD(IndexedTriangleSet);
		CHECK_COLOUR_FIELD(IndexedTriangleStripSet);
		CHECK_COLOUR_FIELD(LineSet);
		CHECK_COLOUR_FIELD(PointSet);
		CHECK_COLOUR_FIELD(TriangleFanSet);
		CHECK_COLOUR_FIELD(TriangleStripSet);
		CHECK_COLOUR_FIELD(TriangleSet);
		CHECK_COLOUR_FIELD(ElevationGrid);
		CHECK_COLOUR_FIELD(GeoElevationGrid);
		CHECK_COLOUR_FIELD(QuadSet);
		CHECK_COLOUR_FIELD(IndexedQuadSet);
	}

	/* if we are down here, we KNOW we do not have a color field */
	return NOTHING; /* do not add any capabilites here */
}
/* Some shapes have Color nodes - if so, then we have other shaders */
static int getShapeFogShader (struct X3D_Node *myGeom) {
	struct X3D_Node *realNode;

	POSSIBLE_PROTO_EXPANSION(struct X3D_Node *,myGeom,realNode);

	if (realNode == NULL) return NOTHING;

	/* go through each node type that can have a Color node, and if it is not NULL
	   we know we have a Color node */

	switch (realNode->_nodeType) {
		CHECK_FOGCOORD_FIELD(IndexedFaceSet);
		CHECK_FOGCOORD_FIELD(IndexedLineSet);
		CHECK_FOGCOORD_FIELD(IndexedTriangleFanSet);
		CHECK_FOGCOORD_FIELD(IndexedTriangleSet);
		CHECK_FOGCOORD_FIELD(IndexedTriangleStripSet);
		CHECK_FOGCOORD_FIELD(LineSet);
		CHECK_FOGCOORD_FIELD(PointSet);
		CHECK_FOGCOORD_FIELD(TriangleFanSet);
		CHECK_FOGCOORD_FIELD(TriangleStripSet);
		CHECK_FOGCOORD_FIELD(TriangleSet);
		CHECK_FOGCOORD_FIELD(ElevationGrid);
		//CHECK_FOGCOORD_FIELD(GeoElevationGrid);
		CHECK_FOGCOORD_FIELD(QuadSet);
		CHECK_FOGCOORD_FIELD(IndexedQuadSet);
	}

	/* if we are down here, we KNOW we do not have a color field */
	return NOTHING; /* do not add any capabilites here */
}
void compile_material_if_required(struct X3D_Node *node){
	COMPILE_IF_REQUIRED(node);
}

static int getAppearanceShader (struct X3D_Node *myApp) {
	struct X3D_Appearance *realAppearanceNode;
	struct X3D_Node *realMaterialNode, *realBackMaterialNode;


	int retval = NOTHING;

	/* if there is no appearance node... */
	if (myApp == NULL) return retval;

	POSSIBLE_PROTO_EXPANSION(struct X3D_Appearance *, myApp,realAppearanceNode);
	if (!realAppearanceNode || realAppearanceNode->_nodeType != NODE_Appearance) return retval;
    
	// v4 Appearance.backMaterial (vs v3.3-- TwoSidedMaterial)
	realMaterialNode = realBackMaterialNode = NULL;
	if (realAppearanceNode->backMaterial != NULL) {
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, realAppearanceNode->backMaterial,realBackMaterialNode);
	}
	if (realAppearanceNode->material != NULL) {
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, realAppearanceNode->material,realMaterialNode);	
	}
	if(realMaterialNode || realBackMaterialNode) {
		if(1){
			struct fw_MaterialParameters *p, *q;
			int material, texture, multitex, twosided, physical, unlit;
			material = texture = multitex = twosided = physical = unlit = FALSE;
		
			p = q = NULL;
			if(realMaterialNode){
				compile_material_if_required(realMaterialNode);
				switch(realMaterialNode->_nodeType){
					case NODE_Material:
						{
							struct X3D_Material *mnode = (struct X3D_Material*)realMaterialNode;
							p = (struct fw_MaterialParameters*)mnode->_material;
							material = TRUE;
						}
						break;
					case NODE_PhysicalMaterial:
						{
							struct X3D_PhysicalMaterial *mnode = (struct X3D_PhysicalMaterial*)realMaterialNode;
							p = (struct fw_MaterialParameters*)mnode->_material;
							physical = TRUE;
						}
						break;
					case NODE_UnlitMaterial:
						{
							struct X3D_UnlitMaterial *mnode = (struct X3D_UnlitMaterial*)realMaterialNode;
							p = (struct fw_MaterialParameters*)mnode->_material;
							unlit = TRUE;
						}
						break;
					case NODE_TwoSidedMaterial:
						{
							struct X3D_TwoSidedMaterial *mnode = (struct X3D_TwoSidedMaterial*)realMaterialNode;
							p = (struct fw_MaterialParameters*)mnode->_material;
							q = (struct fw_MaterialParameters*)mnode->_backMaterial;
							material = TRUE;
							twosided = TRUE;
						}
						break;
					default:
						break;
				}
			}
			if(realBackMaterialNode){
				compile_material_if_required(realBackMaterialNode);
				switch(realBackMaterialNode->_nodeType){
					case NODE_Material:
						{
							struct X3D_Material *mnode = (struct X3D_Material*)realBackMaterialNode;
							q = (struct fw_MaterialParameters*)mnode->_material;
							material = TRUE;
						}
						break;
					case NODE_PhysicalMaterial:
						{
							struct X3D_PhysicalMaterial *mnode = (struct X3D_PhysicalMaterial*)realBackMaterialNode;
							q = (struct fw_MaterialParameters*)mnode->_material;
							physical = TRUE;
						}
						break;
					case NODE_UnlitMaterial:
						{
							struct X3D_UnlitMaterial *mnode = (struct X3D_UnlitMaterial*)realBackMaterialNode;
							q = (struct fw_MaterialParameters*)mnode->_material;
							unlit = TRUE;
						}
						break;
					case NODE_TwoSidedMaterial:
						{
							//not permitted
							//struct X3D_TwoSidedMaterial *mnode = (struct X3D_TwoSidedMaterial*)realMaterialNode;
							//p = (struct fw_MaterialParameters*)mnode->_material;
							//q = (struct fw_MaterialParameters*)mnode->_backMaterial;
							//unlit = TRUE;
						}
						break;
					default:
						break;
				}
			}
			if(p){
				if(p->nt) texture = TRUE;
				if(p->mt) multitex = TRUE;
			}
			if(q){
				if(q->nt) texture = TRUE;
				if(q->mt) multitex = TRUE;
				twosided = TRUE; //not sure it makes sense what we are doing with TWO, should it be pipeline culling CULL_FACE GL_BACK, GL_FRONT? but need now for Appearance.backMaterial
			}
			if(twosided) retval |= TWO_MATERIAL_APPEARANCE_SHADER;
			if(material) retval |= MATERIAL_APPEARANCE_SHADER;
			if(physical) retval |= PHYSICAL_MATERIAL_APPEARANCE_SHADER;
			if(unlit) retval |= UNLIT_MATERIAL_APPEARANCE_SHADER;
			// need to learn | vs xor etc so this isn't un-applied below if there's a regular appearance.texture
			if(texture) retval |= ONE_TEX_APPEARANCE_SHADER;
			if(multitex) retval |= MULTI_TEX_APPEARANCE_SHADER;
		} else {
			if(realBackMaterialNode || realMaterialNode->_nodeType == NODE_TwoSidedMaterial )  {    
				retval |= TWO_MATERIAL_APPEARANCE_SHADER;
			}
			if (realMaterialNode->_nodeType == NODE_Material || (realBackMaterialNode && realBackMaterialNode->_nodeType == NODE_Material)) {
					retval |= MATERIAL_APPEARANCE_SHADER;
			}
			if (realMaterialNode->_nodeType == NODE_PhysicalMaterial || (realBackMaterialNode && realBackMaterialNode->_nodeType == NODE_PhysicalMaterial)) {
				retval |= PHYSICAL_MATERIAL_APPEARANCE_SHADER;
			}
			if (realMaterialNode->_nodeType == NODE_UnlitMaterial || (realBackMaterialNode && realBackMaterialNode->_nodeType == NODE_UnlitMaterial)) {
				retval |= UNLIT_MATERIAL_APPEARANCE_SHADER;
			}
		}
	}


	if (realAppearanceNode->fillProperties != NULL) {
		struct X3D_Node *fp;
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, realAppearanceNode->fillProperties,fp);
		if(fp){
			if (fp->_nodeType != NODE_FillProperties) {
				ConsoleMessage("getAppearanceShader, fillProperties has a node type of %s",stringNodeType(fp->_nodeType));
			} else {
				// is this a FillProperties node, but is it enabled?
				if (X3D_FILLPROPERTIES(fp)->_enabled)
					retval |= FILL_PROPERTIES_SHADER;
			}
		}
	}


	if (realAppearanceNode->texture != NULL) {
		//printf ("getAppearanceShader - rap node is %s\n",stringNodeType(realAppearanceNode->texture->_nodeType));
		struct X3D_Node *tex;

		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, realAppearanceNode->texture,tex);
		if(tex){
			if ((tex->_nodeType == NODE_ImageTexture) || 
				(tex->_nodeType == NODE_MovieTexture) || 
				(tex->_nodeType == NODE_PixelTexture) ){
				retval |= ONE_TEX_APPEARANCE_SHADER;
			} else if( (tex->_nodeType == NODE_PixelTexture3D) ||
				(tex->_nodeType == NODE_ComposedTexture3D) ||
				(tex->_nodeType == NODE_ImageTexture3D) ) {
				retval |= ONE_TEX_APPEARANCE_SHADER;
				retval |= TEX3D_SHADER; //VOLUME by default
				if(tex->_nodeType == NODE_ComposedTexture3D)
					retval |= TEX3D_LAYER_SHADER; //else VOLUME
			} else if (tex->_nodeType == NODE_MultiTexture) {
				retval |= MULTI_TEX_APPEARANCE_SHADER;
			} else if ((tex->_nodeType == NODE_ComposedCubeMapTexture) ||
						(tex->_nodeType == NODE_ImageCubeMapTexture) || 
						(tex->_nodeType == NODE_GeneratedCubeMapTexture)) {
				retval |= HAVE_CUBEMAP_TEXTURE;
			} else {
				ConsoleMessage ("getAppearanceShader, texture field %s not supported yet\n",
				stringNodeType(tex->_nodeType));
			}
		}
	}
    
	#ifdef SHAPE_VERBOSE
	printf ("getAppearanceShader, returning %x\n",retval);
	#endif //SHAPE_VERBOSE

	return retval;
}


/* find info on the appearance of this Shape and create a shader */
/* 
	The possible sequence of a properly constructed appearance field is:

	Shape.appearance -> Appearance
	
	Appearance.fillProperties 	-> FillProperties
	Appearance.lineProperties 	-> LineProperties
	Appearance.material 		-> Material
					-> TwoSidedMaterial	
	Appearance.shaders		-> ComposedShader
	Appearance.shaders		-> PackagedShader
	Appearance.shaders		-> ProgramShader

	Appearance.texture		-> Texture
	Appearance.texture		-> MultiTexture
	Appearance.textureTransform	->

*/

/* now works with our pushing matricies (norm, proj, modelview) but not for complete shader appearance replacement */
void render_FillProperties (struct X3D_FillProperties *node) {
	GLfloat hatchX;
	GLfloat hatchY;
	GLint algor;
	GLint hatched;
	GLint filled;

	struct matpropstruct *me= getAppearanceProperties();

	hatchX = 0.80f; hatchY = 0.80f;
	algor = node->hatchStyle; filled = node->filled; hatched = node->hatched;
	switch (node->hatchStyle) {
		case 0: break; /* bricking - not standard X3D */
		case 1: hatchX = 1.0f; break; /* horizontal lines */
		case 2: hatchY = 1.0f; break; /* vertical lines */
		case 3: hatchY=1.0f; break; /* positive sloped lines */
		case 4: hatchY=1.0f; break; /* negative sloped lines */
		case 5: break; /* square pattern */
		case 6: hatchY = 1.0f; break; /* diamond pattern */

		default :{
			node->hatched = FALSE; /* woops - something wrong here disable */
		}
	}

	me->filledBool = filled;
	me->hatchedBool = hatched;
	me->hatchPercent[0] = hatchX;
	me->hatchPercent[1] = hatchY;
	me->hatchScale[0] = node->_hatchScale.c[0];
	me->hatchScale[1] = node->_hatchScale.c[1];
	me->algorithm = algor;
	me->hatchColour[0]=node->hatchColor.c[0]; me->hatchColour[1]=node->hatchColor.c[1]; me->hatchColour[2] = node->hatchColor.c[2];
	me->hatchColour[3] = 1.0;
}


void render_LineProperties (struct X3D_LineProperties *node) {
	#ifdef NEED_TO_ADD_TO_SHADER
	much of this was working in older versions of FreeWRL,
	before we went to 100% shader based code. Check FreeWRL
	from (say) 2011 to see what the shader code looked like

	GLushort pat;
	#endif

	if (node->applied) {
		//ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;

		if (node->linewidthScaleFactor > 1.0) {
			struct matpropstruct *me;
			glLineWidth(node->linewidthScaleFactor);
			me= getAppearanceProperties();
			me->pointSize = node->linewidthScaleFactor;
		}


		#ifdef NEED_TO_ADD_TO_SHADER
		if (node->linetype > 1) {
			pat = 0xffff; /* can not support fancy line types - this is the default */
			switch (node->linetype) {
				case 2: pat = 0xff00; break; /* dashed */
				case 3: pat = 0x4040; break; /* dotted */
				case 4: pat = 0x04ff; break; /* dash dot */
				case 5: pat = 0x44fe; break; /* dash dot dot */
				case 6: pat = 0x0100; break; /* optional */
				case 7: pat = 0x0100; break; /* optional */
				case 10: pat = 0xaaaa; break; /* optional */
				case 11: pat = 0x0170; break; /* optional */
				case 12: pat = 0x0000; break; /* optional */
				case 13: pat = 0x0000; break; /* optional */
				default: {}
			}
		}
		#endif 
	}
}

textureTableIndexStruct_s *getTableTableFromTextureNode(struct X3D_Node *textureNode);

int getImageChannelCountFromTTI(struct X3D_Node *appearanceNode ){
	//int channels, imgalpha, isLit, isUnlitGeometry, hasColorNode, whichShapeColorShader;
	int channels, imgalpha, haveTexture;
	struct X3D_Appearance *appearance;
	channels = 0;
	imgalpha = 0;
	haveTexture = 0;
	POSSIBLE_PROTO_EXPANSION(struct X3D_Appearance*,appearanceNode,appearance);

	if(appearance){
		//do I need possible proto expansione of texture, or will compile_appearance have done that?
		if(appearance->texture){
			//mutli-texture? should loop over multitexture->texture nodes? 
			// --to get to get max channels or hasAlpha, or need channels for each one?
			// H0: if nay of the multitextures has an alpha, then its alpha replaces material alpha
			// H1: multitexture alpha is only for composing textures, assumed to take material alpha 
			if(appearance->texture->_nodeType == NODE_MultiTexture || 
				appearance->texture->_nodeType == NODE_ComposedTexture3D ){
				int k;
				struct Multi_Node * mtex = NULL;
				switch(appearance->texture->_nodeType){
					case NODE_MultiTexture: mtex = &((struct X3D_MultiTexture*)appearance->texture)->texture; break;
					case NODE_ComposedTexture3D: mtex = &((struct X3D_ComposedTexture3D*)appearance->texture)->texture; break;
				}
				channels = 0;
				imgalpha = 0;
				if(mtex)
				for(k=0;k<mtex->n;k++){
					textureTableIndexStruct_s *tti = getTableTableFromTextureNode(mtex->p[k]);
					haveTexture = 1;
					if(tti){
						//new Aug 6, 2016, check LoadTextures.c for your platform channel counting
						//NoImage=0, Luminance=1, LuminanceAlpha=2, RGB=3, RGBA=4
						//PROBLEM: if tti isn't loaded -with #channels, alpha set-, we don't want to compile child
						channels = max(channels,tti->channels);
						imgalpha = max(tti->hasAlpha,imgalpha);
						//if(tti->status < TEX_NEEDSBINDING) 
						//	printf("."); //should Unmark node compiled
					}
				}
			}else if(appearance->texture->_nodeType == NODE_ComposedCubeMapTexture){
				int k;
				struct X3D_Node* p[6];
				struct X3D_ComposedCubeMapTexture * ccmt;
				ccmt = (struct X3D_ComposedCubeMapTexture *)appearance->texture;
				p[0] = ccmt->top;
				p[1] = ccmt->left;
				p[2] = ccmt->front;
				p[3] = ccmt->right;
				p[4] = ccmt->back;
				p[5] = ccmt->bottom;
				for(k=0;k<6;k++){
					if(p[k]){
						textureTableIndexStruct_s *tti = getTableTableFromTextureNode(p[k]);
						haveTexture = 1;
						if(tti){
							//new Aug 6, 2016, check LoadTextures.c for your platform channel counting
							//NoImage=0, Luminance=1, LuminanceAlpha=2, RGB=3, RGBA=4
							//PROBLEM: if tti isn't loaded -with #channels, alpha set-, we don't want to compile child
							channels = max(channels,tti->channels);
							imgalpha = max(tti->hasAlpha,imgalpha);
						}
					}
				}
			}else{
				//single texture:
				textureTableIndexStruct_s *tti = getTableTableFromTextureNode(appearance->texture);
				haveTexture = 1;
				if(tti){
					//new Aug 6, 2016, check LoadTextures.c for your platform channel counting
					//NoImage=0, Luminance=1, LuminanceAlpha=2, RGB=3, RGBA=4
					//PROBLEM: if tti isn't loaded -with #channels, alpha set-, we don't want to compile child
					channels = tti->channels; 
					imgalpha = tti->hasAlpha;
					//if(tti->status < TEX_NEEDSBINDING) 
					//	printf("."); //should Unmark node compiled
				}
			}
		}
	}
	channels = haveTexture ? channels : 0; //-1;
	return channels;
}

void initialize_fw_MaterialParameters(struct fw_MaterialParameters *mat){
	memset(mat,0,sizeof(struct fw_MaterialParameters));
	mat->ambient = .2f;
	mat->shininess = .2f;
	vecset3f(mat->diffuse,1.0f,1.0f,1.0f); //saves boolean math in shader if at 1
	vecset3f(mat->baseColor,1.0f,1.0f,1.0f);
	mat->type = MAT_NONE;
}
void initialize_front_and_back_material_params(){
	ppComponent_Shape p;
   	ttglobal tg = gglobal();
	p = (ppComponent_Shape)tg->Component_Shape.prv;

	initialize_fw_MaterialParameters(&p->appearanceProperties.fw_FrontMaterial);
	initialize_fw_MaterialParameters(&p->appearanceProperties.fw_BackMaterial);
}

//unsigned int getShaderFlags();
shaderflagsstruct getShaderFlags();
struct X3D_Node *getFogParams();
void update_effect_uniforms();
bool setupShaderB();
void textureTransform_start();
void reallyDraw();
void resend_textureprojector_matrix();



void child_Shape (struct X3D_Shape *node) {
	struct X3D_Node *tmpNG;  
	//int channels;
	struct X3D_Virt *v;

	ppComponent_Shape p;
   	ttglobal tg = gglobal();

	COMPILE_IF_REQUIRED

	/* JAS - if not collision, and render_geom is not set, no need to go further */
	/* printf ("child_Shape vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	 render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */

	if(!(node->geometry)) { return; }

	RECORD_DISTANCE

	if((renderstate()->render_collision) || (renderstate()->render_sensitive) || (renderstate()->render_other)) {
		/* only need to forward the call to the child */
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *,node->geometry,tmpNG);
		render_node(tmpNG);
		return;
	}
	p = (ppComponent_Shape)tg->Component_Shape.prv;

	/* initialization. This will get overwritten if there is a texture in an Appearance
	   node in this shape (see child_Appearance) */
	tg->RenderFuncs.last_texture_type = NOTEXTURE;
	tg->RenderFuncs.shapenode = node;
	
	/* copy the material stuff in preparation for copying all to the shader */
	initialize_front_and_back_material_params();

	if((renderstate()->render_cube) && hasGeneratedCubeMapTexture((struct X3D_Appearance*)node->appearance))
		return; //don't draw if this node uses a generatedcubemaptexture and its a cubemaptexture generation pass; is there more optimal place to do this?

	/* now, are we rendering blended nodes or normal nodes?*/
	if (renderstate()->render_blend == (node->_renderFlags & VF_Blend)) {
		int isUserShader; //colorSource, isLit, alphaSource, 
		s_shader_capabilities_t *scap;
		//unsigned int shader_requirements;
		shaderflagsstruct shader_requirements;
		memset(&shader_requirements,0,sizeof(shaderflagsstruct));

		//prep_Appearance
		RENDER_MATERIAL_SUBNODES(node->appearance); //child_Appearance

		/* enable the shader for this shape */
		//ConsoleMessage("turning shader on %x",node->_shaderTableEntry);

		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->geometry,tmpNG);

		shader_requirements.base = node->_shaderflags_base; //_shaderTableEntry;  
		shader_requirements.effects = node->_shaderflags_effects;
		shader_requirements.usershaders = node->_shaderflags_usershaders;
		isUserShader = shader_requirements.usershaders ? TRUE : FALSE; // >= USER_DEFINED_SHADER_START ? TRUE : FALSE;
		//if(!p->userShaderNode || !(shader_requirements >= USER_DEFINED_SHADER_START)){
		if(!isUserShader){
			//for Luminance and Luminance-Alpha images, we have to tinker a bit in the Vertex shader
			// New concept of operations Aug 26, 2016
			// in the specs there are some things that can replace other things (but not the reverse)
			// Texture can repace CPV, diffuse and 111
			// CPV can replace diffuse and 111
			// diffuse can replace 111
			// Texture > CPV > Diffuse > (1,1,1)
			// so there's a kind of order / sequence to it.
			// There can be a flag at each step saying if you want to replace the prior value (otherwise modulate)
			// Diffuse replacing or modulating (111) is the same thing, no flag needed
			// Therefore we need at most 2 flags for color:
			// TEXTURE_REPLACE_PRIOR and CPV_REPLACE_PRIOR.
			// and other flag for alpha: ALPHA_REPLACE_PRIOR (same as ! WANT_TEXALPHA)
			// if all those are false, then its full modulation.
			// our WANT_LUMINANCE is really == ! TEXTURE_REPLACE_PRIOR
			// we are missing a CPV_REPLACE_PRIOR, or more precisely this is a default burned into the shader

			int channels,modulation,scenefile_specversion;
			//modulation:
			//- for Castle-style full-modulation of texture x CPV x mat.diffuse
			//     and texalpha x (1-mat.trans), set 2
			//- for specs table 17-2 RGB Tex replaces CPV with modulation 
			//     of table 17-2 entries with mat.diffuse and (1-mat.trans) set 1
			//- for specs table 17-3 as written and ignoring modulation sentences
			//    so CPV replaces diffuse, texture replaces CPV and diffuse- set 0
			// testing: KelpForest SharkLefty.x3d has CPV, ImageTexture RGB, and mat.diffuse
			//    29C.wrl has mat.transparency=1 and LumAlpha image, modulate=0 shows sphere, 1,2 inivisble
			//    test all combinations of: modulation {0,1,2} x shadingStyle {gouraud,phong}: 0 looks bright texture only, 1 texture and diffuse, 2 T X C X D
			channels = getImageChannelCountFromTTI(node->appearance);
			// specversion <= 330 use v3.3 table 17-3
			// specversion >= 400 modulate everything
			scenefile_specversion = X3D_PROTO(node->_executionContext)->__specversion;
			// p->modulation; 0)scenefile specversion 1)v3.3- 2) v4.0+ (dug9 Mar 28, 2020)
			
			switch(fwl_get_modulation()){
				case 0:
					//allows mixing modulations depending on which inline/proto/scenefile the shape was defined in
					modulation = scenefile_specversion >= 400 ? TRUE : FALSE; 
					break;
				case 1:
					modulation = FALSE; break;
				case 2:
					modulation = TRUE; break;
				default:
					modulation = FALSE;
			}
			if(modulation == TRUE){
				shader_requirements.base |= MODULATE_TEXTURE; //web3d most browsers default: texture replaces prior by default
			}
			if(!channels || (channels == 1 || channels == 3))
				shader_requirements.base |= MODULATE_ALPHA;  //A = (1-TM)
			if(channels && (channels == 1 || channels == 2) )
				shader_requirements.base |= MODULATE_COLOR;  //ODrgb = IT x ICrgb

			//getShaderFlags() are from non-leaf-node shader influencers: 
			//   fog, local_lights, clipplane, Effect/EffectPart (for CastlePlugs) ...
			// - as such they may be different for the same shape node DEF/USEd in different branches of the scenegraph
			// - so they are ORd here before selecting a shader permutation
			shader_requirements.base |= getShaderFlags().base; 
			shader_requirements.effects |= getShaderFlags().effects;
			//if(shader_requirements & FOG_APPEARANCE_SHADER)
			//	printf("fog in child_shape\n");
		}
		//printf("child_shape shader_requirements base %d effects %d user %d\n",shader_requirements.base,shader_requirements.effects,shader_requirements.usershaders);
		scap = getMyShaders(shader_requirements);
		enableGlobalShader(scap);
		//enableGlobalShader (getMyShader(shader_requirements)); //node->_shaderTableEntry));

		//see if we have to set up a TextureCoordinateGenerator type here
		if (tmpNG && tmpNG->_intern) {
			if (tmpNG->_intern->tcoordtype == NODE_TextureCoordinateGenerator) {
				getAppearanceProperties()->texCoordGeneratorType = tmpNG->_intern->texgentype;
				//ConsoleMessage("shape, matprop val %d, geom val %d",getAppearanceProperties()->texCoordGeneratorType, node->geometry->_intern->texgentype);
			}
		}
		//userDefined = (whichOne >= USER_DEFINED_SHADER_START) ? TRUE : FALSE;
		//if (p->userShaderNode != NULL && shader_requirements >= USER_DEFINED_SHADER_START) {
		if(isUserShader && p->userShaderNode){
			//we come in here right after a COMPILE pass in APPEARANCE which renders the shader, which sets p->userShaderNode
			//if nothing changed with appearance -no compile pass- we don't come in here again
			//ConsoleMessage ("have a shader of type %s",stringNodeType(p->userShaderNode->_nodeType));
			switch (p->userShaderNode->_nodeType) {
				case NODE_ComposedShader:
					if (X3D_COMPOSEDSHADER(p->userShaderNode)->isValid) {
						if (!X3D_COMPOSEDSHADER(p->userShaderNode)->_initialized) {
							sendInitialFieldsToShader(p->userShaderNode);
						}
					}
					break;
				case NODE_ProgramShader:
					if (X3D_PROGRAMSHADER(p->userShaderNode)->isValid) {
						if (!X3D_PROGRAMSHADER(p->userShaderNode)->_initialized) {
							sendInitialFieldsToShader(p->userShaderNode);
						}
					}

					break;
				case NODE_PackagedShader:
					if (X3D_PACKAGEDSHADER(p->userShaderNode)->isValid) {
						if (!X3D_PACKAGEDSHADER(p->userShaderNode)->_initialized) {
							sendInitialFieldsToShader(p->userShaderNode);
						}
					}

					break;
			}
		}
		//update effect field uniforms
		if(shader_requirements.effects){
			update_effect_uniforms();
		}


		#ifdef SHAPEOCCLUSION
		beginOcclusionQuery((struct X3D_VisibilitySensor*)node,renderstate()->render_geom); //BEGINOCCLUSIONQUERY;
		#endif
		//call stack to get from child_shape to sendMaterialsToShader and sendLightInfo as of Aug 13, 2016
		//child_shape
		//- render_node
		//-- render_indexedfaceset (or other specific shape)
		//--- render_polyrep
		//---- sendArraysToGPU (or sendElementsToGPU)
		//----- setupShader
		//------ sendMaterialsToShader
		//          Uniforms being sent for materials and hatching
		//--------- sendLightInfo
		//           Uniforms sent for lights
		//----- glDrawArrays/glDrawElements
		
		//we have a shader, now start sending it data
		//clear_bound_textures(); //testing only
		clear_textureUnit_used(); //appearance.texture material.textureXXX, PTMs.texture all need TEXTURE0+ XXX, where xxx starts from 0
		clear_material_samplers(); //PTM and material.textureXXX share frag shader sampler2D textureUnit[16] array
		textureTransform_start(); //send regular appearance.textures to shader
		resend_textureprojector_matrix();  
		setupShaderB();  //send materials, fill patters miscalaneous to shader
		//print_bound_textures("s"); //testing only, uncomment clear_bound_textues too
		render_node(tmpNG);

		//printf("%s",stringNodeType(tmpNG->_nodeType));
		//solid TRUE/FALSE on geom controls if backface culling
		reallyDraw();
		FW_GL_BINDBUFFER(GL_ARRAY_BUFFER, 0);
		FW_GL_BINDBUFFER(GL_ELEMENT_ARRAY_BUFFER, 0);
		textureTransform_end();

		#ifdef SHAPEOCCLUSION
		endOcclusionQuery((struct X3D_VisibilitySensor*)node,renderstate()->render_geom); //ENDOCCLUSIONQUERY;
		#endif

		//fin_Appearance
		if(node->appearance){
			struct X3D_Appearance *tmpA;
			POSSIBLE_PROTO_EXPANSION(struct X3D_Appearance *,node->appearance,tmpA);
			if(tmpA->effects.n)
				fin_sibAffectors(X3D_NODE(tmpA),&tmpA->effects);
		}

	}

	/* any shader turned on? if so, turn it off */

	//ConsoleMessage("turning shader off");
	finishedWithGlobalShader();
	//p->material_twoSided = NULL;
	//p->material_oneSided = NULL;
	p->userShaderNode = NULL;
	tg->RenderFuncs.shapenode = NULL;
    
	/* load the identity matrix for textures. This is necessary, as some nodes have TextureTransforms
		and some don't. So, if we have a TextureTransform, loadIdentity */
    
	if (p->this_textureTransform) {
		p->this_textureTransform = NULL;
		FW_GL_MATRIX_MODE(GL_TEXTURE);
		FW_GL_LOAD_IDENTITY();
		FW_GL_MATRIX_MODE(GL_MODELVIEW);
	}
    
	/* LineSet, PointSets, set the width back to the original. */
	{
		float gl_linewidth = tg->Mainloop.gl_linewidth;
		glLineWidth(gl_linewidth);
		p->appearanceProperties.pointSize = gl_linewidth;
	}

	/* did the lack of an Appearance or Material node turn lighting off? */
	LIGHTING_ON;

	/* turn off face culling */
	DISABLE_CULL_FACE;
}

void compile_Shape (struct X3D_Shape *node) {
	int whichAppearanceShader = 0;
	int whichShapeColorShader = 0;
	int whichShapeFogShader = 0;
	bool isUnlitGeometry = false;
	int hasTextureCoordinateGenerator = 0;
	int whichUnlitGeometry = 0;
	struct X3D_Node *tmpN = NULL;
	struct X3D_Node *tmpG = NULL;
	// struct X3D_Appearance *appearance = NULL;
	int userDefinedShader = 0;
//	int colorSource, alphaSource, channels, isLit;


	// ConsoleMessage ("**** Compile Shape ****");


	POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->geometry,tmpG);
	whichShapeColorShader = getShapeColourShader(tmpG);
	whichShapeFogShader = getShapeFogShader(tmpG);

	isUnlitGeometry = getIfLinePoints(tmpG);
	hasTextureCoordinateGenerator = getShapeTextureCoordGen(tmpG);

	POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->appearance,tmpN);


	/* first - does appearance have a shader node? */
	userDefinedShader = hasUserDefinedShader(tmpN);

	//Aug 26 2016 change: don't fiddle around with flags here, rather report the state of the node
	// - then do any fiddling in child_shape, when we have the channel count and modulation flag
	if(isUnlitGeometry)
		whichUnlitGeometry = HAVE_LINEPOINTS_COLOR;
	whichAppearanceShader = getAppearanceShader(tmpN);

	/* in case we had no appearance, etc, we do the bland NO_APPEARANCE_SHADER */
	//node->_shaderTableEntry= 
	node->_shaderflags_base = (whichShapeColorShader | whichShapeFogShader | whichAppearanceShader | 
				hasTextureCoordinateGenerator | whichUnlitGeometry ); //| userDefinedShader);
	node->_shaderflags_usershaders = userDefinedShader;
	node->_shaderflags_effects = 0;


	//if (node->_shaderTableEntry == NOTHING) 
	//	node->_shaderTableEntry = NO_APPEARANCE_SHADER;

	if (node->_shaderflags_base == NOTHING) 
		node->_shaderflags_base = NO_APPEARANCE_SHADER;


	//printf ("compile_Shape, node->_shaderTableEntry is %x\n",node->_shaderTableEntry);

	MARK_NODE_COMPILED
}
//void register_node_gc(void *node, void *p);

void compile_TwoSidedMaterial (struct X3D_TwoSidedMaterial *node) {
	float *p;
	struct fw_MaterialParameters *q;
	/* verify that the numbers are within range */
	node->ambientIntensity = fclamp(node->ambientIntensity,0.0f,1.0f);
	node->shininess = fclamp(node->shininess,0.0f,1.0f);
	node->transparency = fclamp(node->transparency,0.0f,1.0f);
	fvecclamp3f(node->diffuseColor.c,0.0f,1.0f);
	fvecclamp3f(node->emissiveColor.c,0.0f,1.0f);
	fvecclamp3f(node->specularColor.c,0.0f,1.0f);

	if(!node->_material){
		node->_material = malloc(sizeof(struct fw_MaterialParameters));
		register_node_gc(node,node->_material);
	}
	memset(node->_material,0,sizeof(struct fw_MaterialParameters));
	q = (struct fw_MaterialParameters *)node->_material;
	vecset3f(q->baseColor,1.0f,1.0f,1.0f); //saves boolean math in shader
	veccopy3f(q->diffuse,node->diffuseColor.c);
	veccopy3f(q->emissive,node->emissiveColor.c);
	veccopy3f(q->specular,node->specularColor.c);
	q->ambient = node->ambientIntensity;
	q->shininess = node->shininess;
	q->transparency = node->transparency;
	q->type = MAT_REGULAR;


	if (node->separateBackColor) {
		node->backAmbientIntensity = fclamp(node->backAmbientIntensity,0.0f,1.0f);
		node->backShininess = fclamp(node->backShininess,0.0f,1.0f);
		node->backTransparency = fclamp(node->backTransparency,0.0f,1.0f);
		fvecclamp3f(node->backDiffuseColor.c,0.0f,1.0f);
		fvecclamp3f(node->backEmissiveColor.c,0.0f,1.0f);
		fvecclamp3f(node->backSpecularColor.c,0.0f,1.0f);

		if(!node->_backMaterial){
			node->_backMaterial = malloc(sizeof(struct fw_MaterialParameters));
			register_node_gc(node,node->_backMaterial);
		}
		memset(node->_backMaterial,0,sizeof(struct fw_MaterialParameters));
		q = (struct fw_MaterialParameters *)node->_backMaterial;
		vecset3f(q->baseColor,1.0f,1.0f,1.0f); //saves boolean math in shader
		veccopy3f(q->diffuse,node->backDiffuseColor.c);
		veccopy3f(q->emissive,node->backEmissiveColor.c);
		veccopy3f(q->specular,node->backSpecularColor.c);
		q->ambient = node->backAmbientIntensity;
		q->shininess = node->backShininess;
		q->transparency = node->backTransparency;
		q->type = MAT_REGULAR;

	} else {
		/* just copy the front materials to the back */
		if(!node->_backMaterial){
			node->_backMaterial = malloc(sizeof(struct fw_MaterialParameters));
			register_node_gc(node,node->_backMaterial);
		}
		memset(node->_backMaterial,0,sizeof(struct fw_MaterialParameters));
		memcpy(node->_backMaterial,node->_material,sizeof(struct fw_MaterialParameters));
	}
	MARK_NODE_COMPILED
}

void render_TwoSidedMaterial (struct X3D_TwoSidedMaterial *node) {
	
	COMPILE_IF_REQUIRED
	{
		ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;
		if (node != NULL) {
			memcpy (&p->appearanceProperties.fw_FrontMaterial, node->_material, sizeof (struct fw_MaterialParameters));
			memcpy (&p->appearanceProperties.fw_BackMaterial, node->_backMaterial, sizeof (struct fw_MaterialParameters));
		}
	}
}

void compile_UnlitMaterial (struct X3D_UnlitMaterial *node) {
	struct X3D_Node **tnodes;
	struct fw_MaterialParameters *q;
	/* verify that the numbers are within range */
	node->transparency = fclamp(node->transparency,0.0f,1.0f);
	fvecclamp3f(node->emissiveColor.c,0.0f,1.0f);

	if(!node->_material){
		node->_material = malloc(sizeof(struct fw_MaterialParameters));
		register_node_gc(node,node->_material);
	}
	memset(node->_material,0,sizeof(struct fw_MaterialParameters));

	q = (struct fw_MaterialParameters *)node->_material;
	vecset3f(q->baseColor,1.0f,1.0f,1.0f); //saves boolean math in shader
	vecset3f(q->diffuse,1.0f,1.0f,1.0f); //saves boolean math in shader
	veccopy3f(q->emissive,node->emissiveColor.c);
	q->transparency = node->transparency;
	q->type = MAT_UNLIT;

	//new v4 textures
	// [0] normal [1] emissive [2] diffuse OR baseColor [3] specular/shiny OR metallic/roughness [4] ambient
	tnodes = q->textures;
	memset(tnodes,0,5*sizeof(void *));
	if(node->normalTexture)
	{
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->normalTexture,tnodes[0]);
	}
	if(node->emissiveTexture)
	{
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->emissiveTexture,tnodes[1]);
	}
	int *cindex = q->cindex;
	cindex[0] = node->normalTextureChannel;
	cindex[1] = node->emissiveTextureChannel;
	q->nt = 0; //assume no material.texturexxx to start
	for(int i=0;i<5;i++){
		q->tcount[i] = 0; //default: no texture for this material function
		q->tstart[i] = q->nt; //shader: start looping over tindex where we left off, for tcount loops
		if(tnodes[i]){
			if(tnodes[i]->_nodeType == NODE_MultiTexture) {
				struct X3D_MultiTexture *mt = (struct X3D_MultiTexture*)tnodes[i];
				q->tcount[i] = mt->texture.n;
				q->nt += mt->texture.n;
				q->mt++;
			}else{
				//single texture
				q->nt++;
				q->tcount[i] = 1;
			}
		}
	}
	MARK_NODE_COMPILED
}
void render_UnlitMaterial (struct X3D_UnlitMaterial *node) {
	COMPILE_IF_REQUIRED
	{
		ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;
		if (node != NULL) {
			if(get_isBackMaterial()){
				memcpy (&p->appearanceProperties.fw_BackMaterial, node->_material, sizeof (struct fw_MaterialParameters));
			}else{
				memcpy (&p->appearanceProperties.fw_FrontMaterial, node->_material, sizeof (struct fw_MaterialParameters));
			}
		}
	}
}


/*

PBR Physics Based Rendering
https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#materials 
- describes how to do BRDF calculations (don't I have a book on BRDF? with shaders?)
https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#appendix-b-brdf-implementation
- Appendix B shows the BRDF math, and link to example viewer implementation:
https://github.com/KhronosGroup/glTF-Sample-Viewer/ 
https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/master/src/shaders/metallic-roughness.frag
- implements BRDF in frag, including ifdefs for 'maps' vs scalars.
https://www.cs.virginia.edu/~jdl/bib/appearance/analytic%20models/schlick94b.pdf 
- Schlick BRDF model
example x3dom:
https://github.com/x3dom/x3dom/blob/master/src/nodes/Shape/PhysicalMaterial.js
exmaple CGE:
https://github.com/castle-engine/castle-engine/blob/master/src/x3d/opengl/glsl/source/lighting_model_physical/shading_phong.fs

H: specular-glossiness and metallic-roughness are different ways to declare the same thing
so only one is needed. And since web3d does specular-glossiness in the regular material, no need for it in the physical.

PhysicalMaterialNode:	
the textures are optional, and have specific packing of effects
occlusionRoughnessMetallicTexture  (occlusion=R,Roughness=G,Metallic=B)

There are a lot of (optional) textures with this, with the v4 extended Material node, and with PTM projective texture mapping.
IDEA: generalize what we did with PTM: 
- have a generic list of sampler2D textureUnit[xx] 
- and through a separate int32 array say which textureUnit goes with which texture.
That would allow combining PTM and (PhysicalMaterial or Matierial with textures) 
-- in a flexible way that minimizes (GPU limited resource) sampler2Ds

GPU textureUnits / samplers needed:
Gross: 7
max needed: 4 (assuming physics channel packing): normal, emissive, baseColor, occlusion-metallic-roughness
possible Array-ization assuming same widthxheight for all:
1 samplerArray for the physics, 1 sampler2D for baseColorTexture

*/
void compile_PhysicalMaterial (struct X3D_PhysicalMaterial *node) {
	struct X3D_Node **tnodes;
	struct fw_MaterialParameters *q;
	/* verify that the numbers are within range */
	node->roughness = fclamp(node->roughness,0.0f,1.0f);
	node->metallic = fclamp(node->metallic,0.0f,1.0f);
	node->transparency = fclamp(node->transparency,0.0f,1.0f);
	fvecclamp3f(node->baseColor.c,0.0f,1.0f);
	fvecclamp3f(node->emissiveColor.c,0.0f,1.0f);

	if(!node->_material){
		node->_material = malloc(sizeof(struct fw_MaterialParameters));
		register_node_gc(node,node->_material);
	}
	memset(node->_material,0,sizeof(struct fw_MaterialParameters));

	q = (struct fw_MaterialParameters *)node->_material;
	vecset3f(q->diffuse,1.0f,1.0f,1.0f); //saves boolean math in shader
	veccopy3f(q->baseColor,node->baseColor.c);
	veccopy3f(q->emissive,node->emissiveColor.c);
	q->metallic = node->metallic;
	q->roughness = node->roughness;
	q->transparency = node->transparency;
	q->type = MAT_PHYSICAL;

	//new v4 textures   
	// [0] normal [1] emissive [2] diffuse OR baseColor [3] specular/shiny OR metallic/roughness [4] ambient
	tnodes = q->textures;
	memset(tnodes,0,5*sizeof(void *)); 
	if(node->normalTexture)
	{
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->normalTexture,tnodes[0]);
	}
	if(node->emissiveTexture)
	{
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->emissiveTexture,tnodes[1]);
	}
	if(node->baseTexture)
	{
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->baseTexture,tnodes[2]);
	}
	if(node->metallicRoughnessTexture)
	{
		POSSIBLE_PROTO_EXPANSION(struct X3D_Node *, node->metallicRoughnessTexture,tnodes[3]);
	}
	int *cindex = q->cindex;
	cindex[0] = node->normalTextureChannel;
	cindex[1] = node->emissiveTextureChannel;
	cindex[2] = node->baseTextureChannel;
	cindex[3] = node->metallicRoughnessTextureChannel;
	q->nt = 0; //assume no material.texturexxx to start
	for(int i=0;i<5;i++){
		q->tcount[i] = 0; //default: no texture for this material function
		q->tstart[i] = q->nt; //shader: start looping over tindex where we left off, for tcount loops
		if(tnodes[i]){
			if(tnodes[i]->_nodeType == NODE_MultiTexture) {
				struct X3D_MultiTexture *mt = (struct X3D_MultiTexture*)tnodes[i];
				q->tcount[i] = mt->texture.n;
				q->nt += mt->texture.n;
				q->mt++;
			}else{
				//single texture
				q->nt++;
				q->tcount[i] = 1;
			}
		}
	}
	MARK_NODE_COMPILED
}

void render_PhysicalMaterial (struct X3D_PhysicalMaterial *node) {
	
	COMPILE_IF_REQUIRED
	{
		ppComponent_Shape p = (ppComponent_Shape)gglobal()->Component_Shape.prv;
		if (node != NULL) {
			if(get_isBackMaterial()){
				memcpy (&p->appearanceProperties.fw_BackMaterial, node->_material, sizeof (struct fw_MaterialParameters));
			}else{
				memcpy (&p->appearanceProperties.fw_FrontMaterial, node->_material, sizeof (struct fw_MaterialParameters));
			}
		}
	}
}

