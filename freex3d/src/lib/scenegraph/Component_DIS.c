
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




*/

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
	compile_Transform((struct X3D_Transform*)node);
	have_DIS = 1;
}

/* do transforms, calculate the distance */
void prep_EspduTransform (struct X3D_EspduTransform *node) {
	//option 1: done during startofloopnodeupdates prep_ pass
	//option 2: as option 1, except receive part called from frontend thread on receive
	/* pseudo-code
	if(sender){
		compute velocities from delta pose
		delta_time = time() - last_send_time
		if(delta_time > send_interval){
			recompute_pdu_from_node
			//over-writing has an advantage: if no frontend capability or permission
			//	to network, backend doesn't hang or buffer-overflow
			convert_pdu_2_streambuf
			add_or_overwrite_sender_list_entity_item(ip,port,entity,pdu_stream*)
			update last send time
		}
	}else if(receiver){
		if(node->incominglist.n){
			//process incoming pdus
			if(flooded) 
				just take last recieved pdu
			else
				iterate over pdus by send timestamp (which may be different than recv order)
			clear list
		}else{
			//initial velocities are 0, so if no incoming updates, 
			//  the entities don't move
			dead_reckoning update
		}
		change++
	}
	//else idle
	*/
	prep_Transform((struct X3D_Transform *)node);
}

void fin_EspduTransform (struct X3D_EspduTransform *node) {
	fin_Transform((struct X3D_Transform*)node);
} 

void child_EspduTransform (struct X3D_EspduTransform *node) {
	child_Transform((struct X3D_Transform*)node);
}

