

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include "../vrml_parser/Structs.h"
#include "../vrml_parser/CRoutes.h"
#include "../main/headers.h"

#include "../input/EAIHeaders.h"
#include "../input/EAIHelpers.h"
#include "../opengl/Frustum.h"
#include "../opengl/OpenGL_Utils.h"
#include "../opengl/Textures.h"

#include "Component_Networking.h"
#include "Children.h"
#include "../scenegraph/RenderFuncs.h"
#include "Polyrep.h"
#include <libFreeWRL.h>
#include <list.h>
#include <io_http.h>
#include "quaternion.h"

// GLTF
//https://github.com/jkuhlmann/cgltf 
//- include 100 line recursive json parser (how does data come out?) etc.
//- first 600 lines of header is API. next 4000 lines is CGLTF_IMPLEMENTATION
#define  CGLTF_IMPLEMENTATION 1
#include "cgltf.h"


// a list of loaded gltf file units, with one unit representing one .glb or one (.gltf,.bin)
// June 22, 2020: in theory this list should be per-execution_context (Scene, Inline, ProtoBody)
// as should texture unit array. For now, will be per freewrl main-scene-gglobal.
typedef struct gltf_unit {
	cgltf_data *data;	//parsed to cgltf nodes
	int bin_loaded;		// spawned x3d nodes that need buffer.data check this to know when they can 'compile'
	unsigned char *bin;	//where to place data for .bin resource loader, not used for .glb
	int bin_len;
	unsigned char *blob;//.glb blob includes .bin and textures, .gltf blob only json text and possible inlined textures
	int blob_len;
	Stack *bin_file_list;
} gltf_unit;

typedef struct pgltf_loader{
	Stack *gltf_units;
}* ppgltf_loader;
void *gltf_loader_constructor(){
	void *v = MALLOCV(sizeof(struct pgltf_loader));
	memset(v,0,sizeof(struct pgltf_loader));
	return v;
}
void gltf_loader_init(struct tgltf_loader *t){
	//public
	//private
	t->prv = gltf_loader_constructor();
	{
		ppgltf_loader p = (ppgltf_loader)t->prv;
		p->gltf_units = newStack(struct gltf_unit*);
	}
}
void gltf_loader_clear(struct tgltf_loader *t){
	//public
	//private
	{
		ppgltf_loader p = (ppgltf_loader)t->prv;
		//june 22, 2020 not done: detailed cleanup
		//cgltf_free(data);
		deleteStack(struct gltf_unit*,p->gltf_units);
	}
}
//ppgltf_loader p = (ppgltf_loader)gglobal()->gltf_loader.prv;


struct name_node {
char *name;
int nodeclass;
struct X3D_Node * node;
};
static Stack *defs = NULL;
struct X3D_Node *USE_node(char *name, int nodeclass){
	if(name){
		if(!defs) defs = newStack(struct name_node);
		struct name_node nn;
		for(int i=0;i<defs->n;i++){
			nn = vector_get(struct name_node,defs,i);
			if(!strcmp(name,nn.name) && nodeclass == nn.nodeclass){
				return nn.node;
			}
		}
	}
	return NULL;
}
struct X3D_Node *DEF_node(struct X3D_Node *ectx, char *name, int nodetype){
	if(!defs) defs = newStack(struct name_node);
	struct name_node nn;
	struct X3D_Node *node = createNewX3DNode(nodetype);
	add_node_to_broto_context(X3D_PROTO(ectx),X3D_NODE(node));
	if(name){
		nn.name = name;
		nn.nodeclass = getSAI_X3DNodeType(nodetype);
		nn.node = node;
		stack_push(struct name_node,defs,nn);
	}

return node;
}


void* set_MeshRep(void* _meshrep) {
	struct X3D_MeshRep* meshrep = NULL;
	//to be called from compile_BufferGeometry
	if (!_meshrep) {
		_meshrep = MALLOC(struct X3D_MeshRep*, sizeof(struct X3D_MeshRep));
		memset(_meshrep, 0, sizeof(struct X3D_MeshRep));
	}
	meshrep = (struct X3D_MeshRep*)_meshrep;
	meshrep->itype = 3; //0 meshrep 1 linerep 2 polyrep 3 meshrep
	meshrep->mode = 4; //0 Meshs 1-3 lines 4-6 mesh: 4 TRIANGLES 5 TRIANGLE_STRIP 6 TRIANGLE_FAN
	//buffer(buffersize) - unlike PointRep for x3d/x3dv nodes, PointSet 1:1 PointRep 1:1 geomBuffer
	// for gltf we assume BufferGeometry 1:1 MeshRep m:1 geomBuffer
	//npoints,attrib[]
	//nindex,index
	//set_geomBuffer(Meshrep->buffer);
	return meshrep;
}
#include "Component_Shape.h"
void render_MeshRep(void* _meshrep) {
	//like render_PointRep
	struct X3D_MeshRep* meshrep = (struct X3D_MeshRep*)_meshrep;
	struct geomBuffer* gb = meshrep->buffer;
	if (!gb || gb->VBO < 1) return;
	//old-stile GL_POINTS rendering - opengl generates point triangles in geometry shader automatically

	glBindBuffer(GL_ARRAY_BUFFER, gb->VBO);
	struct bufAccess* ba = &meshrep->attrib[0];
	FW_GL_VERTEX_POINTER(ba->dataSize, ba->dataType, ba->byteStride, (GLfloat*)BUFFER_OFFSET(ba->byteOffset)); //dataSize, dataType, stride, pointer
	//sendAttribToGPU(FW_VERTEX_POINTER_TYPE, dataSize, dataType, GL_FALSE, stride, pointer, 0, __FILE__, __LINE__);

	// do we have colours?
	ba = &meshrep->attrib[1];
	if (ba->in_use) {
		FW_GL_COLOR_POINTER(ba->dataSize, ba->dataType, ba->byteStride, (GLfloat*)BUFFER_OFFSET(ba->byteOffset)); //dataSize, dataType, stride, pointer
	}

	// do we have fogcoord?
	ba = &meshrep->attrib[2];
	if (ba->in_use) {
		FW_GL_FOG_POINTER(ba->dataType, ba->byteStride, (GLfloat*)BUFFER_OFFSET(ba->byteOffset)); //dataType, stride, pointer
	}
	// do we have normals?
	ba = &meshrep->attrib[3];
	if (ba->in_use) {
		FW_GL_NORMAL_POINTER(ba->dataType, ba->byteStride, (GLfloat*)BUFFER_OFFSET(ba->byteOffset));
	}

	// do we have UV?
	{
		//see also textureCoord_send 
		int kuv = 0;
		s_shader_capabilities_t* me;
		for (int j = 0; j < 4; j++) {
			ba = &meshrep->attrib[4 + j];

			if (ba->in_use) {
				kuv++;
				FW_GL_TEXCOORD_POINTER(ba->dataSize, ba->dataType, ba->byteStride, (GLfloat*)BUFFER_OFFSET(ba->byteOffset), j); //dataSize, dataType, stride, pointer, texID
			}
		}
		me = getAppearanceProperties()->currentShaderProperties;
		glUniform1i(me->nTexCoordChannels, kuv);  //PBR: send all you got, and say how many (channels)
	}

	// do we have indexes?
	ba = &meshrep->index;
	// don't send indexes as arrays - wait till draw command and pass as parameter
	//where do normals-per-face live?
	if (ba->in_use) {
		char* indices = meshrep->buffer->address + ba->byteOffset;
		//sendElementsToGPU(GL_TRIANGLES, ntri * 3, uindexs);  //WORKS
		//glDrawElements(	GL_TRIANGLES, ntri*3, GL_UNSIGNED_INT, indexs); //WORKS
		saveElementsForGPU0(GL_TRIANGLES, meshrep->nindex, ba->dataType, indices);
	}
	else {
		saveArraysForGPU(GL_TRIANGLES, 0, meshrep->ncoord);
		//sendArraysToGPU(GL_TRIANGLES, 0, meshrep->ncoord);
	}
	reallyDrawOnce();

	//printf for debugging accessors.
	if (0) {
		char* paddress;
		for (int i = 0; i < meshrep->ncoord; i++) {
			printf("%d [", i);
			for (int k = 0; k < 8; k++) {
				ba = &meshrep->attrib[k]; //point
				if (ba->in_use) {
					paddress = get_Attribi(ba, gb, i);;
					float* ai = (float*)paddress;
					printf(" [");
					for (int j = 0; j < ba->dataSize; j++)
						printf("%f ", ai[j]);
					printf("]");
				}
			}
			printf("\n");
		}
		printf("");
	}
}
void rendray_MeshRep(void* _meshrep) {
	struct X3D_MeshRep* mr = (struct X3D_MeshRep*)_meshrep;
	if (!mr) return;
	if (!mr->ncoord) return;

	struct geomBuffer* gb = mr->buffer;
	if (!gb || gb->loaded < 2) return;

	//this doesn't work with large pick rays for geo size scenes
	//struct X3D_Virt *virt;
	struct X3D_Node* genericNodePtr;
	int pt;
	float point[3][3];
	int cindex[3];
	struct point_XYZ v1, v2, v3;
	float pt1, pt2, pt3;
	struct point_XYZ hitpoint;
	float tmp1, tmp2;
	float v1len, v2len, v3len;
	float v12pt;
	struct point_XYZ t_r1, t_r2;
	struct bufAccess* bai, * ba;
	char* paddress;

	get_current_ray(&t_r1, &t_r2);
	bai = &mr->index;
	ba = &mr->attrib[0];

	int ntri = mr->ncoord / 3;
	if (mr->index.in_use) {
		bai = &mr->index;
		ntri = mr->nindex / 3;
		for (int i = 0, j=0; i < ntri; i++) {
			if (mr->index.in_use) {
				for (int k = 0; k < 3; k++) {
					paddress = get_Attribi(bai, gb, i * 3 + k);
					int ic;
					if (bai->dataType == GL_UNSIGNED_SHORT) {
						unsigned short* ip = (unsigned short*)paddress;
						ic = (int)(*ip);
					}
					else {
						int* ip = (int*)paddress;
						ic = (int)(*ip);
					}
					cindex[k] = ic;
				}
			}
			else {
				for (int k = 0; k < 3; k++)
					cindex[k] = i * 3 + k;

			}
			for (int k = 0; k < 3; k++) {
				paddress = get_Attribi(ba, gb, cindex[k]);
				veccopy3f(point[k], (float*)paddress);
			}
			//intersect ray with triangle

			/*
			printf ("have points (%f %f %f) (%f %f %f) (%f %f %f)\n",
				point[0][0],point[0][1],point[0][2],
				point[1][0],point[1][1],point[1][2],
				point[2][0],point[2][1],point[2][2]);
			*/

			/* First we need to project our point to the surface */
			/* Poss. 1: */
			/* Solve s1xs2 dot ((1-r)r1 + r r2 - pt0)  ==  0 */
			/* I.e. calculate s1xs2 and ... */
			v1.x = point[1][0] - point[0][0];
			v1.y = point[1][1] - point[0][1];
			v1.z = point[1][2] - point[0][2];
			v2.x = point[2][0] - point[0][0];
			v2.y = point[2][1] - point[0][1];
			v2.z = point[2][2] - point[0][2];
			v1len = (float)sqrt(VECSQ(v1)); VECSCALE(v1, 1 / v1len);
			v2len = (float)sqrt(VECSQ(v2)); VECSCALE(v2, 1 / v2len);
			v12pt = (float)VECPT(v1, v2);

			/* this will get around a divide by zero further on JAS */
			if (fabs(v12pt - 1.0) < 0.00001) continue;

			/* if we have a degenerate triangle, we can't compute a normal, so skip */
			if ((fabs(v1len) > 0.00001) && (fabs(v2len) > 0.00001)) {

				/* v3 is our normal to the surface */
				VECCP(v1, v2, v3);
				v3len = (float)sqrt(VECSQ(v3)); VECSCALE(v3, 1 / v3len);
				pt1 = (float)VECPT(t_r1, v3);
				pt2 = (float)VECPT(t_r2, v3);
				pt3 = (float)(v3.x * point[0][0] + v3.y * point[0][1] + v3.z * point[0][2]);
				/* Now we have (1-r)pt1 + r pt2 - pt3 = 0
				 * r * (pt1 - pt2) = pt1 - pt3
				 */
				tmp1 = pt1 - pt2;
				if (!APPROX(tmp1, 0)) {
					float ra, rb;
					float k, l;
					struct point_XYZ p0h;

					tmp2 = (float)((pt1 - pt3) / (pt1 - pt2));
					hitpoint.x = MRATX(tmp2);
					hitpoint.y = MRATY(tmp2);
					hitpoint.z = MRATZ(tmp2);
					/* Now we want to see if we are in the triangle */
					/* Projections to the two triangle sides */
					p0h.x = hitpoint.x - point[0][0];
					p0h.y = hitpoint.y - point[0][1];
					p0h.z = hitpoint.z - point[0][2];
					ra = (float)VECPT(v1, p0h);
					if (ra < 0.0f) { continue; }
					rb = (float)VECPT(v2, p0h);
					if (rb < 0.0f) { continue; }
					/* Now, the condition for the point to
					 * be inside
					 * (ka + lb = p)
					 * (k + l b.a = p.a)
					 * (k b.a + l = p.b)
					 * (k - (b.a)**2 k = p.a - (b.a)*p.b)
					 * k = (p.a - (b.a)*(p.b)) / (1-(b.a)**2)
					 */
					k = (ra - v12pt * rb) / (1 - v12pt * v12pt);
					l = (rb - v12pt * ra) / (1 - v12pt * v12pt);
					k /= v1len; l /= v2len;
					if (k + l > 1 || k < 0 || l < 0) {
						continue;
					}
					rayhit(((float)(tmp2)),
						((float)(hitpoint.x)),
						((float)(hitpoint.y)),
						((float)(hitpoint.z)),
						((float)(v3.x)),
						((float)(v3.y)),
						((float)(v3.z)),
						((float)-1), ((float)-1), "polyrep");
				}
				/*
				} else {
					printf ("render_ray_polyrep, skipping degenerate triangle\n");
				*/
			}
		}
	}
}
void delete_MeshRep(void* meshrep) {
	//like delete_PointRep
	struct X3D_MeshRep* mr;
	mr = (struct X3D_MeshRep*)meshrep;
	subtract_geomBufferUser(mr->buffer);
	FREE_IF_NZ(mr);
}

int parse_gltf_node(struct X3D_Node *ectx, struct X3D_Node **spot, cgltf_data * data, cgltf_node *node, gltf_unit *unit){
// june 22, 2020 not done: skinned / rigged animated charactors, points, lines and various things noted below.
// generally we got glb and gltf+bin to load and render a bit - a proof of concept.
// biggest thing left: inline and in-bin textures - do we need a BufferTexture node (to bypass freewrl spaghetti code)?
	//content part
	int show = FALSE;
	struct Vector vector;
	struct Vector* pp = &vector;
	memset(pp, 0, sizeof(struct Vector));

	if(node->camera){
		//june 22, 2020 not done: add viewpoint here
		struct X3D_Node* viewpoint = NULL;
		cgltf_camera* camera = node->camera;
		viewpoint = (struct X3D_Node*)USE_node(camera->name, X3DBindableNode);
		if (!viewpoint) {
			if (camera->type == cgltf_camera_type_perspective) {
				struct X3D_Viewpoint* vp = (struct X3D_Viewpoint*)DEF_node(ectx, camera->name, NODE_Viewpoint);
				cgltf_camera_perspective perspective = camera->data.perspective;
				vp->fieldOfView = perspective.yfov;
				vp->aspectRatio = perspective.aspect_ratio > 0.0f ? perspective.aspect_ratio : .75;
				vp->farClippingPlane = perspective.zfar;
				vp->nearClippingPlane = perspective.znear;
				vp->description = newASCIIString(camera->name);
				vecset3f(vp->position.c, 0.0f, 0.0f, 0.0f); //let the transform position (default is 0 0 10)
				//vp->navigationInfo = createNewX3DNode(NODE_NavigationInfo);
				viewpoint = X3D_NODE(vp);
				//printf("vp desc %s", vp->description->strptr);
			}
			else if (camera->type == cgltf_camera_type_orthographic) {
				struct X3D_OrthoViewpoint* vp = (struct X3D_OrthoViewpoint*)DEF_node(ectx, camera->name, NODE_OrthoViewpoint);
				cgltf_camera_orthographic ortho = camera->data.orthographic;
				vp->fieldOfView.p[0] *= ortho.xmag;
				vp->fieldOfView.p[1] *= ortho.ymag;
				vp->fieldOfView.p[2] *= ortho.xmag;
				vp->fieldOfView.p[3] *= ortho.ymag;
				vp->farClippingPlane = ortho.zfar;
				vp->nearClippingPlane = ortho.znear;
				vp->description = newASCIIString(camera->name);
				vecset3f(vp->position.c, 0.0f, 0.0f, 0.0f); //let the transform position (default is 0 0 10)
				//vp->navigationInfo = createNewX3DNode(NODE_NavigationInfo);
				viewpoint = X3D_NODE(vp);
				//printf("vp desc %s", vp->description->strptr);
				//printf("orth");
			}
		}
		if (viewpoint) {
			vector_pushBack(struct X3D_Node*, pp, viewpoint);
			//printf("adding viewpoint\n");
		}

	}
	if (node->light) {
		//m++;
		//p = realloc(p,m*sizeof(struct X3D_Node*));
		// june 22, 2020 not done: add punctual (directional, point, spot) light here
		// - not done and not supported yet EnvironmentLight
		//cgltf_light_type_invalid, 0
		//cgltf_light_type_directional, 1
		//cgltf_light_type_point, 2
		//cgltf_light_type_spot, 3

		struct X3D_Node* light = NULL;
		cgltf_light* plight = node->light;
		light = (struct X3D_Node*)USE_node(plight->name, X3DLightNode);
		if (!light) {
			if (node->light->type > cgltf_light_type_invalid) {
				//cgltf_light
				//char* name;
				//cgltf_float color[3];
				//cgltf_float intensity;
				//cgltf_light_type type;
				//cgltf_float range;
				//cgltf_float spot_inner_cone_angle;
				//cgltf_float spot_outer_cone_angle;

				switch (node->light->type) {
				case cgltf_light_type_directional:
				{
					struct X3D_DirectionalLight* dl = (struct X3D_DirectionalLight*)DEF_node(ectx, plight->name, NODE_DirectionalLight);
					veccopy3f(dl->color.c, plight->color);
					dl->intensity = plight->intensity;
					light = X3D_NODE(dl);
				}
				break;
				case cgltf_light_type_point:
				{
					struct X3D_PointLight* pl = (struct X3D_PointLight*)DEF_node(ectx, plight->name, NODE_PointLight);
					veccopy3f(pl->color.c, plight->color);
					pl->intensity = plight->intensity;
					pl->radius = plight->range;
					light = X3D_NODE(pl);
				}
				break;
				case cgltf_light_type_spot:
				{
					struct X3D_SpotLight* sl = (struct X3D_SpotLight*)DEF_node(ectx, plight->name, NODE_SpotLight);
					veccopy3f(sl->color.c, plight->color);
					sl->intensity = plight->intensity;
					sl->radius = plight->range;
					sl->beamWidth = plight->spot_inner_cone_angle;
					sl->cutOffAngle = plight->spot_outer_cone_angle;
					light = X3D_NODE(sl);
				}

				default:
					break;
				}
			}
			if (light) {
				vector_pushBack(struct X3D_Node*, pp, light);
				//printf("adding light\n");
			}
		}
	}
	if(node->mesh){
		//gltf mesh is like our shape: it refers to material and to geometry/accessor
		//we unconditionally add a Shape node, even if appearance and geometry are null
		struct X3D_Shape *sn = (struct X3D_Shape*) USE_node(node->mesh->name,X3DBoundedObject);
		if(!sn){
			sn = (struct X3D_Shape*) DEF_node(ectx,node->mesh->name,NODE_Shape);
			for(int j=0;j<node->mesh->primitives_count;j++){
				cgltf_primitive *prim = &node->mesh->primitives[j];
				if(prim->material){
					//typedef struct cgltf_material
					//{
					//	char* name;
					//	cgltf_bool has_pbr_metallic_roughness;
					//	cgltf_bool has_pbr_specular_glossiness;
					//	cgltf_bool has_clearcoat;
					//	cgltf_pbr_metallic_roughness pbr_metallic_roughness;
					//	cgltf_pbr_specular_glossiness pbr_specular_glossiness;
					//	cgltf_clearcoat clearcoat;
					//	cgltf_texture_view normal_texture;
					//	cgltf_texture_view occlusion_texture;
					//	cgltf_texture_view emissive_texture;
					//	cgltf_float emissive_factor[3];
					//	cgltf_alpha_mode alpha_mode;
					//	cgltf_float alpha_cutoff;
					//	cgltf_bool double_sided;
					//	cgltf_bool unlit;
					//	cgltf_extras extras;
					//} cgltf_material;		
					// june 22, 2020: not done: normal texture (supported by freewrl), occlusion texture (not supported)
					// - and see below for material-type=specific not-dones.
					if(prim->material->unlit){
						int mtype = NODE_UnlitMaterial;
						struct X3D_UnlitMaterial* mat = (struct X3D_UnlitMaterial*) USE_node(prim->material->name,X3DMaterialNode);
						if(!mat){
							mat = (struct X3D_UnlitMaterial*) DEF_node(ectx,prim->material->name,mtype);
							veccopy3f(mat->emissiveColor.c,prim->material->emissive_factor);
							//mat->emissiveTextureChannel 
							if(prim->material->emissive_texture.texture->image->buffer_view) { //->buffer->data){
								if(show) printf("image loaded for us\n");
							}else{
								if(show) printf("image not loaded uri = %s\n",prim->material->emissive_texture.texture->image->uri);
								if (prim->material->emissive_texture.texture) {
									if (prim->material->emissive_texture.texture->image->buffer_view) { //->buffer->data){
										if (show) printf("image loaded for us - in theory a PixelTexture\n");
									}
									else {
										char* iname = prim->material->emissive_texture.texture->image->name;
										char* iuri = prim->material->emissive_texture.texture->image->uri;
										if (show) printf("image not loaded uri = %s\n", iuri);
										struct X3D_Node* image = USE_node(iname, X3DTextureNode);
										if (!image) {
											image = DEF_node(ectx, iname, NODE_ImageTexture);
											struct X3D_ImageTexture* it = (struct X3D_ImageTexture*)image;
											it->url.p = malloc(sizeof(void*));
											it->url.p[0] = newASCIIString(iuri);
											it->url.n = 1;
										}
										mat->emissiveTexture = image;
									}
								}

							}
						}
						sn->appearance = createNewX3DNode(NODE_Appearance);
						X3D_APPEARANCE(sn->appearance)->material = X3D_NODE(mat);
					}else if(prim->material->has_pbr_metallic_roughness){
						int mtype = NODE_PhysicalMaterial;
						struct X3D_PhysicalMaterial* mat = (struct X3D_PhysicalMaterial*) USE_node(prim->material->name,X3DMaterialNode);
						if(!mat){
							//typedef struct cgltf_pbr_metallic_roughness
							//{
							//	cgltf_texture_view base_color_texture;
							//	cgltf_texture_view metallic_roughness_texture;
							//
							//	cgltf_float base_color_factor[4];
							//	cgltf_float metallic_factor;
							//	cgltf_float roughness_factor;
							//
							//	cgltf_extras extras;
							//} cgltf_pbr_metallic_roughness;
							// june 22, 2020 done: url loaded texture
							// - not done: inline (base64 for gltf) / in-bin textures (for .glb)
							cgltf_pbr_metallic_roughness *pbr = &prim->material->pbr_metallic_roughness;
							mat = (struct X3D_PhysicalMaterial*) DEF_node(ectx,prim->material->name,mtype);
							veccopy3f(mat->emissiveColor.c,prim->material->emissive_factor);
							veccopy3f(mat->baseColor.c,pbr->base_color_factor);
							mat->transparency = 1.0f - pbr->base_color_factor[3];
							mat->metallic = pbr->metallic_factor;
							mat->roughness = pbr->roughness_factor;
							if(pbr->base_color_texture.texture){
								if(pbr->base_color_texture.texture->image->buffer_view){ //->buffer->data){
									if(show) printf("image loaded for us\n");
								}else{
									char* iname = pbr->base_color_texture.texture->image->name;
									char* iuri = pbr->base_color_texture.texture->image->uri;
									if (show) printf("image not loaded uri = %s\n", iuri);
									struct X3D_Node* image = USE_node(iname, X3DTextureNode);
									if (!image) {
										image = DEF_node(ectx, iname, NODE_ImageTexture);
										struct X3D_ImageTexture* it = (struct X3D_ImageTexture*)image;
										it->url.p = malloc(sizeof(void*));
										it->url.p[0] = newASCIIString(iuri);
										it->url.n = 1;
									}
									mat->baseTexture = image;
								}
							}
							if (pbr->metallic_roughness_texture.texture) {
								if (pbr->metallic_roughness_texture.texture->image->buffer_view) { //->buffer->data){
									if (show) printf("image loaded for us - in theory a PixelTexture\n");
								}
								else {
									char* iname = pbr->metallic_roughness_texture.texture->image->name;
									char* iuri = pbr->metallic_roughness_texture.texture->image->uri;
									if (show) printf("image not loaded uri = %s\n", iuri);
									struct X3D_Node* image = USE_node(iname, X3DTextureNode);
									if (!image) {
										image = DEF_node(ectx, iname, NODE_ImageTexture);
										struct X3D_ImageTexture* it = (struct X3D_ImageTexture*)image;
										it->url.p = malloc(sizeof(void*));
										it->url.p[0] = newASCIIString(iuri);
										it->url.n = 1;
									}
									mat->metallicRoughnessTexture = image;
									mat->metallicRoughnessTextureMapping = newASCIIString("one");

								}
							}
							if (prim->material->emissive_texture.texture) {
								if (prim->material->emissive_texture.texture->image->buffer_view) { //->buffer->data){
									if (show) printf("image loaded for us - in theory a PixelTexture\n");
								}
								else {
									char* iname = prim->material->emissive_texture.texture->image->name;
									char* iuri = prim->material->emissive_texture.texture->image->uri;
									if (show) printf("image not loaded uri = %s\n", iuri);
									struct X3D_Node* image = USE_node(iname, X3DTextureNode);
									if (!image) {
										image = DEF_node(ectx, iname, NODE_ImageTexture);
										struct X3D_ImageTexture* it = (struct X3D_ImageTexture*)image;
										it->url.p = malloc(sizeof(void*));
										it->url.p[0] = newASCIIString(iuri);
										it->url.n = 1;
									}
									mat->emissiveTexture = image;
									mat->emissiveTextureMapping = newASCIIString("one");

								}
							}
							if (prim->material->normal_texture.texture) {
								if (prim->material->normal_texture.texture->image->buffer_view) { //->buffer->data){
									if (show) printf("image loaded for us - in theory a PixelTexture\n");
								}
								else {
									char* iname = prim->material->normal_texture.texture->image->name;
									char* iuri = prim->material->normal_texture.texture->image->uri;
									if (show) printf("image not loaded uri = %s\n", iuri);
									struct X3D_Node* image = USE_node(iname, X3DTextureNode);
									if (!image) {
										image = DEF_node(ectx, iname, NODE_ImageTexture);
										struct X3D_ImageTexture* it = (struct X3D_ImageTexture*)image;
										it->url.p = malloc(sizeof(void*));
										it->url.p[0] = newASCIIString(iuri);
										it->url.n = 1;
									}
									mat->normalTexture = image;
									mat->normalTextureMapping = newASCIIString("one");

								}
							}
							if (prim->material->occlusion_texture.texture) {
								if (prim->material->occlusion_texture.texture->image->buffer_view) { //->buffer->data){
									if (show) printf("image loaded for us - in theory a PixelTexture\n");
								}
								else {
									char* iname = prim->material->occlusion_texture.texture->image->name;
									char* iuri = prim->material->occlusion_texture.texture->image->uri;
									if (show) printf("image not loaded uri = %s\n", iuri);
									struct X3D_Node* image = USE_node(iname, X3DTextureNode);
									if (!image) {
										image = DEF_node(ectx, iname, NODE_ImageTexture);
										struct X3D_ImageTexture* it = (struct X3D_ImageTexture*)image;
										it->url.p = malloc(sizeof(void*));
										it->url.p[0] = newASCIIString(iuri);
										it->url.n = 1;
									}
									mat->occlusionTexture = image;
									mat->occlusionTextureMapping = newASCIIString("one");

								}
							}


						}
						sn->appearance = createNewX3DNode(NODE_Appearance);
						X3D_APPEARANCE(sn->appearance)->material = X3D_NODE(mat);
					}else if(prim->material->has_pbr_specular_glossiness){
						int mtype = NODE_Material;
						struct X3D_Material* mat = (struct X3D_Material*) USE_node(prim->material->name,X3DMaterialNode);
						if(!mat){
							//typedef struct cgltf_pbr_specular_glossiness
							//{
							//	cgltf_texture_view diffuse_texture;
							//	cgltf_texture_view specular_glossiness_texture;
							//
							//	cgltf_float diffuse_factor[4];
							//	cgltf_float specular_factor[3];
							//	cgltf_float glossiness_factor;
							//} cgltf_pbr_specular_glossiness;
							// june 22, 2020 not done: textures for diffuse and specular_glossiness
							// neither url nor inline / in-bin handled
							mat = (struct X3D_Material*) DEF_node(ectx,prim->material->name,mtype);
							cgltf_pbr_specular_glossiness *pbr = &prim->material->pbr_specular_glossiness;
							veccopy3f(mat->emissiveColor.c,prim->material->emissive_factor);
							veccopy3f(mat->diffuseColor.c,pbr->diffuse_factor);
							veccopy3f(mat->specularColor.c,pbr->specular_factor);
							mat->shininess = pbr->glossiness_factor;
							mat->transparency = 1.0f - pbr->diffuse_factor[3];

							if (pbr->specular_glossiness_texture.texture) {
								if (pbr->diffuse_texture.texture->image->buffer_view) { //->buffer->data){
									if (show) printf("image loaded for us - in theory a PixelTexture\n");
								}
								else {
									char* iname = pbr->specular_glossiness_texture.texture->image->name;
									char* iuri = pbr->specular_glossiness_texture.texture->image->uri;
									if (show) printf("image not loaded uri = %s\n", iuri);
									struct X3D_Node* image = USE_node(iname, X3DTextureNode);
									if (!image) {
										image = DEF_node(ectx, iname, NODE_ImageTexture);
										struct X3D_ImageTexture* it = (struct X3D_ImageTexture*)image;
										it->url.p = malloc(sizeof(void*));
										it->url.p[0] = newASCIIString(iuri);
										it->url.n = 1;
									}
									mat->specularTexture = image;
									mat->shininessTexture = image;
									mat->specularTextureMapping = newASCIIString("one");
									mat->shininessTextureMapping = newASCIIString("one");

								}
							}

							if (pbr->diffuse_texture.texture) {
								if (pbr->diffuse_texture.texture->image->buffer_view) { //->buffer->data){
									if (show) printf("image loaded for us - in theory a PixelTexture\n");
								}
								else {
									char* iname = pbr->diffuse_texture.texture->image->name;
									char* iuri = pbr->diffuse_texture.texture->image->uri;
									if (show) printf("image not loaded uri = %s\n", iuri);
									struct X3D_Node* image = USE_node(iname, X3DTextureNode);
									if (!image) {
										image = DEF_node(ectx, iname, NODE_ImageTexture);
										struct X3D_ImageTexture* it = (struct X3D_ImageTexture*)image;
										it->url.p = malloc(sizeof(void*));
										it->url.p[0] = newASCIIString(iuri);
										it->url.n = 1;
									}
									mat->diffuseTexture = image;
									mat->diffuseTextureMapping = newASCIIString("one");

								}
							}

							if (prim->material->emissive_texture.texture) {
								if (prim->material->emissive_texture.texture->image->buffer_view) { //->buffer->data){
									if (show) printf("image loaded for us - in theory a PixelTexture\n");
								}
								else {
									char* iname = prim->material->emissive_texture.texture->image->name;
									char* iuri = prim->material->emissive_texture.texture->image->uri;
									if (show) printf("image not loaded uri = %s\n", iuri);
									struct X3D_Node* image = USE_node(iname, X3DTextureNode);
									if (!image) {
										image = DEF_node(ectx, iname, NODE_ImageTexture);
										struct X3D_ImageTexture* it = (struct X3D_ImageTexture*)image;
										it->url.p = malloc(sizeof(void*));
										it->url.p[0] = newASCIIString(iuri);
										it->url.n = 1;
									}
									mat->emissiveTexture = image;
									mat->emissiveTextureMapping = newASCIIString("one");

								}
							}

							if (prim->material->normal_texture.texture) {
								if (prim->material->normal_texture.texture->image->buffer_view) { //->buffer->data){
									if (show) printf("image loaded for us - in theory a PixelTexture\n");
								}
								else {
									char* iname = prim->material->normal_texture.texture->image->name;
									char* iuri = prim->material->normal_texture.texture->image->uri;
									if (show) printf("image not loaded uri = %s\n", iuri);
									struct X3D_Node* image = USE_node(iname, X3DTextureNode);
									if (!image) {
										image = DEF_node(ectx, iname, NODE_ImageTexture);
										struct X3D_ImageTexture* it = (struct X3D_ImageTexture*)image;
										it->url.p = malloc(sizeof(void*));
										it->url.p[0] = newASCIIString(iuri);
										it->url.n = 1;
									}
									mat->normalTexture = image;
									mat->normalTextureMapping = newASCIIString("one");

								}
							}

							if (prim->material->occlusion_texture.texture) {
								if (prim->material->occlusion_texture.texture->image->buffer_view) { //->buffer->data){
									if (show) printf("image loaded for us - in theory a PixelTexture\n");
								}
								else {
									char* iname = prim->material->occlusion_texture.texture->image->name;
									char* iuri = prim->material->occlusion_texture.texture->image->uri;
									if (show) printf("image not loaded uri = %s\n", iuri);
									struct X3D_Node* image = USE_node(iname, X3DTextureNode);
									if (!image) {
										image = DEF_node(ectx, iname, NODE_ImageTexture);
										struct X3D_ImageTexture* it = (struct X3D_ImageTexture*)image;
										it->url.p = malloc(sizeof(void*));
										it->url.p[0] = newASCIIString(iuri);
										it->url.n = 1;
									}
									mat->occlusionTexture = image;
									mat->occlusionTextureMapping = newASCIIString("one");

								}
							}
							if (prim->material->emissive_texture.texture) {
								if (prim->material->emissive_texture.texture->image->buffer_view) { //->buffer->data){
									if (show) printf("image loaded for us - in theory a PixelTexture\n");
								}
								else {
									char* iname = prim->material->emissive_texture.texture->image->name;
									char* iuri = prim->material->emissive_texture.texture->image->uri;
									if (show) printf("image not loaded uri = %s\n", iuri);
									struct X3D_Node* image = USE_node(iname, X3DTextureNode);
									if (!image) {
										image = DEF_node(ectx, iname, NODE_ImageTexture);
										struct X3D_ImageTexture* it = (struct X3D_ImageTexture*)image;
										it->url.p = malloc(sizeof(void*));
										it->url.p[0] = newASCIIString(iuri);
										it->url.n = 1;
									}
									mat->emissiveTexture = image;
									mat->emissiveTextureMapping = newASCIIString("one");
								}
							}



						}
						sn->appearance = createNewX3DNode(NODE_Appearance);
						X3D_APPEARANCE(sn->appearance)->material = X3D_NODE(mat);
						if (0) {
							//experiment to flip texture vertically
							struct X3D_TextureTransform* tt = createNewX3DNode(NODE_TextureTransform);
							vecset2f(tt->scale.c, 1.0f, -1.0f);
							tt->mapping = newASCIIString("one");
							X3D_APPEARANCE(sn->appearance)->textureTransform = X3D_NODE(tt);
						}

					} 

				}
				// https://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/rendering.html
				// https://www.khronos.org/files/gltf20-reference-guide.pdf
				struct X3D_Node *gn = NULL;
				switch(prim->type){
					case cgltf_primitive_type_points:
					case cgltf_primitive_type_lines:
					case cgltf_primitive_type_line_loop:
					case cgltf_primitive_type_line_strip:
						break;
					case cgltf_primitive_type_triangles:
					{
						cgltf_float element_float[16];
						int acount = prim->attributes_count;
						if(show){
							//printout to help get the idea of whats in the structs
							printf("triangles\n");
							for(int ii=0;ii<acount;ii++){
								printf("attr %s indx %d ",prim->attributes[ii].name,prim->attributes[ii].index);
								switch(prim->attributes[ii].type){
									case cgltf_attribute_type_invalid: printf("invalid");break;
									case cgltf_attribute_type_position: printf("position");break;
									case cgltf_attribute_type_normal: printf("normal");break;
									case cgltf_attribute_type_tangent: printf("tangent");break;
									case cgltf_attribute_type_texcoord: printf("texcoord");break;
									case cgltf_attribute_type_color: printf("color");break;
									case cgltf_attribute_type_joints: printf("joints");break;
									case cgltf_attribute_type_weights: printf("weights");break;
									default: break;
								}
							
								const cgltf_accessor* blob = prim->attributes[ii].data;
								cgltf_size nfloats = cgltf_num_components(blob->type) * blob->count;
								printf(" nfloats = %d accessor type %d count %d ",(int)nfloats,blob->type, (int)blob->count);
								switch(blob->type){
									case cgltf_type_scalar: printf("SCALAR");break;
									case cgltf_type_vec2: printf("VEC2");break;
									case cgltf_type_vec3: printf("VEC3");break;
									default: break;
								}
								printf("\n");
								cgltf_float element_float[16];
								for (cgltf_size index = 0; index < blob->count; index++)
								{
									cgltf_accessor_read_float(blob, index, element_float, 16);
									printf("%d %f %f %f\n",(int)index,element_float[0],element_float[1],element_float[2]);
								}
							}
							{
								// indexes for indexedtriangleset
								const cgltf_accessor* blob = prim->indices;
								cgltf_uint element_int;
								printf("triangle indices\n");
								int ntri = blob->count / 3;
								for (int i = 0; i < ntri; i++)
								{
									printf("%d [",i);
									for(int j=0;j<3;j++){
										int index = (i*3)+j;
										cgltf_accessor_read_uint(blob, index, &element_int, 1);
										printf("%d ",element_int);
									}
									printf("]\n");
								}
							}
						}
						gn = createNewX3DNode(NODE_BufferGeometry); //NODE_TriangleSet);
						add_node_to_broto_context(X3D_PROTO(ectx),X3D_NODE(gn));
						sn->geometry = gn;
						struct X3D_BufferGeometry *ts = (struct X3D_BufferGeometry*)gn;
						int lookup_attrib_index[] = {
							//typedef enum cgltf_attribute_type
							//{
						-1,	//	cgltf_attribute_type_invalid, //0
						0,	//	cgltf_attribute_type_position, //1
						3,	//	cgltf_attribute_type_normal, //2
						-1,	//	cgltf_attribute_type_tangent, //3
						4,	//4-7	cgltf_attribute_type_texcoord, //4
						1,	//	cgltf_attribute_type_color, //5
						-1,	//	cgltf_attribute_type_joints, //6
						-1,	//	cgltf_attribute_type_weights, //7
						2, //fog not mentioned in cgltf
						};
						int lookup_GL_type[] = {
						-1,					//0	 cgltf_component_type_invalid,
						GL_BYTE,			//1	cgltf_component_type_r_8, /* BYTE */
						GL_UNSIGNED_BYTE,	//2 cgltf_component_type_r_8u, /* UNSIGNED_BYTE */
						GL_SHORT,			//3	cgltf_component_type_r_16, /* SHORT */
						GL_UNSIGNED_SHORT,	//4	cgltf_component_type_r_16u, /* UNSIGNED_SHORT */
						GL_UNSIGNED_INT,	//5	cgltf_component_type_r_32u, /* UNSIGNED_INT */
						GL_FLOAT,			//6	cgltf_component_type_r_32f, /* FLOAT */
						GL_DOUBLE,			//7 not convered by cgltf
						};
						struct X3D_MeshRep* mr;
						mr = set_MeshRep(NULL);
						ts->_intern = (struct X3D_GeomRep*)mr;
						//set per-vertex attributes (coord, color, fog, normal, UV[0-4]) 
						acount = prim->attributes_count;
						for (int ii = 0; ii < acount; ii++) {
							const cgltf_accessor* blob = prim->attributes[ii].data;
							int iat = lookup_attrib_index[prim->attributes[ii].type];
							if (iat < 0) continue;
							if (iat == 0) {
								//vertex accessor, take the vertex count
								mr->ncoord = blob->count;
							}
							struct bufAccess* ba = &mr->attrib[iat];
							//Q. which buffer? It might already be allocated. 
							// in freewrl we allocate once

							mr->buffer = find_buffer_in_broto_context_from_cgltf_buffer(ectx, blob->buffer_view->buffer);
							if (!mr->buffer) {
								//first use of buffer, allocate
								mr->buffer = add_geomBuffer0(ectx,blob->buffer_view->buffer->size, 1);
								mr->buffer->cgltf_buffer = (char*)blob->buffer_view->buffer;
								if (blob->buffer_view->buffer->data) {
									memcpy(mr->buffer->address, blob->buffer_view->buffer->data, blob->buffer_view->buffer->size);
									mr->buffer->loaded = 1;
									mr->buffer->cgltf_buffer = (char*) blob->buffer_view->buffer;
									// can't do in this thread, wait for compile_BufferGeometry set_geomBuffer(mr->buffer);
								} else {
									//how / where do we connect this to uri loading via resource fetch?
								}
							} else {
								add_geomBufferUser(mr->buffer);
							}
							ba->byteOffset = blob->buffer_view->offset;
							ba->dataSize = cgltf_num_components(blob->type);
							ba->dataType = lookup_GL_type[blob->component_type];
							ba->byteStride = blob->stride; //the bufferView also has a stride
							ba->byteSize = ba->dataSize * lookup_dataType_size(ba->dataType);
							ba->in_use = 1;
						}
						{
							// indexes for indexedtriangleset
							const cgltf_accessor* blob = prim->indices;
							struct bufAccess* ba = &mr->index;
							ba->byteOffset = blob->buffer_view->offset;
							ba->dataSize = cgltf_num_components(blob->type);
							ba->dataType = lookup_GL_type[blob->component_type];
							ba->byteStride = blob->stride; //the bufferView also has a stride
							ba->byteSize = ba->dataSize * lookup_dataType_size(ba->dataType);
							ba->in_use = 1;
							mr->nindex = blob->count;
						}
						if (prim->attributes->data->has_max && prim->attributes->data->has_min) {
							float *emin = prim->attributes->data->min;
							float* emax = prim->attributes->data->max;
							extent6f_constructor(ts->_extent, emin[0], emin[1], emin[2], emax[0], emax[1], emax[2]);
						}
					}
					break;
					case cgltf_primitive_type_triangle_strip:
						//June 22, 2020 possible in gltf format, but not implemented here
						// hypothesis: one BufferGeometry node (above) can handle all geom types
						// including points and lines
						printf("triangle strip\n");
						break;
					case cgltf_primitive_type_triangle_fan:
						printf("triangle fan\n");
						break;
				}
			}
		}
		//printf("adding shape\n");
		vector_pushBack(void *, pp, sn);
	}
	if(node->skin){
		//vector_pushBack(void *, pp, skin);
	}
	if(node->weights_count){
	}
	size_t estart = node->extras.start_offset;
	size_t eend = node->extras.end_offset;
	//children part
	int mc = node->children_count;
	if(mc){

		struct X3D_Node* pn;
		for(int i=0;i<mc;i++){
			if (parse_gltf_node(ectx, &pn, data, node->children[i], unit)) {
				vector_pushBack(struct X3D_Node*, pp, pn);
			}
		}
		//printf("adding children\n");
	}
	int got_something = FALSE;
	if (pp->n) {
		//transform part: gltf has a flat scenegraph, with each node having a transform and a thing, with thing being mesh, camera. Like Blender.
		struct X3D_Transform* t = createNewX3DNode(NODE_Transform);
		if (node->has_matrix) {
			//parse matrix into TRS
			//
			printf("gltf_loader not parsing matrix yet\n");
		}
		else {
			vecset3f(t->translation.c, 0.0f, 0.0f, 0.0f);
			vecset3f(t->scale.c, 1.0f, 1.0f, 1.0f);
			vecset4f(t->rotation.c, 0.0f, 1.0f, 0.0f, 0.0f);
			if (node->has_rotation ) {
				float* r = t->rotation.c;
				double rd[4];
				Quaternion q;
				q.x = node->rotation[0];
				q.y = node->rotation[1];
				q.z = node->rotation[2];
				q.w = node->rotation[3];
				quaternion_normalize(&q);
				quaternion_to_vrmlrot(&q, &rd[0], &rd[1], &rd[2], &rd[3]);
				double2float(r, rd, 4);
				//r[3] = -r[3];
			}
			if (node->has_scale ) {
				veccopy3f(t->scale.c, node->scale);
				//printf("scale %f %f %f\n", t->scale.c[0], t->scale.c[1], t->scale.c[2]);
			}
			if (node->has_translation ) {
				veccopy3f(t->translation.c, node->translation);
			}
		}
		t->children.n = pp->n;
		t->children.p = pp->data;
		if (0) {
			printf("translation %f %f %f\n", t->translation.c[0], t->translation.c[1], t->translation.c[2]);
			printf("scale %f %f %f\n", t->scale.c[0], t->scale.c[1], t->scale.c[2]);
			printf("rotation %f %f %f %f\n", t->rotation.c[0], t->rotation.c[1], t->rotation.c[2], t->rotation.c[3]);
			printf("number of children %d\n", t->children.n);
			for (int k = 0; k < t->children.n; k++)
				printf("   %d %s\n", k, stringNodeType(t->children.p[k]->_nodeType));
		}
		for (int i = 0; i < t->children.n; ++i) {
			ADD_PARENT(t->children.p[i], X3D_NODE(t));
		}
		add_node_to_broto_context(X3D_PROTO(ectx), X3D_NODE(t));

		*spot = X3D_NODE(t);
		got_something = TRUE;
	}
	return got_something;
}

int parse_gltf(struct X3D_Node *ectx, struct Multi_Node *spot, cgltf_data * data, gltf_unit *unit){
	int n = data->scene[0].nodes_count;
	spot->p = realloc(spot->p, n * sizeof(struct X3D_Node *));
	n = 0; //we may not know how to parse them all, so don't count ones we don't parse.
	for(int i=0;i<data->scene[0].nodes_count;i++){
		if( parse_gltf_node(ectx,&spot->p[n],data,data->scene[0].nodes[i], unit) ) n++;
	}
	spot->n = n;
	//printf("got %d children\n", n);
	int ret = TRUE;
	return ret;
}

struct uri_data {
	char *uri;
	void **data;
	int data_size;
	void* cdata;
};
cgltf_result cgltf_load_buffers_except_files(const cgltf_options* options, cgltf_data* data, Stack *file_list)
{
	if (options == NULL)
	{
		return cgltf_result_invalid_options;
	}

	if (data->buffers_count && data->buffers[0].data == NULL && data->buffers[0].uri == NULL && data->bin)
	{
		if (data->bin_size < data->buffers[0].size)
		{
			return cgltf_result_data_too_short;
		}

		data->buffers[0].data = (void*)data->bin;
	}

	for (cgltf_size i = 0; i < data->buffers_count; ++i)
	{
		if (data->buffers[i].data)
		{
			continue;
		}

		const char* uri = data->buffers[i].uri;

		if (uri == NULL)
		{
			continue;
		}

		if (strncmp(uri, "data:", 5) == 0)
		{
			const char* comma = strchr(uri, ',');

			if (comma && comma - uri >= 7 && strncmp(comma - 7, ";base64", 7) == 0)
			{
				cgltf_result res = cgltf_load_buffer_base64(options, data->buffers[i].size, comma + 1, &data->buffers[i].data);

				if (res != cgltf_result_success)
				{
					return res;
				}
			}
			else
			{
				return cgltf_result_unknown_format;
			}
		}
		else if (strstr(uri, "://") == NULL )
		{
			struct uri_data ud;
			ud.uri = uri;
			ud.cdata = &data->buffers[i]; //after phase 2 parse, we will look up geombuffer from this buffer address
			ud.data_size = data->buffers[i].size;
			stack_push(struct uri_data,file_list,ud);
		}
		else
		{
			return cgltf_result_unknown_format;
		}
	}

	return cgltf_result_success;
}


//ret = X3DParse(ectx, X3D_NODE(nRn), (const char*)input);
int parser_do_parse_gltf(const char *input, const int len, struct X3D_Node *ectx, struct X3D_Node *myParent)
{
	// ectx - the context node - either Inline or Scene
	// rNr temporary group container node where we'll put the new nodes as children (should have been struct MFNode * field of container)
	int ret = FALSE;
	{
		cgltf_options options;
		memset(&options, 0, sizeof(cgltf_options));
		cgltf_data* data = NULL;
		ppgltf_loader p = (ppgltf_loader)gglobal()->gltf_loader.prv;
		gltf_unit *unit = malloc(sizeof(gltf_unit));
		memset(unit,0,sizeof(gltf_unit));
		stack_push(gltf_unit*,p->gltf_units,unit);
		
		unit->blob = malloc(len);
		unit->blob_len = len;
		memcpy(unit->blob,input,len);  //resource process garbage collects input. For .glb we need to keep blob
		cgltf_result result = cgltf_parse(	&options, (void*) unit->blob, unit->blob_len, &data);
		unit->data = data;
		if (result == cgltf_result_success)
		{
			printf("gltf parsed into cgltf scene struct\n");
			/* TODO make awesome stuff */
			//char *local_path = getContext ectx->_
			//result = cgltf_load_buffers(&options, data, "./");
			Stack *file_list = newStack(struct uri_data);
			result = cgltf_load_buffers_except_files(&options, data,file_list);

			if(result == cgltf_result_success && file_list->n == 0){
				unit->bin_loaded = TRUE;
			}else if(result == cgltf_result_file_not_found){
				printf("gltf .bin file not found ... yet\n");
				//generate a resource to fetch .bin
			}else if(file_list->n){
				resource_item_t *res;
				struct X3D_Proto *context = X3D_PROTO(ectx);

				//compact file list?
				//spawn resource(s) to fetch>
				unit->bin_file_list = file_list;
				int nn = 1;
				nn = file_list->n;
				for (int k = 0; k < nn; k++) {
					struct uri_data* ud = vector_get_ptr(struct uri_data, file_list, k);
					char* uri = ud->uri;
					res = resource_create_single(uri);
					res->ectx = ectx;
					res->media_type = resm_unknown; // resm_bin;
					res->resm_specific = ud->cdata;
					resource_identify(context->_parentResource, res);
					res->actions = resa_download | resa_load | resa_process;
					resitem_enqueue(ml_new(res));
				}
			}
			// 1. go over struct, creating x3d nodes and nesting them
			struct Multi_Node *spot;
			if(myParent->_nodeType == NODE_Proto || myParent->_nodeType == NODE_Inline )
				spot = &((struct X3D_Proto*)(myParent))->__children;
			else
				spot = &((struct X3D_Group*)(myParent))->children;
			spot->p = NULL; spot->n = 0;
			parse_gltf(ectx,spot,data,unit);
			for(int j=0;j<spot->n;j++)
				ADD_PARENT(X3D_NODE(spot->p[j]), X3D_NODE(ectx));
			// documentation: """Note that cgltf does not load the contents of extra files such as buffers or images into memory by default. 
			//	You'll need to read these files yourself using URIs from data.buffers[] or data.images[] respectively. """
			ret = TRUE;
		}
	}

	return ret;
}

// .glb has the .bin binary buffers inside and we can load and parse in one shot
// .glTF refers to a separate .bin file
// 1 we parse either to cgltf nodes
// 2 then check if bin loaded as part of glb, and if so apply
// 3 else we spawn a resource loader to fetch .bin (should work also over http)
// 4 we parse into x3d nodes either way, putting gltf_unit* in nodes that need the bin data
// 5 nodes check gltf_unit, and delay compile_ until .bin loaded flag set 
// 6 when .bin resource loads here, we apply bin to cgltf nodes buffer.data and set the gltf_unit-loaded flag

int gltf_load_bin(resource_item_t *res){
	// late arriving .bin (for .gltf separated unit)
	cgltf_buffer* buffer = res->resm_specific;
	if (buffer) {
		struct geomBuffer *gb = find_buffer_in_broto_context_from_cgltf_buffer(res->ectx, buffer);
		if (gb) {
			openned_file_t* of = res->openned_files;
			int len = of->fileDataSize;
			char* input = of->fileData;
			memcpy(gb->address, input, len);
			gb->loaded = 1;
		}
	}

	return TRUE;
}

int parser_process_res_gltf(resource_item_t *res){
	//these media types (require us to) generate x3d scene nodes and can be a scene unto themselves, 
	// or inline body
	//some embed needed resources, others request more resources which are placed in their node fields.

	int parsedOk = FALSE;
	switch(res->media_type){
		case resm_glb:
		case resm_gltf:
			// .bin gl buffers and images are packed into one .glb file
			//text/json gltf file can inline some .bin and img buffers as text
			// but more normally separate .bin binary buffer file and image urls
			parsedOk = parser_process_res_VRML_X3D(res);
			break;
		case resm_bin:
			//gltf can be exported with separate binary buffer file
			// like loading image textures, the bin needs to catch-up to the 
			// parsed x3d node, so after applying binary to parsed cgltf nodes,
			// sets a flag that previously spawned x3d nodes can check to see 
			// when binary gl buffer data has been loaded and applied to primitives
			parsedOk = gltf_load_bin(res);
			break;
		//cesium related - for future geo/cesium work
		case resm_json:
			break;
		case resm_b3dm:
			break;
		case resm_i3dm:
			break;
		case resm_pnts:
			break;
		case resm_cmpt:
			break;
	
	}
	return parsedOk;
}
float* extent6f_fromBufferAccess(float* e6, struct geomBuffer* gb, struct bufAccess* ba, int ncoord) {
	extent6f_clear(e6);
	float point[3];
	double* dp;
	float* fp;
	char* paddress;
	memset(point, 0, 3 * sizeof(float));
	for (int i = 0; i < ncoord; i++) {
		paddress = get_Attribi(ba, gb, i);
		if (ba->dataType == GL_FLOAT) {
			fp = (float*)paddress;
			for (int j = 0; j < ba->dataSize; j++) {
				point[j] = fp[j];
			}
		}
		else if (ba->dataType == GL_DOUBLE) {
			dp = (double*)paddress;
			double2float(point, dp, ba->dataSize);
		}
		extent6f_union_vec3f(e6, point);
	}
	return e6;
}
void compile_BufferGeometry(struct X3D_BufferGeometry *node){
	struct X3D_MeshRep* mr = (struct X3D_MeshRep*) node->_intern;
	if (mr) {
		//if (0) {
		//	printf("compile_BufferGeometry context = %p\n", get_executionContext());
		//	struct geomBuffer* gb = find_buffer_in_broto_context_from_cgltf_buffer(get_executionContext(), mr->buffer->cgltf_buffer);
		//	if (gb) printf("found geomBuffer in this thread executionContext\n");
		//	else printf("didn't find geombuffer in this thread executionContext\n");
		//}
		if (mr->buffer->loaded == 1)
			set_geomBuffer(mr->buffer);
		if (mr->buffer->loaded == 2) {
			float e6[6];
			extent6f_clear(e6);

			MARK_NODE_COMPILED
			if (mr->attrib[0].in_use) {
				//update extent if not set
				if (!extent6f_isSet(node->_extent)) {
					extent6f_fromBufferAccess(e6, mr->buffer, &mr->attrib[0], mr->ncoord);
				}
			}
			if(0) if (mr->attrib[4].in_use) {
				//experiment to reverse UV V coordinate to see if freewrl textures are upside down
				float* fp;
				char* paddress;
				struct geomBuffer* gb = mr->buffer;
				struct bufAccess* ba;
				ba = &mr->attrib[0];

				for (int i = 0; i < mr->ncoord; i++) {
					paddress = get_Attribi(ba, gb, i);
					fp = (float*)paddress;
					fp[1] = 1.0f - fp[1];
				}
			}
			if (0) {
				extent6f_printf(node->_extent);
				printf("cgltf min,max\n");
				extent6f_printf(e6);
				printf("compile_BufferGeometry extent\n");
			}
			if(extent6f_isSet(e6))
				extent6f_copy(node->_extent, e6);
		}
	}
}
void render_BufferGeometry(struct X3D_BufferGeometry *node){
	
	//we lazy-load .bin binary buffer part for .gltf, so have to wait 
	// till its loaded. .glb loads in one shot 
	COMPILE_IF_REQUIRED;
	setExtent(node->EXTENT_MAX_X, node->EXTENT_MIN_X, node->EXTENT_MAX_Y,
		node->EXTENT_MIN_Y, node->EXTENT_MAX_Z, node->EXTENT_MIN_Z,
		X3D_NODE(node));
	render_MeshRep(node->_intern);
}
void rendray_BufferGeometry(struct X3D_BufferGeometry* node) {
	/* is this structure still loading? */
	if (!node) return;
	/* is this structure still loading? */
	if (!(node->_intern)) {
		return;
	}
	if (node->_ichange == 0) return; //not compiled yet

	rendray_MeshRep(node->_intern);
}

void collide_BufferGeometry(struct X3D_BufferGeometry *node){
}
