/*


X3D Rendering Component

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
#include "Component_Shape.h"
#include "../scenegraph/RenderFuncs.h"
#include "../scenegraph/Polyrep.h"

#define FW_MAXCLIPPLANES 4

typedef struct pComponent_Rendering{

	Stack *clipplane_stack;
	float clipplanes[4*FW_MAXCLIPPLANES];
}* ppComponent_Rendering;
void *Component_Rendering_constructor(){
	void *v = MALLOCV(sizeof(struct pComponent_Rendering));
	memset(v,0,sizeof(struct pComponent_Rendering));
	return v;
}
void Component_Rendering_init(struct tComponent_Rendering *t){
	//public

	//private
	t->prv = Component_Rendering_constructor();
	{
		ppComponent_Rendering p = (ppComponent_Rendering)t->prv;
		p->clipplane_stack = newStack(usehit);
	}
}
void Component_Rendering_clear(struct tComponent_Rendering *t){
	ppComponent_Rendering p = (ppComponent_Rendering)t->prv;
	deleteVector(struct X3D_Node*,p->clipplane_stack);
}



/* find a bounding box that fits the coord structure. save it in the common-node area for extents.*/
void findExtentInCoord0 (struct X3D_Node *node, int count, float* coord, int dimensions) {
	int i;

	INITIALIZE_EXTENT

	if (!coord || count < 1 || dimensions < 1) return;
	//assume dimensions > dimension are zero ie for 2D assume 3rd dimension is extent 0,0
	if (dimensions < 3)
		node->EXTENT_MAX_Z = node->EXTENT_MIN_Z = 0.0f;
	if (dimensions < 2)
		node->EXTENT_MAX_Y = node->EXTENT_MIN_Y = 0.0f;
	for (i=0; i<count; i++) {
		float* point = &coord[i * dimensions];
		if (point[0] > node->EXTENT_MAX_X) node->EXTENT_MAX_X = point[0];
		if (point[0] < node->EXTENT_MIN_X) node->EXTENT_MIN_X = point[0];
		if (dimensions > 1) {
			if (point[1] > node->EXTENT_MAX_Y) node->EXTENT_MAX_Y = point[1];
			if (point[1] < node->EXTENT_MIN_Y) node->EXTENT_MIN_Y = point[1];
			if (dimensions > 2) {
				if (point[2] > node->EXTENT_MAX_Z) node->EXTENT_MAX_Z = point[2];
				if (point[2] < node->EXTENT_MIN_Z) node->EXTENT_MIN_Z = point[2];
			}
		}
	}
	/* printf ("extents %f %f, %f %f, %f %f\n",node->EXTENT_MIN_X, node->EXTENT_MAX_X,
	  node->EXTENT_MIN_Y, node->EXTENT_MAX_Y, node->EXTENT_MIN_Z, node->EXTENT_MAX_Z); */
}
void findExtentInCoord(struct X3D_Node* node, int count, struct SFVec3f* coord) {
	findExtentInCoord0(node, count, (float*)coord, 3);
}
void render_IndexedTriangleFanSet (struct X3D_IndexedTriangleFanSet *node) {
	//COMPILE_POLY_IF_REQUIRED(node->coord, node->fogCoord, node->color, node->normal, node->texCoord)
	if (!compile_poly_if_required(node, node->coord, node->fogCoord, node->color, node->normal, node->texCoord))return;
	CULL_FACE(node->solid)
	render_polyrep(node);
}

void render_IndexedTriangleSet (struct X3D_IndexedTriangleSet *node) {
	//COMPILE_POLY_IF_REQUIRED(node->coord, node->fogCoord, node->color, node->normal, node->texCoord)
	if (!compile_poly_if_required(node, node->coord, node->fogCoord, node->color, node->normal, node->texCoord))return;
	CULL_FACE(node->solid)
	render_polyrep(node);

}

void render_IndexedTriangleStripSet (struct X3D_IndexedTriangleStripSet *node) {
	//COMPILE_POLY_IF_REQUIRED( node->coord, node->fogCoord, node->color, node->normal, NULL)
	if (!compile_poly_if_required(node, node->coord, node->fogCoord, node->color, node->normal, NULL))return;
	CULL_FACE(node->solid)
	render_polyrep(node);
}

void render_TriangleFanSet (struct X3D_TriangleFanSet *node) {
		//COMPILE_POLY_IF_REQUIRED (node->coord, node->fogCoord, node->color, node->normal, node->texCoord)
		if (!compile_poly_if_required(node, node->coord, node->fogCoord, node->color, node->normal, node->texCoord)) return;
		CULL_FACE(node->solid)
		render_polyrep(node);
}

void render_TriangleStripSet (struct X3D_TriangleStripSet *node) {
	//COMPILE_POLY_IF_REQUIRED(node->coord, node->fogCoord, node->color, node->normal, node->texCoord)
	if (!compile_poly_if_required(node, node->coord, node->fogCoord, node->color, node->normal, node->texCoord))return;
	CULL_FACE(node->solid)
	render_polyrep(node);
}

void render_TriangleSet (struct X3D_TriangleSet *node) {
	//COMPILE_POLY_IF_REQUIRED(node->coord, node->fogCoord, node->color, node->normal, node->texCoord)
	if (!compile_poly_if_required(node, node->coord, node->fogCoord, node->color, node->normal, node->texCoord))return;
	CULL_FACE(node->solid)
	render_polyrep(node);
}



void* set_LineRep(void *_linerep, struct SFVec3f *points, struct SFVec2f *points2D, 
		struct SFColorRGBA *colorRgba, struct SFColor *color, float *fog,
		int nsegments, int *counts, int *starts, int *skindex)
	{
	//to be called from compile_Polyline2D, _Arc2D, _ArcClose2D, _Circle2D, _LineSet, _IndexedLineSet
	if(!_linerep){
		_linerep = MALLOC(struct X3D_LineRep*,sizeof(struct X3D_LineRep));
		memset(_linerep,0,sizeof(struct X3D_LineRep));
	}
	struct X3D_LineRep *linerep = (struct X3D_LineRep *)_linerep;
	linerep->itype = 1; //0 points 1 lines 2 mesh
	linerep->mode = 1; //1 LINES 	2 LINE_LOOP 3 LINE_STRIP
	linerep->point = points;
	linerep->point2D = points2D;
	linerep->colorRgba = colorRgba;
	linerep->color = color;
	linerep->fogcoord = fog;
	linerep->skindex = skindex;
	linerep->count = counts;
	linerep->start = starts;
	linerep->nsegments = nsegments;
	linerep->npoint = 0;
	for(int i=0;i<nsegments;i++){
		linerep->npoint += counts[i];
	}
	//printf("nseg %d",nsegments);
	//for(int i=0;i<nsegments;i++){
	//	printf("[%d] count %d start %d\n",i,linerep->count[i],linerep->start[i]);
	//}
	return linerep;
}
void clear_LineRep(void *_linerep){
	if(_linerep){
		struct X3D_LineRep *linerep = (struct X3D_LineRep *)_linerep;
		//doesnt own these, caller does
		//FREE_IF_NZ(linerep->point);
		//FREE_IF_NZ(linerep->point2D);
		//FREE_IF_NZ(linerep->colorRgba);
		//FREE_IF_NZ(linerep->color);
		//FREE_IF_NZ(linerep->start);
		//owns this
		FREE_IF_NZ(linerep->prev);
		FREE_IF_NZ(linerep->next);
		memset(_linerep,0,sizeof(struct X3D_LineRep));
	}
}
#define DESIRE(whichOne,zzz) ((whichOne & zzz)==zzz)
void render_LineRep(struct X3D_LineRep *linerep){
	//to be called from render_Polyline2D, _Arc2D, _ArcClose2D, _Circle2D, _LineSet, _IndexedLineSet
	if (linerep && linerep->nsegments > 0) {
		if (linerep->color) {
			FW_GL_COLOR_POINTER (3,GL_FLOAT,0,(float *)linerep->color);
		} else if(linerep->colorRgba) {
			FW_GL_COLOR_POINTER (4,GL_FLOAT,0,(float *)linerep->colorRgba);
		}
		if (linerep->fogcoord) {
			FW_GL_FOG_POINTER (GL_FLOAT,0,(float*)linerep->fogcoord);
		}
		if(linerep->point){
			FW_GL_VERTEX_POINTER (3,GL_FLOAT,0,(float *)linerep->point);
		}else if(linerep->point2D){
			FW_GL_VERTEX_POINTER (2,GL_FLOAT,0,(float *)linerep->point2D);
		}
		if (linerep->skindex) {
			FW_GL_CINDEX_POINTER(GL_INT, 0, (int*)linerep->skindex);
		}
		if(getAppearanceProperties()->linetype > 1){
			//uh-oh - glLineStipple broken and someone wants a dashed line. We'll make our own
			//all the home-made dashed line algos send previous and next point as attribute arrays to vertex shader
			if(!linerep->prev){
				linerep->prev = MALLOC(struct SFVec3f*,linerep->npoint*3*sizeof(float)); 
				for(int i=1;i<linerep->npoint;i++){
					if(linerep->point2D){
						veccopy2f(linerep->prev[i].c,linerep->point2D[i-1].c);
					}else if(linerep->point){
						veccopy3f(linerep->prev[i].c,linerep->point[i-1].c);
					}
				}
				if(linerep->point2D){
					veccopy2f(linerep->prev[0].c,linerep->point2D[0].c);
				}else if(linerep->point){
					veccopy3f(linerep->prev[0].c,linerep->point[0].c);
				}
			}
			//for GL_LINE_STRIP we don't need the nexts in the shader
			if(1) if(!linerep->next){
				linerep->next = MALLOC(struct SFVec3f*,linerep->npoint*3*sizeof(float)); 
				if(0){
					//conventinoal next point like prev
					for(int i=0;i<linerep->npoint-1;i++){
						if(linerep->point2D){
							veccopy2f(linerep->next[i].c,linerep->point2D[i+1].c);
						}else if(linerep->point){
							veccopy3f(linerep->next[i].c,linerep->point[i+1].c);
						}
					}
					if(linerep->point2D){
						veccopy2f(linerep->next[linerep->npoint-1].c,linerep->point2D[linerep->npoint-1].c);
					}else if(linerep->point){
						veccopy3f(linerep->next[linerep->npoint-1].c,linerep->point[linerep->npoint-1].c);
					}
				}else if(1){
					//float (index,segmentcount) aka findex method for vertex shader to determine 
					// if its on a starting or ending line of a polyline
					// assumes vertexes are packed in order of segments. good luck
					int knext = 0;
					for (int i=0; i<linerep->nsegments; i++) {
						for(int j=0;j<linerep->count[i];j++) {
							linerep->next[knext].c[0] = (float)j;
							linerep->next[knext].c[1] = (float)linerep->count[i];
							knext++;
						}
					}
					if(knext != linerep->npoint) printf("ouch in render_LineRep findexes %d points %d\n",knext,linerep->npoint);
				}
			}

		    s_shader_capabilities_t *me = getAppearanceProperties()->currentShaderProperties;
			if (me->prevVertex != -1) {
				glEnableVertexAttribArray(me->prevVertex);
				glVertexAttribPointer(me->prevVertex, 3, GL_FLOAT, FALSE, 0, linerep->prev);
			}
			if (me->nextVertex != -1 && linerep->next) {
				glEnableVertexAttribArray(me->nextVertex);
				glVertexAttribPointer(me->nextVertex, 3, GL_FLOAT, FALSE, 0, linerep->next);
			}

		}
		
		for (int i=0; i<linerep->nsegments; i++) {
			//https://www.khronos.org/registry/OpenGL-Refpages/es3.0/html/glDrawArrays.xhtml
        	sendArraysToGPU (GL_LINE_STRIP, linerep->start[i], linerep->count[i]);

		}
	}
}


void compile_IndexedLineSet (struct X3D_IndexedLineSet *node) {
	int i, ivertex;		/* temporary */
	struct SFVec3f *points;
	struct SFVec3f *newpoints;
	struct SFVec3f *oldpoint;
	struct SFColorRGBA *newcolors;
	struct SFColorRGBA *oldcolor;
	int* skindex;

	int npoints;
	int maxCoordFound;		/* for bounds checking				*/
	struct X3D_Color *cc;
	int nSegments;			/* how many individual lines in this shape 	*/
	int ipoly;			/* for colours, !cpv, this is the color index 	*/
	int nVertices;			/* how many vertices in the streamed set	*/
	int segLength;			/* temporary					*/
	int *vertCountPtr;		/* temporary, for vertexCount filling		*/
	int **indxStartPtr;	/* temporary, for creating pointer to index arr */
	//int *vertCountPtr;		/* temporary, for vertexCount filling		*/
	//int **indxStartPtr;	/* temporary, for creating pointer to index arr */
	float *fog, *newfog;
	int nfog;

	int * pt;
	//int * pt;
	int vtc;			/* temp counter - "vertex count"		*/
	int curcolor;			/* temp for colorIndexing.			*/
	int * colorIndInt;			/* used for streaming colors			*/
	int * colorIndShort;			/* used for streaming colors			*/
	//int * colorIndShort;			/* used for streaming colors			*/
	int * starts; //array of starting indexes integers, used for glDrawArrays method (vs glDrawElements that takes array of index pointers)
	int * counts; //array of point counts per segment

	/* believe it or not - material emissiveColor can affect us... */
	GLfloat defcolorRGBA[] = {1.0f, 1.0f, 1.0f,1.0f};

    /* we either use the (sizeof (int)) indices passed in from user, or calculated int one */
	colorIndInt = NULL;
	colorIndShort = NULL;

	MARK_NODE_COMPILED
	nSegments = 0;
	node->__segCount = 0;
	clear_LineRep(node->_intern);

	/* ok, what we do is this. Although this is Indexed, colours and vertices can have
	   different indexes; so we make them all the same. To do this, we create another
	   index (really simple one - element x contains x...) that we send to the OpenGL
	   calls. 

	   So first, we find out the maximum coordinate requested in the IndexedLineSet,
	   AND, we find out how many line segments there are.
	*/

	if (node->coord) {
		struct Multi_Vec3f *dtmp;
		dtmp = getCoordinate (node->coord, "IndexedLineSet");
		npoints = dtmp->n;
		points = dtmp->p;

		/* find the extents */
		findExtentInCoord(X3D_NODE(node), npoints, points);
	} else {
		return; /* no coordinates - nothing to do */
	}

	if (node->coordIndex.n == 0) return; /* no coord indexes - nothing to do */

	/* sanity check that we have enough coordinates */
	maxCoordFound = -1000;
	nSegments = 1;
	nVertices = 0;
	for (i=0; i<node->coordIndex.n; i++) {
		/* make sure that the coordIndex is greater than -1 */
		if (node->coordIndex.p[i] < -1) {
			ConsoleMessage ("IndexedLineSet - coordIndex less than 0 at %d\n",i);
			return;
		}

		/* count segments; dont bother if the very last number is -1 */
		if (node->coordIndex.p[i] == -1) {
			if (i!=((node->coordIndex.n)-1)) nSegments++;
		} else nVertices++;

		/* try to find the highest coordinate index for bounds checking */
		if (node->coordIndex.p[i] > maxCoordFound) maxCoordFound = node->coordIndex.p[i];
	}
	if (maxCoordFound > npoints) {
		//JAS - cheat - remove the BoundingBox for root node by simply making the coordinate element count 0,
		//JAS - but if we do that, we get this console message.
		//JAS ConsoleMessage ("IndexedLineSet - not enough coordinates - coordindex contains higher index\n");
		return;
	}

	/* so, at this step, we know how many line segments "nSegments" we require (starting at 0)
	   and, what the maximum coordinate is. So, lets create the new index... 
	   create the index for the arrays. Really simple... Used to index
	   into the coords, so, eg, __vertArr is [0,1,2], which means use
	   coordinates 0, 1, and 2 
	   
		https://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/rendering.html#IndexedLineSet
		11.4.5 has a 2D table for when 

		1) color field not NULL
					CPV=true						CPV=false
		colorIndex	A colorIndex	per vertex		C colorIndex[ipoly] per polyline
		NULL		B coordIndex	per vertex		D Color[ipoly] per polyline

		can be passed to opengl as:
			A. GL_LINE_STRIP vertex 1:1 color
			B. GL_LINE_STRIP vertex 1:1 color
			C. GL_LINE_STRIP vertex m:1 constant color for polyline
			D. GL_LINE_STRIP vertex m:1 constant color for polyline
		and tell the vetex shader to look in attribute fw_Color for a color
			- and do that in compile_shape > whichShapeColorShader = getShapeColourShader(node) and shape node->_shaderflags_base |= whichShapeColorShader

		2) color field NULL
		- if Material > material.emissive
		-- else (1,1,1)
	   
   */
	FREE_IF_NZ (node->__vertArr);
	node->__vertArr = MALLOC (int *, sizeof(int)*(nVertices+1));
	FREE_IF_NZ(node->__skindex);

	pt = (int *)node->__vertArr;
	//node->__vertArr = MALLOC (int *, sizeof(int)*(nVertices+1));
	//pt = (int *)node->__vertArr;

	for (vtc = 0; vtc < nVertices; vtc++) {
		*pt=vtc; pt++; /* ie, index n contains the number n */
	}

    
	/* now, lets go through and; 1) copy old vertices into new vertex array; and 
	   2) create an array of indexes into "__vertArr" for sending to the GL call */


	FREE_IF_NZ (node->__vertIndx);
	FREE_IF_NZ (node->__starts);
	node->__vertIndx = MALLOC (int **,sizeof(int*)*(nSegments+2));
	node->__starts = MALLOC (int *, sizeof(int)*(nSegments+2));
	//node->__vertIndx = MALLOC (int **,sizeof(int*)*(nSegments+2));
	//printf("mallocing %d segnments at address %u\n",nSegments+2,node->__vertIndx);
	FREE_IF_NZ (node->__vertices);
	node->__vertices = MALLOC (struct SFVec3f *, sizeof(struct SFVec3f)*(nVertices+1));

	FREE_IF_NZ (node->__vertexCount);
	FREE_IF_NZ (node->__counts);
	node->__vertexCount = MALLOC (int *,sizeof(int)*(nSegments+2));
	node->__counts = MALLOC (int *, sizeof(int)*(nSegments+2));
	//node->__vertexCount = MALLOC (int *,sizeof(int)*(nSegments+2));

	int *colorIndInt2 = NULL;

		/* do we have to worry about colours? */
	/* sanity check the colors, if they exist */
	if (node->color) {
		/* we resort the color nodes so that we have an RGBA color node per vertex */
		FREE_IF_NZ (node->__xcolours);
		node->__xcolours = MALLOC (struct SFColorRGBA *, sizeof(struct SFColorRGBA)*(nVertices+1));
		newcolors = (struct SFColorRGBA *) node->__xcolours;
		POSSIBLE_PROTO_EXPANSION(struct X3D_Color *, node->color,cc)
		/* cc = (struct X3D_Color *) node->color; */
		if(cc) 
		if ((cc->_nodeType != NODE_Color) && (cc->_nodeType != NODE_ColorRGBA)) {
			ConsoleMessage ("make_IndexedLineSet, color node, expected %d got %d\n", NODE_Color, cc->_nodeType);
			return;
		}

		/* 4 choices here - we have colorPerVertex, and, possibly, a ColorIndex */

		if (node->colorPerVertex) {
			/* assume for now that we are using the coordIndex for colour selection */
			colorIndInt = node->coordIndex.p; 
			/* so, we have a color per line segment. Lets check this stuff... */
			if ((node->colorIndex.n)>0) {
				if ((node->colorIndex.n) < (node->coordIndex.n)) {
					ConsoleMessage ("IndexedLineSet - expect more colorIndexes to match coords\n");
					return;
				}
				//cae A use colorIndex[ivertex] to get a color per vertex
                colorIndInt = node->colorIndex.p; /* use ColorIndex */
			} else {
				//cae B use coordIndex[ivertex] to get a color per vertex
				//colorIndShort = node->__vertArr;
				colorIndInt2 = node->coordIndex.p;
			}
		} else {

			/* so, we have a color per polyline. Lets check this stuff... */
			if ((node->colorIndex.n)>0) {
				if ((node->colorIndex.n) < (nSegments)) {
					ConsoleMessage ("IndexedLineSet - expect more colorIndexes to match coords\n");
					return;
				}
				// case C. use colorIndex[ipolyline] to get a color per polyline
				colorIndInt = node->colorIndex.p; /* use ColorIndex */
			} else {
				/* we are using the simple index for colour selection */
				// case D: use [jpolyline] to get color per polyline
				colorIndShort = node->__vertArr;
				//colorIndInt2 = node->coordIndex.p;                 
			}
		}

	}
	fog = newfog = NULL;
	nfog = 0;
	if(node->fogCoord){
		struct X3D_FogCoordinate *fc;
		FREE_IF_NZ (node->__xfog);
		node->__xfog = MALLOC (float *, sizeof(float)*(nVertices+1));
		newfog = node->__xfog;
		POSSIBLE_PROTO_EXPANSION(struct X3D_FogCoordinate *, node->fogCoord,fc)
		/* cc = (struct X3D_Color *) node->color; */
		if(fc) {
			if (fc->_nodeType != NODE_FogCoordinate) {
				ConsoleMessage ("make_IndexedLineSet, fog node, expected %d got %d\n", NODE_FogCoordinate, fc->_nodeType);
				return;
			}
			fog = fc->depth.p;
			nfog = fc->depth.n;
		}
		//use node->coordindex.p to get a fog
	}


	indxStartPtr = (int **)node->__vertIndx;
	starts = (int *)node->__starts;
	//indxStartPtr = (int **)node->__vertIndx;
	//printf("0 address %u\n",indxStartPtr);
	newpoints = node->__vertices;
	vertCountPtr = (int *) node->__vertexCount;
	//vertCountPtr = (int *) node->__vertexCount;
    counts = (int *)node->__counts;
	pt = (int *)node->__vertArr;
	//pt = (int *)node->__vertArr;

	vtc=0;
	segLength=0;
	*indxStartPtr = pt; /* first segment starts off at index zero */
	int istart = 0, ip = 0;
	indxStartPtr++;
	//printf("1 address %u\n",indxStartPtr);

	ipoly = 0;
	ivertex = 0;
	starts[istart] = ip;

	for (i=0; i<node->coordIndex.n+1; i++) {
		/* count segments; dont bother if the very last number is -1 */
		//because we may or may not have a -1 at the end of coordIndex - no requirement
		if (node->coordIndex.p[i] == -1 || i == (node->coordIndex.n)) {
			/* record the old segment length */
			*vertCountPtr = segLength;
			*indxStartPtr =  pt;
			counts[istart] = segLength;
			istart++;
			starts[istart] = ip;
			ipoly++;
			if(i < (node->coordIndex.n)){
				/* new segment */
				indxStartPtr++;
				//printf("2 address %u\n",indxStartPtr);
				segLength=0;
				vertCountPtr ++;
				if(colorIndInt2) 
					ivertex++;
			}
		} else {
			/* new vertex */
			oldpoint = &points[node->coordIndex.p[i]];
			memcpy (newpoints, oldpoint,sizeof(struct SFColor));
			if(fog){
				int index = node->coordIndex.p[i];
				if(index < nfog)
					newfog[ip] = fog[index];
				else
					newfog[ip] = fog[nfog-1];
			}
			if(node->color){
				/* have a vertex, match colour  */
				do {
					if (node->colorPerVertex) {
						if (colorIndInt != NULL) 
							curcolor = colorIndInt[ivertex];
						else
							curcolor = colorIndInt2[ivertex];
							//curcolor = colorIndShort[ivertex];
					} else {
						if (colorIndInt != NULL)
							curcolor = colorIndInt[ipoly];
						else
							//curcolor = colorIndInt2[ipoly];
							curcolor = ipoly;//colorIndShort[ipoly];
					}
					ivertex++;
				}while(curcolor == -1 && curcolor < cc->color.n);
				if ((curcolor < 0) || (curcolor >= cc->color.n)) {
					ConsoleMessage ("IndexedLineSet, colorIndex %d (for vertex %d or segment %d) out of range (0..%d)\n",
						curcolor, i, ipoly, cc->color.n);
					return;
				}


				/* copy the correct color over for this vertex */
				if (cc->_nodeType == NODE_Color) {
					struct SFColor* oldcolor = (struct SFColor *) cc->color.p;
					memcpy (newcolors, defcolorRGBA, sizeof (defcolorRGBA));
					memcpy (newcolors, &oldcolor[curcolor],sizeof(struct SFColor));
				} else {
					struct SFColorRGBA *oldcolor = (struct SFColorRGBA *)cc->color.p;
					memcpy (newcolors, &oldcolor[curcolor],sizeof(struct SFColorRGBA));
				}
				//printf ("ipoly %d ci %d colour selected %f %f %f %f\n",ipoly, curcolor, newcolors->c[0],newcolors->c[1],newcolors->c[2],newcolors->c[3]);

				newcolors ++; 

			}
			newpoints ++; 
			segLength ++;
			pt ++;
			ip++;
		}
	}
	/*
	if(0){
		int k=0;
		vertCountPtr = (int *) node->__vertexCount;
		//vertCountPtr = (int *) node->__vertexCount;
		newcolors = (struct SFColorRGBA *) node->__xcolours;
		float * vert = node->__vertices;
		printf("ipoly=%d \n",ipoly);
		for(int j=0;j<ipoly;j++){
			printf("poly %d verts %d\n",j,(int)vertCountPtr[j]);
			for(int i=0;i<vertCountPtr[j];i++){

				struct SFColorRGBA *rgba = &newcolors[k];
				//printf ("ipoly %d ci %d colour selected %f %f %f %f\n",j, i, rgba->c[0],rgba->c[1],rgba->c[2],rgba->c[3]);
				printf(" ipoly %d ci %d coord %3.1f %3.1f %3.1f\n",j,i,vert[3*k],vert[3*k+1],vert[3*k+2]);
				k++;
			}
		}
	}
	*/
	/* finish this for loop off... */
	node->__segCount = nSegments; /* we passed, so we can render */
	//for(int i=0;i<nSegments;i++){
	//	printf("starts[%d] = %d counts %d\n",i,starts[i],counts[i]);
	//}
	node->_intern = set_LineRep(node->_intern,node->__vertices,NULL,node->__xcolours,NULL,node->__xfog,node->__segCount, (int *)node->__counts,(int*)node->__starts,NULL);
}

void render_IndexedLineSet (struct X3D_IndexedLineSet *node) {
    int **indxStartPtr;
	int *count;
	int i;
	ttglobal tg = gglobal();

	LIGHTING_OFF
	DISABLE_CULL_FACE
	
	COMPILE_IF_REQUIRED

	setExtent( node->EXTENT_MAX_X, node->EXTENT_MIN_X, node->EXTENT_MAX_Y,
			node->EXTENT_MIN_Y, node->EXTENT_MAX_Z, node->EXTENT_MIN_Z,
			X3D_NODE(node));

	render_LineRep((struct X3D_LineRep*)node->_intern);
}
static struct dataType_byteSize {
	int dataType;
	int byteSize;
} dataTypeSize_lookup[] = {
	{GL_BYTE, 1},
	{GL_SHORT, 2},
	{GL_UNSIGNED_SHORT,2},
	{GL_INT,4},
	{GL_UNSIGNED_INT,4},
	{GL_FLOAT, 4},
	{GL_DOUBLE,8},
	{-1,0},
};

int lookup_dataType_size(int dataType) {
	struct dataType_byteSize* typeSize;
	int byteSize = 0;
	int index = -1;
	typeSize = NULL;
	for(index = 0;; index++) {
		if (dataTypeSize_lookup[index].dataType < 0) break;
		if (dataType == dataTypeSize_lookup[index].dataType) {
			byteSize = dataTypeSize_lookup[index].byteSize;
			break;
		}
	}

	return byteSize;
}
void add_buffer_to_broto_context0(void *ectx, void* buffer) {
	struct X3D_Proto* context = X3D_PROTO(ectx);
	//printf("add_buffer_to_broto_context context = %p\n", context);
	if (context) {
		Stack* __GC;
		if (!context->__GC)
			context->__GC = newVector(struct X3D_Node*, 4);
		__GC = context->__GC;
		stack_push(void*, __GC, buffer);
	}
}
void add_buffer_to_broto_context(void *buffer) {
	struct X3D_Proto* context = X3D_PROTO(get_executionContext());
	add_buffer_to_broto_context0(context, buffer);
}
void remove_buffer_from_broto_context(void * buffer) {
	struct X3D_Proto* context = X3D_PROTO(get_executionContext());
	if (context) {
		if (context->__GC) {
			int i;
			for (i = 0; i < vectorSize(context->__GC); i++) {
				void* ns = vector_get(void *, context->__GC, i);
				if (ns == buffer) {
					vector_remove_elem(void *, context->__GC, i);
					break; //assumes its not added twice or more.
				}
			}
		}
	}
}
struct geomBuffer * find_buffer_in_broto_context_from_cgltf_buffer(void *ectx, void* cgltf_buffer) {
	struct geomBuffer* found = NULL;
	struct X3D_Proto* context = X3D_PROTO(ectx);
	//printf("find_buffer_in_broto_context_from_cgltf_buffer context = %p\n", context);

	if (context) {
		if (context->__GC) {
			int i;
			for (i = 0; i < vectorSize(context->__GC); i++) {
				void* ns = vector_get(void*, context->__GC, i);
				if (ns) {
					struct geomBuffer* gb = (struct geomBuffer*)ns;
					if (gb->cgltf_buffer == cgltf_buffer) {
						found = gb;
						break;
					}
				}
			}
		}
	}
	return found;
}

struct geomBuffer* add_geomBuffer0(void *ectx, int buffersize, int users) {
	struct geomBuffer* gb = MALLOC(struct geomBuffer*, sizeof(struct geomBuffer));
	memset(gb, 0, sizeof(struct geomBuffer));
	gb->address = malloc(buffersize);
	memset(gb->address, 0, buffersize);
	gb->byteSize = buffersize;
	gb->users = users;
	add_buffer_to_broto_context0(ectx,gb); //do we need to track buffers allocated? If so, adding to broto context might help
	return gb;
}
struct geomBuffer* add_geomBuffer(int buffersize, int users) {
	struct X3D_Proto* context = X3D_PROTO(get_executionContext());
	return add_geomBuffer0(context, buffersize, users);
}
void set_geomBuffer(struct geomBuffer* gb) {
	if (gb->VBO < 1) {
		glGenBuffers(1, (GLuint*)&gb->VBO);
	}
	glBindBuffer(GL_ARRAY_BUFFER, (GLuint)gb->VBO);
	glBufferData(GL_ARRAY_BUFFER, gb->byteSize, gb->address, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	gb->loaded = 2;
}
void update_geomBufferSize(struct geomBuffer* gb, int buffersize) {
	gb->address = realloc(gb->address,buffersize);
	memset(gb->address, 0, buffersize);
	gb->byteSize = buffersize;
	set_geomBuffer(gb);
}
void remove_geomBuffer(struct geomBuffer *gb) {
	glDeleteBuffers(1, &gb->VBO);
	remove_buffer_from_broto_context(gb); //do we need to track?
	FREE_IF_NZ(gb->address);
	FREE_IF_NZ(gb);
}
void subtract_geomBufferUser(struct geomBuffer* gb) {
	gb->users--;
	if (gb->users < 1) remove_geomBuffer(gb);
}
void add_geomBufferUser(struct geomBuffer* gb) {
	gb->users++;
}
int set_Attrib(struct bufAccess* ba, int dataSize, int dataType, int byteOffset) {
	int byteSize;
	ba->dataSize = dataSize;
	ba->in_use = 1;
	ba->dataType = dataType;
	ba->byteOffset = byteOffset;
	byteSize = ba->byteSize = ba->dataSize * lookup_dataType_size(ba->dataType);
	return byteSize;
}
char* get_Attribi(struct bufAccess* ba, struct geomBuffer *gb, int index) {
	return &gb->address[ba->byteOffset + index * ba->byteStride];
}

void* set_PointRep(void* _pointrep, float* points, int pointSize, int npoint,
	float* color, int colorSize, int ncolor, float* fog, int nfog)
{
	struct X3D_PointRep* pointrep = NULL;
	//to be called from compile_PolyPoint2D, compile_PointSet
	//deep copies points, colors, fog. X3D_PointRep owns the copies
	if (!points || npoint == 0) return NULL;

	if (!_pointrep) {
		_pointrep = MALLOC(struct X3D_PointRep*, sizeof(struct X3D_PointRep));
		memset(_pointrep, 0, sizeof(struct X3D_PointRep));
	}
	pointrep = (struct X3D_PointRep*)_pointrep;
	pointrep->itype = 0; //0 pointrep 1 linerep 2 polyrep
	pointrep->mode = 0; //0 points

	//experiment to see if we can do it all with sharable buffer, like glTF
	int nbyte_per_point = set_Attrib(&pointrep->attrib[0], pointSize, GL_FLOAT, 0);
	if (color) {
		nbyte_per_point += set_Attrib(&pointrep->attrib[1], colorSize, GL_FLOAT, nbyte_per_point);
	}
	if (fog) {
		nbyte_per_point += set_Attrib(&pointrep->attrib[2], 1, GL_FLOAT, nbyte_per_point);
	}
	//we have a certain way of packing attributes here,
	// but gltf can have them packed separately with separate strides
	for (int i = 0; i < 3; i++) pointrep->attrib[i].byteStride = nbyte_per_point;
	//pointrep->attribByteStride = nbyte_per_point;
	pointrep->ncoord = npoint;
	int buffersize = nbyte_per_point * pointrep->ncoord;
	if (!pointrep->buffer)
		pointrep->buffer = add_geomBuffer(buffersize,1);
	else
		update_geomBufferSize(pointrep->buffer, buffersize);
	struct geomBuffer* gb = pointrep->buffer;
	struct bufAccess* ba = &pointrep->attrib[0];
	for (int i = 0; i < pointrep->ncoord; i++) {
		char* target = get_Attribi(ba, gb, i);
		memcpy(target, &points[i * pointSize], pointSize * sizeof(float));
	}

	if (color) {
		ba = &pointrep->attrib[1];
		for (int i = 0; i < pointrep->ncoord; i++) {
			char* target = get_Attribi(ba, gb, i);
			int j = min(i, ncolor - 1);
			//rgba[3] = 1.0; //default opacity, don't need if respect dataSize
			memcpy(target, &color[j * colorSize], colorSize * sizeof(float));
		}
	}

	if (fog) {
		ba = &pointrep->attrib[2];
		for (int i = 0; i < pointrep->ncoord; i++) {
			char* target = get_Attribi(ba, gb, i);
			int j = min(i, nfog - 1);
			memcpy(target, &fog[j], sizeof(float));
		}
	}
	set_geomBuffer(pointrep->buffer);
	return pointrep;
}

void compile_PointSet (struct X3D_PointSet *node) {
	int ncolor = 0, nfog = 0, npoint = 0, colorSize = 3;
	struct X3D_Color *cc;
	float* fog = NULL;
	float* points = NULL;
	float* colors = NULL; // , * colorRGBA = NULL;

	/* do nothing, except get the extents here */
	MARK_NODE_COMPILED

	if (node->coord) {
		struct Multi_Vec3f *dtmp;
		dtmp = getCoordinate (node->coord, "PointSet");
		if (dtmp) {
			points = (float*)dtmp->p;
			/* find the extents */
			findExtentInCoord(X3D_NODE(node), dtmp->n, dtmp->p);

			if (dtmp->n == 0) return;
			npoint = dtmp->n;
		}
	}

	if (node->color) {
		POSSIBLE_PROTO_EXPANSION(struct X3D_Color *, node->color,cc)
		if(cc){
			ncolor = cc->color.n;
			colors = (float*)cc->color.p;
			if (cc->_nodeType == NODE_Color) {
				colorSize = 3;
			}
			else if (cc->_nodeType == NODE_ColorRGBA) {
				colorSize = 4;
			}
			else {
				ConsoleMessage("make_PointSet, expected %d got %d\n", NODE_Color, cc->_nodeType);
				ncolor = 0;
			}
		}
	}
	if (node->fogCoord) {
		struct X3D_FogCoordinate *fc = NULL;
		POSSIBLE_PROTO_EXPANSION(struct X3D_FogCoordinate *, node->fogCoord,fc)
		if(fc){
			if (fc->_nodeType != NODE_FogCoordinate) {
				ConsoleMessage ("make_PointSet fogCoord, expected %d got %d\n", NODE_FogCoordinate, fc->_nodeType);
			} else {
				nfog = fc->depth.n;
				fog = fc->depth.p;
			}
		}
	}
	node->_intern = set_PointRep(node->_intern, points, 3, npoint, colors, colorSize, ncolor, fog, nfog);
}

//same as Particle system quads
static GLfloat quadtris [18] = {-.5f,-.5f,0.0f, .5f,-.5f,0.0f, .5f,.5f,0.0f,   .5f,.5f,0.0f, -.5f,.5f,0.0f, -.5f,-.5f,0.0f,};
static GLfloat twotrisnorms [18] = {0.f,0.f,1.f, 0.f,0.f,1.f, 0.f,0.f,1.f,    0.f,0.f,1.f, 0.f,0.f,1.f, 0.f,0.f,1.f,};
static GLfloat twotristex [12] = {0.f,0.f, 1.f,0.f, 1.f,1.f,    1.f,1.f, 0.f,1.f, 0.f,0.f};

void render_PointRep(void* _pointrep) {
	struct X3D_PointRep* pointrep = (struct X3D_PointRep*)_pointrep;
	if (getAppearanceProperties()->pointMethod == PM_NONE) {
		//old style simple only, see render_PointSet for fancy.
		struct geomBuffer *gb = pointrep->buffer;
		if (!gb || gb->VBO < 1) return;
		//old-stile GL_POINTS rendering - opengl generates point triangles in geometry shader automatically

		glBindBuffer(GL_ARRAY_BUFFER, gb->VBO);
		struct bufAccess* ba = &pointrep->attrib[0];
		FW_GL_VERTEX_POINTER(ba->dataSize, ba->dataType, ba->byteStride, (GLfloat*)BUFFER_OFFSET(ba->byteOffset)); //dataSize, dataType, stride, pointer
		//sendAttribToGPU(FW_VERTEX_POINTER_TYPE, dataSize, dataType, GL_FALSE, stride, pointer, 0, __FILE__, __LINE__);

		// do we have colours?
		ba = &pointrep->attrib[1];
		if (ba->in_use) {
			FW_GL_COLOR_POINTER(ba->dataSize, ba->dataType, ba->byteStride, (GLfloat*)BUFFER_OFFSET(ba->byteOffset)); //dataSize, dataType, stride, pointer
		}
		// do we have fogcoord?
		ba = &pointrep->attrib[2];
		if (ba->in_use) {
			FW_GL_FOG_POINTER(ba->dataType, ba->byteStride, (GLfloat*)BUFFER_OFFSET(ba->byteOffset)); //dataType, stride, pointer
		}
		sendArraysToGPU(GL_POINTS, 0, pointrep->ncoord);
		//printf for debugging accessors.
		if (0) {
			char* paddress;
			for (int i = 0; i < pointrep->ncoord; i++) {
				printf("%d [", i);
				ba = &pointrep->attrib[0]; //point
				paddress = get_Attribi(ba, gb, i);;
				float* ai = (float*)paddress;
				for (int j = 0; j < ba->dataSize; j++) {
					printf("%f ", ai[j]);
				}
				printf("]");
				ba = &pointrep->attrib[1]; //color
				if (ba->in_use) {
					paddress = get_Attribi(ba, gb, i);;
					float* ai = (float*)paddress;
					printf(" [");
					for (int j = 0; j < ba->dataSize; j++)
						printf("%f ", ai[j]);
					printf("]");
				}
				ba = &pointrep->attrib[2]; //fog
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
	} else {
		//PointProperties needs fancy scaling or sprite texturing 
		//  we send a ParticleSystem-like quad
		//  and send the vertex (and fogCoord, CPV) as a uniform, in a loop over the vertices.
		// - see also render_Polypoint2D which uses a simpler version of below
		s_shader_capabilities_t* mysp = getAppearanceProperties()->currentShaderProperties;

		FW_GL_VERTEX_POINTER(3, GL_FLOAT, 0, (GLfloat*)quadtris); //node->_tris); //quadtris);
		FW_GL_NORMAL_POINTER(GL_FLOAT, 0, twotrisnorms);
		FW_GL_TEXCOORD_POINTER(2, GL_FLOAT, 0, twotristex, 0);
		glUniform1i(mysp->nTexCoordChannels, 1);
		glUniform1i(mysp->flipuv, 0);
		sendArraysToGPU(GL_TRIANGLES, 0, 6);
		GLint ppos = mysp->pointPosition; //GET_UNIFORM(mysp->myShaderProgram,"u_pointPosition");
		GLint pcpv = mysp->pointCPV;
		GLint pfog = mysp->pointFogCoord;
		int markertype = getAppearanceProperties()->markerType;
		markertype = markertype > 1 && markertype < 27 ? markertype : 0;
		if (markertype) {
			//push built-in marker texture
			printf("push marker texture/");
		}

		for (int i = 0; i < pointrep->ncoord; i++) {
			//send uniform
			float point[3];
			memset(point, 0, 3 * sizeof(float));
			struct bufAccess* ba;
			char *paddress;
			struct geomBuffer* gb = pointrep->buffer;
			ba = &pointrep->attrib[0];
			paddress = get_Attribi(ba, gb, i);;
			memcpy(point, paddress, ba->byteSize);
			glUniform3fv(ppos, 1, point);
			ba = &pointrep->attrib[1];
			if (pcpv > -1 && ba->in_use) {
				float rgba[4];
				rgba[3] = 1.0;
				paddress = get_Attribi(ba, gb, i);;
				memcpy(rgba, paddress, ba->byteSize);
				glUniform4fv(pcpv, 1, rgba);
			}
			ba = &pointrep->attrib[2];
			if (pfog > -1 && ba->in_use) {
				float fog;
				paddress = get_Attribi(ba, gb, i);;
				memcpy(&fog, paddress, ba->byteSize);
				glUniform1f(pfog, fog );
			}

			//draw
			reallyDrawOnce();
		}
		if (markertype) {
			//pop built-in marker texture
			printf("pop marker texture/");
		}
		clearDraw(); //child_shape also does this, redundant>
	}
}

void delete_PointRep(void* _pointrep) {
	struct X3D_PointRep* pr;
	pr = (struct X3D_PointRep*)_pointrep;
	subtract_geomBufferUser(pr->buffer);
	FREE_IF_NZ(pr);
}
void render_PointSet (struct X3D_PointSet *node) {
	struct X3D_PointRep* pointrep;
	ttglobal tg = gglobal();
	if(node->coord && node->coord->_ichange != node->coord->_change)
		node->_ichange++;
	if (node->color && node->color->_ichange != node->color->_change)
		node->_ichange++;
	COMPILE_IF_REQUIRED
		
	pointrep = (struct X3D_PointRep*)node->_intern;
	if (!pointrep)return;

	setExtent( node->EXTENT_MAX_X, node->EXTENT_MIN_X, node->EXTENT_MAX_Y,
			node->EXTENT_MIN_Y, node->EXTENT_MAX_Z, node->EXTENT_MIN_Z,
			X3D_NODE(node));

	LIGHTING_OFF
	DISABLE_CULL_FACE

	render_PointRep(pointrep);
}

void render_LineSet (struct X3D_LineSet *node) {

	struct X3D_Color *cc;
	GLint **indices;
	GLsizei *count;
	int i;
	struct Multi_Vec3f* points;
	ttglobal tg = gglobal();

	LIGHTING_OFF
	DISABLE_CULL_FACE

	COMPILE_IF_REQUIRED
	
	setExtent( node->EXTENT_MAX_X, node->EXTENT_MIN_X, node->EXTENT_MAX_Y,
			node->EXTENT_MIN_Y, node->EXTENT_MAX_Z, node->EXTENT_MIN_Z,
			X3D_NODE(node));

	render_LineRep((struct X3D_LineRep*)node->_intern);
}


void compile_LineSet (struct X3D_LineSet *node) {
	int vtc;		/* which vertexCount[] we should be using for this line segment */
	int c;			/* temp variable */
	struct SFVec3f *coord=0; int ncoord;
	//struct SFColor *color=0; 
	int ncolor=0;
	int *vertexC; int nvertexc;
	int totVertexRequired;

	struct X3D_Color *cc;
	GLint *pt;
	int **vpt;
	int *starts;
	//int *pt;
	//int **vpt;

	MARK_NODE_COMPILED
	node->__segCount = 0; /* assume this for now */
	clear_LineRep(node->_intern);


	nvertexc = (node->vertexCount).n; vertexC = (node->vertexCount).p;
	if (nvertexc==0) return;
	totVertexRequired = 0;

	//printf ("compile_LineSet, nvertexc %d\n",nvertexc);


	/* sanity check vertex counts */
	for  (c=0; c<nvertexc; c++) {
		totVertexRequired += vertexC[c];
		if (vertexC[c]<2) {
			ConsoleMessage ("make_LineSet, we have a vertexCount of %d, must be >=2,\n",vertexC[c]);
			return;
		}
	}

	if (node->coord) {
		struct Multi_Vec3f *dtmp;
		dtmp = getCoordinate (node->coord, "IndexedLineSet");
		ncoord = dtmp->n;
		coord = dtmp->p;

		/* find the extents */
		findExtentInCoord(X3D_NODE(node), ncoord, coord);
	} else {
		return; /* no coordinates - nothing to do */
	}

	/* check that we have enough vertexes */
	if (totVertexRequired > ncoord) {
		ConsoleMessage ("make_LineSet, not enough points for vertexCount (vertices:%d points:%d)\n",
			totVertexRequired, ncoord);
		return;
	}
 
	if (node->color) {
		/* cc = (struct X3D_Color *) node->color; */
		POSSIBLE_PROTO_EXPANSION(struct X3D_Color *, node->color,cc)
		if(cc){
			if ((cc->_nodeType != NODE_Color) && (cc->_nodeType != NODE_ColorRGBA)) {
				ConsoleMessage ("make_LineSet, expected %d got %d\n", NODE_Color, cc->_nodeType);
			} else {
				ncolor = cc->color.n;
				//color = cc->color.p;
			}
		}
		/* check that we have enough verticies for the Colors */
		if (totVertexRequired > ncolor) {
			ConsoleMessage ("make_LineSet, not enough colors for vertexCount (vertices:%d colors:%d)\n",
				totVertexRequired, ncolor);
			return;
		}
	}

	/* create the index for the arrays. Really simple... Used to index
	   into the coords, so, eg, __vertArr is [0,1,2], which means use
	   coordinates 0, 1, and 2 */
	FREE_IF_NZ (node->__vertArr);
	node->__vertArr = MALLOC (GLuint *, sizeof(GLuint)*(ncoord));
	pt = (GLint *)node->__vertArr;
	//pt = (int *)node->__vertArr;
	for (vtc = 0; vtc < ncoord; vtc++) {
		*pt=vtc; pt++; /* ie, index n contains the number n */
	}

	/* create the index for each line segment. What happens here is
	   that we create an array of pointers; each pointer points into
	   the __vertArr array - this gives a starting index for each line
	   segment The LENGTH of each segment (good question) comes from the
	   vertexCount parameter of the LineSet node */
	FREE_IF_NZ (node->__vertIndx);
	node->__vertIndx = MALLOC (int **, sizeof(int*)*(nvertexc));
	FREE_IF_NZ (node->__starts);
	node->__starts = MALLOC (int *,sizeof(int)*(nvertexc));
	//node->__vertIndx = MALLOC (int **, sizeof(int*)*(nvertexc));
	c = 0;
	pt = (GLint *)node->__vertArr;
	vpt = (int**) node->__vertIndx;
	starts = (int*) node->__starts;
	//pt = (int *)node->__vertArr;
	//vpt = (int**) node->__vertIndx;
	
	for (vtc=0; vtc<nvertexc; vtc++) {
		//printf ("in position %d of __vertIndx, we have put pointer to %u\n",vtc,*pt);
		vpt[vtc] =  (int*) pt;
		//vpt[vtc] =  (int*) pt;
		pt += vertexC[vtc];
		if(vtc == 0)
			starts[vtc] = 0;
		else
			starts[vtc] = starts[vtc-1] + vertexC[vtc];
	}

	/* if we made it this far, we are ok tell the rendering engine that we are ok */
	node->__segCount = nvertexc;
	//for(int i=0;i<node->__segCount;i++){
	//	printf("[%d] start %d count %d\n",i,((int*)node->__starts)[i],vertexC[i]);
	//}
	float *fog = NULL;
	if(node->fogCoord){
		struct X3D_FogCoordinate *fogcoord = (struct X3D_FogCoordinate*)node->fogCoord;
		fog = fogcoord->depth.p;
	}
	node->_intern = set_LineRep(node->_intern,coord,NULL,NULL,NULL,fog,nvertexc,vertexC,(int*)node->__starts,NULL);
}

/* ClipPlane
	http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/rendering.html#ClipPlanes
	http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/rendering.html#ClipPlane
	GLES2 supports only frustum clipplane. For user clipplanes:
	https://www.khronos.org/registry/gles/specs/2.0/es_cm_spec_2.0.25.pdf
	"Userclipping planes can be emulated by dot product clipping plane and vertex, and threshold result in shader"
	http://mrkaktus.org/opengl-clip-planes-explained/
	- shows different gl versions and different support technique / mechanism, desktop different than ES2 different than ES3
	A few links here to clipping in shader es 2:
	http://stackoverflow.com/questions/7408855/clipping-planes-in-opengl-es-2-0
	https://www.opengl.org/discussion_boards/showthread.php/171914-How-to-activate-clip-planes-via-shader

	Implementation Suggestions:
	A. render_hier plubming
	11.2.4.4 > scoping of clipplanes > 
	- their transform siblings are affected and children below
		- (dug9: maybe it could/should have been a grouping node, with its own children, like collision?)
		- to implement, you would use a stack, and push going down, pop coming back
		- need to flag parent like other sibling-affectors - see OpenGL_UTils.c VF_Sensitive
		- in render_node(node) on render_geom pass (doesn't make sense on any other pass?)
			-on prep-side of children, test for VF_ClipPlane on parent, go through children to find ClipPlane, push clipplane
			if node & VF_ClipPlane
				ifound = push_child_clipplane(node);
				if( ifound) pushed_clipplane = TRUE;
			-on fin side of children, if pushed_clipplane pop_child_clipplane
	B. shader plumbing
			https://www.opengl.org/discussion_boards/showthread.php/171914-How-to-activate-clip-planes-via-shader
			in shader:
			uniform vec4 ClipPlane[MaxClipPlanes];
			...
			for ( int i=0; i<MaxClipPlanes; i++ )
			{
			   gl_ClipDistance[i] = dot( ClipPlane[i], vec4(MCvertex,1.0));
			}
*/
float *getTransformedClipPlanes(){
	#define tactic_one_shot 1
	#define tactic_point_plus_normal 2
	int i,nsend, tactic;
	double modelviewmatrix[16], meinv[16], u2me[16], me2u[16], me2ut[16], u2met[16],  *M, *MIT;
	ppComponent_Rendering p = (ppComponent_Rendering)gglobal()->Component_Rendering.prv;

	nsend =  min(FW_MAXCLIPPLANES,vectorSize(p->clipplane_stack));
	//Q. how transform a plane by a matrix?
	//option 1: 4x4 * 4x1
	//https://www.opengl.org/discussion_boards/showthread.php/159564-Clever-way-to-transform-plane-by-matrix
	//tactic = tactic_one_shot; //works
	//option 2: convert abcd plane into normal + 3d point, transform point and normal, then convert back to abcd
	//http://stackoverflow.com/questions/7685495/transforming-a-3d-plane-by-4x4-matrix
	tactic = tactic_point_plus_normal; //works
	FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelviewmatrix);
	matinverseAFFINE(meinv,modelviewmatrix);

	for(i=0;i<nsend;i++){
		//we take from the top-most end of the stack, in case more than MAX, using vectorget
		int j;
		struct X3D_ClipPlane *cplane;
		double dplane[4],dplane2[4];
		usehit uhit;
		uhit = vector_get(usehit,p->clipplane_stack,i);
		cplane = (struct X3D_ClipPlane *)uhit.node;
		matmultiplyAFFINE(u2me,uhit.mvm,meinv);
		mattranspose(u2met,u2me);
		matinverseAFFINE(me2u,u2me);
		mattranspose(me2ut,me2u);
		M = u2me;    //matrix, for transforming point
		MIT = me2ut;  //matrix inverse transpose, for transforming normal

		for(j=0;j<4;j++) dplane[j] = cplane->plane.c[j]; //float to double
		if(tactic == tactic_one_shot){
			//works
			transformFULL4d(dplane2,dplane,MIT);
		}
		else 
		{
			//tactic_point_plus_normal - works
			//vector4 O = (xyz * d, 1)
			double O4[4], N4[4], d;
			vecscaled(O4,dplane,-dplane[3]);
			O4[3] = 1.0;
			//vector4 N = (xyz, 0)
			veccopyd(N4,dplane);
			N4[3] = 0.0;
			//O = M * O
			transformAFFINEd(O4,O4,M);
			//N = transpose(invert(M)) * N
			transformAFFINEd(N4,N4,MIT);
			//xyz = N.xyz
			veccopyd(dplane2,N4);
			//d = dot(O.xyz, N.xyz)	
			d = vecdotd(O4,N4);
			dplane2[3] = -d;
		}
		for(j=0;j<4;j++) p->clipplanes[i*4 + j] = (float) dplane2[j]; //double to float
	}
	return p->clipplanes;
}
int getClipPlaneCount(){
	int nsend;
	ppComponent_Rendering p = (ppComponent_Rendering)gglobal()->Component_Rendering.prv;
	nsend =  min(FW_MAXCLIPPLANES,vectorSize(p->clipplane_stack));
	return nsend;
}
void pushShaderFlags(shaderflagsstruct flags);
void popShaderFlags();

void sib_prep_ClipPlane(struct X3D_Node *parent, struct X3D_Node *sibAffector){
	//search for child clipplane
	//if found and enabled
	if(sibAffector && sibAffector->_nodeType == NODE_ClipPlane){
		//	 push on stack like push_sensor() 
		struct X3D_ClipPlane * cplane = (struct X3D_ClipPlane*)sibAffector;
		if(cplane->enabled == TRUE){
			//unsigned int shaderflags;
			shaderflagsstruct shaderflags;
			double modelviewmatrix[16];
			usehit uhit;
			ppComponent_Rendering p = (ppComponent_Rendering)gglobal()->Component_Rendering.prv;

			shaderflags = getShaderFlags();
			shaderflags.base |= CLIPPLANE_SHADER;
			pushShaderFlags(shaderflags);
			
			//http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/rendering.html#ClipPlanes
			//we'll snapshot the modelview matrix and push with clipplane node onto stack, 
			//for use when applying in leaf shape node
			uhit.node = X3D_NODE(cplane); //x3dnode clipplane
			FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelviewmatrix);
			memcpy(uhit.mvm,modelviewmatrix,16*sizeof(double)); //deep copy
			stack_push(usehit,p->clipplane_stack,uhit); //fat elements do another deep copy

			//jplane = vectorSize(p->clipplane_stack) -1;
			//if(jplane < FW_MAXCLIPPLANES){
			//	memcpy(&p->clipplanes[jplane*4],cplane->plane.c,4*sizeof(float));
			//}

			//	somehow add to end of list of clipplanes available to shader (but how precisely?)
			//  	https://www.opengl.org/discussion_boards/showthread.php/171914-How-to-activate-clip-planes-via-shader
			//		m_pProgram->SetUniform("ClipPlane[0]", vect, 4, 1);  
			//		//except construct the name string: k = clipplanestac.n-1; "ClipPlane[%1d]",k
			//		glEnable(GL_CLIP_DISTANCE0); //except DISTANCEk ?
			//		might need: to set a flag indicating which shader to use?

		}
	}
}
void sib_fin_ClipPlane(struct X3D_Node *parent, struct X3D_Node *sibAffector){
	//pop clipplane
	if(sibAffector && sibAffector->_nodeType == NODE_ClipPlane){
		struct X3D_ClipPlane * cplane = (struct X3D_ClipPlane*)sibAffector;
		if(cplane->enabled == TRUE){
			ppComponent_Rendering p = (ppComponent_Rendering)gglobal()->Component_Rendering.prv;
			stack_pop(usehit,p->clipplane_stack);
			popShaderFlags();
		}
	}
}