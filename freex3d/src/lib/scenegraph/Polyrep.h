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


//gltf componentTypes
#define GLTF_BYTE 5120
#define GLTF_UNSIGNED_BYTE 5121
#define GLTF_SHORT 5122 
#define GLTF_UNSIGNED_SHORT 5123 //– used with SCALAR for indices
#define GLTF_UNSIGNED_INT 5125 
#define GLTF_FLOAT 5126 //– used with VEC3 for POSITIONand NORMALand with VEC2and TEXCOORD_0
//gltf targets
#define GLTF_ARRAY_BUFFER 34962
#define GLTF_ELEMENT_ARRAY_BUFFER 34963

//gltf types "SCALAR" "VEC2" "VEC3" "VEC4" "MATRIX2" "MATRIX3" "MATRIX4"
#define GLTF_SCALAR 0
#define GLTF_VEC2 1
#define GLTF_VEC3 2
#define GLTF_VEC4 3
#define GLTF_MATRIX2 4
#define GLTF_MATRIX3 5
#define GLTF_MATRIX4 6
//buffer list is per context (Scene, Proto, Inline) 
// ..so buffer can be shared between shape nodes in same context, 
//..unloaded when inline or scene unloaded or users = 0
struct buffer {
	void* address; //if not NULL then owns it
	int size;
	int loaded; //FALSE until data copied in, even if allocated
	int users; //when falls to zero, free()
};
//in gltf the valance isn't accessor 1:1 bufferView
// in freewrl we assume 1:1 and deep copy the bufferAccess for each GeomRep when m:1
struct bufferAccess {
	//untyped access, like gltf bufferView
	char* byteAddress; //computed once from buffer.address + byteOffset for convenience
	int byteStride; //position, normal, color-per-vertex, UV[4] can be per-vertex, index by itself
	int byteOffset; //multiple arrays and even multiple shapes can share same blob buffer.
	int buffer; //0 1 2 .. some indirection so shape can check if buffer loaded
	//typed access, like gltf accessor
	int componentType; //BYTE 5120, SHORT, FLOAT ..
	int type; // 0-SCALAR, 1-VEC2..
	int count;
	GLuint VBO;
};
struct bufferAccess buffers[VBO_COUNT];




struct X3D_GeomRep {
	int itype; //0 PointRep 1 LineRep 2 PolyRep
	int mode;  //0 Points 1-3 lines 4-6 mesh
};
struct X3D_PointRep {
	int itype; //0 PointRep 1 LineRep 2 PolyRep
	int mode;  //0 Points 1-3 lines 4-6 mesh
	float *coord;
	float *color;
	float *fog;
	int ncoord;
	GLuint coordVBO;
	GLuint colorVBO;
	GLuint fogVBO;
	//blob method, for fun and for gltf .bin blob preparation
	GLuint blobVBO;
	float* blob;
	int blobSize;
	int coordSize;
	int colorOffset;
	int colorSize;
	int fogOffset;
	int floatStride;

};
void* set_PointRep(void* _pointrep, float* points, int pointSize, int npoint,
	float* color, int colorSize, int ncolor, float* fog, int nfog);
void render_PointRep(void* pointrep);
void delete_PointRep(void* pointrep);

struct X3D_LineRep {
	// will hold commmon GL_LINE_STRIP parameters from
	// PolyLine2D, Arc2D, ArcClose2D_LINE, Circle2D
	// LineSet, IndexedLineSet
	// analogous to PolyRep for triangle nodes
	// motivation for this extra level of common abstraction for lines:
	// - Appearance.LineProperties.linetype - dashed lines require extra prev,next vertices and other info sent
	//   (glLineStipple not working with our shader system)
	int itype; //0 PointRep 1 LineRep 2 PolyRep
	int mode;  //0 Points 1-3 lines 4-6 mesh: 1 LINES 	2 LINE_LOOP 3 LINE_STRIP

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
};
/* Internal representation of IndexedFaceSet, Text, Extrusion & ElevationGrid:
 * set of triangles.
 * done so that we get rid of concave polygons etc.
 */
struct X3D_PolyRep { /* Currently a bit wasteful, because copying */
	int itype; //0 PointRep 1 LineRep 2 PolyRep
	int mode;  //0 Points 1-3 lines 4-6 mesh: 4 TRIANGLES 5 TRIANGLE_STRIP 6 TRIANGLE_FAN
	int irep_change;
	int ccw;	/* ccw field for single faced structures */
	int ntri; /* number of triangles */
	int streamed;	/* is this done the streaming pass? */

	/* indicies for arrays. OpenGL ES 2.0 - unsigned short for the DrawArrays call */
	GLuint* cindex;   /* triples (per triangle) */
	GLuint* colindex;   /* triples (per triangle) */
	GLuint* norindex;
	GLuint* tcindex; /* triples or null */
	ushort* tri_indices;
	ushort* wire_indices;

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