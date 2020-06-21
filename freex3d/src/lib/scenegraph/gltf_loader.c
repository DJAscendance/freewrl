

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

#include <libFreeWRL.h>
#include <list.h>
#include <io_http.h>

// GLTF
//https://github.com/jkuhlmann/cgltf 
//- include 100 line recursive json parser (how does data come out?) etc.
//- first 600 lines of header is API. next 4000 lines is CGLTF_IMPLEMENTATION
#define  CGLTF_IMPLEMENTATION 1
#include "cgltf.h"

typedef struct gltf_unit {
	cgltf_data *data;
	int bin_loaded;
	unsigned char *cgltf_bin;
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
	if(!defs) defs = newStack(struct name_node);
	struct name_node nn;
	for(int i=0;i<defs->n;i++){
		nn = vector_get(struct name_node,defs,i);
		if(!strcmp(name,nn.name)){
			return nn.node;
		}
	}
	return NULL;
}
struct X3D_Node *DEF_node(struct X3D_Node *ectx, char *name, int nodetype){
	if(!defs) defs = newStack(struct name_node);
	struct name_node nn;
	struct X3D_Node *node = createNewX3DNode(nodetype);
	add_node_to_broto_context(X3D_PROTO(ectx),X3D_NODE(node));

	nn.name = name;
	nn.node = node;
	stack_push(struct name_node,defs,nn);
return node;
}


struct X3D_PolyRep * create_polyrep0();
int parse_gltf_node(struct X3D_Node *ectx, struct X3D_Node **spot, cgltf_data * data, cgltf_node *node){
	//transform part
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
	}
	if(node->light){
		m++;
		t->children.p = realloc(t->children.p,m*sizeof(struct X3D_Node*));
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
					if(prim->material->unlit){
						int mtype = NODE_UnlitMaterial;
						struct X3D_UnlitMaterial* mat = (struct X3D_UnlitMaterial*) USE_node(prim->material->name);
						if(!mat){
							mat = (struct X3D_UnlitMaterial*) DEF_node(ectx,prim->material->name,mtype);
							veccopy3f(mat->emissiveColor.c,prim->material->emissive_factor);
							//mat->emissiveTextureChannel 
							if(prim->material->emissive_texture.texture->image->buffer_view->buffer->data){
								printf("image loaded for us\n");
							}else{
								printf("image not loaded uri = %s\n",prim->material->emissive_texture.texture->image->uri);
							}
							/*
typedef struct cgltf_material
{
	char* name;
	cgltf_bool has_pbr_metallic_roughness;
	cgltf_bool has_pbr_specular_glossiness;
	cgltf_bool has_clearcoat;
	cgltf_pbr_metallic_roughness pbr_metallic_roughness;
	cgltf_pbr_specular_glossiness pbr_specular_glossiness;
	cgltf_clearcoat clearcoat;
	cgltf_texture_view normal_texture;
	cgltf_texture_view occlusion_texture;
	cgltf_texture_view emissive_texture;
	cgltf_float emissive_factor[3];
	cgltf_alpha_mode alpha_mode;
	cgltf_float alpha_cutoff;
	cgltf_bool double_sided;
	cgltf_bool unlit;
	cgltf_extras extras;
} cgltf_material;					
*/	}
					}else if(prim->material->has_pbr_metallic_roughness){
						int mtype = NODE_PhysicalMaterial;
						struct X3D_PhysicalMaterial* mat = (struct X3D_PhysicalMaterial*) USE_node(prim->material->name);
						if(!mat){
							cgltf_pbr_metallic_roughness *pbr = &prim->material->pbr_metallic_roughness;
							mat = (struct X3D_PhysicalMaterial*) DEF_node(ectx,prim->material->name,mtype);
							
							if(pbr->base_color_texture.texture){
								if(pbr->base_color_texture.texture->image->buffer_view->buffer->data){
									printf("image loaded for us\n");
								}else{
									printf("image not loaded uri = %s\n",pbr->base_color_texture.texture->image->uri);
								}
							}

						}
					}else if(prim->material->has_pbr_specular_glossiness){
						int mtype = NODE_Material;
						struct X3D_Material* mat = (struct X3D_Material*) USE_node(prim->material->name);
						if(!mat){
							mat = (struct X3D_Material*) DEF_node(ectx,prim->material->name,mtype);
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
						if(1){
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
								printf(" nfloats = %d accessor type %d count %d ",nfloats,blob->type,blob->count);
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
									printf("%d %f %f %f\n",index,element_float[0],element_float[1],element_float[2]);
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
						{
							enum {
								GLTF_USE_BUFFER_COPY = 1,
								GLTF_USE_RECOMPILE = 2,
							};
							//struct X3D_TriangleSet *ts = (struct X3D_TriangleSet*)gn;
							struct X3D_BufferGeometry *ts = (struct X3D_BufferGeometry*)gn;
							ts->_bufferdata = prim;
							if(0){
								int use_method = GLTF_USE_BUFFER_COPY;//GLTF_USE_RECOMPILE;
								if(use_method == GLTF_USE_RECOMPILE){
								}else if(use_method == GLTF_USE_BUFFER_COPY){
									//struct X3D_PolyRep * r = create_polyrep0(); //does vertex and index genBuffer
									//ts->_intern = r;
									for(int ii=0;ii<acount;ii++){
										const cgltf_accessor* blob = prim->attributes[ii].data;
										size_t size = blob->count * cgltf_num_components(blob->type) * sizeof(float);
										void *data = &blob->buffer_view->buffer[blob->buffer_view->offset];
										switch(prim->attributes[ii].type){
											case cgltf_attribute_type_position:
											////copy to coord vbo
											//r->ntri = blob->count / 3;
											//FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,r->VBO_buffers[VERTEX_VBO]);
											//glBufferData(GL_ARRAY_BUFFER,size,data, GL_STATIC_DRAW);
											break;

											//FW_GL_BINDBUFFER(GL_ELEMENT_ARRAY_BUFFER,r->VBO_buffers[INDEX_VBO]);
									
											case cgltf_attribute_type_normal:
											//copy to normals vbo
											//glGenBuffers(1,&r->VBO_buffers[NORMAL_VBO]);
											//FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,r->VBO_buffers[NORMAL_VBO]);
											//glBufferData(GL_ARRAY_BUFFER,size,data, GL_STATIC_DRAW);
											break;

											case cgltf_attribute_type_texcoord:
											//copy to texcoord
											//glGenBuffers(1,&r->VBO_buffers[TEXTURE_VBO0]);
											//FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,r->VBO_buffers[TEXTURE_VBO0]);
											//glBufferData(GL_ARRAY_BUFFER,size,data, GL_STATIC_DRAW);
											break;

											//cooy to vertex color vbo
											//if (r->color) {
											//	if (r->VBO_buffers[COLOR_VBO] == 0) glGenBuffers(1,&r->VBO_buffers[COLOR_VBO]);            
											//	FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,r->VBO_buffers[COLOR_VBO]);
											//	glBufferData(GL_ARRAY_BUFFER,r->ntri*sizeof(struct SFColorRGBA)*3,r->color, GL_STATIC_DRAW);
											//	// needed by recalculateColorField ... FREE_IF_NZ(r->color);
											//}
											//if (newfog) {
											//	if (r->VBO_buffers[FOG_VBO] == 0) glGenBuffers(1,&r->VBO_buffers[FOG_VBO]);            
											//	FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,r->VBO_buffers[FOG_VBO]);
											//	glBufferData(GL_ARRAY_BUFFER,r->ntri*sizeof(float)*3,r->actualFog, GL_STATIC_DRAW);
											//}



											default:
											break;
										}
									}
									//r->streamed=TRUE;
								}
							}
						}
					}
					break;
					case cgltf_primitive_type_triangle_strip:
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
			parse_gltf_node(ectx,&t->children.p[i+m],data,node->children[i]);
		}
		m += mc;
	}
	t->children.n = m;
	add_node_to_broto_context(X3D_PROTO(ectx),X3D_NODE(t));
	*spot = X3D_NODE( t );
	return TRUE;
}

int parse_gltf(struct X3D_Node *ectx, struct Multi_Node *spot, cgltf_data * data){
	int n = data->scene[0].nodes_count;
	spot->p = realloc(spot->p, n * sizeof(struct X3D_Node *));
	for(int i=0;i<data->scene[0].nodes_count;i++){
		parse_gltf_node(ectx,&spot->p[i],data,data->scene[0].nodes[i]);
	}
	spot->n = n;
	int ret = TRUE;
	return ret;
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
		char *floating_copy = malloc(len);
		memcpy(floating_copy,input,len);
		register_node_gc(ectx,floating_copy);
		cgltf_result result = cgltf_parse(	&options, (void*) floating_copy, len, &data);
		if (result == cgltf_result_success)
		{
			printf("gltf parsed into cgltf scene struct\n");
			/* TODO make awesome stuff */
			result = cgltf_load_buffers(&options, data, "./");
			if(result == cgltf_result_success){
				// 1. go over struct, creating x3d nodes and nesting them
				struct Multi_Node *spot;
				if(myParent->_nodeType == NODE_Proto || myParent->_nodeType == NODE_Inline )
					spot = &((struct X3D_Proto*)(myParent))->__children;
				else
					spot = &((struct X3D_Group*)(myParent))->children;
				spot->p = NULL; spot->n = 0;
				parse_gltf(ectx,spot,data);
				// 2. for exta files send url request and have a place to put it in the x3d node created for it
				// documentation: """Note that cgltf does not load the contents of extra files such as buffers or images into memory by default. 
				//	You'll need to read these files yourself using URIs from data.buffers[] or data.images[] respectively. """
				//cgltf_free(data);
				ret = TRUE;
			}else if(result == cgltf_result_file_not_found){
				printf("gltf .bin file not found\n");
			}
		}
	}

	return ret;
}
// .glb has the .bin binary buffers inside and we can load and parse in one shot
// .glTF refers to a separate .bin file, and we can parse into x3d nodes until we have it.
// - and we don't know if and what nane the .bin is until we parse glTF into cgltf nodes
// - that means we need 2 steps:
// 1. parse into cgltf nodes, get the uri of the .bin 
// if there is a separate .bin
// 2. schedule the .bin with resources so it can download/load into a blob
// 3. wait for the bins to show up
// 4. paste the bins in to .data
// 5. continue on to x3d node parsing
struct resm_gltf_stuff {
	int gltf_parsed;
	//int num_bin;
	//int j_bin;
	cgltf_data* data;
	Stack *file_list;
};
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
int gltf_parse_to_cgltf(resource_item_t *res){
	struct resm_gltf_stuff * stuff = (struct resm_gltf_stuff *)res->resm_specific;
	if(!stuff){
		res->resm_specific = stuff = malloc(sizeof(struct resm_gltf_stuff));
		memset(stuff,0,sizeof(struct resm_gltf_stuff));
	}
	if(!stuff->gltf_parsed){
		cgltf_options options;
		memset(&options, 0, sizeof(cgltf_options));
		cgltf_data* data = NULL;
		openned_file_t *of = res->openned_files;
		int len = of->fileDataSize;
		char * input = of->fileData;
		char *floating_copy = malloc(len);
		memcpy(floating_copy,input,len);
		register_node_gc(res->ectx,floating_copy);
		cgltf_result result = cgltf_parse(	&options, (void*) floating_copy, len, &data);

		if (result == cgltf_result_success)
		{
			printf("gltf parsed into cgltf scene struct\n");
			stuff->gltf_parsed = TRUE;
			stuff->data = data;
			//check if we need to load bins, and get that started
			Stack *file_list = newStack(struct uri_data);
			result = cgltf_load_buffers_except_files(&options, data,file_list);
			if(file_list->n > 0){ 
				stuff->file_list = file_list;
			}
		}

	}
	return 3;
}
int gltf_load_bin(resource_item_t *res){
	struct resm_gltf_stuff * stuff = (struct resm_gltf_stuff *)res->resm_specific;
	if(stuff && stuff->file_list && stuff->file_list->n > 0){
		//swap urls to load next part
		//thunk down to resm_download | _load or copy, retire, and launch new resource

	}
	return FALSE;
}
int parser_process_res_gltf(resource_item_t *res){
	//these media types (require us to) generate x3d scene nodes and can be a scene unto themselves, 
	// or inline body
	//some embed needed resources, others request more resources which are placed in their node fields.

	int parsedOk = FALSE;
	switch(res->media_type){
		case resm_glb:
			// .bin gl buffers and images are packed into one .glb file
			parsedOk = parser_process_res_VRML_X3D(res);
			break;
		case resm_gltf: {
			//text/json gltf file can inline some .bin and img buffers as text
			// but more normally separate .bin binary buffer file and image urls
				int idone = gltf_parse_to_cgltf(res);
				if(idone){
					parsedOk = gltf_load_bin(res);
					if(parsedOk) 
						parsedOk = parser_process_res_VRML_X3D(res);
				}
			}
			break;
		case resm_bin:
			//gltf can be exported with separate binary buffer file
			// the buffer needs to catch up to / join into the parsed gltf
			// before we parse/convert gltf into our freewrl-type geometry nodes
					parsedOk = gltf_load_bin(res);
					if(parsedOk) 
						parsedOk = parser_process_res_VRML_X3D(res);
			break;
		//cesium related
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

}
void render_BufferGeometry(struct X3D_BufferGeometry *node){

	//CULL_FACE(node->solid)
	if(!node->_bufferdata) return;
	// taken from the OpenGL.org website:
	#define BUFFER_OFFSET(i) ((char *)NULL + (i))
	cgltf_primitive *prim = (cgltf_primitive*)node->_bufferdata;
	int acount = prim->attributes_count;
	if(node->_vbo.p == NULL){
		 node->_vbo.p = malloc((acount+1) * sizeof(int));
		 memset(node->_vbo.p,0,(acount+1) * sizeof(int));
		 node->_vbo.n = acount + 1;
		 for(int i=0;i<(acount+1);i++) node->_vbo.p[i] = -1;
	}

	if(prim && acount){
		printf("render_BufferGeometry triangles\n");

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
			printf(" nfloats = %d accessor type %d count %d ",nfloats,blob->type,blob->count);
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
				printf("%d %f %f %f\n",index,element_float[0],element_float[1],element_float[2]);
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

//return;
//glEnableClientState(GL_VERTEX_ARRAY);             // activate vertex position array
//glEnableClientState(GL_NORMAL_ARRAY);             // activate vertex normal array
//glEnableClientState(GL_TEXTURE_COORD_ARRAY);      // activate texture coord array
	FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,0);

	for(int ii=0;ii<acount;ii++){
		const cgltf_accessor* blob = prim->attributes[ii].data;
		int isize = cgltf_num_components(blob->type);
		size_t size = blob->count * isize * sizeof(float);
		void *data = &blob->buffer_view->buffer[blob->buffer_view->offset];
		switch(prim->attributes[ii].type){
			case cgltf_attribute_type_position:
			////copy to coord vbo
			if(node->_vbo.p[ii] == -1){
				glGenBuffers(1,(GLuint*) &node->_vbo.p[ii]);
				FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,node->_vbo.p[ii]);
				//glEnableVertexAttribArray( LOC );
				cgltf_float element_float[16];
				float *fdata = (float*) malloc(size);
				for (cgltf_size index = 0; index < blob->count; index++)
				{
					cgltf_accessor_read_float(blob, index, &fdata[index*3], 3);
					printf("%d %f %f %f\n", index, fdata[index*3 +0],fdata[index*3 +1],fdata[index*3 +2]);
				}
				//glVertexAttribPointer( LOC   ,isize, GL_FLOAT, FALSE, blob->stride, fdata);
				//FW_GL_VERTEX_POINTER(3, GL_FLOAT,size, fdata); 
				glBufferData(GL_ARRAY_BUFFER,size,fdata, GL_STATIC_DRAW);

			}else{
				FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,node->_vbo.p[ii]);
				FW_GL_VERTEX_POINTER(3,GL_FLOAT,0,0);
			}

			break;

			//FW_GL_BINDBUFFER(GL_ELEMENT_ARRAY_BUFFER,r->VBO_buffers[INDEX_VBO]);
									
			case cgltf_attribute_type_normal:
			//copy to normals vbo
			if(node->_vbo.p[ii] == -1){
				glGenBuffers(1,(GLuint*) &node->_vbo.p[ii]);
				FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,node->_vbo.p[ii]);
				//glEnableVertexAttribArray( LOC );
				float *fdata = (float*) malloc(size);
				for (cgltf_size index = 0; index < blob->count; index++)
				{
					cgltf_accessor_read_float(blob, index, &fdata[index*3], 3);
					printf("%d %f %f %f\n", index, fdata[index*3 +0],fdata[index*3 +1],fdata[index*3 +2]);
				}
				glBufferData(GL_ARRAY_BUFFER,size,fdata, GL_STATIC_DRAW);
				FW_GL_BINDBUFFER(GL_ARRAY_BUFFER, 0);

				//FW_GL_NORMAL_POINTER(GL_FLOAT, size, fdata); 

				//glVertexAttribPointer( LOC ,isize, GL_FLOAT, TRUE, blob->stride, fdata);
			}else{
				FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,node->_vbo.p[ii]);
				FW_GL_NORMAL_POINTER(GL_FLOAT,0,0);
			}

			break;

			case cgltf_attribute_type_texcoord:
			//copy to texcoord
			if(node->_vbo.p[ii] == -1){
				glGenBuffers(1,(GLuint*) &node->_vbo.p[ii]);
				FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,node->_vbo.p[ii]);
				glBufferData(GL_ARRAY_BUFFER,size,data, GL_STATIC_DRAW);
				//glEnableVertexAttribArray(node->_vbo.p[ii]);
				float *fdata = (float*) malloc(size);
				for (cgltf_size index = 0; index < blob->count; index++)
				{
					cgltf_accessor_read_float(blob, index, &fdata[index*2], 2);
					printf("%d %f %f \n", index, fdata[index*2 +0],fdata[index*2 +1]);
				}
				glBufferData(GL_ARRAY_BUFFER,size,fdata, GL_STATIC_DRAW);
				FW_GL_BINDBUFFER(GL_ARRAY_BUFFER, 0);

				//FW_GL_TEXCOORD_POINTER(2, GL_FLOAT, size, fdata,0); 

				//glVertexAttribPointer(node->_vbo.p[ii],isize, GL_FLOAT, TRUE, blob->stride, data);
			}else{
				FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,node->_vbo.p[ii]);
				FW_GL_TEXCOORD_POINTER(2,GL_FLOAT,0,0,0);
			}

			break;

			//cooy to vertex color vbo
			//if (r->color) {
			//	if (r->VBO_buffers[COLOR_VBO] == 0) glGenBuffers(1,&r->VBO_buffers[COLOR_VBO]);            
			//	FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,r->VBO_buffers[COLOR_VBO]);
			//	glBufferData(GL_ARRAY_BUFFER,r->ntri*sizeof(struct SFColorRGBA)*3,r->color, GL_STATIC_DRAW);
			//	// needed by recalculateColorField ... FREE_IF_NZ(r->color);
			//}
			//if (newfog) {
			//	if (r->VBO_buffers[FOG_VBO] == 0) glGenBuffers(1,&r->VBO_buffers[FOG_VBO]);            
			//	FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,r->VBO_buffers[FOG_VBO]);
			//	glBufferData(GL_ARRAY_BUFFER,r->ntri*sizeof(float)*3,r->actualFog, GL_STATIC_DRAW);
			//}



			default:
			break;
		}

	}
	if(acount && prim && prim->indices){
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
	{
		// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glDrawElements.xhtml
		const cgltf_accessor* blob = prim->indices;
		int ntri = blob->count / 3;
		static int *indexs = NULL;
		static unsigned short *uindexs = NULL;
		int isize = 1;
		size_t size = blob->count * isize * sizeof(int);

		if(node->_vbo.p[acount] == -1){
			cgltf_uint element_int;

			unsigned int *indu = malloc(blob->count * sizeof(unsigned int));
			uindexs = malloc(blob->count * sizeof(unsigned short));
			for (int i = 0; i < ntri; i++)
			{
				for(int j=0;j<3;j++){
					int index = (i*3)+j;
					cgltf_accessor_read_uint(blob, index, &element_int, 1);
					indu[index] = element_int;
					uindexs[index] = element_int;
				}
			}
			FW_GL_BINDBUFFER(GL_ELEMENT_ARRAY_BUFFER,node->_vbo.p[acount]);
			indexs = indu;
 			glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indu, GL_STATIC_DRAW);
			FW_GL_BINDBUFFER(GL_ELEMENT_ARRAY_BUFFER, 0);

		}else{
			FW_GL_BINDBUFFER(GL_ELEMENT_ARRAY_BUFFER,node->_vbo.p[acount]);
 			//glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, 0,0);
			//FW_GL_ELEMENT_POINTER(2,GL_FLOAT,0,0,0);
			
		}
			/*
			glGenBuffers(1, &node->_vbo.p[acount]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, node->_vbo.p[acount]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, blob->count  * sizeof(unsigned int), &indu[0], GL_STATIC_DRAW);
		}else{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, node->_vbo.p[acount]);
		}		*/
		//sendArraysToGPU(GL_TRIANGLES,0,ntri*3);
		//sendArraysToGPU(GL_TRIANGLES,0,0);
		sendElementsToGPU(GL_TRIANGLES,ntri*3,uindexs);  //WORKS
		//sendElementsToGPU(GL_TRIANGLES,0,NULL);

		//glDrawElements(	GL_TRIANGLES, ntri*3, GL_UNSIGNED_INT, indexs); //WORKS
	}
	printf("done render_BufferGeometry\n");
	//FW_GL_BINDBUFFER(GL_ARRAY_BUFFER, node->__cylinderVBO);

	//FW_GL_VERTEX_POINTER(3, GL_FLOAT, (GLsizei)sizeof(struct MyVertex), (GLfloat *)BUFFER_OFFSET(0));
	//	//The starting point of the VBO, for the vertices
	//if(DESIRE(getShaderFlags().base,SHADINGSTYLE_FLAT)){
	//	//The starting point of normals, (3+3+2)*4  = 32 +  bytes away
	//	FW_GL_NORMAL_POINTER(GL_FLOAT, (GLsizei) sizeof(struct MyVertex), (GLfloat *)BUFFER_OFFSET(32));   
	//}else{
	//	//The starting point of normals, 3*4  = 12 +  bytes away
	//	FW_GL_NORMAL_POINTER(GL_FLOAT, (GLsizei) sizeof(struct MyVertex), (GLfloat *)BUFFER_OFFSET(12));   
	//}
	/* set up texture drawing for this guy */
	//mtf.pre_canned_textureCoords = NULL;
	//mtf.TC_size = 2;
	//mtf.TC_type = GL_FLOAT;
	//mtf.TC_stride = sizeof(struct MyVertex);
	//mtf.TC_pointer = BUFFER_OFFSET(24);
	//textureCoord_send(&mtf);
	/* FW_GL_BINDBUFFER(GL_ELEMENT_ARRAY_BUFFER, ConeIndxVBO); */

	//if(DESIRE(getShaderFlags().base,SHADINGSTYLE_WIRE)){
	//	//wireframe triangles
	//	sendElementsToGPU(GL_LINES,node->__cylinderTriangles *2,node->__wireindices);
	//}else{
//		sendArraysToGPU(GL_TRIANGLES,0,node->_vbo.p[0]);
	//}
	/* turn off */
	FW_GL_BINDBUFFER(GL_ARRAY_BUFFER, 0);
//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);	
}
void rendray_BufferGeometry(struct X3D_BufferGeometry *node){
}
void collide_BufferGeometry(struct X3D_BufferGeometry *node){
}
