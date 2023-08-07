/*


X3D Particle Systems Component

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
#include "../vrml_parser/CRoutes.h"
#include "../main/headers.h"

#include "../world_script/fieldSet.h"
#include "../x3d_parser/Bindable.h"
#include "Collision.h"
#include "quaternion.h"
#include "Viewer.h"
#include "../opengl/Frustum.h"
#include "../opengl/Material.h"
#include "../opengl/OpenGL_Utils.h"
#include "../input/EAIHelpers.h"	/* for newASCIIString() */

#include "Polyrep.h"
#include "RenderFuncs.h"
#include "LinearAlgebra.h"
//#include "Component_ParticleSystems.h"
#include "Children.h"
#include "Component_Shape.h"
#include "../opengl/Textures.h"

typedef struct pComponent_ParticleSystems{
	int something;
}* ppComponent_ParticleSystems;
void *Component_ParticleSystems_constructor(){
	void *v = MALLOCV(sizeof(struct pComponent_ParticleSystems));
	memset(v,0,sizeof(struct pComponent_ParticleSystems));
	return v;
}
void Component_ParticleSystems_init(struct tComponent_ParticleSystems *t){
	//public
	//private
	t->prv = Component_ParticleSystems_constructor();
	{
		ppComponent_ParticleSystems p = (ppComponent_ParticleSystems)t->prv;
		p->something = 0;
	}
}
void Component_ParticleSystems_clear(struct tComponent_ParticleSystems *t){
	//public
}

//ppComponent_ParticleSystems p = (ppComponent_ParticleSystems)gglobal()->Component_ParticleSystems.prv;

/*	Particle Systems
	http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/particle_systems.html
	examples:
		Non- x3d:
			https://stemkoski.github.io/Three.js/#particlesystem-shader
		x3d scenes:
	links:
		http://mmaklin.com/uppfra_preprint.pdf
		Nice particle physics
		http://www.nvidia.com/object/doc_characters.html
		Nvidia link page for game programmming with shaders

	Fuzzy Design:
	1. Update position of particles from a particleSystem node
		Eval particles after events (after the do_tick) and befre RBP physics
			see http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/concepts.html#ExecutionModel
		positions are update wrt the local system of the particleSystme node
		each particle has a struct { lifetime remaining, position, velocity vector, ??}
		up to 10,000 particles (per particle node)
		randomizing: use C srand(time) once, and rand() for each randomizing. Scale by Variation field
		CPU design: iterate over particles, updating each one
		GPU design ie openCL: do same thing in massive parallel
	2. Render
		during geom pass of render_hier, in render_particleSystem()
		CPU design: iterate over particles like children:
			updating transform stack with particle position
			updating appearance f(time)
			calling render_node(node) on each particle
		GPU design: send arrray of particle positions/states to GPU
			in shader iterate over positions, re-rendering for each
			
	PROBLEM with trying to do physics in shader: 
	x how do you update the state of each particle in a way the next frame can access?
	vs on cpu, if you have 80 particles, you can whip through them, updating their state, 
	- and resending the state via attribute array on each frame
	- also more flexible when doing geometryType="GOEMETRY" / goemetry node, that code is CPU

	PROBLEM with sending just the particle position, and generating the sprite geometry
	in the shader: GLES2 doesn't have gometry shaders.

	GLES2 has no gl_VertexID per vertex in vertex shader, so:
	- send glAttributeArray of xyz positions to match vertices?
	- send repetitive triangles - same 3 xyz repetitively?
	- in shader add position to triangle verts?
	- or send glAttributeArray of sprite ID of length nvert
	-- and uniform3fv of xyz of length nsprite
	-- then lookup xyz[spriteID] in vertex shader?

	EASIEST CPU/GPU SPLIT:
		1. just send geometry for 1 particle to shader
		2. cpu loop over particles:
			foreach liveparticle
				send position to shader
				send texcoord to shader
				send cpv to shader
				gl_DrawArrays

	POSITION
	because position is just xyz (not orientation or scale) the shader could
		take a vec3 for that, and add it on before transforming from local to view
	for non-GEOMETRY, the transform needs to keep the face normal parallel to the view Z
		H: you could do that by transforming 0,0,0 to view, and adding on gl_vertex 
		x but that wouldn't do scale, or orientation if you need it
		- for scale also transform gl_vertex, get the |diff| from 0 for scale

	PHYSICS - I don't see any rotational momentum needs, which involve cross products
	- so forces F, positions p, velocities v, accelerations a are vec3
	- mass, time are scalars
	- physics:
	  F = m * a
	   a = F/m
	  v2 = v1 + a*dt
	  p2 = p1 + .5(v1 + v2)*dt
	  p2 = p1 + v1*dt + .5*a*dt**2 
	   p - position
	   v - velocity
	   a - acceleration
	   dt - delta time = (time2 - time1)
	   m - mass
	   F - force

	RANDOM DIRECTIONS
	http://math.stackexchange.com/questions/44689/how-to-find-a-random-axis-or-unit-vector-in-3d

	RANDOM TRIANGLE COORDS
	picking a random triangle won't evenly distribute by area, so we can be approx with point inside tri too
	can pick 2 0-1 range numbers, and use as barycentric coords b1,b2:
	https://en.wikipedia.org/wiki/Barycentric_coordinate_system
	p = b1*p1 + b2*p2 + (1 - b1 - b2)*p3

	COMPARISONS - H3D, Octaga, Xj3D claim particle physics
	- Octaga responds to particle size, has good force and wind effects

Its like a Shape node, or is a shape node, 
set geometry
set basic appearance
foreach liveparticle
	update texcoord
	update color (color per vertex)
	update position
	gl_DrawArrays or gl_DrawElements

Options in freewrl:
1. per-particle child_Shape 
	foreach liveparticle
		push particle position onto transform stack
		fiddle with appearance node on child
		call child_Shape(particle)
2. per-particle-system child_Shape
	call child_Shape
		if(geometryType 'GEOMETRY')
		.... sendArraysToGPU (hack)
				foreach liveparticle
					update position
					glDrawArrays
		.... sendElementsToGPU (hack)
				foreach liveparticle
					update position
					glDrawElements
		if(geometryType !GEOMETRY)
			send vbo with one line or quad or 2 triangles or point
			foreach liveparticle
				update position
				update texcoord
				update color (color per vertex)
				gl_DrawArrays or gl_DrawElements
3. refactor child shape to flatten the call hierarchy
	child shape:
	a) determine shader flags
	b) compile/set/bind shader program
	c) set appearance - pull setupShader out of sendArraysToGPU
		set material
		set texture
		set shader
		...
	d) set geometry vertices and type element/array, line/triangle
		if GEOMETRY: render(geom node) - except don't call gl_DrawArrays or gl_DrawElements
		PROBLEM: some geometries -cylinder, cone- are made from multiple calls to gl_DrawElements
		OPTION: refactor so uses polyrep, or single permuatation call
				(cylinder: 4 permutations: full, bottom, top, no-ends)
		else !GEOMETRY: send one quad/line/point/triangle 
	e) foreach liveparticle
		update position
		update texcoord
		update color (CPV)
		gl_Draw xxx: what was set in d) above
4. half-flatten child_shape
	as in #3, except just take setupShader out of sendArraysToGPU/sendElementsToGPU, and put in 
		child_shape body
	then child_particlesystem can be a copy of child_shape, with loop over particles 
		calling render_node(geometry) for GEOMETRY type (resending vbos)
5. make sendArrays etc a callback function, different for particles
CHOICE: #3 
	setup shader //sends materials, matrices to shader
	render_node(geometry) //sends vertex data to shader, saves call parameters to gl_DrawArrays/Elements
	foreach liveparticle
		update particle-specific position, color, texcoords
		reallyDrawOnce() //calls glDrawArrays or Elements
	clearDraw()

*/


float uniformRand(){
	// 0 to 1 inclusive
	static int once = 0;
	unsigned int ix;
	float rx;

	if(!once)
		srand((unsigned int) TickTime());
	once = 1;
	ix = rand();  
	rx = (float)ix/(float)RAND_MAX; //you would do (RAND_MAX - 1) here for excluding 1
	return rx;
}
float uniformRandCentered(){
	return uniformRand() - .5f; //center on 0
}
void circleRand2D(float *xy){
	//random xy on a circle area radius 1
	float radius2;
	for(;;){
		xy[0] = 2.0f*(uniformRand() - .5f);
		xy[1] = 2.0f*(uniformRand() - .5f);
		radius2 = xy[0]*xy[0] + xy[1]*xy[1];
		if(radius2 <= 1.0f) break;
	}
}
float normalRand(){
	// in -.5 to .5 range
	float rxy[2];
	// by just taking points in a circle radius, this emulates the falloff of a normal curve in one dimension
	//     .
	//  . x    .
	// :        :
	//  .      .
	//     .   o
	//
	circleRand2D(rxy);
	return (float)(rxy[0]*.5); //scale from -1 to 1 into -.5 to .5 range
}

void randomTriangleCoord_dug9_uneducated_guess(float *p, float* p1, float *p2, float *p3){
	// get 2 random barycentric coords 0-1, and use those
	// https://en.wikipedia.org/wiki/Barycentric_coordinate_system
	// x I think b1 + b2 can be > 1.0 and thats wrong
	int i;
	float b1, b2;
	b1 = uniformRand();
	b2 = uniformRand();
	for(i=0;i<3;i++){
		p[i] = b1*p1[i] + b2*p2[i] + (1.0f - b1 - b2)*p3[i];
	}
}
void randomTriangleCoord(float *p, float* p1, float *p2, float *p3){
	// http://math.stackexchange.com/questions/18686/uniform-random-point-in-triangle
	int i;
	float r1, r2, sqr1,sqr2;
	r1 = uniformRand();
	r2 = uniformRand();
	sqr1 = sqrtf(r1);
	sqr2 = sqrtf(r2);
	for(i=0;i<3;i++){
		p[i] = (1.0f - sqr1)*p1[i] + (sqr1*(1.0f - sqr2))*p2[i] + (r2*sqr1)*p3[i];
	}

}
void randomPoint3D(float *xyz){
	//- .5 to .5 range
	xyz[0] = (uniformRand() - .5f);
	xyz[1] = (uniformRand() - .5f);
	xyz[2] = (uniformRand() - .5f);
}
void randomDirection(float *xyz){
	//random xyz direction from a point
	//http://math.stackexchange.com/questions/44689/how-to-find-a-random-axis-or-unit-vector-in-3d
	float radius3;
	for(;;){
		//get random point in a unit cube
		//xyz[0] = (uniformRand() - .5f);
		//xyz[1] = (uniformRand() - .5f);
		//xyz[2] = (uniformRand() - .5f);
		randomPoint3D(xyz);
		//discard point if outside unit sphere
		radius3 = xyz[0]*xyz[0] + xyz[1]*xyz[1] + xyz[2]*xyz[2];
		if(radius3 <= 1.0f && radius3 > 0.0000001f){
			//vecnormalize3f(xyz,xyz);
			//normalize direction to point
			vecscale3f(xyz,xyz,1.0f/sqrtf(radius3));
			break;
		}
	}
}

typedef struct {
	//store at end of current iteration, for use on next iteration
	float age;
	float lifespan; //assigned at birth
	float size[2];  //assigned at birth
	float position[3];
	float velocity[3];
	float origin[3]; //zero normally. For boundedphysics, updated on each reflection to be last reflection point.
	float direction[3]; //normalized last non-zero velocity vector
	float speed;
	float mass;
	float surfaceArea;
	int sink; //assigned after birth in MapPhysics, for MapPhysics, MapEmitter
} particle;
enum {
	GEOM_QUAD = 1,
	GEOM_LINE = 2,
	GEOM_POINT = 3,
	GEOM_SPRITE = 4,
	GEOM_TRIANGLE = 5,
	GEOM_GEOMETRY = 6,
	GEOM_HANIM = 7,
};
struct {
const char *name;
int type;
} geomtype_table [] = {
{"QUAD",GEOM_QUAD},
{"LINE",GEOM_LINE},
{"POINT",GEOM_POINT},
{"SPRITE",GEOM_SPRITE},
{"TRIANGLE",GEOM_TRIANGLE},
{"GEOMETRY",GEOM_GEOMETRY},
{"HANIM",GEOM_HANIM},
{NULL,0},
};
int lookup_geomtype(const char *name){
	int iret,i;
	iret=i=0;
	for(;;){
		if(geomtype_table[i].name == NULL) break;
		if(!strcmp(geomtype_table[i].name,name)){
			iret = geomtype_table[i].type;
			break;
		}
		i++;
	}
	return iret;
}
//GLfloat quadtris [18] = {1.0f,1.0f,0.0f, -1.0f,1.0f,0.0f, -1.0f,-1.0f,0.0f,    1.0f,1.0f,0.0f, -1.0f,-1.0f,0.0f, 1.0f,-1.0f,0.0f};
static GLfloat quadtris [18] = {-.5f,-.5f,0.0f, .5f,-.5f,0.0f, .5f,.5f,0.0f,   .5f,.5f,0.0f, -.5f,.5f,0.0f, -.5f,-.5f,0.0f,};
static GLfloat twotrisnorms [18] = {0.f,0.f,1.f, 0.f,0.f,1.f, 0.f,0.f,1.f,    0.f,0.f,1.f, 0.f,0.f,1.f, 0.f,0.f,1.f,};
static GLfloat twotristex [12] = {0.f,0.f, 1.f,0.f, 1.f,1.f,    1.f,1.f, 0.f,1.f, 0.f,0.f};

void compile_Shape (struct X3D_Shape *node);
// COMPILE PARTICLE SYSTEM
void compile_ParticleSystem(struct X3D_ParticleSystem *node){
	int i,j, maxparticles;
	float *vertices; //*boxtris, 
	Stack *_particles;

	//ConsoleMessage("compile_particlesystem\n");
	//delegate to compile_shape - same order to appearance, geometry fields
	compile_Shape((struct X3D_Shape*)node);

	node->_geometryType = lookup_geomtype(node->geometryType->strptr);
	if(node->_tris == NULL){
		node->_tris = MALLOC(void *,18 * sizeof(float));
		//memcpy(node->_tris,quadtris,18*sizeof(float));
	}
	vertices = (float*)(node->_tris);
	//rescale vertices, in case scale changed
	for(i=0;i<6;i++){
		float *vert, *vert0;
		vert0 = &quadtris[i*3];
		vert = &vertices[i*3];
		vert[0] = vert0[0]*node->particleSize.c[0];
		vert[1] = vert0[1]*node->particleSize.c[1];
		vert[2] = vert0[2];
	}

	if(node->texCoordRamp || node->texCoord){
		int ml,mq,mt,n;
		struct X3D_TextureCoordinate* tc;
		if(node->texCoordRamp) 
			tc = (struct X3D_TextureCoordinate*)node->texCoordRamp;
		else
			tc = (struct X3D_TextureCoordinate*)node->texCoord;
		n = node->texCoordKey.n;
		mq = n*4; //quad
		ml = n*2; //2 pt line
		mt = n*6; //2 triangles

		//malloc for both lines and tex, in case changed on the fly
		if(!node->_ttex)
			node->_ttex = MALLOC(void *,mt*2*sizeof(float));
		if(!node->_ltex)
			node->_ltex = MALLOC(void *,ml*2*sizeof(float));
		if(tc->point.n == mq){
			//enough tex coords for quads, expand to suit triangles
			//  4 - 3
			//  5 / 2  2 triangle config
			//  0 _ 1
			float *ttex, *ltex;
			ttex = (float*)node->_ttex;
			for(i=0;i<n;i++){
				int k;
				for(j=0,k=0;j<4;j++,k++){
					float *p = (float*)(float *)&tc->point.p[i*4 + j];
					veccopy2f(&ttex[(i*6 + k)*2],p);
					if(k==0){
						veccopy2f(&ttex[(i*6 + 5)*2],p); //copy to 5 (last of 0-6 2-triangle)
					}
					if(k==2){
						k++;
						veccopy2f(&ttex[(i*6 + k)*2],p); //copy 2 to 3 (start of 2nd triangle
					}
				}
			}
			if(0) for(i=0;i<n;i++){
				for(j=0;j<6;j++)
					printf("%f %f,",ttex[(i*6 + j)*2 +0],ttex[(i*6 + j)*2 +1]);
				printf("\n");
			}
			//for(i=0;i<(n*6*2);i++){
			//	printf("%f \n",ttex[i]);
			//}

			ltex = (float*)node->_ltex;
			for(i=0;i<n;i++){
				// make something up for lines
				for(j=0;j<2;j++){
					float p[2];
					struct SFVec2f *sf = (struct SFVec2f *)&tc->point.p[i*4 + j];
					p[0] = sf->c[0];
					p[1] = min(sf->c[1],.9999f); //clamp texture here otherwise tends to wrap around
					veccopy2f(&ltex[(i*2 + j)*2],p);
					
				}
			}
		}
		if(tc->point.n == ml){
			//enough points for lines
			float *ttex, *ltex;

			ltex = (float*)node->_ltex;
			for(i=0;i<n;i++){
				// copy lines straightforwardly
				for(j=0;j<2;j++){
					float p[2];
					struct SFVec2f *sf = (struct SFVec2f *)&tc->point.p[i*2 + j];
					p[0] = sf->c[0];
					p[1] = min(sf->c[1],.9999f); //clamp texture here otherwise tends to wrap around
					veccopy2f(&ltex[(i*2 + j)*2],p);
				}
			}
			if(0) for(i=0;i<n;i++){
				printf("%f %f, %f %f\n",ltex[i*2*2 + 0],ltex[i*2*2 + 1],ltex[i*2*2 + 2],ltex[i*2*2 + 3]);
			}
			//make something up for triangles
			ttex = (float*)node->_ttex;
			for(i=0;i<n;i++){
				float *p;
				j = i;
				p = (float*)(float *)&tc->point.p[j*2 + 0];
				veccopy2f(&ttex[(i*6 + 0)*2],p); //copy to 0 
				veccopy2f(&ttex[(i*6 + 5)*2],p); //copy to 5
				p = (float*)(float *)&tc->point.p[j*2 + 1];
				veccopy2f(&ttex[(i*6 + 1)*2],p); //copy to 1
				j++;
				j = j == n ? j - 1 : j; //clamp to last
				p = (float*)(float *)&tc->point.p[j*2 + 1];
				veccopy2f(&ttex[(i*6 + 2)*2],p); //copy to 2
				veccopy2f(&ttex[(i*6 + 3)*2],p); //copy to 3
				p = (float*)(float *)&tc->point.p[j*2 + 0];
				veccopy2f(&ttex[(i*6 + 4)*2],p); //copy to 4
			}
		}
	}
	maxparticles = min(node->maxParticles,10000);
	if(node->_particles == NULL)
		node->_particles = newVector(particle,maxparticles);
	_particles = node->_particles;
	if(_particles->allocn < maxparticles) {
		//resize /realloc vector, set nalloc, in case someone changed maxparticles on the fly
		_particles->data = realloc(_particles->data,maxparticles);
		_particles->allocn = maxparticles;
	}
	if(!node->_lasttime || node->enabled && !node->_lastEnabled)
		node->_lasttime = TickTime();
	if(node->enabled && !node->_lastEnabled){
		node->isActive = TRUE;
		MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_ParticleSystem, isActive));
	}else if(!node->enabled && node->_lastEnabled){
		node->isActive = FALSE;
		MARK_EVENT (X3D_NODE(node),offsetof (struct X3D_ParticleSystem, isActive));
	}
	node->_lastEnabled = node->enabled;
	MARK_NODE_COMPILED
}

//PHYSICS
void prep_windphysics(struct X3D_Node *physics){
	//per-frame gustiness update
	struct X3D_WindPhysicsModel *px = (struct X3D_WindPhysicsModel *)physics;
	float speed;
	speed = px->speed * (1.0f + uniformRandCentered()*px->gustiness);
	px->_frameSpeed = speed;
}
void apply_windphysics(particle *pp, struct X3D_Node *physics, float dtime){
	// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/particle_systems.html#WindPhysicsModel
	struct X3D_WindPhysicsModel *px = (struct X3D_WindPhysicsModel *)physics;
	if(px->enabled && pp->mass != 0.0f){
		float pressure;
		float force, speed;
		float turbdir[3], pdir[3],acceleration[3], v2[3];
		speed = px->_frameSpeed;
		pressure = powf(10.0f,2.0f*log10f(speed)) * .64615f;
		force = pressure * pp->surfaceArea;
		randomDirection(turbdir);
		vecscale3f(turbdir,turbdir,px->turbulence);
		vecadd3f(pdir,px->direction.c,turbdir);
		vecnormalize3f(pdir,pdir);
		vecscale3f(pdir,pdir,force);

		vecscale3f(acceleration,pdir,1.0f/pp->mass);
		vecscale3f(v2,acceleration,dtime);
		vecadd3f(pp->velocity,pp->velocity,v2);
		float flen = veclength3f(pp->velocity);
		if (flen > 0.0f) {
			vecscale3f(pp->direction, pp->velocity, 1.0f / flen);
		}

	}
}
int intersect_polyrep(struct X3D_Node *node, float *p1, float *p2, float *nearest, float *normal);

void compile_geometry(struct X3D_Node *gnode){
	//this needs generalizing, for all triangle geometry, and I don't know how
	if(gnode)
	switch(gnode->_nodeType){
		case NODE_IndexedFaceSet:
		{
			struct X3D_IndexedFaceSet *node = (struct X3D_IndexedFaceSet *)gnode;
			//COMPILE_POLY_IF_REQUIRED (node->coord, node->fogCoord, node->color, node->normal, node->texCoord)
			if (!compile_poly_if_required(node, node->coord, node->fogCoord, node->color, node->normal, node->texCoord))return;
		}
		break;
		default:
		break;
	}
	
}
int intersect_geometry(struct X3D_Node *gnode, float *p1, float *p2, float *nearest, float *normal){
	//this needs generalizing for all triangle geometry 
	int iret = 0;
	if(gnode)
	switch(gnode->_nodeType){
		case NODE_IndexedFaceSet:
			iret = intersect_polyrep(gnode,p1,p2,nearest,normal);
		break;
		default:
		break;
	}
	return iret;
}
void apply_boundedphysics(particle *pp, struct X3D_Node *physics, float *positionChange){
	struct X3D_BoundedPhysicsModel *px = (struct X3D_BoundedPhysicsModel *)physics;
	if(px->enabled && px->geometry ) {   //&& pp->mass != 0.0f){
		//shall we assume its a 100% elastic bounce?
		int nintersections;
		// int ntries;
		struct X3D_Node *node = (struct X3D_Node *) px->geometry;
		float pos1[3], pos2[3], pnearest[3],normal[3], delta[3];
		static int count;

		//make_IndexedFaceSet((struct X3D_IndexedFaceSet*)px->geometry);
		//COMPILE_POLY_IF_REQUIRED (node->coord, node->fogCoord, node->color, node->normal, node->texCoord)
		if(NODE_NEEDS_COMPILING)
			compile_geometry(node);

		//if polyrep
		//veccopy3f(pos1,pp->position);

		veccopy3f(pos1,pp->origin);
		//vecadd3f(pos2,pp->position,positionChange);
		vecadd3f(pos2,pp->position,positionChange);
		//ntries = 0;
		count = 0;
		//for(;;) //in theory we may travel far enough for 2 bounces
		{
			//if(pos2[0] >= .5f)
			//	printf("should hit\n");
			nintersections = intersect_geometry(px->geometry,pos1,pos2,pnearest,normal);
			if(nintersections > 0){
				float d[3], r[3], rn[3], rd[3], n[3], ddotnn[3], orthogn[3], orthogn2[3];
				float ddotn, speed, dlengthi,dlength;
				vecdif3f(delta,pos2,pos1);
				dlength = veclength3f(delta);
				count++;

				// get reflection 
				//  pos1
				// o|\d
				// n<--x
				// o|/r
				// ddotn = dot(d,n)*n //projection of d onto n, as vector
				// o = d - ddotn  //vector orthogonal to n, such that d = ddotn + o
				// r = ddotn - o 
				// or 
				// r = ddotn - (d - ddotn)
				// or
				// r = 2*ddotn - d
				// or
				// r = -(d - 2*dot(d,n)*n)
				// http://math.stackexchange.com/questions/13261/how-to-get-a-reflection-vector
			
				vecdif3f(d,pnearest,pos1);
				dlengthi = veclength3f(d);
				vecnormalize3f(n,normal);
				ddotn = vecdot3f(d,n);
				//ddotn = -ddotn; //assuming the surface normal is pointing out
				vecscale3f(ddotnn,n,ddotn);
				vecdif3f(orthogn,d,ddotnn);
				vecscale3f(orthogn2,orthogn,2.0f);
				vecdif3f(r,d,orthogn2);
				vecscale3f(r,r,-1.0f); //reverse direction
				vecnormalize3f(rn,r);
				// update the velocity vector direction (keep speed constant, assuming elastic bounce)
				// specs: could use an elasticity factor
				speed = veclength3f(pp->velocity);
				vecscale3f(pp->velocity,rn,speed);
				float flen = veclength3f(pp->velocity);
				if (flen > 0.0f) {
					vecscale3f(pp->direction, pp->velocity, 1.0f / flen);
				}

				//do positionChange here, and zero positionchange for calling code
				vecscale3f(rd,rn,dlength - dlengthi);
				vecadd3f(pp->position,pnearest,rd);
				vecscale3f(positionChange,positionChange,0.0f);
				veccopy3f(pos1,pnearest);
				veccopy3f(pp->origin,pos1);
				veccopy3f(pos2,pp->position);
				if(0) pp->age = 1000.0f; //simply expire if / when goes outside 
				// specs could add death-on-hitting-wall
			}
			//break;
			//if(nintersections == 0)break;
			//ntries++;
			//if(ntries > 3)break;
		}
	}
}
void apply_forcephysics(particle *pp, struct X3D_Node *physics, float dtime){
	// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/particle_systems.html#ForcePhysicsModel
	// I think Octaga mis-interprets the [0 -9.81 0] force as acceleartion of gravity.
	// it will be if mass==1. Otherwise you need to scale your force by mass:
	// ie if your mass is 10, then force needs to be [0 98.1 0]
	struct X3D_ForcePhysicsModel *px = (struct X3D_ForcePhysicsModel *)physics;
	//a = F/m;
	//v += a*dt
	if(px->enabled && pp->mass != 0.0f){
		float acceleration[3], v2[3];
		vecscale3f(acceleration,px->force.c,1.0f/pp->mass);
		vecscale3f(v2,acceleration,dtime);
		vecadd3f(pp->velocity,pp->velocity,v2);
		float flen = veclength3f(pp->velocity);
		if (flen > 0.0f) {
			vecscale3f(pp->direction, pp->velocity, 1.0f / flen);
		}

	}
}

//EMITTERS
void apply_ConeEmitter(particle *pp, struct X3D_Node *emitter){
	// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/particle_systems.html#ConeEmitter
	// like point emitter, except we need work in the direction:
	// 2 randoms, one for angle-from-direction < e.angle, and one for angle-around-direction 0-2PI
	struct X3D_ConeEmitter *e = (struct X3D_ConeEmitter *)emitter;
	float direction[3], tilt, azimuth, speed;
	{
		//prep - can be done once per direction
		//need 2 orthogonal axes 
		//a) find a minor axis
		float amin;
		float orthog1[3],orthog2[3];
		int i,imin,method;
		vecnormalize3f(direction,e->direction.c);
		amin = min(min(fabsf(direction[0]),fabsf(direction[1])),fabsf(direction[2]));
		imin = 0;
		for(i=0;i<3;i++){
			if(fabsf(direction[i]) == amin){
				imin = i; break;
			}
		}
		//make a vector with the minor axis dominant
		for(i=0;i<3;i++) orthog1[i] = 0.0f;
		orthog1[imin] = 1.0f;
		//orthog1 will only be approximately orthogonal to direction
		//do a cross product to get ortho2
		veccross3f(orthog2,direction,orthog1);
		//orthog2 will be truely orthogonal
		//cross orthog2 with direction to get truely orthog1
		veccross3f(orthog1,direction,orthog2);

		//for this particle
		method = 2;
		if(method == 1){
			//METHOD 1: TILT + AZIMUTH
			//tends to crowd/cluster around central direction, and fade with tilt
			//(due to equal chance of tilt angle, but larger area to cover as tilt goes up)
			//direction = cos(tilt)*direction
			//az = cos(azimuth)*orthog1 + sin(azimuth)*orthog2
			//az = sin(tilt)*az
			//direction += az;
			//where
			// tilt - from e.direction axis
			// azimuth - angle around e.direction vector (in plane orthogonal to direction vector)
			// az[3] - vector in the orthogonal plane, in direction of azimuth
			float az[3],az1[3],az2[3],ctilt,stilt,caz,saz;
			tilt = uniformRand()*e->angle;
			ctilt = cosf(tilt);
			stilt = sinf(tilt);
			azimuth = uniformRand()*2.0f*(float)PI;
			caz = cosf(azimuth);
			saz = sinf(azimuth);
			vecscale3f(az1,orthog1,caz);
			vecscale3f(az2,orthog2,saz);
			vecadd3f(az,az1,az2);
			//now az is a unit vector in orthogonal plane, in direction of azimuth
			vecscale3f(az,az,stilt);
			//now az is scaled for adding to direction
			vecscale3f(direction,direction,ctilt);
			//direction is shrunk (or reversed) to account for tilt
			vecadd3f(direction,direction,az);
			//now direction is unit vector in tilt,az direction from e.direction
		}
		if(method == 2){
			//METHOD 2: POINT IN CIRCLE
			//tends to give even distribution over circle face of cone
			//xy = randomCircle
			//orthog = x*orthog1 + y*orthog2
			//direction += orthog
			float xy[2], orx[3],ory[3], orthog[3];
			circleRand2D(xy);
			//orthog = x*orthog1 + y*orthog2
			vecscale3f(orx,orthog1,xy[0]);
			vecscale3f(ory,orthog2,xy[1]);
			vecadd3f(orthog,orx,ory);
			vecscale3f(orthog,orthog,sinf(e->angle));
			vecscale3f(direction,direction,cosf(e->angle));
			//direction += orthog
			vecadd3f(direction,orthog,direction);
			//normalize(direction)
			vecnormalize3f(direction,direction);
		}

	}
	memcpy(pp->position,e->position.c,3*sizeof(float));
	speed = e->speed*(1.0f + uniformRandCentered()*e->variation);
	vecscale3f(pp->velocity,direction,speed);
	float flen = veclength3f(direction);
	if (flen > 0.0f) {
		vecscale3f(pp->direction, direction, 1.0f / flen);
	}
	pp->mass = e->mass*(1.0f + uniformRandCentered()*e->variation);
	pp->surfaceArea = e->surfaceArea*(1.0f + uniformRandCentered()*e->variation);

}
void apply_ExplosionEmitter(particle *pp, struct X3D_Node *emitter){
	// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/particle_systems.html#ExplosionEmitter
	// like point emitter, except always random direction
	// the create-all-at-time-zero is handled up one level
	struct X3D_ExplosionEmitter *e = (struct X3D_ExplosionEmitter *)emitter;
	float direction[3], speed;
	memcpy(pp->position,e->position.c,3*sizeof(float));
	randomDirection(direction);
	speed = e->speed*(1.0f + uniformRandCentered()*e->variation);
	vecscale3f(pp->velocity,direction,speed);
	float flen = veclength3f(direction);
	if (flen > 0.0f) {
		vecscale3f(pp->direction, direction, 1.0f / flen);
	}
	pp->mass = e->mass*(1.0f + uniformRandCentered()*e->variation);
	pp->surfaceArea = e->surfaceArea*(1.0f + uniformRandCentered()*e->variation);
}
void apply_PointEmitter(particle *pp, struct X3D_Node *emitter){
	// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/particle_systems.html#PointEmitter
	// like explosion, except may have a non-random direction
	struct X3D_PointEmitter *e = (struct X3D_PointEmitter *)emitter;
	float direction[3], speed;
	memcpy(pp->position,e->position.c,3*sizeof(float));
	if(veclength3f(e->direction.c) < .00001){
		randomDirection(direction);
	}else{
		memcpy(direction,e->direction.c,3*sizeof(float));
		vecnormalize3f(direction,direction);
	}
	speed = e->speed*(1.0f + uniformRandCentered()*e->variation);
	vecscale3f(pp->velocity,direction,speed);
	float flen = veclength3f(direction);
	if (flen > 0.0f) {
		vecscale3f(pp->direction, direction, 1.0f / flen);
	}
	pp->mass = e->mass*(1.0f + uniformRandCentered()*e->variation);
	pp->surfaceArea = e->surfaceArea*(1.0f + uniformRandCentered()*e->variation);
	
}
enum {
	POLYLINEEMITTER_METHODA = 1,
	POLYLINEEMITTER_METHODB = 2,
};
void compile_PolylineEmitter(struct X3D_Node *node){
	struct X3D_PolylineEmitter *e = (struct X3D_PolylineEmitter *)node;
	float *segs = NULL;
	//e->_method = POLYLINEEMITTER_METHODA;
	e->_method = POLYLINEEMITTER_METHODB;
	if(e->coord && e->coordIndex.n > 1){
		//convert IndexedLineSet to pairs of coordinates
		int i,k,ind, n,nseg = 0;
		float *pts[2];
		struct X3D_Coordinate *coord = (struct X3D_Coordinate *)e->coord;
		n = e->coordIndex.n;
		segs = MALLOC(void*,2*3*sizeof(float) *n*2 ); //2 vertices per lineseg, and 1 lineseg per coordindex should be more than enough
		k = 0;
		nseg = 0;
		for(i=0;i<e->coordIndex.n;i++){
			ind = e->coordIndex.p[i];
			if( ind == -1) {
				k = 0;
				continue;
			}
			pts[k] = (float*)&coord->point.p[ind];
			k++;
			if(k==2){
				veccopy3f(&segs[(nseg*2 +0)*3],pts[0]);
				veccopy3f(&segs[(nseg*2 +1)*3],pts[1]);
				pts[0] = pts[1];
				nseg++;
				k = 1;
			}
		}
		e->_segs = segs;
		e->_nseg = nseg;
	}
	if(e->_method == POLYLINEEMITTER_METHODB){
		int i;
		float *portions, totaldist, dist, delta[3];
		portions = MALLOC(float *,e->_nseg * sizeof(float));
		e->_portions = portions;
		totaldist = 0.0f;
		for(i=0;i<e->_nseg;i++){
			vecdif3f(delta,&segs[(i*2 + 1)*3],&segs[(i*2 + 0)*3]);
			dist = veclength3f(delta);
			//printf("dist %d %f\n",i,dist);
			portions[i] = dist;
			totaldist += dist;
		}
		for(i=0;i<e->_nseg;i++){
			portions[i] = portions[i]/totaldist;
			//printf("portion %d %f\n",i,portions[i]);
		}
	}
	MARK_NODE_COMPILED
}
void apply_PolylineEmitter(particle *pp, struct X3D_Node *node){
	// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/particle_systems.html#PolylineEmitter
	float direction[3], speed;

	struct X3D_PolylineEmitter *e = (struct X3D_PolylineEmitter *)node;
	//like point emitter, except posiion is drawn randomly along polyline
	//option A: pick segment index at random, then pick distance along segment at random (Octaga?)
	//option B: pick random 0-1, then map that to cumulative distance along polyline
	if(NODE_NEEDS_COMPILING)
		compile_PolylineEmitter(node);
	memset(pp->position,0,3*sizeof(float)); //in case no coords/polyline/segs
	if(e->_method == POLYLINEEMITTER_METHODA && e->_nseg){
		float *segs, delta[3], pos[3];
		// pick a segment at random:
		int iseg = (int) floorf(uniformRand() * (float)e->_nseg);
		//pick a point on the segment
		float fraction = uniformRand();
		segs = (float *)e->_segs;
		vecdif3f(delta,&segs[(iseg*2 + 1)*3],&segs[(iseg*2 + 0)*3]);
		vecscale3f(delta,delta,fraction);
		vecadd3f(pos,&segs[(iseg*2 + 0)*3],delta);
		veccopy3f(pp->position,pos);
	}
	if(e->_method == POLYLINEEMITTER_METHODB && e->_nseg){
		//pick rand 0-1
		int i;
		float cumulative, fraction, *portions, *segs, delta[3], pos[3], segfraction;
		fraction = uniformRand();
		portions = (float*)e->_portions;
		cumulative = 0.0f;
		for(i=0;i<e->_nseg;i++){
			cumulative +=portions[i];
			if(cumulative > fraction){
				segfraction = (cumulative - fraction) / portions[i];
				segs = (float *)e->_segs;
				vecdif3f(delta,&segs[(i*2 + 1)*3],&segs[(i*2 + 0)*3]);
				vecscale3f(delta,delta,segfraction);
				vecadd3f(pos,&segs[(i*2 + 0)*3],delta);
				veccopy3f(pp->position,pos);
				break;
			}
		}
	}

	//the rest is like point emitter:
//not for polyline see above	memcpy(pp->position,e->position.c,3*sizeof(float));
	if(veclength3f(e->direction.c) < .00001){
		randomDirection(direction);
	}else{
		memcpy(direction,e->direction.c,3*sizeof(float));
		vecnormalize3f(direction,direction);
	}
	speed = e->speed*(1.0f + uniformRandCentered()*e->variation);
	vecscale3f(pp->velocity,direction,speed);
	float flen = veclength3f(direction);
	if (flen > 0.0f) {
		vecscale3f(pp->direction, direction, 1.0f / flen);
	}
	pp->mass = e->mass*(1.0f + uniformRandCentered()*e->variation);
	pp->surfaceArea = e->surfaceArea*(1.0f + uniformRandCentered()*e->variation);

}
int getPolyrepTriangleCount(struct X3D_Node *node);
int getPolyrepTriangleByIndex(struct X3D_Node *node, int index, float *v1, float *v2, float *v3);

void apply_SurfaceEmitter(particle *pp, struct X3D_Node *emitter){
	struct X3D_SurfaceEmitter *e = (struct X3D_SurfaceEmitter *)emitter;
	struct X3D_Node *node;

	node = e->surface ? e->surface : e->geometry;
	if(node){
		int index, ntri;
		float fraction;
		float speed;
		float xyz[3], v1[3],v2[3],v3[3],e1[3],e2[3], normal[3], direction[3];

		if(NODE_NEEDS_COMPILING){
			compile_geometry(X3D_NODE(node));
		}


		fraction = uniformRand();
		ntri = getPolyrepTriangleCount(node);
		if(ntri){
			index = (int)floorf(fraction * (float)(ntri-1));
			getPolyrepTriangleByIndex(node,index,v1,v2,v3);
			randomTriangleCoord(xyz,v1,v2,v3);
			vecdif3f(e1,v2,v1);
			vecdif3f(e2,v3,v1);
			veccross3f(normal,e1,e2);
			vecnormalize3f(direction,normal);

		}

		//the rest is like point emitter
		memcpy(pp->position,xyz,3*sizeof(float));
		speed = e->speed*(1.0f + uniformRandCentered()*e->variation);
		vecscale3f(pp->velocity,direction,speed);
		float flen = veclength3f(direction);
		if (flen > 0.0f) {
			vecscale3f(pp->direction, direction, 1.0f / flen);
		}
		pp->mass = e->mass*(1.0f + uniformRandCentered()*e->variation);
		pp->surfaceArea = e->surfaceArea*(1.0f + uniformRandCentered()*e->variation);
	}

}



void apply_VolumeEmitter(particle* pp, struct X3D_Node* emitter) {
	struct X3D_VolumeEmitter* e = (struct X3D_VolumeEmitter*)emitter;
	if (!e->_ifs && e->coord) {
		struct X3D_IndexedFaceSet* ifs;
		ifs = createNewX3DNode0(NODE_IndexedFaceSet);
		ifs->coord = e->coord;
		ifs->coordIndex = e->coordIndex;
		compile_geometry(X3D_NODE(ifs));
		e->_ifs = ifs;
	}
	if (e->_ifs) {
		int nint, i, isInside;
		float xyz[3], plumb[3], nearest[3], normal[3];
		float direction[3], speed;
		struct X3D_IndexedFaceSet* ifs = (struct X3D_IndexedFaceSet*)e->_ifs;

		isInside = FALSE;
		for (i = 0; i < 10; i++) {
			randomPoint3D(xyz);
			//spread random points over box
			xyz[0] *= ifs->EXTENT_MAX_X - ifs->EXTENT_MIN_X;
			xyz[1] *= ifs->EXTENT_MAX_Y - ifs->EXTENT_MIN_Y;
			xyz[2] *= ifs->EXTENT_MAX_Z - ifs->EXTENT_MIN_Z;
			veccopy3f(plumb, xyz);
			plumb[2] = ifs->EXTENT_MIN_Z - 1.0f; //ray end point below box
			nint = intersect_geometry(e->_ifs, xyz, plumb, nearest, normal);
			nint = abs(nint) % 2;
			if (nint == 1) {
				isInside = TRUE;
				break; //if there's an odd number of intersections, its inside, else even outside
			}
		}
		if (!isInside)
			vecscale3f(xyz, xyz, 0.0f); //emit from 0
		//the rest is like point emitter
		memcpy(pp->position, xyz, 3 * sizeof(float));
		if (veclength3f(e->direction.c) < .00001) {
			randomDirection(direction);
		}
		else {
			memcpy(direction, e->direction.c, 3 * sizeof(float));
			vecnormalize3f(direction, direction);
		}
		speed = e->speed * (1.0f + uniformRandCentered() * e->variation);
		vecscale3f(pp->velocity, direction, speed);
		float flen = veclength3f(direction);
		if (flen > 0.0f) {
			vecscale3f(pp->direction, direction, 1.0f / flen);
		}
		pp->mass = e->mass * (1.0f + uniformRandCentered() * e->variation);
		pp->surfaceArea = e->surfaceArea * (1.0f + uniformRandCentered() * e->variation);
	}
}


// BEGIN HUMANOID PARTICLE SECTION >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
unsigned char* sample_image(textureTableIndexStruct_s* tt, float x, float y) {
	//x, y in range 0.0 to 1.0
	int px, py, ix, iy;
	px = tt->x;
	py = tt->y;
	ix = (int)( px * x );
	iy = (int)(py * y);
	unsigned char* pixel = &tt->texdata[(iy * px + ix) * 4]; // tt->channels];
	return pixel;
}
void set_image_pixel_color(unsigned char * image, int cols, int rows, unsigned char * pixel, int x, int y) {
	memcpy(&image[(y * cols + x) * 4], pixel, 3);
}
unsigned char* get_image_pixel_color(unsigned char* image, int cols, int rows, int x, int y) {
	return &image[(y * cols + x) * 4];
}
void set_image_pixel_transparency(unsigned char* image, int cols, int rows, unsigned char transparency, int x, int y) {
	image[(y * cols + x) * 4 + 3] = transparency;
}
unsigned char get_image_pixel_transparency(unsigned char* image, int cols, int rows, int x, int y) {
	return image[(y * cols + x) * 4 + 3];
}
unsigned char get_image_pixel_channel(unsigned char* image, int cols, int rows, int channel, int x, int y) {
	return image[(y * cols + x) * 4 + channel];
}
void set_image_pixel_channel(unsigned char* image, int cols, int rows, unsigned char c, int channel, int x, int y) {
	image[(y * cols + x) * 4 + channel] = c;
}
void print_image_channel(unsigned char* imageRGBA, int channel, int width, int height) {
	for (int k = 0; k < width; k += 10) printf("%d         ", k / 10);
	printf("\n");
	for (int j = 0; j < height; j++) {
		for (int k = 0; k < width; k++) {
			unsigned char c = get_image_pixel_channel(imageRGBA, width, height, channel, k, j);
			printf("%c", c + 'A');
		}
		printf(" %2d\n", j);
	}
}


float* extent4f_clear(float* e) {
	e[0] = 10000.0f;
	e[1] = 10000.0f;
	e[2] = -10000.0f;
	e[3] = -10000.0f;
	return e;
}
int extent4f_isSet(float* e4) {
	//extents are set with min > max, so a way to tell
	// if they are set is to check if min <= max or max >= min
	int iret;
	float* e = e4;
	iret = (e[2] >= e[0] && e[3] >= e[1]) ? TRUE : FALSE;
	return iret;
}
float * extent4f_union_extent4f(float *e4, float *ein4){
	int i, isa, isb;
	isa = extent4f_isSet(e4);
	isb = extent4f_isSet(ein4);
	if (isa && isb)
	for (i = 0; i < 2; i++) {
		e4[i]   = min(e4[i], ein4[i]); //the miniumum of the minimums
		e4[i+2] = max(e4[i+2], ein4[i+2]); //the maximum of the maximums
	}
	else if (isb) veccopy4f(e4, ein4);
	return e4;
}
float* extent4f_union_vec2f(float* extent4, float* p2) {
	int i, isa, isb;
	isa = extent4f_isSet(extent4);
	if (!isa)
		for (i = 0; i < 2; i++) {
			extent4[i]   = p2[i];
			extent4[i+2] = p2[i];
		}
	for (i = 0; i < 2; i++) {
		extent4[i]   = min(extent4[i], p2[i]);
		extent4[i+2] = max(extent4[i+2], p2[i]);
	}
	return extent4;
}
void extent4f_printf(float* extent4) {
	printf("min %f %f max %f %f \n", extent4[0], extent4[1], extent4[2], extent4[3]);
}
float* pixel2color3(float * color, unsigned char* pixel) {
	for (int i = 0; i < 3; i++)
		color[i] = ((float)(int)pixel[i]) / 255.0f;
	return color;
}
void apply_MapEmitter(particle* pp, struct X3D_Node* emitter) {
	struct X3D_MapEmitter* e = (struct X3D_MapEmitter*)emitter;
	vecset3f(pp->position, 0.0f, 0.0f, 0.0f);
	pp->speed = e->speed;
	pp->sink = -1; //we won't assign a sink until physics, because that's when we count the sinks
	if (e->functionMap) {
		//printf("functionMap type %s\n", stringNodeType(e->functionMap->_nodeType));
		render_node(e->functionMap);
		textureTableIndexStruct_s* tt = getTableTableFromTextureNode(e->functionMap);
		if (tt && tt->status >= TEX_READ) {
			if (e->emitterColor.n)
			{
				if (!e->classified) {
					//make a 2D box around each emitter color area, so we don't have to 
					// search the whole image pixel by pixel on each frame
					printf("start classifying emitter..\n");
					e->eboxes.p = malloc(e->emitterColor.n * sizeof(struct SFVec4f));
					e->eboxes.n = e->emitterColor.n;
					e->iboxes.p = malloc(e->emitterColor.n * sizeof(struct SFVec4f));
					e->iboxes.n = e->emitterColor.n;
					for (int i = 0; i < e->eboxes.n; i++) {
						extent4f_clear(e->eboxes.p[i].c);
						extent4f_clear(e->iboxes.p[i].c);
					}
					//for now assume one human-step-sized grid cell is 1m
					int isteps[2];
					isteps[0] = (int)(e->gridSize.c[0] + .5f);
					isteps[1] = (int)(e->gridSize.c[1] + .5f);
					for(int i=0; i< isteps[0];i++)
						for (int j = 0; j < isteps[1]; j++) {
							float x, y, s[3], exy[2], ixy[2];
							//image sampling coords
							x = (float)i / (float)e->gridSize.c[0];
							y = (float)j / (float)e->gridSize.c[1];
							ixy[0] = x;
							ixy[1] = y;
							//scene grid coords in meters
							exy[0] = (float)i - e->gridSize.c[0]/2.0f;
							exy[1] = (float)j - e->gridSize.c[1]/2.0f;
							unsigned char* pixel = sample_image(tt, x, y);
							pixel2color3(s, pixel);

							for (int k = 0; k < e->emitterColor.n; k++) {
								float* c = e->emitterColor.p[k].c;
								int is_close = vecclose3f(s, c, e->colorMatchTolerance);
								if (is_close) {
									extent4f_union_vec2f(e->iboxes.p[k].c, ixy);
									extent4f_union_vec2f(e->eboxes.p[k].c, exy);
								}
								//printf("k %d c %f %f %f s %f %f %f close %d\n", k, c[0], c[1], c[2], s[0], s[1], s[2], is_close);
							}

						}
					for (int i = 0; i < e->eboxes.n; i++) {
						extent4f_printf(e->eboxes.p[i].c);
						extent4f_printf(e->iboxes.p[i].c);
					}
					printf("..end classfying emitter\n");
					e->classified = TRUE;
				}
				// emit one from one randomly chosen emitter color area 
				//  but only from the emitter areas found in the function_map image
				int valid_regions = 0;
				for (int i = 0; i < e->iboxes.n; i++)
					if (extent4f_isSet(e->eboxes.p[i].c)) valid_regions++;
				int iregion = (int)(uniformRand() * (float)(valid_regions));
				int nvalid = -1;
				int ivalid = 0;
				for (int i = 0; i < e->iboxes.n; i++) 
				{
					int is_set = extent4f_isSet(e->iboxes.p[i].c);
					if (is_set) nvalid++;
					if (is_set && nvalid == iregion) {
						int i = iregion;
						float x, y, xyz[3], s[3];
						float exy[2], ixy[2];
						int more = TRUE;
						do {
							x = uniformRand();
							y = uniformRand();
							//scale to image box
							ixy[0] = x * (e->iboxes.p[i].c[2] - e->iboxes.p[i].c[0]) + e->iboxes.p[i].c[0];
							ixy[1] = y * (e->iboxes.p[i].c[3] - e->iboxes.p[i].c[1]) + e->iboxes.p[i].c[1];
							//scale to scene box
							exy[0] = x * (e->eboxes.p[i].c[2] - e->eboxes.p[i].c[0]) + e->eboxes.p[i].c[0];
							exy[1] = y * (e->eboxes.p[i].c[3] - e->eboxes.p[i].c[1]) + e->eboxes.p[i].c[1];

							unsigned char* pixel = sample_image(tt, ixy[0], ixy[1]);
							pixel2color3(s, pixel);
							if (vecclose3f(s, e->emitterColor.p[i].c, e->colorMatchTolerance))
								more = FALSE;
						} while (more);
						vecset3f(xyz, exy[0], exy[1], 0.0f);
						//the rest is like point emitter
						printf("iregion %d xy %f %f valid_regions %d", iregion, exy[0], exy[1], valid_regions);
						veccopy3f(pp->position, xyz);
						break;
					}
				}
			}
		}
	}

}

int emitter_loaded(struct X3D_Node* emitter) {
	int loaded = FALSE;
	switch (emitter->_nodeType) {
	case NODE_MapEmitter:
	{
		struct X3D_MapEmitter* e = (struct X3D_MapEmitter*)emitter;
		if (e->functionMap) {
			textureTableIndexStruct_s* tt = getTableTableFromTextureNode(e->functionMap);
			tt->no_gl = TRUE; //don't load in GL, and preserve texdata for processing
			render_node(e->functionMap);
			if (tt && tt->status >= TEX_READ) loaded = TRUE;
			//printf("functionMap type %s loaded %d \n", stringNodeType(e->functionMap->_nodeType), loaded);
		}
	}
	break;
	default:
		loaded = TRUE;
		break;
	}
	return loaded;
}
void norm2image(int *ixy, int *isize, float *fxy) {
	//convert from 0-1 floats to image pixel coords
	ixy[0] = (int)(fxy[0] * (float)isize[0] + .5f);
	ixy[1] = (int)(fxy[1] * (float)isize[1] + .5f);
	ixy[0] = max(min(isize[0] - 1, ixy[0]),0);
	ixy[1] = max(min(isize[1] - 1, ixy[1]),0);
}
void image2norm(float* fxy, int* ixy, int* isize) {
	//convert from image pixel coords to 0-1 floats
	fxy[0] = (float)ixy[0] / (float)isize[0];
	fxy[1] = (float)ixy[1] / (float)isize[1];
}


void apply_mapphysics(particle* pp, struct X3D_Node* physics, float dtime) {
	struct X3D_MapPhysicsModel* px = (struct X3D_MapPhysicsModel*)physics;
	//a = F/m;
	//v += a*dt
	if (px->enabled ) {
		if (!px->classified) {
			if (px->functionMap) {
				//printf("functionMap type %s\n", stringNodeType(e->functionMap->_nodeType));
				textureTableIndexStruct_s* tt = getTableTableFromTextureNode(px->functionMap);
				tt->no_gl = TRUE;
				render_node(px->functionMap);
				
				if (tt && tt->status >= TEX_READ) {
				
					if (px->sinkColor.n)
					{
						printf("start classifying physics..\n");
						//make a 2D box around each sink color area, so we don't have to 
						// search the whole image pixel by pixel on each frame
						px->eboxes.p = malloc(px->sinkColor.n * sizeof(struct SFVec4f));
						px->eboxes.n = px->sinkColor.n;
						px->iboxes.p = malloc(px->sinkColor.n * sizeof(struct SFVec4f));
						px->iboxes.n = px->sinkColor.n;
						for (int i = 0; i < px->eboxes.n; i++) {
							extent4f_clear(px->eboxes.p[i].c);
							extent4f_clear(px->iboxes.p[i].c);
						}
						//for now assume one human-step-sized grid cell is 1m
						int isteps[2];
						isteps[0] = (int)(px->gridSize.c[0] + .5f);
						isteps[1] = (int)(px->gridSize.c[1] + .5f);
						for (int i = 0; i < isteps[0]; i++)
							for (int j = 0; j < isteps[1]; j++) {
								float x, y, s[3], exy[2], ixy[2];
								//image sampling coords
								x = (float)i / (float)px->gridSize.c[0];
								y = (float)j / (float)px->gridSize.c[1];
								ixy[0] = x;
								ixy[1] = y;
								//scene grid coords in meters
								exy[0] = (float)i - px->gridSize.c[0] / 2.0f;
								exy[1] = (float)j - px->gridSize.c[1] / 2.0f;
								unsigned char* pixel = sample_image(tt, x, y);
								pixel2color3(s, pixel);

								for (int k = 0; k < px->sinkColor.n; k++) {
									float* c = px->sinkColor.p[k].c;
									int is_close = vecclose3f(s, c, px->colorMatchTolerance);
									if (is_close) {
										extent4f_union_vec2f(px->iboxes.p[k].c, ixy);
										extent4f_union_vec2f(px->eboxes.p[k].c, exy);
									}
									//printf("k %d c %f %f %f s %f %f %f close %d\n", k, c[0], c[1], c[2], s[0], s[1], s[2], is_close);
								}

							}
						printf("sink boxes\n");
						for (int i = 0; i < px->eboxes.n; i++) {
							extent4f_printf(px->eboxes.p[i].c);
							extent4f_printf(px->iboxes.p[i].c);
						}
						// generate one sink map for each sink color area
						int sinkmapsize = isteps[0] * isteps[1] * 4;
						px->_sinkmaps = malloc(sinkmapsize * (px->eboxes.n + 1)); //one for each sink, plus a population map
						unsigned char* sinkmaps = (unsigned char*)px->_sinkmaps;
						unsigned char* popmap = &sinkmaps[0]; 
						memset(popmap, 0, sinkmapsize);

						for (int i = 0; i < px->iboxes.n; i++)
						{
							
							int is_set = extent4f_isSet(px->iboxes.p[i].c);
							if (is_set) {
							
								//generate_sink_map()
								unsigned char* texdata = &sinkmaps[(i+1) * sinkmapsize]; // malloc(isteps[0] * isteps[1] * 4);
								memset(texdata, 0, isteps[0] * isteps[1] * 4);
								//start sink map at center of ibox
								int icenter[2];
								float fcenter[2];
								float* bbox = &px->iboxes.p[i].c[0];
								fcenter[0] = ((bbox[0] + bbox[2]) / 2.0f);
								fcenter[1] = ((bbox[1] + bbox[3]) / 2.0f);
								norm2image(icenter, isteps, fcenter);
								unsigned char pixel[3];
								pixel[0] = 1; //1/255 is almost black, and we increase toward 255/255 white as the flooding progresses
								pixel[1] = 1;
								pixel[2] = 1;
								set_image_pixel_color(texdata, isteps[0], isteps[1], pixel, icenter[0], icenter[1]);
								//ideally a queue is used for breadth-first flood-filling
								//2023 freewrl doesn't have a queue data structure
								//will use 2 vectors, and alternate: current round, next round
								//and use transparency to mark pixel 0=not done 1/255=queued 2/255=processed
								struct ixy { int x, y; };
								struct Vector* current = newVector(struct ixy, 100);
								struct Vector* next = newVector(struct ixy, 100);
								struct Vector* tmp;
								struct ixy p, q, nebor[8];
								unsigned char done, steps, * funcp;
								float color[3];
								//neighboring pixel relative coordinates, we'll do 8 surrounding pixels.
								for (int i = 0; i < 8; i++) nebor[i].x = nebor[i].y = 0;
								nebor[0].y = nebor[1].y = nebor[2].y = -1;
								nebor[0].x = nebor[3].x = nebor[5].x = -1;
								nebor[2].x = nebor[4].x = nebor[7].x = 1;
								nebor[5].y = nebor[6].y = nebor[7].y = 1;
								p.x = icenter[0];
								p.y = icenter[1];
								set_image_pixel_transparency(texdata, isteps[0], isteps[1], 1, p.x, p.y);
								stack_push(struct ixy, current, p);
								int more = TRUE;
								steps = 0;
								
								while (more) {
									steps++;
									pixel[0] = pixel[1] = pixel[2] = steps;
									for (int i = 0; i < vectorSize(current); i++) {
										p = vector_get(struct ixy, current, i);
										done = get_image_pixel_transparency(texdata, isteps[0], isteps[1], p.x, p.y);
										if (done < 2) {
											//mark as done and set the steps distance to sink
											set_image_pixel_transparency(texdata, isteps[0], isteps[1], 2, p.x, p.y);
											set_image_pixel_color(texdata, isteps[0], isteps[1], pixel, p.x, p.y);
											//queue any un-done neighbors for next loop
											for (int j = 0; j < 8; j++) {
												q.x = p.x + nebor[j].x;
												q.y = p.y + nebor[j].y;
												//skip if outside image
												if (q.x < 0 || q.x >= isteps[0] || q.y < 0 || q.y >= isteps[1]) continue;

												//skip if obstacle in functionMap
												float xx, yy;
												xx = (float)q.x / px->gridSize.c[0];
												yy = (float)q.y / px->gridSize.c[1];
												funcp = sample_image(tt, xx,yy);
												pixel2color3(color, funcp);
												int is_close = vecclose3f(color, px->obstacleColor.c, px->colorMatchTolerance);
												if (is_close) continue;

												//skip if its already queued in next
												done = get_image_pixel_transparency(texdata, isteps[0], isteps[1], q.x, q.y);
												if (done > 0) continue;

												//queue it and flag it as queued 1
												stack_push(struct ixy, next, q);
												set_image_pixel_transparency(texdata, isteps[0], isteps[1], 1, q.x, q.y);

											}
										}
									} //more in current queue to flood fill
									//recycle current vector, and swap current and next vectors
									vector_clear(current);
									tmp = current;
									current = next;
									next = tmp;
									more = vectorSize(current);
								} //more to flood fill
								if (0) {
									//print flood map to screen as characters, with A==0
									printf("flood map %d x steps %d y steps %d\n", i, isteps[0], isteps[1]);
									print_image_channel(texdata, 0, isteps[0], isteps[1]);
									//for (int j = 0; j < isteps[1]; j++) {
									//	for (int k = 0; k < isteps[0]; k++) {
									//		unsigned char c = get_image_pixel_channel(texdata, isteps[0], isteps[1], 0, k, j);
									//		printf("%c", c + 'A');
									//	}
									//	printf("\n");
									//}
								}
								
							} //if box is_set
							
						} //for each box
						px->classified = TRUE;
						printf("..end classifying physics\n");
					} //if sinkcolors
					
				} //if tt
				
			} //functionmap
		} //classified
		else {
			//classified, use sink maps
			// coordinate systems:
			// a) scene units, the classification bboxes
			// b) functionMap image pixels, computed from tt->x, tt->y given 0-1 image fraction coords
			// c) sinkmap grid pixels isteps, ibboxes
			// we've been assuming the gridSize is in m and we have one grid cell per meter
			//  and scene grid centered on 0,0
			// and we've been assuming the functionMap image covers the same area as the gridSize (but different resolution)

			int isteps[2];
			isteps[0] = (int)(px->gridSize.c[0] + .5f);
			isteps[1] = (int)(px->gridSize.c[1] + .5f);
			int sinkmapsize = isteps[0] * isteps[1] * 4;
			unsigned char* sinkmaps = (unsigned char*)px->_sinkmaps; //have all sink maps + pop map packed in one malloc

			//first sink map is the population map showing where particles are, for particle collision avoidance
			unsigned char* popmap = &sinkmaps[0]; 
			//mapemitter assigns a destination (sink) at random to particle
			unsigned char* sinkmap; 
			unsigned char* sinkcolor, * funccolor, sinkuchar;
			float color[3];
			struct ixy { int x, y; };
			struct ixy p, q, nebor[8];
			int debug = FALSE;
			// we use the imageTexture functionmap below for checking for pauseZone
			textureTableIndexStruct_s* tt = getTableTableFromTextureNode(px->functionMap);

			//sinkmap first assigned here, now that we have the sink count
			if (pp->sink == -1) {
				pp->sink = (int)(uniformRand() * (float)px->sinkColor.n);
				printf("pp.sink = %d\n", pp->sink);
			}
			sinkmap	= &sinkmaps[sinkmapsize * (pp->sink + 1)];
			//pp.position is in scene/ground coords centered on 0,0
			// we need grid coords of same size, but shifted wrt 0,0
			p.x = (int)(pp->position[0] + px->gridSize.c[0]*.5f + .5f);
			p.y = (int)(pp->position[1] + px->gridSize.c[1]*.5f + .5f);
			if(debug) printf("pp.position %f %f p %d %d\n", pp->position[0], pp->position[1], p.x, p.y);
			if (p.x < 0 || p.x >= isteps[0] || p.y < 0 || p.y >= isteps[1]) {
				//vecset3f(pp->position, 0.0f, 0.0f, 0.0f);
				//vecset3f(pp->velocity, 0.0f, 0.0f, 0.0f);
				return;
			}

			for (int i = 0; i < 8; i++) nebor[i].x = nebor[i].y = 0;
			nebor[0].y = nebor[1].y = nebor[2].y = -1;
			nebor[0].x = nebor[3].x = nebor[5].x = -1;
			nebor[2].x = nebor[4].x = nebor[7].x = 1;
			nebor[5].y = nebor[6].y = nebor[7].y = 1;

			//which way to go? 
			//check if we are on the sink/destination, if so recycle.
			sinkcolor = get_image_pixel_color(sinkmap, isteps[0], isteps[1], p.x, p.y);
			if (sinkcolor[0] == 1) {
				//end of life, recycle - clear from population map
				set_image_pixel_channel(popmap, isteps[0], isteps[1],0, 0, p.x, p.y);
				pp->age = pp->lifespan;
			}
			else {
				//check if we are on a wait area, will affect neighbor decision
				float xx, yy;
				xx = (float)p.x / px->gridSize.c[0];
				yy = (float)p.y / px->gridSize.c[1];
				funccolor = sample_image(tt, xx,yy);
				pixel2color3(color, funccolor);
				int on_wait = vecclose3f(color, px->pauseColor.c, px->colorMatchTolerance);

				//check neighbors and rank by shortest distance
				int nlist, ilist[8], dlist[8], iscore[8];
				int ishortest = -1;
				int dshortest = 1000000;
				//printf("sink map %d x steps %d y steps %d\n", pp->sink, isteps[0], isteps[1]);
				//print_image_channel(sinkmap, 0, isteps[0], isteps[1]);
				if(debug) print_image_channel(popmap, 0, isteps[0], isteps[1]);

				for (int i = 0; i < 8; i++) {
					dlist[i] = 2000000;
					iscore[i] = 0;
					q.x = p.x + nebor[i].x;
					q.y = p.y + nebor[i].y;
					//skip if outside image
					if (q.x < 0 || q.x >= isteps[0] || q.y < 0 || q.y >= isteps[1]) continue;
					iscore[i] = 1;
					
					//sinkcolor = get_image_pixel_color(sinkmap, isteps[0], isteps[1], q.x, q.y);
					//printf("nebor %d q %d %d sinkcolor %d %d %d\n", i, q.x, q.y, sinkcolor[0], sinkcolor[1], sinkcolor[2]);
					sinkuchar = get_image_pixel_channel(sinkmap, isteps[0], isteps[1], 0, q.x, q.y);
					//printf("nebor %d q %d %d sinkred %d \n", i, q.x, q.y, sinkuchar);
					//skip if obstacle
					//if (sinkcolor[0] == 0) continue;
					if (sinkuchar == 0) continue;
					iscore[i] = 2;
					//skip if we aren't on waitzone, and next is waitzone and wait function is on
					if (!on_wait) {
						xx = (float)q.x / px->gridSize.c[0];
						yy = (float)q.y / px->gridSize.c[1];
						funccolor = sample_image(tt, xx, yy);
						pixel2color3(color, funccolor);
						if (px->pauseState) {
							int is_wait = vecclose3f(color, px->pauseColor.c, px->colorMatchTolerance);
							if (is_wait) continue; //skip if its an active wait area and we aren't already on it
						}
					}
					iscore[i] = 3;
					//skip if someone already populating grid cell (avoid particle collision)
					unsigned char populated = get_image_pixel_channel(popmap, isteps[0], isteps[1], 0, q.x, q.y);
					//printf("nebor %d populated %d\n", i, populated);
					if (populated) continue;
					iscore[i] = 4;
					dlist[i] = sinkuchar;
					if (dlist[i] < dshortest) {
						ishortest = i;
						dshortest = dlist[i];
						iscore[i] = 5;
					}
				}
				if (debug) {
					for (int m = 0; m < 8; m++) printf("iscore[%d]=%d,", m, iscore[m]);
					printf("\n");
				}
				if (ishortest > -1) {
					//move toward ishortest neighbor
					q.x = p.x + nebor[ishortest].x;
					q.y = p.y + nebor[ishortest].y;
					float pxy[3], qxy[3], diff[3], dir[3];
					pxy[0] = (float)p.x;
					pxy[1] = (float)p.y;
					pxy[2] = 0.0f;
					qxy[0] = (float)q.x;
					qxy[1] = (float)q.y;
					qxy[2] = 0.0f;
					vecdif3f(diff, qxy, pxy);
					vecnormalize3f(dir, diff);

					vecscale3f(pp->velocity, dir, pp->speed);
					float flen = veclength3f(dir);
					if (flen > 0.0f) {
						vecscale3f(pp->direction, dir, 1.0f / flen);
					}

					//clear last location
					set_image_pixel_channel(popmap, isteps[0], isteps[1], 0,0, p.x, p.y);
					//mark new location
					if(0) set_image_pixel_channel(popmap, isteps[0], isteps[1], 1,0, q.x, q.y);
					if(debug) printf("shortest %d velocity %f %f particle %p\n", ishortest, pp->velocity[0], pp->velocity[1], pp);

				}
				else {
					//wait / stand
					if(debug) printf("waiting particle %p\n", pp);
					vecset3f(pp->velocity, 0.0f, 0.0f, 0.0f);
				}
				if(debug) getchar();
			} //end of life
		} //classified
	} //enabled
}
// END HUMANOID PARTICLE SECTION <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


void updateColorRamp(struct X3D_ParticleSystem *node, particle *pp, GLint cramp){
	int j,k,ifloor, iceil, found;
	float rgbaf[4], rgbac[4], rgba[4], fraclife;
	found = FALSE;
	fraclife = pp->age / pp->lifespan;
	for(j=0;j<node->colorKey.n;j++){
		if(node->colorKey.p[j] <= fraclife && node->colorKey.p[j+1] > fraclife){
			ifloor = j;
			iceil = j+1;
			found = TRUE;
			break;
		}
	}
	if(found){
		float spread, fraction;
		struct SFColorRGBA * crgba = NULL;
		struct SFColor *crgb = NULL;
		struct X3D_Node* color_ramp;
		if (node->colorRamp) color_ramp = node->colorRamp;
		else if (node->color) color_ramp = node->color;
		switch(color_ramp->_nodeType){
			case NODE_ColorRGBA: crgba = ((struct X3D_ColorRGBA *)color_ramp)->color.p; break;
			case NODE_Color: crgb = ((struct X3D_Color *)color_ramp)->color.p; break;
			default:
			break;
		}
		spread = node->colorKey.p[iceil] - node->colorKey.p[ifloor];
		fraction = (fraclife - node->colorKey.p[ifloor]) / spread;
		if(crgba){
			memcpy(rgbaf,&crgba[ifloor],sizeof(struct SFColorRGBA));
			memcpy(rgbac,&crgba[iceil],sizeof(struct SFColorRGBA));
		}else if(crgb){
			memcpy(rgbaf,&crgb[ifloor],sizeof(struct SFColor));
			rgbaf[3] = 1.0f;
			memcpy(rgbac,&crgb[iceil],sizeof(struct SFColor));
			rgbac[3] = 1.0f;
		}
		for(k=0;k<4;k++){
			rgba[k] = (1.0f - fraction)*rgbaf[k] + fraction*rgbac[k];
		}
		glUniform4fv(cramp,1,rgba);
	}else{
		//re-use last color
	}
}
void updateTexCoordRamp(struct X3D_ParticleSystem *node, particle *pp, float *texcoord){
	int found, ifloor,j;
	float fraclife, fracKey;

	ifloor = 0; 
	fraclife = pp->age / pp->lifespan;
	fracKey = 1.0f / (float)(node->texCoordKey.n); 
	//if(node->_geometryType != GEOM_LINE)
	fraclife -= fracKey; //for 3 keys, fracKey will be .333
	//    v   0000 v 1111111 v 222    change points
	//        0        .5        1  key
	for(j=0;j<node->texCoordKey.n;j++){
		if( node->texCoordKey.p[j] > fraclife){
			ifloor = j;
			found = TRUE;
			break;
		}
	}
	if(found){
		switch(node->_geometryType){
			case GEOM_LINE:
				FW_GL_TEXCOORD_POINTER (2,GL_FLOAT,0,(float *)&texcoord[ifloor*2*2],0);
			break;
			case GEOM_QUAD:
			case GEOM_TRIANGLE:
				//we use triangles for both quad and triangle, 6 vertices per age
				FW_GL_TEXCOORD_POINTER (2,GL_FLOAT,0,(float *)&texcoord[ifloor*2*6],0);
			break;
			default:
			break;
		}
	}
}

void reallyDrawOnce();
void clearDraw();
GLfloat linepts [6] = {-.5f,0.f,0.f, .5f,0.f,0.f};
ushort lineindices[2] = {0,1};
int getImageChannelCountFromTTI(struct X3D_Node *appearanceNode );
void update_effect_uniforms();
void check_compile(struct X3D_Node* node){
	COMPILE_IF_REQUIRED
}
void child_ParticleSystem(struct X3D_ParticleSystem *node){
	// 
	// ParticleSystem 
	// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/particle_systems.html#ParticleSystem
	// In here we are doing basically what child_Shape does to draw a single geom,
	// except once we send the geometry vertex buffer to the shader,
	// we go into a loop to draw all the particles, sending an xyz position update to the shader
	// for each particle (and color update if colorRamp, or texCoordUpdate if texCoordRamp, and
	//  velocity direction if LINE)
	//
	//s_shader_capabilities_t *caps;
	// static int once = 0;
   	ttglobal tg = gglobal();

	COMPILE_IF_REQUIRED
	//check_compile(node);

	/* initialization. This will get overwritten if there is a texture in an Appearance
	   node in this shape (see child_Appearance) */
	tg->RenderFuncs.last_texture_type = NOTEXTURE;
	tg->RenderFuncs.shapenode = node;

	if (renderstate()->render_depth) {
		if (node->castShadow) {
			PRINT_GL_ERROR_IF_ANY("child_shape depth start");
			s_shader_capabilities_t* scap;
			shaderflagsstruct shader_requirements;
			memset(&shader_requirements, 0, sizeof(shaderflagsstruct));
			shader_requirements.base = node->_shaderflags_base; //_shaderTableEntry;  
			shader_requirements.effects = node->_shaderflags_effects;
			shader_requirements.usershaders = node->_shaderflags_usershaders;

			shader_requirements.depth = TRUE;
			shader_requirements.base |= PARTICLE_SHADER;

			scap = getMyShaders(shader_requirements);
			enableGlobalShader(scap);
			sendMatriciesToShader(scap);  //send matrices
			switch (node->_geometryType) {
			case GEOM_LINE:
			{
				FW_GL_VERTEX_POINTER(3, GL_FLOAT, 0, (float*)linepts);
				sendElementsToGPU(GL_LINES, 2, (ushort*)lineindices);
			}
			break;
			case GEOM_POINT:
			{
				float point[3];
				memset(point, 0, 3 * sizeof(float));
				FW_GL_VERTEX_POINTER(3, GL_FLOAT, 0, (GLfloat*)point);
				sendArraysToGPU(GL_POINTS, 0, 1);
			}
			break;
			case GEOM_QUAD:
			{
				//textureCoord_send(&mtf);
				FW_GL_VERTEX_POINTER(3, GL_FLOAT, 0, (GLfloat*)node->_tris);
				FW_GL_NORMAL_POINTER(GL_FLOAT, 0, twotrisnorms);
				sendArraysToGPU(GL_TRIANGLES, 0, 6);
			}
			break;
			case GEOM_SPRITE:
			{
				//textureCoord_send(&mtf);
				FW_GL_VERTEX_POINTER(3, GL_FLOAT, 0, (GLfloat*)node->_tris);
				FW_GL_NORMAL_POINTER(GL_FLOAT, 0, twotrisnorms);
				sendArraysToGPU(GL_TRIANGLES, 0, 6);
			}
			break;
			case GEOM_TRIANGLE:
			{
				//textureCoord_send(&mtf);
				FW_GL_VERTEX_POINTER(3, GL_FLOAT, 0, (GLfloat*)node->_tris);
				FW_GL_NORMAL_POINTER(GL_FLOAT, 0, twotrisnorms);
				sendArraysToGPU(GL_TRIANGLES, 0, 6);
			}
			break;
			case GEOM_HANIM:
			case GEOM_GEOMETRY:
				render_node(node->geometry);
				break;
			default:
				break;
			}
			GLint ppos, pdir, cr, gtype;

			ppos = GET_UNIFORM(scap->myShaderProgram, "particlePosition");
			pdir = GET_UNIFORM(scap->myShaderProgram, "particleDirection");
			cr = GET_UNIFORM(scap->myShaderProgram, "fw_UnlitColor");
			gtype = GET_UNIFORM(scap->myShaderProgram, "fw_ParticleGeomType");
			glUniform1i(gtype, node->_geometryType); //for SPRITE = 4, screen alignment
			//loop over live particles, drawing each one
			//float estart6[6], eout6[6];
			//extent6f_copy(estart6, peek_group_extent());
			Stack* _particles = node->_particles;

			for (int i = 0; i < vectorSize(_particles); i++) {
				particle pp = vector_get(particle, _particles, i);
				//update particle-specific uniforms
				glUniform3fv(ppos, 1, pp.position);
				//printf("(%f %f %f)", pp.direction[0], pp.direction[1], pp.direction[2]);
				glUniform3fv(pdir, 1, pp.direction);
				//draw
				reallyDrawOnce();
				//extent6f_translate3f(eout6, estart6, pp.position);
				//union_group_extent(eout6);
			}
			clearDraw();
			//cleanup after draw, like child_shape
			FW_GL_BINDBUFFER(GL_ARRAY_BUFFER, 0);
			FW_GL_BINDBUFFER(GL_ELEMENT_ARRAY_BUFFER, 0);
			finishedWithGlobalShader();

			PRINT_GL_ERROR_IF_ANY("child_shape depth end");
		}
		return;
	}
	/* copy the material stuff in preparation for copying all to the shader */
	initialize_front_and_back_material_params();

	prep_BBox((struct BBoxFields*)&node->bboxCenter);

	if (renderstate()->render_blend == (node->_renderFlags & VF_Blend)) {
	if(node->enabled){
	if(TRUE){ //node->isActive){
		int i,j,k,maxparticles;
		double ttime;
		float dtime;
		//int colorSource, alphaSource, isLit, 
		int isUserShader; 
		s_shader_capabilities_t *scap;
		shaderflagsstruct shader_requirements;
		Stack *_particles;
		int allowsTexcoordRamp = FALSE;
		float *texcoord = NULL;
		GLint ppos, pdir, cr, gtype;
		int haveColorRamp,haveTexcoordRamp;

		struct X3D_Node *tmpNG;

		ttime = TickTime();
		dtime = (float)(ttime - node->_lasttime); //increment to particle age

		//if(!once)
		//	printf("child particlesystem \n");


		//RETIRE remove deceased/retired particles (by packing vector)
		_particles = node->_particles;
		maxparticles = min(node->maxParticles,10000);
		for(i=0,j=0;i<vectorSize(_particles);i++){
			particle pp = vector_get(particle,_particles,i);
			pp.age += dtime;
			if(pp.age < pp.lifespan){
				vector_set(particle,_particles,j,pp); //pack vector to live position j
				j++;
			}
		}

		//PREP PHYSICS - wind geta a per-frame gustiness update
		for(k=0;k<node->physics.n;k++){
			switch(node->physics.p[k]->_nodeType){
				case NODE_WindPhysicsModel:
					prep_windphysics(node->physics.p[k]); break;
				default:
					break;
			}
		}
		//APPLY PHYSICS
		for(i=0;i<vectorSize(_particles);i++){
			particle pp = vector_get(particle,_particles,i);
			float positionChange[3];


			//update velocity vector based on physics accelerations
			// A = F/M
			// V2 = V1 + A*dT
			for(k=0;k<node->physics.n;k++){
				switch(node->physics.p[k]->_nodeType){
					case NODE_WindPhysicsModel:
						apply_windphysics(&pp,node->physics.p[k],dtime); break;
					case NODE_ForcePhysicsModel:
						apply_forcephysics(&pp,node->physics.p[k],dtime); break;
					case NODE_MapPhysicsModel:
						apply_mapphysics(&pp, node->physics.p[k], dtime); break;
					default:
						break;
				}
			}

			//update position: P1 = P0 + .5(V0 + V1) x dT
			vecscale3f(positionChange,pp.velocity,.5f * dtime);

			for(k=0;k<node->physics.n;k++){
				switch(node->physics.p[k]->_nodeType){
					case NODE_BoundedPhysicsModel:
						apply_boundedphysics(&pp,node->physics.p[k],positionChange); break;
					default:
						break;
				}
			}
			vecadd3f(pp.position,pp.position,positionChange);

			vector_set(particle,_particles,i,pp); //pack vector to live position j
		}

		//CREATE via emitters (implied dtime = 0, so no physics on first frame)
		_particles->n = j;
		if(node->createParticles && _particles->n < maxparticles && node->emitter && emitter_loaded(node->emitter)){
			//create new particles to reach maxparticles limit
			int n_per_frame, n_needed, n_this_frame;
			float particles_per_second, particles_per_frame;
			n_needed = maxparticles - _particles->n;
			//for explosion emitter, we want them all created on the first pass
			//for all the rest we want maxparticles spread over a lifetime, so by the
			//time some start dying, well be at maxparticles
			//particles_per_second [p/s] = maxparticles[p] / particleLifetime[s]
			particles_per_second = (float)node->maxParticles / (float) node->particleLifetime;
			//particles_per_frame [p/f] = particles_per_second [p/s] / frames_per_second [f/s]
			particles_per_frame = particles_per_second * dtime;
			particles_per_frame += node->_remainder;
			n_per_frame = (int)particles_per_frame;
			node->_remainder = particles_per_frame - (float)n_per_frame;
			n_this_frame = min(n_per_frame,n_needed);
			if(node->emitter->_nodeType == NODE_ExplosionEmitter)
				n_this_frame = n_needed;
			j = _particles->n;
			for(i=0;i<n_this_frame;i++,j++){
				particle pp;
				memset(&pp,0,sizeof(particle)); 
				vecset3f(pp.origin, 0.0f, 0.0f, 0.0f);//for bounded physics
				pp.age = 0.0f;
				pp.lifespan = node->particleLifetime * (1.0f + uniformRandCentered()*node->lifetimeVariation);
				veccopy2f(pp.size,node->particleSize.c);
				//emit particles
				switch(node->emitter->_nodeType){
					case NODE_ConeEmitter:		apply_ConeEmitter(&pp,node->emitter); break;
					case NODE_ExplosionEmitter: apply_ExplosionEmitter(&pp,node->emitter); 
						node->createParticles = FALSE;
						break;
					case NODE_PointEmitter:		apply_PointEmitter(&pp,node->emitter); break;
					case NODE_PolylineEmitter:	apply_PolylineEmitter(&pp,node->emitter); break;
					case NODE_SurfaceEmitter:	apply_SurfaceEmitter(&pp,node->emitter); break;
					case NODE_VolumeEmitter:	apply_VolumeEmitter(&pp,node->emitter); break;
					case NODE_MapEmitter:		apply_MapEmitter(&pp, node->emitter); break;
					default:
						break;
				}
				//save particle
				vector_set(particle,_particles,j,pp);
			}
			_particles->n = j;
		}

		//prepare to draw, like child_shape
		//render appearance
		//BORROWED FROM CHILD SHAPE >>>>>>>>>

		//unsigned int shader_requirements;
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

			//ParticleSystem flag
			shader_requirements.base |= PARTICLE_SHADER;
			if(node->colorRamp || node->color)
				shader_requirements.base |= HAVE_UNLIT_COLOR;
		}
		//printf("child_shape shader_requirements base %d effects %d user %d\n",shader_requirements.base,shader_requirements.effects,shader_requirements.usershaders);
		scap = getMyShaders(shader_requirements);
		enableGlobalShader(scap);
		//enableGlobalShader (getMyShader(shader_requirements)); //node->_shaderTableEntry));

		//see if we have to set up a TextureCoordinateGenerator type here
		if (tmpNG && tmpNG->_intern && tmpNG->_intern->itype == 2) {
			struct X3D_PolyRep* tmppr = (struct X3D_PolyRep*)tmpNG->_intern;
			if (tmppr->tcoordtype == NODE_TextureCoordinateGenerator) {
				getAppearanceProperties()->texCoordGeneratorType = tmppr->texgentype;
				//ConsoleMessage("shape, matprop val %d, geom val %d",getAppearanceProperties()->texCoordGeneratorType, node->geometry->_intern->texgentype);
			}
		}
		//userDefined = (whichOne >= USER_DEFINED_SHADER_START) ? TRUE : FALSE;
		//if (p->userShaderNode != NULL && shader_requirements >= USER_DEFINED_SHADER_START) {
		#ifdef ALLOW_USERSHADERS
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
		#endif //ALLOW_USERSHADERS
		//update effect field uniforms
		if(shader_requirements.effects){
			update_effect_uniforms();
		}

		//<<<<< BORROWED FROM CHILD SHAPE


		//send materials, textures, matrices to shader
		clear_textureUnit_used(); //appearance.texture material.textureXXX, PTMs.texture all need TEXTURE0+ XXX, where xxx starts from 0
		clear_material_samplers(); //PTM and material.textureXXX share frag shader sampler2D textureUnit[16] array
		clear_materialparameters_per_draw_counts(); //especially diffuse texture counts which both appearance and material share

		textureTransform_start();
		// maybe too much? resend_textureprojector_matrix();  
		setupShaderB();
		//send vertex buffer to shader
		allowsTexcoordRamp = FALSE;
		texcoord = NULL;
		switch(node->_geometryType){
			case GEOM_LINE: 
			{
				FW_GL_VERTEX_POINTER (3,GL_FLOAT,0,(float *)linepts);
				sendElementsToGPU(GL_LINES,2,(ushort *)lineindices);
				texcoord = (float*)node->_ltex;
				allowsTexcoordRamp = TRUE;
			}
			break;
			case GEOM_POINT: 
			{
				float point[3];
				memset(point,0,3*sizeof(float));
				FW_GL_VERTEX_POINTER (3,GL_FLOAT,0,(GLfloat *)point);
        		sendArraysToGPU (GL_POINTS, 0, 1);
			}
			break;
			case GEOM_QUAD: 
			{
				//textureCoord_send(&mtf);
				FW_GL_VERTEX_POINTER (3,GL_FLOAT,0,(GLfloat *)node->_tris);
				FW_GL_NORMAL_POINTER (GL_FLOAT,0,twotrisnorms);
				sendArraysToGPU (GL_TRIANGLES, 0, 6);
				texcoord = (float*)node->_ttex;
				allowsTexcoordRamp = TRUE;
			}
			break;
			case GEOM_SPRITE: 
			{
				//textureCoord_send(&mtf);
				FW_GL_VERTEX_POINTER (3,GL_FLOAT,0,(GLfloat *)node->_tris);
				FW_GL_NORMAL_POINTER (GL_FLOAT,0,twotrisnorms);
				sendArraysToGPU (GL_TRIANGLES, 0, 6);
			}
			break;
			case GEOM_TRIANGLE: 
			{
				//textureCoord_send(&mtf);
				FW_GL_VERTEX_POINTER (3,GL_FLOAT,0,(GLfloat *)node->_tris);
				FW_GL_NORMAL_POINTER (GL_FLOAT,0,twotrisnorms);
				sendArraysToGPU (GL_TRIANGLES, 0, 6);
				texcoord = (float*)node->_ttex;
				allowsTexcoordRamp = TRUE;
			}
			break;
			case GEOM_GEOMETRY: 
				render_node(node->geometry);
			break;
			case GEOM_HANIM:
				render_node(node->geometry);
				break;
			default:
				break;
		}

		ppos = GET_UNIFORM(scap->myShaderProgram,"particlePosition");
		pdir = GET_UNIFORM(scap->myShaderProgram, "particleDirection");
		cr = GET_UNIFORM(scap->myShaderProgram,"fw_UnlitColor");
		gtype = GET_UNIFORM(scap->myShaderProgram,"fw_ParticleGeomType");
		glUniform1i(gtype,node->_geometryType); //for SPRITE = 4, screen alignment
		//loop over live particles, drawing each one
		haveColorRamp = node->colorRamp || node->color ? TRUE : FALSE;
		haveColorRamp = haveColorRamp && cr > -1;
		haveTexcoordRamp = node->texCoordRamp || node->texCoord ? TRUE : FALSE;
		haveTexcoordRamp = haveTexcoordRamp && allowsTexcoordRamp && texcoord; 
		if(haveTexcoordRamp){
			//glUniform1i(scap->nTexMatrix,0);
			glUniform1i(scap->nTexCoordChannels,1);
			glUniform1i(scap->flipuv, 0);
			//glUniform1i(scap->textureCount,1);
		}
		float estart6[6], eout6[6];
		//extent6f_copy(estart6, peek_group_extent());
		extent6f_clear(estart6);
		for(i=0;i<vectorSize(_particles);i++){
			particle pp = vector_get(particle,_particles,i);
			//update particle-specific uniforms
			glUniform3fv(ppos,1,pp.position);
			glUniform3fv(pdir, 1, pp.direction);
			//printf("(%f %f %f)", pp.direction[0], pp.direction[1], pp.direction[2]);
			if(haveColorRamp)
				updateColorRamp(node,&pp,cr);
			if(haveTexcoordRamp)
				updateTexCoordRamp(node,&pp,texcoord);
			if(node->_geometryType == GEOM_LINE){
				float lpts[6], vel[3];
				vecnormalize3f(vel,pp.velocity);
				vecscale3f(&lpts[3],vel,.5f*node->particleSize.c[1]);
				vecscale3f(&lpts[0],vel,-.5f*node->particleSize.c[1]);
				FW_GL_VERTEX_POINTER (3,GL_FLOAT,0,(float *)lpts);
			}
			//draw
			reallyDrawOnce();
			//extent6f_translate3f(eout6, estart6, pp.position);
			//union_group_extent(eout6);
			//printf("pp.pos %f %f %f\n", pp.position[0], pp.position[1], pp.position[2]);
			extent6f_union_vec3f(estart6,pp.position);
		}
		memcpy(node->_extent, estart6, 6 * sizeof(float));
		clearDraw();
		//cleanup after draw, like child_shape
		FW_GL_BINDBUFFER(GL_ARRAY_BUFFER, 0);
		FW_GL_BINDBUFFER(GL_ELEMENT_ARRAY_BUFFER, 0);
		textureTransform_end();

		//BORROWED FROM CHILD_SHAPE >>>>>>
		//fin_Appearance
		if(node->appearance){
			struct X3D_Appearance *tmpA;
			POSSIBLE_PROTO_EXPANSION(struct X3D_Appearance *,node->appearance,tmpA);
			if(tmpA->effects.n)
				fin_sibAffectors(X3D_NODE(tmpA),&tmpA->effects);
		}
		/* any shader turned on? if so, turn it off */

		//ConsoleMessage("turning shader off");
		finishedWithGlobalShader();
#ifdef HAVE_P
		p->material_twoSided = NULL;
		p->material_oneSided = NULL;
		p->userShaderNode = NULL;
#endif
		tg->RenderFuncs.shapenode = NULL;
    
		/* load the identity matrix for textures. This is necessary, as some nodes have TextureTransforms
			and some don't. So, if we have a TextureTransform, loadIdentity */
    
#ifdef HAVE_P
		if (p->this_textureTransform) {
			p->this_textureTransform = NULL;
#endif //HAVE_P
			FW_GL_MATRIX_MODE(GL_TEXTURE);
			FW_GL_LOAD_IDENTITY();
			FW_GL_MATRIX_MODE(GL_MODELVIEW);
#ifdef HAVE_P
		}
#endif    
		/* LineSet, PointSets, set the width back to the original. */
		{
			float gl_linewidth = tg->Mainloop.gl_linewidth;
			glLineWidth(gl_linewidth);
#ifdef HAVE_P
			p->appearanceProperties.pointSize = gl_linewidth;
#endif
		}

		/* did the lack of an Appearance or Material node turn lighting off? */
		LIGHTING_ON;

		/* turn off face culling */
		DISABLE_CULL_FACE;

		//<<<<< BORROWED FROM CHILD_SHAPE

		node->_lasttime = ttime;
	} //isActive
	} //enabled
	} //VF_Blend
	// once = 1;
	fin_BBox((struct X3D_Node*)node, (struct BBoxFields*)&node->bboxCenter, FALSE);

}
