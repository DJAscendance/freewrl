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

/* Internal representation of IndexedFaceSet, Text, Extrusion & ElevationGrid:
 * set of triangles.
 * done so that we get rid of concave polygons etc.
 */
struct X3D_PolyRep { /* Currently a bit wasteful, because copying */
	int itype;
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