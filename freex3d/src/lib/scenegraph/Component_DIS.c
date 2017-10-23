
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


/*******************************************************************

	X3D DIS Component

*********************************************************************/


#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>
#include <iglobal.h>
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
#include "LinearAlgebra.h"
#include "Children.h"

/*
typedef struct pComponent_DIS{
	int something;
}* ppComponent_DIS;
void *Component_DIS_constructor(){
	void *v = MALLOCV(sizeof(struct pComponent_DIS));
	memset(v,0,sizeof(struct pComponent_DIS));
	return v;
}
void Component_DIS_init(struct tComponent_DIS *t){
	//public
	//private
	t->prv = Component_DIS_constructor();
	{
		ppComponent_DIS p = (ppComponent_DIS)t->prv;
		p->something = 0;
	}
}
void Component_DIS_clear(struct tComponent_DIS *t){
	//public
}

http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/dis.html
https://github.com/open-dis/open-dis-cpp
http://www.web3d.org/x3d/content/examples/Basic/DistributedInteractiveSimulation/
https://en.wikipedia.org/wiki/Distributed_Interactive_Simulation
http://open-dis.sourceforge.net/Open-DIS.html



Problem: our C .h and the DIS.lib (cpp) .h clash, very messy
x didn't find a combination of headers that worked

Options:
1. clean up our headers
2. convert DIS.lib objects we need to flat C structs
	- about 30 structs
	a) manually, from .cpp
	b) hack xmlpg https://github.com/open-dis/xmlpg CppGenerator.java 
		into a CGenerator.java and generate flat C
3. wrap DIS objects -just ones we need- in flat C interfaces (about 30)
4. somehow show cpp just the C structs it needs, like X3D_EspduTransform
	- about 5 x3d structs
Choice: option 2.b
- benefits: easy to interface, could do just .h (no lib), code & license is ours/freewrl
-disadvantages: someone has to do hacking upstream in CGenerator.java, and
	duplicate all the CppUtils (that wrap the pdu classes) in C,
	and mistakes can happen during transcription (risk)


*/
//#define WITH_DIS 1
#ifdef WITH_DIS
#include "../DIS/DIS.h"
#endif //WITH_DIS


void prep_EspduTransform_cpp(struct X3D_EspduTransform *node);
void fin_EspduTransform_cpp(struct X3D_EspduTransform *node);
void compile_EspduTransform_cpp(struct X3D_EspduTransform *node);

static int have_DIS = 0;
static int allow_DIS = 0;
void fwl_init_DIS(){
	//from commandline --DIS or -D
	allow_DIS = 1;
}
int fwl_doing_DIS(){
	return allow_DIS && have_DIS ? 1 : 0;
}
int fwl_get_allow_DIS(){
	return allow_DIS;
}
void fwl_set_allow_DIS(int allow){
	allow_DIS = allow ? 1 : 0;
}


void compile_EspduTransform (struct X3D_EspduTransform *node) { 
	//compile_EspduTransform0(node);
	compile_Transform((struct X3D_Transform*)node);
}

/* do transforms, calculate the distance */
void prep_EspduTransform (struct X3D_EspduTransform *node) {

	//prep_EspduTransform0(node);
	//else standalone
	prep_Transform((struct X3D_Transform *)node);
	have_DIS = 1;

}

void fin_EspduTransform (struct X3D_EspduTransform *node) {
	//fin_EspduTransform0(node);
	fin_Transform((struct X3D_Transform*)node);
} 

void child_EspduTransform (struct X3D_EspduTransform *node) {
	child_Transform((struct X3D_Transform*)node);
}


//DIS - Distributed Interactive Simulation communication
void fwl_sendreceive_DIS(){
	//just the buffer in/out is handled here
	//the interpretation/parsing/packing of pdus is done in the backend
	//this might need to be in the front end so platforms with sandbox restrictions on communication
	//can do this in the native language/technology if necessary - I'll find out later.
/* pseudo code design:
	if(dis_sendlist.n > 0){
		//backend will queue up pdus to send,
		//frontend fetches them one by one from the queue
		//this isolates potentialy platform-specific things like networking in frontend
		//while avoiding having the backend call into the frontend which is often in a dfferent
		//language technology 
		loop over sendlist:
		(n,buf,ip,port) = get_next_send_from_backend()
		sendTo(buf,n,ip,port)
		// flushing queue should be done in BACKEND, start of each loop
		// so if no frontend capability, the queue doesn't overflow
	}
	if(dis_recvlist.n > 0){
	  loop over all recv channels or connect-switch or libevent
		if(not yet opened)
			open
		non-blocking recv or recvfrom or recv with short timeout
		if(got something) dis_incoming_to_backend(ip,port,stream,len)
	}
*/
	printf("yo from fwl_sendreceive_DIS\n");
}

#ifdef USING_DIS
#include "../DIS/DIS.c"
#endif USING_DIS
