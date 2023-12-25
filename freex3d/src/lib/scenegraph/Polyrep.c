/*


???

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
#include "../scenegraph/Component_Shape.h"
#include "../scenegraph/RenderFuncs.h"

#include "Polyrep.h"
#include "LinearAlgebra.h"
#include "Tess.h"

void * compile_poly_if_required(void* node, void* coord, void* fogCoord, void* color, void* normal, void* texCoord) {
//#define COMPILE_POLY_IF_REQUIRED(a,b,c,d,e)
//if(!compile_poly_if_required(node,a,b,c,d,e))return;
	struct X3D_Node* nd = X3D_NODE(node);
	if(!nd->_intern || nd->_change != ((struct X3D_PolyRep *)(nd->_intern))->irep_change) { \
		compileNode ((void *)compile_polyrep,node,coord,fogCoord,color,normal,texCoord);
	}
	return nd->_intern;
}

/* How many faces are in this IndexedFaceSet?			*/

int count_IFS_faces(int cin, struct Multi_Int32 *coordIndex, struct facepar *faceok) {
	/* lets see how many faces we have */
	int pointctr=0;
	int max_points_per_face = 0;
	int min_points_per_face = 99999;
	int i;
	int faces = 0;

	if (coordIndex == NULL) return 0;
	if (coordIndex->n == 0) return 0;
	faceok[faces].start = 0;
	for(i=0; i<cin; i++) {

		if((coordIndex->p[i] == -1) || (i==cin-1)) {
			faceok[faces].end = i-1;

			if(coordIndex->p[i] != -1) {
				faceok[faces].end = i;
				pointctr++;
			}

			faces++;
			faceok[faces].start = i+1;
			if (pointctr > max_points_per_face)
				max_points_per_face = pointctr;
			if (pointctr < min_points_per_face)
				min_points_per_face = pointctr;
			pointctr = 0;
		} else pointctr++;
	}


	/*	
	printf ("this structure has %d faces\n",faces);
	printf ("	max points per face %d\n",max_points_per_face);
	printf ("	min points per face %d\n\n",min_points_per_face);
	*/
	
	if (faces < 1) {
		/* printf("an IndexedFaceSet with no faces found\n"); */
		return (0);
	}
	return faces;
}


/* Generate the normals for each face of an IndexedFaceSet	*/
/* create two datastructures:					*/
/* 	- face normals; given a face, tell me the normal	*/
/*	- point-face;   for each point, tell me the face(s)	*/

int IFS_face_normals (
	struct SFVec3f *facenormals, //struct point_XYZ *facenormals,
	struct facepar *faceok,
	int *pointfaces,
	int faces,
	int npoints,
	int cin,
	struct SFVec3f *points,
	struct Multi_Int32 *coordIndex,
	int ccw) {

	int tmp_a = 0, this_face_finished;
	int i,checkpoint;
	int facectr;
	int pt_1, pt_2, pt_3;
	float AC, BC;
	struct SFVec3f *c1,*c2,*c3;
	float a[3]; float b[3];

	int retval = FALSE;
	int new_way = TRUE;

	float this_vl;
	//struct point_XYZ thisfaceNorms;

	/* printf ("IFS_face_normals, faces %d\n",faces); */

	/*  Assume each face is ok for now*/
	for(i=0; i<faces; i++) {
		faceok[i].OK = TRUE;
		//printf("face %d start %d end %d\n",i,faceok[i].start,faceok[i].end);
	}

	/*  calculate normals for each face*/
	for(i=0; i<faces; i++) {
		/* lets decide which normal to choose here, in case of more than 1 triangle.
		   we choose the triangle with the greatest vector length hoping that it is
		   the least "degenerate" of them all */
		this_vl = 0.0f;
		//facenormals[i].x = 0.0;
		//facenormals[i].y = 0.0;
		//facenormals[i].z = 1.0;
		vecset3f(facenormals[i].c,0.0f, 0.0f, 1.0f);

		if((faceok[i].end - faceok[i].start + 1) < 3) {
			printf ("IndexedFaceNormals: have a face with two or less vertexes\n");
			faceok[i].OK = FALSE;
		}
		if(faceok[i].OK){
			/* check to see that the coordIndex does not point to a
				point that is outside the range of our point array */
			for(int k=faceok[i].start;k<=faceok[i].end;k++){
				checkpoint = coordIndex->p[k];
				if (checkpoint < 0 || checkpoint >= npoints) {
					printf ("Indexed Geometry face %d has a point out of range,",i);
					printf (" point is %d, should be between 0 and %d\n", checkpoint,npoints-1);
					faceok[i].OK = FALSE;
				}
			}
		}
		/* face has passed checks so far... */
		if (faceok[i].OK) {
			/* printf ("face %d ok\n",i); */
			/* check for degenerate triangles -- we go through all triangles in a face to see which
			   triangle has the largest vector length */
			this_face_finished = FALSE;
			tmp_a = faceok[i].start;
			pt_1 = tmp_a;
			//printf("face %d first index pt_1 %d\n",i,pt_1);
			if (ccw) {
				/* printf ("IFS face normals CCW\n"); */
				pt_2 = tmp_a+1; pt_3 = tmp_a+2;
			} else {
				/* printf ("IFS face normals *NOT* CCW\n"); */
				pt_3 = tmp_a+1; pt_2 = tmp_a+2;
			}

			do {
				float fnorm[3], fnormlen, delta[3];
				//printf("do pt1 %d pt2 %d pt3 %d\n",pt_1,pt_2,pt_3);

				/* first three coords give us the normal */
				c1 = &(points[coordIndex->p[pt_1]]);
				c2 = &(points[coordIndex->p[pt_2]]);
				c3 = &(points[coordIndex->p[pt_3]]);

				//a[0] = c2->c[0] - c1->c[0];
				//a[1] = c2->c[1] - c1->c[1];
				//a[2] = c2->c[2] - c1->c[2];
				//b[0] = c3->c[0] - c1->c[0];
				//b[1] = c3->c[1] - c1->c[1];
				//b[2] = c3->c[2] - c1->c[2];
				vecdif3f(a,c2->c,c1->c);
				vecdif3f(b,c3->c,c1->c);

				//printf ("a0 %f a1 %f a2 %f b0 %f b1 %f b2 %f\n", a[0],a[1],a[2],b[0],b[1],b[2]);

				//thisfaceNorms.x = a[1]*b[2] - b[1]*a[2];
				//thisfaceNorms.y = -(a[0]*b[2] - b[0]*a[2]);
				//thisfaceNorms.z = a[0]*b[1] - b[0]*a[1];
				veccross3f(fnorm,a,b);
				/* printf ("vector length is %f\n",calc_vector_length (thisfaceNorms));  */
				//printf("axb=%f %f %f\n",fnorm[0],fnorm[1],fnorm[2]);
				/* is this vector length greater than a previous one? */
				//if (calc_vector_length(thisfaceNorms) > this_vl) {
				fnormlen= veclength3f(fnorm);
				if(fnormlen > this_vl) {
					/* printf ("for face, using points %d %d %d\n",pt_1, pt_2, pt_3);  */
					this_vl = fnormlen; //calc_vector_length(thisfaceNorms);
					veccopy3f(facenormals[i].c,fnorm);
					//facenormals[i].x = thisfaceNorms.x;
					//facenormals[i].y = thisfaceNorms.y;
					//facenormals[i].z = thisfaceNorms.z;
				}

				/* lets skip along to next triangle in this face */

				//AC=(c1->c[0]-c3->c[0])*(c1->c[1]-c3->c[1])*(c1->c[2]-c3->c[2]);
				//BC=(c2->c[0]-c3->c[0])*(c2->c[1]-c3->c[1])*(c2->c[2]-c3->c[2]);
				AC = veclength3f(vecdif3f(delta,c1->c,c2->c));
				BC = veclength3f(vecdif3f(delta,c2->c,c3->c));
				/* printf ("AC %f ",AC); printf ("BC %f \n",BC); */

				/* we have 3 points, a, b, c */
				/* we also have 3 vectors, AB, AC, BC */
				/* find out which one looks the closest one to skip out */
				/* either we move both 2nd and 3rd points, or just the 3rd */

				if (ccw) {
					/* printf ("moving along IFS face normals CCW\n");  */
					//if (fabs(AC) < fabs(BC)) { pt_2++; }
					if (AC < BC) { pt_2++; }
					pt_3++;
				} else {
					/* printf ("moving along IFS face normals *NOT* CCW\n"); */
					/* if (fabs(AC) < fabs(BC)) { pt_3++; } */
					pt_2++;
				}

				/* skip forward to the next couple of points - if possible */
				/* printf ("looking at %d, cin is %d\n",tmp_a, cin); */
				tmp_a ++;
				this_face_finished = tmp_a + 2 > faceok[i].end;
			} while (!this_face_finished);

			if (APPROX(this_vl,0.0)) {
				/* printf ("face %d is degenerate\n",i); */
				faceok[i].OK = FALSE;
			} else {
				/* printf ("face %d is ok\n",i); */
				//normalize_vector(&facenormals[i]);
				vecnormalize3f(facenormals[i].c,facenormals[i].c);

			/*	
			printf ("vertices \t%f %f %f\n\t\t%f %f %f\n\t\t%f %f %f\n",
				c1->c[0],c1->c[1],c1->c[2],
				c2->c[0],c2->c[1],c2->c[2],
				c3->c[0],c3->c[1],c3->c[2]);
			*/
			//printf ("face %3d normal %5.2f %5.2f %5.2f\n",i,facenormals[i].c[0],facenormals[i].c[1],facenormals[i].c[2]);
		
			}
			

		}

		// printf ("for face %d, vec len is %f\n",i,this_vl);
	}


	/* do we have any valid faces??? */
	for(i=0; i<faces; i++) {
		if (faceok[i].OK == TRUE) {
			retval = TRUE;
		}
	}
	if (!retval) 
		return retval; /* nope, lets just drop out of here */
	

	/* now, go through each face, and make a point-face list
	   so that I can give it a point later, and I will know which face(s)
	   it belong to that point */
	/* printf ("\nnow generating point-face list\n");   */
	for (i=0; i<npoints; i++) { pointfaces[i*POINT_FACES]=0; }
	if(new_way){
	for(i=0;i<faces;i++){
		if(faceok[i].OK)
		for(int j=faceok[i].start;j<=faceok[i].end;j++){
			tmp_a = coordIndex->p[j];
			//printf ("pointfaces, coord %d coordIndex %d face %d\n",j,tmp_a,i); 
			tmp_a *= POINT_FACES;
			add_to_face (tmp_a,i,pointfaces);
		}
	}
	}else{ //new way
	facectr=0;
	for(i=0; i<cin; i++) {
		tmp_a=coordIndex->p[i];
		if (tmp_a == -1) {
			facectr++;
		} else {
			if (faceok[facectr].OK) {
				//printf ("pointfaces, coord %d coordIndex %d face %d\n",i,tmp_a,facectr); 
				tmp_a*=POINT_FACES;
				add_to_face (tmp_a,facectr,pointfaces);
			} else {
			// 	printf ("skipping add_to_face for invalid face %d\n",facectr);
			}
		}
	}
	} //new way

	/*
	 printf ("\ncheck \n");
	for (i=0; i<npoints; i++) {
		int tmp_b;

		tmp_a = i*POINT_FACES;
		printf ("point %d is in %d faces, these are:\n ", i, pointfaces[tmp_a]);
		for (tmp_b=0; tmp_b<pointfaces[tmp_a]; tmp_b++) {
			printf ("%d ",pointfaces[tmp_a+tmp_b+1]);
		}
		printf ("\n");
	}
	*/

	return retval;
}



/* Tesselated faces MAY have the wrong normal calculated. re-calculate after tesselation	*/

void Extru_check_normal (
	struct SFVec3f *facenormals, //struct point_XYZ *facenormals,
	int this_face,
	int direction,
	struct X3D_PolyRep  *rep_,
	int ccw) {

	/* only use this after tesselator as we get coord indexes from global var */
	struct SFVec3f *c1,*c2,*c3;
	float a[3], b[3], fnorm[3], fnormlen;
	int zz1, zz2;
	ttglobal tg = gglobal();

	if (ccw) {
		zz1 = 1;
		zz2 = 2;
	} else {
		zz1 = 2;
		zz2 = 1;
	}

	/* first three coords give us the normal */
 	c1 = (struct SFVec3f *) &rep_->actualCoord[3*tg->Tess.global_IFS_Coords[0]];
 	c2 = (struct SFVec3f *) &rep_->actualCoord[3*tg->Tess.global_IFS_Coords[zz1]];
 	c3 = (struct SFVec3f *) &rep_->actualCoord[3*tg->Tess.global_IFS_Coords[zz2]];

	/*printf ("Extru_check_normal, coords %d %d %d\n",global_IFS_Coords[0],
		global_IFS_Coords[1],global_IFS_Coords[2]);
	printf ("Extru_check_normal vertices \t%f %f %f\n\t\t%f %f %f\n\t\t%f %f %f\n",
		c1->c[0],c1->c[1],c1->c[2],
		c2->c[0],c2->c[1],c2->c[2],
		c3->c[0],c3->c[1],c3->c[2]);
	*/

	a[0] = c2->c[0] - c1->c[0];
	a[1] = c2->c[1] - c1->c[1];
	a[2] = c2->c[2] - c1->c[2];
	b[0] = c3->c[0] - c1->c[0];
	b[1] = c3->c[1] - c1->c[1];
	b[2] = c3->c[2] - c1->c[2];
	vecdif3f(a,c2->c,c1->c);
	vecdif3f(b,c3->c,c1->c);
	veccross3f(fnorm,a,b);

	//facenormals[this_face].x = a[1]*b[2] - b[1]*a[2] * direction;
	//facenormals[this_face].y = -(a[0]*b[2] - b[0]*a[2]) * direction;
	//facenormals[this_face].z = a[0]*b[1] - b[0]*a[1] * direction;
	fnormlen = veclength3f(fnorm);
	//if (APPROX(calc_vector_length (facenormals[this_face]),0.0)) { 
	if (APPROX(fnormlen,0.0f)) { 
		ConsoleMessage ("WARNING: FreeWRL got degenerate triangle; OpenGL tesselator should not give degenerate triangles back %f\n",
			fnormlen); //fabs(calc_vector_length (facenormals[this_face])));
	}
	vecnormalize3f(facenormals[this_face].c,fnorm);
	
	//normalize_vector(&facenormals[this_face]);
	/* printf ("facenormal for %d is %f %f %f\n",this_face, facenormals[this_face].x,
			facenormals[this_face].y, facenormals[this_face].z); */
}

/* Tesselated faces MAY have the wrong normal calculated. re-calculate after tesselation	*/


void IFS_check_normal (
	struct SFVec3f *facenormals, //struct point_XYZ *facenormals,
	int this_face,
	struct SFVec3f *points, int base,
	struct Multi_Int32 *coordIndex, int ccw) {

	struct SFVec3f *c1,*c2,*c3;
	float a[3], b[3], fnorm[3], fnormlen;
	ttglobal tg = gglobal();

	/* printf ("IFS_check_normal, base %d points %d %d %d\n",base,*/
	/* 	global_IFS_Coords[0],global_IFS_Coords[1],global_IFS_Coords[2]);*/
	/* printf ("normal was %f %f %f\n\n",facenormals[this_face].x,*/
	/* 	facenormals[this_face].y,facenormals[this_face].z);*/

	//PROBLEM IF THE FIRST TRIANGLE OF A FACE IS DEGENERATE, THEN 
	// WE GET A DEGENERATE NORMAL / NO NORMAL
	/* first three coords give us the normal */
	c1 = &(points[coordIndex->p[base+tg->Tess.global_IFS_Coords[0]]]);
	if (ccw) {
		c2 = &(points[coordIndex->p[base+tg->Tess.global_IFS_Coords[1]]]);
		c3 = &(points[coordIndex->p[base+tg->Tess.global_IFS_Coords[2]]]);
	} else {
		c3 = &(points[coordIndex->p[base+tg->Tess.global_IFS_Coords[1]]]);
		c2 = &(points[coordIndex->p[base+tg->Tess.global_IFS_Coords[2]]]);
	}

	a[0] = c2->c[0] - c1->c[0];
	a[1] = c2->c[1] - c1->c[1];
	a[2] = c2->c[2] - c1->c[2];
	b[0] = c3->c[0] - c1->c[0];
	b[1] = c3->c[1] - c1->c[1];
	b[2] = c3->c[2] - c1->c[2];
	vecdif3f(a,c2->c,c1->c);
	vecdif3f(b,c3->c,c1->c);
	veccross3f(fnorm,a,b);
	fnormlen = veclength3f(fnorm);
	veccopy3f(facenormals[this_face].c,fnorm);
	//facenormals[this_face].x = a[1]*b[2] - b[1]*a[2];
	//facenormals[this_face].y = -(a[0]*b[2] - b[0]*a[2]);
	//facenormals[this_face].z = a[0]*b[1] - b[0]*a[1];

	/* printf ("vector length is %f\n",calc_vector_length (facenormals[this_face])); */

	//if (APPROX(calc_vector_length (facenormals[this_face]),0.0)) {
	if (APPROX(fnormlen,0.0f)) {
		//printf ("warning: Tesselated surface has invalid normal - if this is an IndexedFaceSet, check coordinates of ALL faces\n");
	} else {

		//normalize_vector(&facenormals[this_face]);
		vecnormalize3f(facenormals[this_face].c,facenormals[this_face].c);

		/* printf ("vertices \t%f %f %f\n\t\t%f %f %f\n\t\t%f %f %f\n",*/
		/* 	c1->c[0],c1->c[1],c1->c[2],*/
		/* 	c2->c[0],c2->c[1],c2->c[2],*/
		/* 	c3->c[0],c3->c[1],c3->c[2]);*/
		//printf ("face %3d normal %5.2f %5.2f %5.2f\n",this_face,facenormals[this_face].c[0],facenormals[this_face].c[1],facenormals[this_face].c[2]);
		/* 	facenormals[this_face].y,facenormals[this_face].z);*/
	}
}


void add_to_face (
	int point,
	int face,
	int *pointfaces) {

	int count;
	if (pointfaces[point] < (POINT_FACES-1)) {
		/* room to add, but is it already there? */
		for (count = 1; count <= pointfaces[point]; count++) {
			if (pointfaces[point+count] == face) return;
		}
		/* ok, we have an empty slot, and face not already added */
		pointfaces[point]++;
		pointfaces[point+ pointfaces[point]] = face;
	}
}

/********************************************************************
 *
 * ElevationGrid Triangle
 *
 */
void Elev_Tri (
	int vertex_ind,
	int this_face,
	int A,
	int D,
	int E,
	int NONORMALS,
	struct X3D_PolyRep *this_Elev,
	struct SFVec3f *facenormals, //struct point_XYZ *facenormals,
	int *pointfaces,
	int ccw) {

	struct SFVec3f *c1,*c2,*c3;
	float a[3], b[3], fnorm[3], fnormlen;
	int tmp;

	/* printf ("Elev_Tri Triangle %d %d %d\n",A,D,E); */

	/* generate normals in a clockwise manner, reverse the triangle */
	if (!(ccw)) {
		tmp = D;
		D = E;
		E = tmp;
	}


	this_Elev->cindex[vertex_ind] = (GLuint)A;
	this_Elev->cindex[vertex_ind+1] = (GLuint)D;
	this_Elev->cindex[vertex_ind+2] = (GLuint)E;

	/*
	printf ("Elev_Tri, vertices for vertex_ind %d are:",vertex_ind);
               c1 = (struct SFVec3f *) &this_Elev->actualCoord[3*A];
               c2 = (struct SFVec3f *) &this_Elev->actualCoord[3*D];
               c3 = (struct SFVec3f *) &this_Elev->actualCoord[3*E];

	printf ("\n%f %f %f\n%f %f %f\n%f %f %f\n\n",
		c1->c[0], c1->c[1],c1->c[2],c2->c[0],c2->c[1],c2->c[2],
		c3->c[0],c3->c[1],c3->c[2]);
	*/


	if (NONORMALS) {
		/* calculate normal for this triangle */
                c1 = (struct SFVec3f *) &this_Elev->actualCoord[3*A];
                c2 = (struct SFVec3f *) &this_Elev->actualCoord[3*D];
                c3 = (struct SFVec3f *) &this_Elev->actualCoord[3*E];

		/*
		printf ("calc norms \n%f %f %f\n%f %f %f\n%f %f %f\n",
		c1->c[0], c1->c[1],c1->c[2],c2->c[0],c2->c[1],c2->c[2],
		c3->c[0],c3->c[1],c3->c[2]);
		*/

		a[0] = c2->c[0] - c1->c[0];
		a[1] = c2->c[1] - c1->c[1];
		a[2] = c2->c[2] - c1->c[2];
		b[0] = c3->c[0] - c1->c[0];
		b[1] = c3->c[1] - c1->c[1];
		b[2] = c3->c[2] - c1->c[2];
		vecdif3f(a,c2->c,c1->c);
		vecdif3f(b,c3->c,c1->c);
		veccross3f(fnorm,a,b);
		vecnormalize3f(fnorm,fnorm);
		veccopy3f(facenormals[this_face].c,fnorm);
		//facenormals[this_face].x = a[1]*b[2] - b[1]*a[2];
		//facenormals[this_face].y = -(a[0]*b[2] - b[0]*a[2]);
		//facenormals[this_face].z = a[0]*b[1] - b[0]*a[1];

		/*
		printf ("facenormals index %d is %f %f %f\n",this_face, facenormals[this_face].x,
				facenormals[this_face].y, facenormals[this_face].z);
		*/

		/* add this face to the faces for this point */
		add_to_face (A*POINT_FACES,this_face,pointfaces);
		add_to_face (D*POINT_FACES,this_face,pointfaces);
		add_to_face (E*POINT_FACES,this_face,pointfaces);
	}
}



/***********************************************************************8
 *
 * Extrusion Texture Mapping
 *
 ***********************************************************************/

void Extru_tex(
	int vertex_ind,
	int tci_ct,
	int A,
	int B,
	int C,
	GLuint *tcindex,
	int ccw,
	int tcindexsize) {

	int j;

	/* bounds check */
	/* printf ("Extru_tex, tcindexsize %d, vertex_ind %d\n",tcindexsize, vertex_ind);  */
	if (vertex_ind+2 >= tcindexsize) {
		printf ("INTERNAL ERROR: Extru_tex, bounds check %d >= %d\n",vertex_ind+2,tcindexsize);
	}

	/* generate textures in a clockwise manner, reverse the triangle */
	if (!(ccw)) { j = B; B = C; C = j; }

	/* ok, we have to do textures; lets do the tcindexes and record min/max */
	tcindex[vertex_ind] = (GLuint)(tci_ct+A);
	tcindex[vertex_ind+1] =(GLuint)(tci_ct+B);
	tcindex[vertex_ind+2] =(GLuint)(tci_ct+C);
}


/*********************************************************************
 *
 * S,T mappings for Extrusions on begin and end caps.
 *
 **********************************************************************/


void Extru_ST_map(
	int triind_start,
	int start,
	int end,
	float *Vals,
	int nsec,
	GLuint *tcindex,
	GLuint *cindex,
	float *GeneratedTexCoords,
	int tcoordsize) {

	int x;
	GLfloat minS = 9999.9f;
	GLfloat maxS = -9999.9f;
	GLfloat minT = 9999.9f;
	GLfloat maxT = -9999.9f;

	GLfloat Srange = 0.0f;
	GLfloat Trange = 0.0f;

	int Point_Zero;	/* the point that all cap tris start at. see comment below */

	/* printf ("Extru_ST, nsec %d\n",nsec); */

	/* find the base and range of S, T */
	for (x=0; x<nsec; x++) {
		 /* printf ("for textures, coord vals %f %f for sec %d\n", Vals[x*2+0], Vals[x*2+1],x); */
		if (Vals[x*2+0] < minS) minS = Vals[x*2+0];
		if (Vals[x*2+0] > maxS) maxS = Vals[x*2+0];
		if (Vals[x*2+1] < minT) minT = Vals[x*2+1];
		if (Vals[x*2+1] > maxT) maxT = Vals[x*2+1];
	}
	Srange = maxS -minS;
	Trange = maxT - minT;

	/* I hate divide by zeroes. :-) */
	if (APPROX(Srange, 0.0)) Srange = 0.001f;
	if (APPROX(Trange, 0.0)) Trange = 0.001f;

	/* printf ("minS %f Srange %f minT %f Trange %f\n",minS,Srange,minT,Trange); */

	/* Ok, we know the min vals of S and T; and the ranges. The way that end cap
	 * triangles are drawn is that we have one common point, the first point in
	 * each triangle. Use this as a base into the Vals index, to generate a S,T
	 * tex coord mapping for the [0,1] range
	 */

	for(x=start; x<end; x++) {
		int tci; 
		// int ci;

		/*
		printf ("Extru_ST_Map: triangle has tex vertices:%d %d %d ",
			tcindex[triind_start*3],
			tcindex[triind_start*3+1] ,
			tcindex[triind_start*3+2]);
		printf ("Extru_ST_Map: coord vertices:%d %d %d\n",
			cindex[triind_start*3],
			cindex[triind_start*3+1] ,
			cindex[triind_start*3+2]);
		*/

		/* for first vertex */
		tci = tcindex[triind_start*3];
		//ci = cindex[triind_start*3];
		Point_Zero = tci;

		if ((tci*3+2) >= tcoordsize) {
			printf ("INTERNAL ERROR: Extru_ST_map(1), index %d greater than %d \n",(tci*3+2),tcoordsize);
			return;
		}

		/* S value */
		GeneratedTexCoords[tci*3+0] = (Vals[(tci-Point_Zero)*2+0] - minS) / Srange ;

		/* not used by render_polyrep */
		GeneratedTexCoords[tci*3+1] = 0;

		/* T value */
		GeneratedTexCoords[tci*3+2] = (Vals[(tci-Point_Zero)*2+1] - minT) / Trange;


		/* for second vertex */
		tci = tcindex[triind_start*3+1];
		//ci = cindex[triind_start*3+1];

		if ((tci*3+2) >= tcoordsize) {
			printf ("INTERNAL ERROR: Extru_ST_map(2), index %d greater than %d \n",(tci*3+2),tcoordsize);
			return;
		}

		/* S value */
		GeneratedTexCoords[tci*3+0] = (Vals[(tci-Point_Zero)*2+0] - minS) / Srange ;

		/* not used by render_polyrep */
		GeneratedTexCoords[tci*3+1] = 0;

		/* T value */
		GeneratedTexCoords[tci*3+2] = (Vals[(tci-Point_Zero)*2+1] - minT) / Trange;


		/* for third vertex */
		tci = tcindex[triind_start*3+2];

		if ((tci*3+2) >= tcoordsize) {
			printf ("INTERNAL ERROR: Extru_ST_map(3), index %d greater than %d \n",(tci*3+2),tcoordsize);
			return;
		}

		/* S value */
		GeneratedTexCoords[tci*3+0] = (Vals[(tci-Point_Zero)*2+0] - minS) / Srange ;

		/* not used by render_polyrep */
		GeneratedTexCoords[tci*3+1] = 0;

		/* T value */
		GeneratedTexCoords[tci*3+2] = (Vals[(tci-Point_Zero)*2+1] - minT) / Trange;

		triind_start++;
	}
}


void do_glNormal3fv(struct SFVec3f *dest, GLfloat *param) {
	struct point_XYZ myp;

	/* normalize all vectors; even if they are coded into a VRML file */

	myp.x = param[0]; myp.y = param[1]; myp.z = param[2];

	normalize_vector (&myp);

	dest->c[0] = (float) myp.x; dest->c[1] = (float) myp.y; dest->c[2] = (float) myp.z;
}





/*********************************************************************
 *
 * render_polyrep : render one of the internal polygonal representations
 * for some nodes
 *
 ********************************************************************/
#define DESIRE(whichOne,zzz) ((whichOne & zzz)==zzz)

void* peek_humanoid_skinCoord();
void render_polyrep(void* node) {
	//struct X3D_Virt *virt;
	struct X3D_Node* renderedNodePtr;
	struct X3D_PolyRep* pr;
	int hasc;


	ttglobal tg = gglobal();

	renderedNodePtr = X3D_NODE(node);
	//virt = virtTable[renderedNodePtr->_nodeType];
	pr = (struct X3D_PolyRep*) renderedNodePtr->_intern;

#ifdef TEXVERBOSE
	printf("\nrender_polyrep, _nodeType %s\n", stringNodeType(renderedNodePtr->_nodeType));
	printf("ntri %d\n", pr->ntri);
#endif

	if (pr->ntri == 0) {
		/* no triangles */
		return;
	}

	// do we have VBOs here? were they removed??
	if ((pr->VBO_buffers[VERTEX_VBO]) == 0) return;

	if (!pr->streamed) {
		printf("render_polyrep, not streamed, returning\n");
		return;
	}

	/* save these values for streaming the texture coordinates later */
	tg->Textures.global_tcin = pr->tcindex;
	tg->Textures.global_tcin_count = pr->ntri * 3;
	tg->Textures.global_tcin_lastParent = node;

	/* we take the geometry here, and push it up the stream. */
	if (0) {
		static int count = 0;
		if (count < 30000)
		{
			extent6f_printf(renderedNodePtr->_extent); printf(" r_p\n");
		}
		count++;
	}
	if (1)     setExtent(renderedNodePtr->EXTENT_MAX_X, renderedNodePtr->EXTENT_MIN_X, renderedNodePtr->EXTENT_MAX_Y,
		renderedNodePtr->EXTENT_MIN_Y, renderedNodePtr->EXTENT_MAX_Z, renderedNodePtr->EXTENT_MIN_Z,
		renderedNodePtr);

	/*  clockwise or not?*/
	if (!pr->ccw) {
		//FW_GL_FRONTFACE(GL_CW);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
	}
	//http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/lighting.html#t-Litcolourandalpha
	//if lit, use colors if colornode and (intensity or no texture)
	hasc = ((pr->VBO_buffers[COLOR_VBO] != 0) || pr->color);

	/* Do we have any colours? Are textures, if present, not RGB? */
	if (hasc) {
		LIGHTING_ON
	}

	/*  status bar, text do not have normals*/
	FW_GL_BINDBUFFER(GL_ARRAY_BUFFER, 0);
	if (pr->VBO_buffers[NORMAL_VBO] != 0) {
		FW_GL_BINDBUFFER(GL_ARRAY_BUFFER, pr->VBO_buffers[NORMAL_VBO]);
		FW_GL_NORMAL_POINTER(GL_FLOAT, 0, 0);
		if (DESIRE(getShaderFlags().base, SHADINGSTYLE_FLAT)) {
			if (pr->last_normal_type != 1)
				glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * pr->ntri * 3, pr->flat_normal, GL_STATIC_DRAW); /* OpenGL-ES */
			pr->last_normal_type = 1;
		}
		else {
			if (pr->last_normal_type != 0)
				glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * pr->ntri * 3, pr->normal, GL_STATIC_DRAW); /* OpenGL-ES */
			pr->last_normal_type = 0;
		}
	}

	if (pr->VBO_buffers[FOG_VBO] != 0) {
		FW_GL_BINDBUFFER(GL_ARRAY_BUFFER, pr->VBO_buffers[FOG_VBO]);
		FW_GL_FOG_POINTER(GL_FLOAT, 0, 0);
	}

	/* colours? */
	if (hasc) {

		FW_GL_BINDBUFFER(GL_ARRAY_BUFFER, pr->VBO_buffers[COLOR_VBO]);
		FW_GL_COLOR_POINTER(4, GL_FLOAT, 0, 0);
	}


	/*  textures?*/
	if (pr->VBO_buffers[TEXTURE_VBO0] != 0) {
		int k;
		struct textureVertexInfo mtf[4] = { {NULL,2,GL_FLOAT,0, NULL,NULL},
			{NULL,2,GL_FLOAT,0, NULL,NULL},{NULL,2,GL_FLOAT,0, NULL,NULL},{NULL,2,GL_FLOAT,0, NULL,NULL} };
		for (k = 0; k < max(1, pr->ntcoord); k++) {
			//FW_GL_BINDBUFFER(GL_ARRAY_BUFFER,pr->VBO_buffers[TEXTURE_VBO0+k]);
			mtf[k].VBO = pr->VBO_buffers[TEXTURE_VBO0 + k];
			mtf[k].TC_size = pr->ntexdim[k];
			if (k > 0) mtf[k - 1].next = &mtf[k];
		}
		textureCoord_send(mtf);
	}
	else {
		ConsoleMessage("skipping tds of textures");
	}
	//humanoid skinning
	if (pr->VBO_buffers[CINDEX_VBO] != 0) {
		PRINT_GL_ERROR_IF_ANY("");

		//in child_humanoid before drawing skin we push the humanoid.coords 
		// and in here if we set the joint index VBO and joint matrix UBO
		printf("SKINNING ");
		FW_GL_BINDBUFFER(GL_ARRAY_BUFFER, pr->VBO_buffers[CINDEX_VBO]);
		PRINT_GL_ERROR_IF_ANY("");
		FW_GL_CINDEX_POINTER(GL_INT, 0, 0);
		PRINT_GL_ERROR_IF_ANY("");

	}

	FW_GL_BINDBUFFER(GL_ARRAY_BUFFER, pr->VBO_buffers[VERTEX_VBO]);
	FW_GL_BINDBUFFER(GL_ELEMENT_ARRAY_BUFFER, pr->VBO_buffers[INDEX_VBO]);
	FW_GL_VERTEX_POINTER(3, GL_FLOAT, 0, 0);

	if (DESIRE(getShaderFlags().base, SHADINGSTYLE_WIRE)) {
		//wireframe triangles
		if (pr->last_index_type != 1)
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * pr->ntri * 3 * 2, pr->wire_indices, GL_STATIC_DRAW); /* OpenGL-ES */
		pr->last_index_type = 1;
		//if (setupShader())
		//	glDrawElements(GL_LINES, pr->ntri*3*2, GL_UNSIGNED_SHORT, NULL);
		sendElementsToGPU(GL_LINES, pr->ntri * 3 * 2, NULL);
	}
	else {
		//surface triangles 
		//glDrawArrays(GL_TRIANGLES,,,) doesn't use indices - its glDrawElements that does
		//if(pr->last_index_type != 0)
		//	glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof (GLushort)*pr->ntri*3,pr->tri_indices,GL_STATIC_DRAW); /* OpenGL-ES */
		pr->last_index_type = 0;
		sendArraysToGPU(GL_TRIANGLES, 0, pr->ntri * 3);
	}

	/* turn VBOs off for now */
	//FW_GL_BINDBUFFER(GL_ARRAY_BUFFER, 0);
	//FW_GL_BINDBUFFER(GL_ELEMENT_ARRAY_BUFFER, 0);

	tg->Mainloop.trisThisLoop += pr->ntri;



	PRINT_GL_ERROR_IF_ANY("");

	if (!pr->ccw) {
		//FW_GL_FRONTFACE(GL_CCW);
		glCullFace(GL_BACK); //restore to default
		glDisable(GL_CULL_FACE);
	}

#ifdef TEXVERBOSE
	{
		int i;
		int* cin;
		float* cod;
		float* tcod;
		tcod = pr->GeneratedTexCoords;
		cod = pr->actualCoord;
		cin = pr->cindex;
		printf("\n\nrender_polyrep:\n");
		for (i = 0; i < pr->ntri * 3; i++) {
			printf("i %d cindex %d vertex %f %f %f", i, cin[i],
				cod[cin[i] * 3 + 0],
				cod[cin[i] * 3 + 1],
				cod[cin[i] * 3 + 2]);

			if (tcod != 0) {
				printf(" tex %f %f",
					tcod[cin[i] * 2 + 0],
					tcod[cin[i] * 2 + 1]);
			}
			printf("\n");
		}
	}
#endif

	PRINT_GL_ERROR_IF_ANY("");


}


/*********************************************************************
 *
 * render_ray_polyrep : get intersections of a ray with one of the
 * polygonal representations
 *
 * currently handled:
 *	rendray_Text 
 *	rendray_ElevationGrid  
 *	rendray_Extrusion 
 *	rendray_IndexedFaceSet  
 *	rendray_ElevationGrid 
 *	rendray_IndexedTriangleSet 
 *	rendray_IndexedTriangleFanSet 
 *	rendray_IndexedTriangleStripSet 
 *	rendray_TriangleSet 
 *	rendray_TriangleFanSet 
 *	rendray_TriangleStripSet 
 *	rendray_GeoElevationGrid 
 */


void render_ray_polyrep_A(void *node) {
	//this doesn't work with large pick rays for geo size scenes
	//struct X3D_Virt *virt;
	struct X3D_Node *genericNodePtr;
	struct X3D_PolyRep *polyRep;
	int i;
	int pt;
	float *point[3];
	struct point_XYZ v1, v2, v3;
	//struct point_XYZ ray;
	float pt1, pt2, pt3;
	struct point_XYZ hitpoint;
	float tmp1,tmp2;
	float v1len, v2len, v3len;
	float v12pt;
	struct point_XYZ t_r1,t_r2;
	//ttglobal tg;
	
	/* is this structure still loading? */
	if (!node) return;
	//tg = gglobal();
	//VECCOPY(t_r1,tg->RenderFuncs.t_r1);
	//VECCOPY(t_r2,tg->RenderFuncs.t_r2);
	get_current_ray(&t_r1, &t_r2);

	//VECCOPY(t_r3,tg->RenderFuncs.t_r3);

	//ray.x = t_r2.x - t_r1.x;
	//ray.y = t_r2.y - t_r1.y;
	//ray.z = t_r2.z - t_r1.z;

	genericNodePtr = X3D_NODE(node);
	//virt = virtTable[genericNodePtr->_nodeType];
	
	/* is this structure still loading? */
	if (!(genericNodePtr->_intern)) {
		/* printf ("render_ray_polyrep - no internal structure, returning\n"); */
		return;
	}

	polyRep = (struct X3D_PolyRep*) genericNodePtr->_intern;
	if (!polyRep->ntri || !polyRep->actualCoord) return;
	/*
	printf("render_ray_polyrep %d '%s' (%d %d): %d\n",node,stringNodeType(genericNodePtr->_nodeType),
		genericNodePtr->_change, polyRep->_change, polyRep->ntri);
	*/

	

	for(i=0; i<polyRep->ntri; i++) {
		for(pt = 0; pt<3; pt++) {
			int ind = polyRep->cindex[i*3+pt];
			point[pt] = (polyRep->actualCoord+3*ind);
		}

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
		v1len = (float) sqrt(VECSQ(v1)); VECSCALE(v1, 1/v1len);
		v2len = (float) sqrt(VECSQ(v2)); VECSCALE(v2, 1/v2len);
		v12pt = (float) VECPT(v1,v2);

		/* this will get around a divide by zero further on JAS */
		if (fabs(v12pt-1.0) < 0.00001) continue;

		/* if we have a degenerate triangle, we can't compute a normal, so skip */
		if ((fabs(v1len) > 0.00001) && (fabs(v2len) > 0.00001)) {

			/* v3 is our normal to the surface */
			VECCP(v1,v2,v3);
			v3len = (float) sqrt(VECSQ(v3)); VECSCALE(v3, 1/v3len);
			pt1 = (float) VECPT(t_r1,v3);
			pt2 = (float) VECPT(t_r2,v3);
			pt3 = (float) (v3.x * point[0][0] + v3.y * point[0][1] + v3.z * point[0][2]);
			/* Now we have (1-r)pt1 + r pt2 - pt3 = 0
			 * r * (pt1 - pt2) = pt1 - pt3
			 */
			 tmp1 = pt1-pt2;
			 if(!APPROX(tmp1,0)) {
			 	float ra, rb;
				float k,l;
				struct point_XYZ p0h;

			 	tmp2 = (float) ((pt1-pt3) / (pt1-pt2));
				hitpoint.x = MRATX(tmp2);
				hitpoint.y = MRATY(tmp2);
				hitpoint.z = MRATZ(tmp2);
				/* Now we want to see if we are in the triangle */
				/* Projections to the two triangle sides */
				p0h.x = hitpoint.x - point[0][0];
				p0h.y = hitpoint.y - point[0][1];
				p0h.z = hitpoint.z - point[0][2];
				ra = (float) VECPT(v1, p0h);
				if(ra < 0.0f) {continue;}
				rb = (float) VECPT(v2, p0h);
				if(rb < 0.0f) {continue;}
				/* Now, the condition for the point to
				 * be inside
				 * (ka + lb = p)
				 * (k + l b.a = p.a)
				 * (k b.a + l = p.b)
				 * (k - (b.a)**2 k = p.a - (b.a)*p.b)
				 * k = (p.a - (b.a)*(p.b)) / (1-(b.a)**2)
				 */
				 k = (ra - v12pt * rb) / (1-v12pt*v12pt);
				 l = (rb - v12pt * ra) / (1-v12pt*v12pt);
				 k /= v1len; l /= v2len;
				 if(k+l > 1 || k < 0 || l < 0) {
				 	continue;
				 }
				 rayhit(((float)(tmp2)),
					((float)(hitpoint.x)),
					((float)(hitpoint.y)),
					((float)(hitpoint.z)),
				 	((float)(v3.x)),
					((float)(v3.y)),
					((float)(v3.z)),
					((float)-1),((float)-1), "polyrep");
			 }
		/*
		} else {
			printf ("render_ray_polyrep, skipping degenerate triangle\n");
		*/
		}
	}
}

int triangle_intersection( float *  V1,  // Triangle vertices
                           float *  V2,
                           float *  V3,
                           float *   O,  //Ray origin
                           float *   D,  //Ray direction
                           float* out );

void render_ray_polyrep_B(void *node) {
	// this doesn't work in townsite_withHud on about the 3rd photo, can't pick in-scene hud
	//struct X3D_Virt *virt;
	struct X3D_Node *genericNodePtr;
	struct X3D_PolyRep *polyRep;
	int i;
	int pt;
	float *point[3];
	struct point_XYZ v1, v2, v3;
	double d1[3], d2[3], dO[3], dD[3];
	float p2[3], O[3], D[3], H[3], scale;
	//struct point_XYZ ray;
	float pt1, pt2, pt3;
	struct point_XYZ hitpoint;
	float tmp1,tmp2;
	float v1len, v2len, v3len;
	float v12pt;
	struct point_XYZ t_r1,t_r2;
	//ttglobal tg;
	
	/* is this structure still loading? */
	if (!node) return;
	get_current_ray(&t_r1, &t_r2);
	genericNodePtr = X3D_NODE(node);
	
	/* is this structure still loading? */
	if (!(genericNodePtr->_intern)) {
		/* printf ("render_ray_polyrep - no internal structure, returning\n"); */
		return;
	}

	polyRep = (struct X3D_PolyRep*) genericNodePtr->_intern;

	/*	
	printf("render_ray_polyrep %d '%s' (%d %d): %d\n",node,stringNodeType(genericNodePtr->_nodeType),
		genericNodePtr->_change, polyRep->_change, polyRep->ntri);
	*/
	//Feb 2018: we have to do some math in double, when working with geospatial or very big polyreps..
	pointxyz2double(dO,&t_r1);
	pointxyz2double(d2,&t_r2);
	vecdifd(dD,d2,dO);
	vecnormald(dD,dD);
	
	//..then once we have difference vectors, we can switch to float
	double2float(O,dO,3);
	double2float(D,dD,3);
	/*
	vecprint3db("dO",dO,"\n");
	vecprint3db("d2",d2,"\n");
	vecprint3fb("O",O,"\n");
	vecprint3fb("D",D,"\n");
	*/
	for(i=0; i<polyRep->ntri; i++) {
		for(pt = 0; pt<3; pt++) {
			int ind = polyRep->cindex[i*3+pt];
			point[pt] = (polyRep->actualCoord+3*ind);
		}
		if(triangle_intersection(point[0],point[1],point[2],O,D,&scale)){
			vecadd3f(H,O,vecscale3f(H,D,scale));
			rayhit(scale,
			H[0],
			H[1],
			H[2],
			0.0f,
			0.0f,
			0.0f,
			-1.0f,-1.0f, "polyrep2");
		}
	}
}

void render_ray_polyrep(void *node) {
	//dug9: out of time and the picking needs a re-do
	// this is a hack to get it working for close range and big (geo) scenes
	double p1[3], p2[3], dd[3], dlength;
	struct point_XYZ t_r1,t_r2;
	get_current_ray(&t_r1, &t_r2);
	pointxyz2double(p1,&t_r1);
	//pointxyz2double(p2,&t_r2);
	//vecdifd(dd,p2,p1);
	dlength = veclengthd(p1);
	if(dlength > 1000.0){
		render_ray_polyrep_B(node);
		//vecprint3db("p1",p1,"");
		//vecprint3db("p2",p2,"\n");
		//printf("B");
	}else{
		render_ray_polyrep_A(node);
		//vecprint3db("p1",p1,"");
		//vecprint3db("p2",p2,"\n");
		//printf("A");
	}

}

// https://en.wikipedia.org/wiki/Möller–Trumbore_intersection_algorithm
#define EPSILON 0.000001
int triangle_intersection( float *  V1,  // Triangle vertices
                           float *  V2,
                           float *  V3,
                           float *   O,  //Ray origin
                           float *   D,  //Ray direction
                           float* out )   // scale of ray direction from origin to intersection
{
  float e1[3], e2[3];  //Edge1, Edge2
  float P[3], Q[3], T[3];
  float det, inv_det, u, v;
  float t;

  //Find vectors for two edges sharing V1
  vecdif3f(e1, V2, V1);
  vecdif3f(e2, V3, V1);
  //Begin calculating determinant - also used to calculate u parameter
  veccross3f(P, D, e2);
  //if determinant is near zero, ray lies in plane of triangle or ray is parallel to plane of triangle
  det = vecdot3f(e1, P);
  //NOT CULLING
  if(det > -EPSILON && det < EPSILON) return 0;
  inv_det = 1.f / det;

  //calculate distance from V1 to ray origin
  vecdif3f(T, O, V1);

  //Calculate u parameter and test bound
  u = vecdot3f(T, P) * inv_det;
  //The intersection lies outside of the triangle
  if(u < 0.f || u > 1.f) return 0;

  //Prepare to test v parameter
  veccross3f(Q, T, e1);

  //Calculate V parameter and test bound
  v = vecdot3f(D, Q) * inv_det;
  //The intersection lies outside of the triangle
  if(v < 0.f || u + v  > 1.f) return 0;

  t = vecdot3f(e2, Q) * inv_det;

  if(t > EPSILON) { //ray intersection
    *out = t;
    return 1;
  }

  // No hit, no win
  return 0;
}

enum {
	RAYTRIALGO_DEFAULT = 1,
	RAYTRIALGO_MULLER = 2,
	//Not implemented: watertight  http://jcgt.org/published/0002/01/05/paper.pdf
};
static int raytrialgo = RAYTRIALGO_MULLER;
int intersect_polyrep(struct X3D_Node *node, float *p1, float *p2, float *nearest, float *normal){
	//need a utility function like part of guts of render_ray_polyrep
	//everything in same coordinate system, no matrix multiplication in here
	//p1, p2 - 2 points forming ray, in direciton from p1 toward p2, of length p2-p1
	//return value:
	//	count of intersections - can be used for inside test (odd - inside, even - outside)
	// nearest[3] - the intersection nearest p1
	// normal[3] - normal at nearest intersection

	//struct X3D_Virt *virt;
	struct X3D_Node *genericNodePtr;
	struct X3D_PolyRep *polyRep;
	int i, nintersections, ihavehit;
	int pt;
	float nearestdist, delta[3];
	float *point[3];
	struct point_XYZ v1, v2, v3;
	//struct point_XYZ ray;
	float pt1, pt2, pt3;
	struct point_XYZ hitpoint;
	float tmp1,tmp2;
	float v1len, v2len, v3len;
	float v12pt;
	struct point_XYZ t_r1,t_r2;
	//ttglobal tg;

	ihavehit = -1;

	/* is this structure still loading? */
	if (!node) return 0;
	//tg = gglobal();
	//VECCOPY(t_r1,tg->RenderFuncs.t_r1);
	//VECCOPY(t_r2,tg->RenderFuncs.t_r2);
//	get_current_ray(&t_r1, &t_r2);
	vecdif3f(delta,p2,p1);
	nearestdist = veclength3f(delta) + .000001f;
	nintersections = 0;
	t_r1.x = p1[0];
	t_r1.y = p1[1];
	t_r1.z = p1[2];
	t_r2.x = p2[0];
	t_r2.y = p2[1];
	t_r2.z = p2[2];
	//VECCOPY(t_r3,tg->RenderFuncs.t_r3);

	//ray.x = t_r2.x - t_r1.x;
	//ray.y = t_r2.y - t_r1.y;
	//ray.z = t_r2.z - t_r1.z;

	genericNodePtr = X3D_NODE(node);
	//virt = virtTable[genericNodePtr->_nodeType];
	
	/* is this structure still loading? */
	if (!(genericNodePtr->_intern)) {
		/* printf ("render_ray_polyrep - no internal structure, returning\n"); */
		return 0;
	}

	polyRep = (struct X3D_PolyRep*) genericNodePtr->_intern;

	/*	
	printf("render_ray_polyrep %d '%s' (%d %d): %d\n",node,stringNodeType(genericNodePtr->_nodeType),
		genericNodePtr->_change, polyRep->_change, polyRep->ntri);
	*/

	
	for(i=0; i<polyRep->ntri; i++) {
		for(pt = 0; pt<3; pt++) {
			int ind = polyRep->cindex[i*3+pt];
			point[pt] = (polyRep->actualCoord+3*ind);
		}
		if(raytrialgo == RAYTRIALGO_MULLER){
			//works for particlephysics > bounded physics
			float tscale, d2[3],delta[3];
			vecdif3f(delta,p2,p1);
			vecnormalize3f(d2,delta); //muller takes a D normalized direction vector
			if(triangle_intersection(point[0],point[1],point[2],p1,d2,&tscale)){
				//printf("muller-trumbore intersection tascale %f nearestdist %f\n",tscale,nearestdist);
				nintersections++;
				if(tscale > 0.0f && tscale < nearestdist){
					//closest so far
					float e1[3],e2[3],nn[3],pd[3];
					vecscale3f(pd,d2,tscale);
					vecadd3f(nearest,p1,pd);
					//compute normal to triangle
					vecdif3f(e1,point[1],point[0]);
					vecdif3f(e2,point[2],point[0]);
					veccross3f(nn,e1,e2);  //e2,e1 points inside, e1,e2 points outside
					vecnormalize3f(normal,nn);
					nearestdist = tscale;
					ihavehit = 1;
				}
			}
		}else if(raytrialgo == RAYTRIALGO_DEFAULT){
			//doesn't work right for particle physics > bounded physics
			//x leaks on right side of IFS box, occassionally leaky left side
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
			v1len = (float) sqrt(VECSQ(v1)); VECSCALE(v1, 1/v1len);
			v2len = (float) sqrt(VECSQ(v2)); VECSCALE(v2, 1/v2len);
			v12pt = (float) VECPT(v1,v2);

			/* this will get around a divide by zero further on JAS */
			if (fabs(v12pt-1.0) < 0.00001) 
				continue;

			/* if we have a degenerate triangle, we can't compute a normal, so skip */

			if ((fabs(v1len) > 0.00001) && (fabs(v2len) > 0.00001)) {

				/* v3 is our normal to the surface */
				VECCP(v1,v2,v3);
				v3len = (float) sqrt(VECSQ(v3)); VECSCALE(v3, 1/v3len);

				pt1 = (float) VECPT(t_r1,v3);
				pt2 = (float) VECPT(t_r2,v3);
				pt3 = (float) (v3.x * point[0][0] + v3.y * point[0][1] + v3.z * point[0][2]);
				/* Now we have (1-r)pt1 + r pt2 - pt3 = 0
				 * r * (pt1 - pt2) = pt1 - pt3
				 */
				 tmp1 = pt1-pt2;
				 if(!APPROX(tmp1,0)) {
			 		float ra, rb;
					float k,l;
					struct point_XYZ p0h;

			 		tmp2 = (float) ((pt1-pt3) / (pt1-pt2));
					hitpoint.x = MRATX(tmp2);
					hitpoint.y = MRATY(tmp2);
					hitpoint.z = MRATZ(tmp2);
					/* Now we want to see if we are in the triangle */
					/* Projections to the two triangle sides */
					p0h.x = hitpoint.x - point[0][0];
					p0h.y = hitpoint.y - point[0][1];
					p0h.z = hitpoint.z - point[0][2];
					ra = (float) VECPT(v1, p0h);
					if(ra < 0.0f) {
						continue;
					}
					rb = (float) VECPT(v2, p0h);
					if(rb < 0.0f) {
						continue;
					}
					/* Now, the condition for the point to
					 * be inside
					 * (ka + lb = p)
					 * (k + l b.a = p.a)
					 * (k b.a + l = p.b)
					 * (k - (b.a)**2 k = p.a - (b.a)*p.b)
					 * k = (p.a - (b.a)*(p.b)) / (1-(b.a)**2)
					 */
					 k = (ra - v12pt * rb) / (1-v12pt*v12pt);
					 l = (rb - v12pt * ra) / (1-v12pt*v12pt);
					 k /= v1len; l /= v2len;
					 if(k+l > 1 || k < 0 || l < 0) {
				 		continue;
					 }
					 //we have a hit
					 nintersections ++;
					 if(tmp2 >= 0.0 && tmp2 < nearestdist){
						ihavehit = 1;
						nearest[0] = (float)hitpoint.x;
						nearest[1] = (float)hitpoint.y;
						nearest[2] = (float)hitpoint.z;
						normal[0] = (float)v3.x;
						normal[1] = (float)v3.y;
						normal[2] = (float)v3.z;
						nearestdist = tmp2;
					}
					 //rayhit(((float)(tmp2)),
						//((float)(hitpoint.x)),
						//((float)(hitpoint.y)),
						//((float)(hitpoint.z)),
					 //	((float)(v3.x)),
						//((float)(v3.y)),
						//((float)(v3.z)),
						//((float)-1),((float)-1), "polyrep");
				 }
			} // if degenerate 
		} // if raytrialgo
	} //for triangle

	return nintersections*ihavehit; //-ve if no hit but intersections of infinite ray for p1,
	// +ve if hit and intersections, 0 -nothing
}
int intersect_polyrep2(struct X3D_Node *node, float *p1, float *p2, Stack *intersection_stack){
	//this one returns a vector of points
	//please allocate intersection_stack before calling ie instersection_stack = newStack(struct intersection_info);
	//p1 - p2 is your plumbline or line, see also particalsystems for use
	//need a utility function like part of guts of render_ray_polyrep
	//everything in same coordinate system, no matrix multiplication in here
	//p1, p2 - 2 points forming ray, in direciton from p1 toward p2, of length p2-p1
	//return value:
	//	count of intersections - can be used for inside test (odd - inside, even - outside)
	// nearest[3] - the intersection nearest p1
	// normal[3] - normal at nearest intersection

	//struct X3D_Virt *virt;
	struct X3D_Node *genericNodePtr;
	struct X3D_PolyRep *polyRep;
	int i, nintersections, ihavehit;
	int pt;
	float nearestdist, delta[3];
	float *point[3];
	struct point_XYZ v1, v2, v3;
	//struct point_XYZ ray;
	float pt1, pt2, pt3;
	struct point_XYZ hitpoint;
	float tmp1,tmp2;
	float v1len, v2len, v3len;
	float v12pt;
	struct point_XYZ t_r1,t_r2;
	//ttglobal tg;

	ihavehit = -1;

	/* is this structure still loading? */
	if (!node) return 0;
	//tg = gglobal();
	//VECCOPY(t_r1,tg->RenderFuncs.t_r1);
	//VECCOPY(t_r2,tg->RenderFuncs.t_r2);
//	get_current_ray(&t_r1, &t_r2);
	vecdif3f(delta,p2,p1);
	nearestdist = veclength3f(delta) + .000001f;
	nintersections = 0;
	t_r1.x = p1[0];
	t_r1.y = p1[1];
	t_r1.z = p1[2];
	t_r2.x = p2[0];
	t_r2.y = p2[1];
	t_r2.z = p2[2];
	//VECCOPY(t_r3,tg->RenderFuncs.t_r3);

	//ray.x = t_r2.x - t_r1.x;
	//ray.y = t_r2.y - t_r1.y;
	//ray.z = t_r2.z - t_r1.z;

	genericNodePtr = X3D_NODE(node);
	//virt = virtTable[genericNodePtr->_nodeType];
	
	/* is this structure still loading? */
	if (!(genericNodePtr->_intern)) {
		/* printf ("render_ray_polyrep - no internal structure, returning\n"); */
		return 0;
	}

	polyRep = (struct X3D_PolyRep*) genericNodePtr->_intern;

	/*	
	printf("render_ray_polyrep %d '%s' (%d %d): %d\n",node,stringNodeType(genericNodePtr->_nodeType),
		genericNodePtr->_change, polyRep->_change, polyRep->ntri);
	*/

	

	for(i=0; i<polyRep->ntri; i++) {
		for(pt = 0; pt<3; pt++) {
			int ind = polyRep->cindex[i*3+pt];
			point[pt] = (polyRep->actualCoord+3*ind);
		}
		if(raytrialgo == RAYTRIALGO_MULLER){
			//works for particlephysics > bounded physics
			float tscale, d2[3],delta[3];
			vecdif3f(delta,p2,p1);
			vecnormalize3f(d2,delta); //muller takes a D normalized direction vector
			if(triangle_intersection(point[0],point[1],point[2],p1,d2,&tscale)){
				//printf("muller-trumbore intersection tascale %f nearestdist %f\n",tscale,nearestdist);
				nintersections++;
				if(tscale > 0.0f && tscale < veclength3f(delta)){  //nearestdist){
					//closest so far
					float e1[3],e2[3],nn[3],pd[3],pi[3],normi[3];
					struct intersection_info iinfo;
					vecscale3f(pd,d2,tscale);
					vecadd3f(pi,p1,pd);
					//compute normal to triangle
					vecdif3f(e1,point[1],point[0]);
					vecdif3f(e2,point[2],point[0]);
					veccross3f(nn,e1,e2);  //e2,e1 points inside, e1,e2 points outside
					vecnormalize3f(normi,nn);
					veccopy3f(iinfo.p,pi);
					veccopy3f(iinfo.normal,normi);
					stack_push(struct intersection_info,intersection_stack,iinfo);
					if(tscale < nearestdist) {
						//veccopy3f(nearest,pi);
						//veccopy3f(normal,normi);
						nearestdist = tscale;
						ihavehit = 1;
					}
				}
			}
		}else if(raytrialgo == RAYTRIALGO_DEFAULT){
			//doesn't work right for particle physics > bounded physics
			//x leaks on right side of IFS box, occassionally leaky left side
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
			v1len = (float) sqrt(VECSQ(v1)); VECSCALE(v1, 1/v1len);
			v2len = (float) sqrt(VECSQ(v2)); VECSCALE(v2, 1/v2len);
			v12pt = (float) VECPT(v1,v2);

			/* this will get around a divide by zero further on JAS */
			if (fabs(v12pt-1.0) < 0.00001) 
				continue;

			/* if we have a degenerate triangle, we can't compute a normal, so skip */

			if ((fabs(v1len) > 0.00001) && (fabs(v2len) > 0.00001)) {

				/* v3 is our normal to the surface */
				VECCP(v1,v2,v3);
				v3len = (float) sqrt(VECSQ(v3)); VECSCALE(v3, 1/v3len);

				pt1 = (float) VECPT(t_r1,v3);
				pt2 = (float) VECPT(t_r2,v3);
				pt3 = (float) (v3.x * point[0][0] + v3.y * point[0][1] + v3.z * point[0][2]);
				/* Now we have (1-r)pt1 + r pt2 - pt3 = 0
				 * r * (pt1 - pt2) = pt1 - pt3
				 */
				 tmp1 = pt1-pt2;
				 if(!APPROX(tmp1,0)) {
			 		float ra, rb;
					float k,l;
					struct point_XYZ p0h;

			 		tmp2 = (float) ((pt1-pt3) / (pt1-pt2));
					hitpoint.x = MRATX(tmp2);
					hitpoint.y = MRATY(tmp2);
					hitpoint.z = MRATZ(tmp2);
					/* Now we want to see if we are in the triangle */
					/* Projections to the two triangle sides */
					p0h.x = hitpoint.x - point[0][0];
					p0h.y = hitpoint.y - point[0][1];
					p0h.z = hitpoint.z - point[0][2];
					ra = (float) VECPT(v1, p0h);
					if(ra < 0.0f) {
						continue;
					}
					rb = (float) VECPT(v2, p0h);
					if(rb < 0.0f) {
						continue;
					}
					/* Now, the condition for the point to
					 * be inside
					 * (ka + lb = p)
					 * (k + l b.a = p.a)
					 * (k b.a + l = p.b)
					 * (k - (b.a)**2 k = p.a - (b.a)*p.b)
					 * k = (p.a - (b.a)*(p.b)) / (1-(b.a)**2)
					 */
					 k = (ra - v12pt * rb) / (1-v12pt*v12pt);
					 l = (rb - v12pt * ra) / (1-v12pt*v12pt);
					 k /= v1len; l /= v2len;
					 if(k+l > 1 || k < 0 || l < 0) {
				 		continue;
					 }
					 //we have a hit
					 nintersections ++;
					 if(tmp2 >= 0.0 && tmp2 < nearestdist){
						ihavehit = 1;
						//nearest[0] = (float)hitpoint.x;
						//nearest[1] = (float)hitpoint.y;
						//nearest[2] = (float)hitpoint.z;
						//normal[0] = (float)v3.x;
						//normal[1] = (float)v3.y;
						//normal[2] = (float)v3.z;
						nearestdist = tmp2;
					}
					 //rayhit(((float)(tmp2)),
						//((float)(hitpoint.x)),
						//((float)(hitpoint.y)),
						//((float)(hitpoint.z)),
					 //	((float)(v3.x)),
						//((float)(v3.y)),
						//((float)(v3.z)),
						//((float)-1),((float)-1), "polyrep");
				 }
			} // if degenerate 
		} // if raytrialgo
	} //for triangle

	return nintersections*ihavehit; //-ve if no hit but intersections of infinite ray for p1,
	// +ve if hit and intersections, 0 -nothing
}

int getPolyrepTriangleCount(struct X3D_Node *node){
	int iret;
	iret = 0;
	if(node->_intern){
		struct X3D_PolyRep *polyrep = (struct X3D_PolyRep *)node->_intern;
		iret = polyrep->ntri;
	}
	return iret;
}
int getPolyrepTriangleByIndex(struct X3D_Node *node, int index, float *v1, float *v2, float *v3){
	int iret;
	iret = 0;
	if(node->_intern){
		struct X3D_PolyRep *polyrep = (struct X3D_PolyRep *)node->_intern;
		veccopy3f(v1,&polyrep->actualCoord[(index*3 + 0)*3]);
		veccopy3f(v2,&polyrep->actualCoord[(index*3 + 1)*3]);
		veccopy3f(v3,&polyrep->actualCoord[(index*3 + 2)*3]);
	}
	return iret;

}

/* make the internal polyrep structure - this will contain the actual RUNTIME parameters for OpenGL */
void compile_polyrep(void *innode, void *coord, void *fogCoord, void *color, void *normal, struct X3D_TextureCoordinate *texCoord) {
	struct X3D_Virt *virt;
	struct X3D_Node *node;
	struct X3D_PolyRep *polyrep;

    //printf ("compile_polyrep, innode %p coord %p color %p normal %p texCoord %p\n",innode,coord,color,normal,texCoord);
    
	node = X3D_NODE(innode);
	virt = virtTable[node->_nodeType];

	/* first time through; make the intern structure for this polyrep node */
	if(!node->_intern) {

		int i;

		node->_intern = MALLOC(struct X3D_GeomRep *, sizeof(struct X3D_PolyRep));
		memset(node->_intern,0,sizeof(struct X3D_PolyRep));
		polyrep = (struct X3D_PolyRep*) node->_intern;
		polyrep->itype = 2; //0 points 1 lines 2 mesh
		polyrep->mode = 4; //4 TRIANGLES 5 TRIANGLE_STRIP 6 TRIANGLE_FAN
		polyrep->ntri = -1;
		//polyrep->cindex = 0; polyrep->actualCoord = 0; polyrep->colindex = 0; polyrep->color = 0;
		//polyrep->norindex = 0; polyrep->normal = 0; polyrep->flat_normal = 0; polyrep->GeneratedTexCoords = 0;
		//polyrep->tri_indices = 0; polyrep->wire_indices = 0; polyrep->actualFog =  0;
		//polyrep->tcindex = 0; 
		//polyrep->tcoordtype = 0;
		polyrep->streamed = FALSE;
		//polyrep->last_index_type = 0; polyrep->last_normal_type = 0;
		

		/* for Collision, default texture generation */
		polyrep->minVals[0] =  999999.9f;
		polyrep->minVals[1] =  999999.9f;
		polyrep->minVals[2] =  999999.9f;
		polyrep->maxVals[0] =  -999999.9f;
		polyrep->maxVals[1] =  -999999.9f;
		polyrep->maxVals[2] =  -999999.9f;

		for (i=0; i<VBO_COUNT; i++) 
			polyrep->VBO_buffers[i] = 0;

		/* printf ("generating buffers for node %p, type %s\n",p,stringNodeType(p->_nodeType)); */
		glGenBuffers(1,&polyrep->VBO_buffers[VERTEX_VBO]);
		glGenBuffers(1,&polyrep->VBO_buffers[INDEX_VBO]);

		/* printf ("they are %u %u %u %u\n",polyrep->VBO_buffers[0],polyrep->VBO_buffers[1],polyrep->VBO_buffers[2],polyrep->VBO_buffers[3]); */

	}

	polyrep = (struct X3D_PolyRep*) node->_intern;
	polyrep->coordinate_node = coord; //for testing if skinning elsewhere

	/* Android, for instance, needs the VBO_buffers re-created. Check to see if this is the case here */
	if (polyrep->VBO_buffers[VERTEX_VBO] == 0) {
		//ConsoleMessage ("re-creating VERTEX VBOs");
		glGenBuffers(1,&polyrep->VBO_buffers[VERTEX_VBO]);
		glGenBuffers(1,&polyrep->VBO_buffers[INDEX_VBO]);
	} 

	/* if multithreading, tell the rendering loop that we are regenning this one */
	/* if singlethreading, this'll be set to TRUE before it is tested	     */
	polyrep->streamed = FALSE;

	FREE_IF_NZ(polyrep->cindex);
	FREE_IF_NZ(polyrep->actualCoord);
	FREE_IF_NZ(polyrep->GeneratedTexCoords[0]);
	FREE_IF_NZ(polyrep->colindex);
	FREE_IF_NZ(polyrep->color);
	FREE_IF_NZ(polyrep->norindex);
	FREE_IF_NZ(polyrep->normal);
	FREE_IF_NZ(polyrep->flat_normal);
	FREE_IF_NZ(polyrep->tcindex);


	/* make the node by calling the correct method see GenPolyRep.c > make_genericfaceset */
	virt->mkpolyrep(node);

	/* now, put the generic internal structure into OpenGL arrays for faster rendering */
	/* if there were errors, then rep->ntri should be 0 */
	if (polyrep->ntri != 0) {
		//float *fogCoord = NULL;
		stream_polyrep(node, coord, fogCoord, color, normal, texCoord);
		/* and, tell the rendering process that this shape is now compiled */
	}
	//else wait for set_coordIndex to be converted to coordIndex
	polyrep->irep_change = node->_change;

}

void delete_geomrep(struct X3D_Node *node){
	// see if node has _intern, if so it's live scenery
	// delete opengl buffers used by polyrep
	// delete internal malloced items in polyrep
	// delete polyrep
	// null node's _intern field
	if(!node) return;
	if (!node->_intern) return;
	// 0 PointRep 1 LineRep 2 PolyRep 3 MeshRep 4 TextureRep 5 LightRep
	switch(node->_intern->itype){
	case 0: //points
		{
			if (node->_nodeType != NODE_PointSet && node->_nodeType != NODE_Polypoint2D) {
				printf("attempting to delete PointRep for nodetype %s\n", stringNodeType(node->_nodeType));
				break;
			}
			delete_PointRep(node->_intern);
			node->_intern = NULL;
		}
		break;
	case 1: //lines
		{
			struct X3D_LineRep* lr;
			lr = (struct X3D_LineRep*)node->_intern;
			//not implemented yet
		}
		break;
	case 2: //mesh
		{
			struct X3D_PolyRep* pr;
			pr = (struct X3D_PolyRep*)node->_intern;
			// ? apr 2015 I think the node->_intern = polyrep will only be populated 
			// in the case of live scenery, not in ProtoDeclares, so if we are in here, 
			// we should have live scenery, and that means gl buffers were assigned
			glDeleteBuffers(VBO_COUNT, pr->VBO_buffers);

			/* indicies for arrays. OpenGL ES 2.0 - unsigned short for the DrawArrays call */
			FREE_IF_NZ(pr->cindex);   /* triples (per triangle) */
			FREE_IF_NZ(pr->colindex);   /* triples (per triangle) */
			FREE_IF_NZ(pr->norindex);
			FREE_IF_NZ(pr->tcindex); /* triples or null */
			FREE_IF_NZ(pr->tri_indices);
			FREE_IF_NZ(pr->wire_indices);
			FREE_IF_NZ(pr->actualCoord); /* triples (per point) */
			FREE_IF_NZ(pr->actualFog); /* float (per point) */
			FREE_IF_NZ(pr->color); /* triples or null */
			FREE_IF_NZ(pr->normal); /* triples or null */
			FREE_IF_NZ(pr->flat_normal);
			FREE_IF_NZ(pr->GeneratedTexCoords[0]);	/* triples (per triangle) of texture coords if there is no texCoord node */
			FREE_IF_NZ(pr);
			node->_intern = NULL;
		}
		break;
	case 3: //MeshRep for gltf_loader.c
		{
			if (node->_nodeType != NODE_BufferGeometry) {
				printf("attempting to delete MeshRep for nodetype %s\n", stringNodeType(node->_nodeType));
				break;
			}
			delete_MeshRep(node->_intern);
			node->_intern = NULL;

		}
		break;
	case 5: //LightRep
		{
			delete_LightRep(node->_intern);
			node->_intern = NULL;
	}
	case 9: //HanimRep
	{
		delete_HanimRep(node->_intern);
		node->_intern = NULL;
	}
	default:
		break;
	}
};
