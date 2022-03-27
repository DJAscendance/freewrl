

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
struct X3D_Node * node;
};
static Stack *defs = NULL;
struct X3D_Node *USE_node(char *name){
	if(name){
		if(!defs) defs = newStack(struct name_node);
		struct name_node nn;
		for(int i=0;i<defs->n;i++){
			nn = vector_get(struct name_node,defs,i);
			if(!strcmp(name,nn.name)){
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
	for (int j = 0; j < 4; j++) {
		ba = &meshrep->attrib[4 + j];
		if (ba->in_use) {
			FW_GL_TEXCOORD_POINTER(ba->dataSize, ba->dataType, ba->byteStride, (GLfloat*)BUFFER_OFFSET(ba->byteOffset),j); //dataSize, dataType, stride, pointer, texID
		}
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
			ba = &meshrep->attrib[0]; //point
			paddress = get_Attribi(ba, gb, i);;
			float* ai = (float*)paddress;
			for (int j = 0; j < ba->dataSize; j++) {
				printf("%f ", ai[j]);
			}
			printf("]");
			ba = &meshrep->attrib[1]; //color
			if (ba->in_use) {
				paddress = get_Attribi(ba, gb, i);;
				float* ai = (float*)paddress;
				printf(" [");
				for (int j = 0; j < ba->dataSize; j++)
					printf("%f ", ai[j]);
				printf("]");
			}
			ba = &meshrep->attrib[2]; //fog
			if (ba->in_use) {
				paddress = get_Attribi(ba, gb, i);;
				float* ai = (float*)paddress;
				printf(" [");
				for (int j = 0; j < ba->dataSize; j++)
					printf("%f ", ai[j]);
				printf("]");
			}

			printf("\n");
		}
		printf("");
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
	//transform part
	int show = FALSE; //TRUE for some printfs
	int m = 0;
	struct X3D_Transform *t = createNewX3DNode(NODE_Transform);
	if(node->has_matrix){
		//parse matrix into TRS
		//
	}else{
		if(node->has_rotation){
			veccopy3f(t->rotation.c,&node->rotation[1]); 
			t->rotation.c[3] = node->rotation[0];
		}
		if(node->has_scale){
			veccopy3f(t->scale.c,node->scale);
		}
		if(node->has_translation){
			veccopy3f(t->translation.c,node->translation);
		}
	}
	//content part
	if(node->camera){
		m++;
		t->children.p = realloc(t->children.p,m*sizeof(struct X3D_Node*));
		//june 22, 2020 not done: add viewpoint here
	}
	if(node->light){
		m++;
		t->children.p = realloc(t->children.p,m*sizeof(struct X3D_Node*));
		// june 22, 2020 not done: add punctual (directional, point, spot) light here
		// - not done and not supported yet EnvironmentLight
	}
	if(node->mesh){
		//gltf mesh is like our shape: it refers to material and to geometry/accessor
		m++;
		t->children.p = realloc(t->children.p,m*sizeof(struct X3D_Node*));
		struct X3D_Shape *sn = (struct X3D_Shape*) USE_node(node->mesh->name);
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
						struct X3D_UnlitMaterial* mat = (struct X3D_UnlitMaterial*) USE_node(prim->material->name);
						if(!mat){
							mat = (struct X3D_UnlitMaterial*) DEF_node(ectx,prim->material->name,mtype);
							veccopy3f(mat->emissiveColor.c,prim->material->emissive_factor);
							//mat->emissiveTextureChannel 
							if(prim->material->emissive_texture.texture->image->buffer_view) { //->buffer->data){
								if(show) printf("image loaded for us\n");
							}else{
								if(show) printf("image not loaded uri = %s\n",prim->material->emissive_texture.texture->image->uri);
							}
						}
						sn->appearance = createNewX3DNode(NODE_Appearance);
						X3D_APPEARANCE(sn->appearance)->material = X3D_NODE(mat);
					}else if(prim->material->has_pbr_metallic_roughness){
						int mtype = NODE_PhysicalMaterial;
						struct X3D_PhysicalMaterial* mat = (struct X3D_PhysicalMaterial*) USE_node(prim->material->name);
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
									if(show) printf("image not loaded uri = %s\n",pbr->base_color_texture.texture->image->uri);
									struct X3D_Node *image = USE_node(pbr->base_color_texture.texture->image->name);
									if(!image){
										 image = DEF_node(ectx,pbr->base_color_texture.texture->image->name,NODE_ImageTexture);
										 struct X3D_ImageTexture *it = (struct X3D_ImageTexture*)image;
										 it->url.p = malloc(sizeof(void*));
										 it->url.p[0] = newASCIIString(pbr->base_color_texture.texture->image->uri);
										 it->url.n = 1;
									}
									mat->baseTexture = image;
								}
							}

						}
						sn->appearance = createNewX3DNode(NODE_Appearance);
						X3D_APPEARANCE(sn->appearance)->material = X3D_NODE(mat);
					}else if(prim->material->has_pbr_specular_glossiness){
						int mtype = NODE_Material;
						struct X3D_Material* mat = (struct X3D_Material*) USE_node(prim->material->name);
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

						}
						sn->appearance = createNewX3DNode(NODE_Appearance);
						X3D_APPEARANCE(sn->appearance)->material = X3D_NODE(mat);
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
							ba->dataType = blob->component_type;
							ba->byteStride = blob->stride; //the bufferView also has a stride
							ba->byteSize = ba->dataSize * lookup_dataType_size(ba->dataType);
							ba->in_use = 1;
							mr->nindex = blob->count;
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
		t->children.p[m-1] = X3D_NODE(sn);
	}
	if(node->skin){
		m++;
		t->children.p = realloc(t->children.p,m*sizeof(struct X3D_Node*));
	}
	if(node->weights_count){
	}
	size_t estart = node->extras.start_offset;
	size_t eend = node->extras.end_offset;
	//children part
	int mc = node->children_count;
	if(mc){
		t->children.p = realloc(t->children.p,(mc+m)*sizeof(struct X3D_Node*));
		for(int i=0;i<mc;i++){
			parse_gltf_node(ectx,&t->children.p[i+m],data,node->children[i],unit);
		}
		m += mc;
	}
	t->children.n = m;
	add_node_to_broto_context(X3D_PROTO(ectx),X3D_NODE(t));
	*spot = X3D_NODE( t );
	return TRUE;
}

int parse_gltf(struct X3D_Node *ectx, struct Multi_Node *spot, cgltf_data * data, gltf_unit *unit){
	int n = data->scene[0].nodes_count;
	spot->p = realloc(spot->p, n * sizeof(struct X3D_Node *));
	for(int i=0;i<data->scene[0].nodes_count;i++){
		parse_gltf_node(ectx,&spot->p[i],data,data->scene[0].nodes[i], unit);
	}
	spot->n = n;
	int ret = TRUE;
	return ret;
}

struct uri_data {
	char *uri;
	void **data;
	int data_size;
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
			ud.data = &data->buffers[i].data;
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
				struct uri_data * ud = vector_get_ptr(struct uri_data,file_list,0);
				char * uri = ud->uri;
				res = resource_create_single(uri);
				res->media_type = resm_unknown; // resm_bin;
				res->resm_specific = unit;
				resource_identify(context->_parentResource, res);
				res->actions = resa_download | resa_load | resa_process;
				resitem_enqueue(ml_new(res));
			}
			// 1. go over struct, creating x3d nodes and nesting them
			struct Multi_Node *spot;
			if(myParent->_nodeType == NODE_Proto || myParent->_nodeType == NODE_Inline )
				spot = &((struct X3D_Proto*)(myParent))->__children;
			else
				spot = &((struct X3D_Group*)(myParent))->children;
			spot->p = NULL; spot->n = 0;
			parse_gltf(ectx,spot,data,unit);
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
	gltf_unit * unit = res->resm_specific;
	if(unit && !unit->bin_loaded){
		openned_file_t *of = res->openned_files;
		int len = of->fileDataSize;
		char * input = of->fileData;
		unit->bin = malloc(len);
		memcpy(unit->bin,input,len);
		unit->bin_len = of->fileDataSize;
		Stack *file_list = unit->bin_file_list;
		struct uri_data *ud;
		for(int i=0;i<vectorSize(file_list); i++){
			// june 22, 2020: I'm not properly handling multiple .bin or whatever the loop is for
			ud = vector_get_ptr(struct uri_data,file_list,i);
			*ud->data = unit->bin;
			ud->data_size = unit->bin_len;
		}
		unit->bin_loaded = TRUE;
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
		if(mr->buffer->loaded == 2)
			MARK_NODE_COMPILED
	}
}
void render_BufferGeometry(struct X3D_BufferGeometry *node){
	
	//we lazy-load .bin binary buffer part for .gltf, so have to wait 
	// till its loaded. .glb loads in one shot 
	COMPILE_IF_REQUIRED;
	render_MeshRep(node->_intern);
}
void rendray_BufferGeometry(struct X3D_BufferGeometry *node){
}
void collide_BufferGeometry(struct X3D_BufferGeometry *node){
}
