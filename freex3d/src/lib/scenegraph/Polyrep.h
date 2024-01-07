/*


Polyrep ???

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
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "LinearAlgebra.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

//buffer list is per context (Scene, Proto, Inline) 
// ..so buffer can be shared between shape nodes in same context, 
//..unloaded when inline or scene unloaded or users = 0
struct geomBuffer {
	char* address; //owns it
	char* cgltf_buffer; //used during parsing for detecting shared buffer
	int byteSize;
	int loaded; //FALSE until data copied in, even if allocated, alows delay-loading
	int users; //when falls to zero, free()
	GLuint VBO;
};
//in gltf the valance isn't accessor 1:1 bufferView
// in freewrl we assume 1:1 and deep copy the bufferAccess for each GeomRep when m:1
struct bufAccess {
	int in_use; //1= have attribute (fog, UV, ColorPerVertex) 0= don't have.
	//untyped access, like gltf bufferView
	int byteOffset; //multiple arrays and even multiple shapes can share same blob buffer.
	int byteStride; //position, normal, color-per-vertex, UV[4] can be per-vertex, index by itself
	//typed access, like gltf accessor
	int dataType; // componentType GL_BYTE 5120, GL_SHORT, GL_INT, GL_FLOAT. GL_DOUBLE.
	int dataSize; // type 1-SCALAR, 2-VEC2 3 VEC3 4 VEC4 9 MAT3 16 MAT4
	int byteSize; // = sizeof(dataType) x dataSize
};



// structs that go in void * node->_intern field
// basically a dumping ground for node-specific states that don't belong in public fields
struct X3D_InternalRep {
	//abstract type for all that go in _intern
	int itype; //0 PointRep 1 LineRep 2 PolyRep 3 MeshRep 4 TextureRep 5 LightRep 6 ProjectorRep 7 SoundRep 8 MidiRep 9 HanimRep
};

struct X3D_HanimRep {
	int itype; //9 HanimRep
	int* PVI; //skinning per-vertex joint matrix index, 4 per vertex
	float* PVW; //skinning per-vertex weight applied to joint matrix, 4 per vertex
	int NV; //number of vertices in skin
	GLuint bo_PVI; //GPU skinning, buffer object bo_ 
	GLuint bo_PVW;
	Stack* JT; //joint transforms
	GLuint bo_JT;
	float* jt32;
	GLuint bo_JN;
	float* jn32;
	int have_skin;
	int joint_changed;
	int PVset;
	//GPU joint displacer method: packed displace array with int displace[dindex[cindex]]
	int joint_displacer_count;
	int dindex_done;
	Stack* dindex_lookup;
	GLuint bo_dindex;
	int* dindex; //dindex[NV] per-original-vertex index into packed displace array
	GLuint bo_displace;
	float* displace; //[4*(ND+1)] - 0th is 0,0,0 
	int ND; //number of unique-vertex displacements
};
struct X3D_TextureRep {
	int itype; //0 PointRep 1 LineRep 2 PolyRep 3 MeshRep 4 TextureRep
	int decoded;  //0=still .jgp/.png/.gif 1=parsed into rectangular rgba texture blob
	int byteOffset; //multiple arrays and even multiple shapes can share same blob buffer.
	int byteSize; //..and the image stored is compressed png/jpeg etc blob, needs to be parsed to get rectangular image
	char* mimeType; //"image/png", "image/jpg" etc so correct image parser can be applied
	//shared buffer approach:
	// indirection to sharable, delay-loadable buffer
	struct geomBuffer* buffer;
	//struct bufAccess image; //
};
void* set_TextureRep(void* _texrep);

struct X3D_GeomRep {
	//abstract type for geometry node types node. _intern field
	int itype; //0 PointRep 1 LineRep 2 PolyRep 3 MeshRep 4 TextureRep
	int mode;  //0 Points 1-3 lines 4-6 mesh
//	void* ectx; //execution context (scene, proto, inline) - where to store shareable buffers
};

struct X3D_LineRep {
	// used for node._intern field by linetype geometry nodes
	// will hold commmon GL_LINE_STRIP parameters from
	// PolyLine2D, Arc2D, ArcClose2D_LINE, Circle2D
	// LineSet, IndexedLineSet
	// analogous to PolyRep for triangle nodes
	// motivation for this extra level of common abstraction for lines:
	// - Appearance.LineProperties.linetype - dashed lines require extra prev,next vertices and other info sent
	//   (glLineStipple not working with our shader system)
	int itype; //0 PointRep 1 LineRep 2 PolyRep 3 MeshRep 4 TextureRep
	int mode;  //0 Points 1-3 lines 4-6 mesh: 1 LINES 	2 LINE_LOOP 3 LINE_STRIP
	void* ectx; //execution context (scene, proto, inline)
	int npoint;
	struct SFVec3f* point;
	struct SFVec2f* point2D;
	struct SFVec3f* prev;
	struct SFVec3f* next;
	int nsegments;
	int* start;
	int* count;
	float* fogcoord;
	struct SFColor* color;
	struct SFColorRGBA* colorRgba;
	int* skindex; //LineSet, IndexedLineSet only, which have coord field which could be humanod.coord Coordinate node
	void* coordinate_node; //we need to know when we are skinning, populate during make_polyrep for testing against a stack push of humanoid.coord
};

struct X3D_TexturableGeomRep {
	//abstract type for geometry node types node. _intern field where geometry can be textured
	int itype; //0 PointRep 1 LineRep 2 PolyRep 3 MeshRep 4 TextureRep
	int mode;  //0 Points 1-3 lines 4-6 mesh
	char* map[4]; //strings from TextureCoordinate.mapping field, or null if .mapping is null or type-of-geometry node had no explicit TextureCoordinate field
//	void* ectx; //execution context (scene, proto, inline) - where to store shareable buffers
};

struct X3D_PointRep {
	// node._intern field for Polypoint2D and PointSet geometry nodes
	int itype; //0 PointRep 1 LineRep 2 PolyRep 3 MeshRep 4 TextureRep
	int mode;  //0 Points 1-3 lines 4-6 mesh
	//for pointrep the map field applies to splat multitexture from appearance.texture field
	char* map[4]; //strings from TextureCoordinate.mapping field, or null if .mapping is null or type-of-geometry node had no explicit TextureCoordinate field
	int ncoord;
	//shared buffer approach:
	// indirection to sharable, delay-loadable buffer
	struct geomBuffer* buffer;
	struct bufAccess attrib[3]; //vertex coord, color per vertex, fog per vertex
	void* coordinate_node; //we need to know when we are skinning, populate during make_polyrep for testing against a stack push of humanoid.coord
};
int lookup_dataType_size(int dataType); //GL_FLOAT -> 4 GL_SHORT - 2
int set_Attrib(struct bufAccess* ba, int dataSize, int dataType, int byteOffset);
char* get_Attribi(struct bufAccess* ba, struct geomBuffer* gb, int index);
struct geomBuffer* add_geomBuffer(int buffersize, int users);
struct geomBuffer* add_geomBuffer0(void* ectx, int buffersize, int users);
void set_geomBuffer(struct geomBuffer* gb);
void update_geomBufferSize(struct geomBuffer* gb, int buffersize);
void remove_geomBuffer(struct geomBuffer* gb);
void subtract_geomBufferUser(struct geomBuffer* gb);
void add_geomBufferUser(struct geomBuffer* gb);
struct geomBuffer* find_buffer_in_broto_context_from_cgltf_buffer(void* ectx, void* cgltf_buffer);
void* set_PointRep(void* _pointrep, float* points, int pointSize, int npoint,
	float* color, int colorSize, int ncolor, float* fog, int nfog);
void render_PointRep(void* pointrep);
void delete_PointRep(void* pointrep);

struct X3D_MeshRep {
	//MeshRep node._intern for BufferGeometry node used by gltf_loader to load generic geometry via Inline url .gltf or .glb
	int itype; //0 PointRep 1 LineRep 2 PolyRep 3 MeshRep 4 TextureRep
	int mode;  //0 Points 1-3 lines 4-6 mesh
	char* map[4]; //strings from TextureCoordinate.mapping field, or null if .mapping is null or type-of-geometry node had no explicit TextureCoordinate field
	int ncoord;
	int nuv; //number of texture coordinate channels
	int flipuv; // 0=uvs are y-up (x3d default), 1=uvs are y-down (gltf) 
	//shared buffer approach:
	// indirection to sharable, delay-loadable buffer
	struct geomBuffer* buffer;
	struct bufAccess attrib[8]; //vertex coord, color per vertex, fog per vertex, normal per vertex, UV per vertex (up to nuv=4)
	int nindex;
	struct bufAccess index;
};
void* set_MeshRep(void* _meshrep);
void render_MeshRep(void* meshrep);
void delete_MeshRep(void* meshrep);
void delete_LightRep(void* _lightrep);
void delete_HanimRep(void* _hanimrep);

/* Internal representation of IndexedFaceSet, Text, Extrusion & ElevationGrid:
 * set of triangles.
 * done so that we get rid of concave polygons etc.
 */
struct X3D_PolyRep { /* Currently a bit wasteful, because copying */
	int itype; //0 PointRep 1 LineRep 2 PolyRep 3 MeshRep 4 TextureRep
	int mode;  //0 Points 1-3 lines 4-6 mesh: 4 TRIANGLES 5 TRIANGLE_STRIP 6 TRIANGLE_FAN
	char* map[4]; //strings from TextureCoordinate.mapping field, or null if .mapping is null or type-of-geometry node had no explicit TextureCoordinate field
	void* ectx; //execution context (scene, proto, inline)
	int irep_change;
	int ccw;	/* ccw field for single faced structures */
	int ntri; /* number of triangles */
	int streamed;	/* is this done the streaming pass? */

	/* indicies for arrays. OpenGL ES 2.0 - unsigned short for the DrawArrays call */
	GLuint* cindex;   /* triples (per triangle) */
	GLuint* colindex;   /* triples (per triangle) */
	GLuint* norindex;
	GLuint* tcindex; /* triples or null */
	GLuint* tri_indices;
	GLuint* wire_indices;
	GLuint* oindex; //original coordinate indexes before streaming

	float* actualCoord; /* triples (per point) */
	float* actualFog; /* float (per point) */
	float* color; /* triples or null */
	float* normal; /* triples or null */
	float* flat_normal; /*triples or null*/
	int last_normal_type; /* 0=regular 1=flat last normal type we put in the vbo normal buffer */
	int last_index_type; /* 0=regular 1=wire last vertex index type we put in the vbo index buffer */
	float* GeneratedTexCoords[4];	/* triples (per triangle) of texture coords if there is no texCoord node */
	int ntexdim[4];  /* number of texture coordinate dimensions, normally 2 xy, 3 xyz, 4 xyzw */
	int ntcoord;		/* number of multitextureCoordinates */
	int tcoordtype; /* type of texture coord node - is this a NODE_TextureCoordGenerator... */
	int texgentype; /* if we do have a TextureCoordinateGenerator, what "TCGT_XXX" type is it? */
	GLfloat minVals[3];		/* for collision and default texture coord generation */
	GLfloat maxVals[3];		/* for collision and default texture coord generation */
	int isRGBAcolorNode;		/* color was originally an RGBA, DO NOT re-write if transparency changes */
	GLuint VBO_buffers[VBO_COUNT];		/* VBO indexen */
	void* coordinate_node; //we need to know when we are skinning, populate during make_polyrep for testing against a stack push of humanoid.coord
};
void findExtentInCoord0(struct X3D_Node* node, int count, float* coord, int dimensions);

/* transformed ray */
//extern struct point_XYZ t_r1;
//extern struct point_XYZ t_r2;
//extern struct point_XYZ t_r3;

struct facepar {
int OK;
int start;
int end;
float normal[3];
float color[3];
float colorRGBA[3];
};

int count_IFS_faces(int cin, struct Multi_Int32 *coordIndex, struct facepar *faceok);

int 
IFS_face_normals(struct SFVec3f *facenormals, //struct point_XYZ *facenormals,
				 struct facepar *faceok,
				 int *pointfaces,
				 int faces,
				 int npoints,
				 int cin,
				 struct SFVec3f *points,
				 struct Multi_Int32 *coordIndex,
				 int ccw);

void
IFS_check_normal(struct SFVec3f *facenormals, //struct point_XYZ *facenormals,
				 int this_face,
				 struct SFVec3f *points,
				 int base,
				 struct Multi_Int32 *coordIndex,
				 int ccw);

void
add_to_face(int point,
			int face,
			int *pointfaces);

void
Elev_Tri(int vertex_ind,
		 int this_face,
		 int A,
		 int D,
		 int E,
		 int NONORMALS,
		 struct X3D_PolyRep *this_Elev,
		 struct SFVec3f *facenormals, //struct point_XYZ *facenormals,
		 int *pointfaces,
		 int ccw);

void
Extru_tex(int vertex_ind,
		  int tci_ct,
		  int A,
		  int B,
		  int C,
		  GLuint *tcindex,
		  int ccw,
		  int tcindexsize);

void Extru_ST_map(
        int triind_start,
        int start,
        int end,
        float *Vals,
        int nsec,
        GLuint *tcindex,
        GLuint *cindex,
        float *GeneratedTexCoords,
        int tcoordsize);

void
Extru_check_normal(struct SFVec3f *facenormals, //struct point_XYZ *facenormals,
				   int this_face,
				   int dire,
				   struct X3D_PolyRep *rep_,
				   int ccw);

void
do_color_normal_reset(void);

void
do_glNormal3fv(struct SFVec3f *dest, GLfloat *param);

void stream_polyrep(void *node, void *coord, void *fogCoord, void *color, void *normal, struct X3D_TextureCoordinate *texCoord);
void compile_polyrep(void *node, void *coord, void *fogCoord, void *color, void *normal, struct X3D_TextureCoordinate *texCoord);

struct intersection_info{
	float dist;
	float p[3];
	float normal[3];
	float texcoord[3];
};
int intersect_polyrep2(struct X3D_Node *node, float *p1, float *p2, Stack *intersection_stack);
void render_ray_polyrep(void *node);