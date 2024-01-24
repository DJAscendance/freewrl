
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
#include "Vector.h"
#include "Component_Geospatial.h"
#include "Component_DIS.h"
#include "Component_Grouping.h"
#include "RenderFuncs.h"

//from CparseParser
void add_node_to_broto_context(struct X3D_Proto *currentContext,struct X3D_Node *node);

#ifndef WIN32
#define SOCKET int
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
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

References:
http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/dis.html
https://github.com/open-dis/open-dis-cpp
http://www.web3d.org/x3d/content/examples/Basic/DistributedInteractiveSimulation/
http://x3dgraphics.com/slidesets/X3dForAdvancedModeling/DistributedInteractiveSimulation.pdf
-- brutzman slideshow on DIS
https://en.wikipedia.org/wiki/Distributed_Interactive_Simulation
http://open-dis.sourceforge.net/Open-DIS.html
http://movesinstitute.org/~mcgredo/MV3500/hla/1278.1-200X%20Draft%2016%20rev%2018.pdf
- 2012 DIS draft
- p.332 7.2.2 espdu struct/contents
- p.665 Annex E dead reckoning formula

Don's references:
a. IITSEC 2017 slideset, DIS 101
   https://gitlab.nps.edu/Savage/NetworkedGraphicsMV3500/raw/master/presentations/IITSEC2018_DIS_Tutorial.pptx

b. X3D and Distributed Interactive Simulation (DIS)
   http://x3dgraphics.com/slidesets/X3dForAdvancedModeling/DistributedInteractiveSimulation.pdf
   (brutzman slides)

c. X3D v3.3 Distributed interactive simulation (DIS) component
   http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/dis.html
  
d. IEEE Standards Maintained by SISO SAC
   https://www.sisostds.org/ProductsPublications/Standards/IEEEStandards.aspx
   
   
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
Choice: option 2.b hack xmlpg CGenerator.java DONE
- benefits: easy to interface, could do just .h (no lib), code & license is ours/freewrl
-disadvantages: someone has to do hacking upstream in CGenerator.java, and
	duplicate all the CppUtils (that wrap the pdu classes) in C,
	and mistakes can happen during transcription (risk)

Problem: Transforms - unclear how goecoords are to be used.
The DIS specs have 5 terms:
global, world, local, body, entity
And there are 2 major geospatial states:
a) geoCoords not used
- geoCoords = 0,0,0 (default) 
b) geoCoords used

Proposed use of terms:
global == world = gc ^see World Coordinates below
local - used only for some Dead Reckoning formulas
	== TCS (topocentric coord sys) from geospatial component, except with axes re-arranged/swizzled
	Draft DIS specs p.675: 
	"The local coordinate system used here is defined by North, East, and Down axes 
	with their origin at the entity's center of bounding volume."
entity == body - the coordinate system for espdu.transform.children 
	-except DIS swizzles so Z is down, X forward, see Gimbals.x3d test scene
World2body = Location,(phi,theta,psi) 
in web3d we can split world2body in 2 parts, to make dead-reckoning in local coords easier:
- World2Local x Local2body
- where:
-- World2local is the gc2tcs from geospatial 
-- local2body is the rest of world2body: local2body = world2local.inverse x world2body
if a) no geoCoords used, then
- World2local == 0
and
- world2body == local2body (we do it all with transform.translation,rotation)
Freewrl strategy:
- we will convert the DIS coordinate axes and naming to web3d conventions during node2pdu and pdu2node
	- convert from geocentric GC to Topocentric TCS if geocoords != 000
- we will be working in web3d coordinate systems elsehwere, including in dead reckoning
	(so no need to swizzle axes if our dead reckonging formulas are all in web3d conventions)
Q. is this compatible with Xj3d convention?
A. don't know. But Gimbal.x3d is showing (what I interpret as) 
	the Dead Reckoning Local coord system as being 
	swizzled and aligned with what looks like our geospatial TCS
	That could indicate where NPS thinking was on how X3D relates to DIS. 
	So its a good bet espduTransform.(translation.rotation) are tcs2body aka local2body.


World Coordinates: 
The Entity State PDU Location field is 64bit wgs84 geocentric coordinates.
In the draft 2012 specs document,
p.3 1.6.3.1 World coordinate system WGS84, meters.
p.4 Figure 1 shows world coordinates.
- they look exactly like GC.
- Z through Northpole
- X through prime meridian
- right handed (Y through bangladesh)
p.48 e) 1) Location with respect to the world
p.330 6.2.98 World Coordinates record
Location of the origin of the entity's or object's coordinate system, target locations,
	 detonation locations, and other points shall be specified by a set of three coordinates: X, Y, and Z, 
	 represented by 64-bit floating point numbers. The world coordinate system shall be as specified in 1.6.3. 
	 The format of the World Coordinates record shall be as shown in Table134.
p.333
h) Entity Location. This field shall specify an entity’s physical location in the simulated world, 
	and shall be represented by a World Coordinates record (see 6.2.98).
p.334 Entity State PDU table 135
/World Coordinates


*/
//#define WITH_DIS 1
#ifdef WITH_DIS
#include "../DIS/DIS.h"

//there's another .pdf with enums
// SISO-REF-010-2015 Enumerations for Simulation Interoperability V21 20150413.pdf
// we'll do just a few here as needed
// SM > DataRecord > datumType
enum UID66 {
Kind = 11110,
Domain  = 11120,
Country  = 11130,
Category = 11140,
Subcategory = 11150,
Specific = 11160,
Extra = 11170,
/*
31000 Position
 31010 Route (Waypoint) type 
 31100 MilGrid10 
 31200 Geocentric Coordinates 
 31210 X 
 31220 Y 
 31230 Z 
 31300 Latitude 
 31400 Longitude 
 ...
 31600 Altitude 
 */
};

#endif //WITH_DIS




static int allow_DIS = 0;
static char* DISaddress = NULL;
static int DISport = 0;
static int DISsite = 0;
static int DISapplication = 0;
void fwl_init_DIS(){
	//from commandline --DIS or -D
	allow_DIS = 1;
}
void fwl_set_DISaddress(char* address) {
	DISaddress = address;
}
void fwl_set_DISport(int port) {
	DISport = port;
}
void fwl_set_DISsite(int site) {
	DISsite = site;
}
void fwl_set_DISapplication(int app) {
	DISapplication = app;
}

int fwl_get_allow_DIS(){
	return allow_DIS;
}
char* fwl_get_DISaddress() {
	return DISaddress;
}
int fwl_get_DISport() {
	return DISport;
}
int fwl_get_DISsite() {
	return DISsite;
}
int fwl_get_DISapplication() {
	return DISapplication;
}
void fwl_set_allow_DIS(int allow){
	allow_DIS = allow ? 1 : 0;
}
//testset is an int added to port when regression tessting DIS sets
// so 2 sets of mulitple freewrl instances don't mix their communications
static int testset = 0;
void fwl_set_testset(int iset) {
	testset = iset;
}
int fwl_get_testset() {
	return testset;
}

#ifdef WITH_DIS

/* 
DIS - Distributed Interactive Simulation communication
Concepts as understood by dug9 Oct 23, 2017

IMPLIED SHARED RECEIVER LOOP
- loop 1:1 socket(IP,port)
- 1st node to declare a port opens it and joins
- other nodes declaring same port join
- reading is at fine time granularity, and times are added up 
- multiple receives on one loop are all read to flush and take pdu with latest timestamp
- if received too soon, packets dropped (and dead reconning used, or radio signal is choppy)
- timestamp is kept as start of interval, and each loop increments it
- on recv packet, it loops over the packet unmarshalling multiple pdus
	- it loops over nodes registered on that port to find a matching entityID/entityId
	- if a match, updates entity
	- if no match, discards/skips

IMPLIED SHARED SENDER LOOP
- loop 1:1 socket(IP,port)
- 1st node to declare a port opens it and joins
- other nodes declaring same port join
- nodes can have different send intervals
- fine-granularity loop increments time and checks who is ready to send
- bundles pdus that are ready to send at the same time

Design Options
A. per-frame - iterate over all send and receive sockets once per frame, 
	-- in the render thread
B. per-socket-direction thread 

Major Issues:
1. change detection 
- for scripts and protos we have special field structs with a change flag;
	we do that because scripts and protos are 'opaque' to routing algorithms
- DIS nodes when receiving are changing fields like a script node might change its fields, opaque to routing
	and for script nodes we iterate over fields after running a script, to check fields for change flag and route
- for sending, somewhat analogously when we change a field via routing on a DIS node,
	we need to let the pdu sending code know which puds changed, so it can send just the changed ones
- options: 
	a) implement nodes in terms of i) script or ii) proto fields
		- don't have an example of script or proto used as builtin 
		- its the parser that generates them from scene file info
		? would that mean a lot of changes to perl code generator and parser code?
		- might be helpful to harmonize all builtin, proto, script and shader nodes to have same field struct
		x but will take a massive refactoring effort to do it
		- and with DIS, you send/receive whole pdus, and maybe only one little thing changed, 
			per-node-field flags wouldn't help because those flags aren't transmitted/received with pdu
	b) pre/post value comparison ie _oldvalue
		- lots of examples of this, but not on such big nodes
	choice: b) SFNode node->_oldState copies entire node
		- generic functions compare old new fields to detect changes
		- but keep option a) in mind for future

2. nodes have a lot of similar fields, resulting in duplicate code
x and freewrl has no structs for 'abstract interface'
options:
a) giant macros - used throughout freewrl for this reason
b) careful ordering of fields so common fields are first, and nodes can be cast to a common type
c) some kind of abstract interface added to code generation system
	- maybe in the future
d) change to OO language and use inheritance and polymorphism
	- maybe in the future
e) functions with switch-case on nodetype
For now in DIS we're going to use b) for espduTransform and 3 radio nodes and entityManager and e)

*/


// http://movesinstitute.org/~mcgredo/MV3500/hla/enum99_2.pdf
// p.6
// 62 and 65 Comment-R seem duplicates, so we set it to Comment-R2

enum PDUType
{
	PDU_OTHER = 0,
	PDU_ENTITY_STATE = 1,
	PDU_FIRE = 2,
	PDU_DETONATION = 3,
	PDU_COLLISION = 4,
	PDU_SERVICE_REQUEST = 5,
	PDU_RESUPPLY_OFFER = 6,
	PDU_RESUPPLY_RECEIVED = 7,
	PDU_RESUPPLY_CANCEL = 8,
	PDU_REPAIR_COMPLETE = 9,
	PDU_REPAIR_RESPONSE = 10,
	PDU_CREATE_ENTITY = 11,
	PDU_REMOVE_ENTITY = 12,
	PDU_START_RESUME = 13,
	PDU_STOP_FREEZE = 14,
	PDU_ACKNOWLEDGE = 15,
	PDU_ACTION_REQUEST = 16,
	PDU_ACTION_RESPONSE = 17,
	PDU_DATA_QUERY = 18,
	PDU_SET_DATA = 19,
	PDU_DATA = 20,
	PDU_EVENT_REPORT = 21,
	PDU_COMMENT = 22,
    PDU_ELECTROMAGNETIC_EMISSION = 23,
    PDU_DESIGNATOR = 24,
    PDU_TRANSMITTER = 25,
    PDU_SIGNAL = 26,
    PDU_RECEIVER = 27,
    PDU_IFF_ATC_NAVAIDS = 28,
    PDU_UNDERWATER_ACOUSTIC = 29,
    PDU_SUPPLEMENTAL_EMISSION_ENTITY_STATE = 30,
    PDU_INTERCOM_SIGNAL = 31,
    PDU_INTERCOM_CONTROL = 32,
    PDU_AGGREGATE_STATE = 33,
    PDU_ISGROUPOF = 34,
    PDU_TRANSFER_CONTROL = 35,
    PDU_ISPARTOF_= 36,
    PDU_MINEFIELD_STATE =37,
    PDU_MINEFIELD_QUERY = 38,
    PDU_MINEFIELD_DATA = 39,
    PDU_MINEFIELD_RESPONSE_NAK = 40,
    PDU_ENVIRONMENTAL_PROCESS = 41,
    PDU_GRIDDED_DATA = 42,
    PDU_POINT_OBJECT_STATE = 43,
    PDU_LINEAR_OBJECT_STATE = 44,
    PDU_AREAL_OBJECT_STATE = 45,
    PDU_TSPI = 46,
    PDU_APPEARANCE = 47,
    PDU_ARTICULATED_PARTS = 48,
    PDU_LE_FIRE = 49,
    PDU_LE_DETONATION = 50,
    PDU_CREATE_ENTITY_R = 51,
    PDU_REMOVE_ENTITY_R = 52,
    PDU_START_RESUME_R = 53,
    PDU_STOP_FREEZE_R = 54,
    PDU_ACKNOWLEDGE_R = 55,
    PDU_ACTION_REQUEST_R = 56,
    PDU_ACTION_RESPONSE_R = 57,
    PDU_DATA_QUERY_R = 58,
    PDU_SET_DATA_R = 59,
    PDU_DATA_R = 60,
    PDU_EVENT_REPORT_R = 61,
    PDU_COMMENT_R = 62,			//62
    PDU_RECORD_QUERY_R = 63,
    PDU_SET_RECORD_R = 64,
    PDU_COMMENT_R2 = 65,		//DUPLICATE OF 62
    PDU_COLLISION_ELASTIC = 66,
    PDU_ENTITY_STATE_UPDATE = 67,
    PDU_ANNOUNCE_OBJECT = 129,
    PDU_DELETE_OBJECT = 130,
    PDU_DESCRIBE_APPLICATION = 131,
    PDU_DESCRIBE_EVENT = 132,
    PDU_DESCRIBE_OBJECT = 133,
    PDU_REQUEST_EVENT = 134,
    PDU_REQUEST_OBJECT = 135,  
};

void axisangle2ypr(float *xyza, float *ypr)
{
	//y = yaw = azimuth
	//p = pitch = elevation
	//r = roll
	//assumes z is up, you re-arrange your inputs if other
	float yaw, pitch, roll, x,y,z,a, xyz[3], flen;
	vecnormalize3f(xyz,xyza);
	x = xyz[0]; y = xyz[1], z=xyz[2], a=xyza[3];
	flen = veclength2f(xyz);
	if(flen == 0.0f){
		yaw = 0.0f;
		pitch = acos(-1.0) * .5; //90
	}else{
		yaw = atan2(y,x);
		pitch = atan(z/flen);
	}
	roll = a;
	ypr[0] = yaw;
	ypr[1] = pitch;
	ypr[2] = roll;
}
void ypr2axisangle(float *ypr, float *xyza)
{
	//y = yaw = azimuth
	//p = pitch = elevation
	//r = roll
	//assumes z is up, you re-arrange your inputs if other
	float yaw, pitch, roll, x,y,z,a, xyz[3];
	yaw = ypr[0];
	pitch = ypr[1];
	roll = ypr[2];
	a = roll;
	x = cos(pitch)*cos(yaw);
	y = cos(pitch)*sin(yaw);
	z = sin(pitch); //or sqrt(1.0 - (x*x + y*y))
	xyz[0] = x;
	xyz[1] = y;
	xyz[2] = z;
	vecnormalize3f(xyza,xyz);
	xyza[3] = a;
}
//dis stores vectors in structs .xyz, we do float[3], conversions:
struct Vector3Float *vec3f2vector3float(struct Vector3Float *b, float *a){
	b->x = a[0];
	b->y = a[1];
	b->z = a[2];
	return b;
}
float *vector3float2vec3f(float *b,struct Vector3Float *a){
	b[0] = a->x;
	b[1] = a->y;
	b[2] = a->z;
	return b;
}
struct Vector3Double *vec3d2vector3double(struct Vector3Double *b, double *a){
	b->x = a[0];
	b->y = a[1];
	b->z = a[2];
	return b;
}
double *vector3double2vec3d(double *b,struct Vector3Double *a){
	b[0] = a->x;
	b[1] = a->y;
	b[2] = a->z;
	return b;
}
// freewrl's once-per-frame timestamp is called TickTime 
// - and TickTime is a double value representing seconds since 1970, including fractions of a second
// DIS Clock Time record is a 64bit consisting of 
//   32bit Hours since 1970 UTC and
//   32bit Timestamp fraction-past-the-hour scaled by 2**31 
//    - the least significant bit- is reserved for flagging 1=AbsoluteTime (vs relative =0)
//   in draft specs see
//     6.2.88 TimeStamp p.319
//     G.4 Time Terminology p.686
//
void TickTime2DISTime(double ticktime, int iabs, unsigned int *hours, unsigned int *hourfraction ){
	double hours1970, fraction;
	unsigned int bitmask;
	hours1970 = floor(ticktime / 3600.0);
	*hours = (unsigned int)hours1970;
	fraction = (ticktime / 3600.0) - hours1970;
	*hourfraction = ((unsigned int)(fraction * pow(2.0,31.0)))<<1;
	bitmask = 0;
	bitmask = ~bitmask;
	if(!iabs)
		bitmask = bitmask << 1; //clear least significant bit
	*hourfraction = *hourfraction & bitmask;
	if(iabs) *hourfraction |= 1;
}
double DISTime2TickTime(unsigned int hours, unsigned int hourfraction){
	//pass in 0 for hours if relative timestamp (then we'll take hours from TickTime())
	double fraction, mantissa;
	int iabs;
	unsigned int bitmask;
	bitmask = 1;
	iabs = (hourfraction & bitmask) != 0 ? TRUE : FALSE;
	hourfraction = hourfraction >> 1; //take everything except least significant bit
	fraction = hourfraction;
	fraction /= pow(2.0,31.0);
	if(iabs)
		mantissa = hours;
	else
		mantissa = floor(TickTime() / 3600.0);
	mantissa += fraction;
	mantissa *= 3600.0;
	return mantissa;

}

//A. per-frame

struct dis_socket {
	int port;
	char *address;
	SOCKET socket;
	struct sockaddr_in saddr;
	int multicastRelayPort;
	char *multicastRelayHost;
	int idir; //0 = receive, 1 = send
	struct Vector *registered;
	double lasttime;
};


void print_stream(unsigned char *buf, int nbytes){
	int i,j;
	for(i=0;i<min(210,nbytes);i+=10){
		int j;
		printf("%d\t",i);
		for(j=0;j<10;j++){
			printf("%5d",(int)buf[i+j]);
		}
		printf("\n");
	}
}
//TCS - geospatial topocentric coordinate system
//local - DIS equivalent
double *tcs2localswizzled(double *local,double *tcs){
	local[0] = -tcs[2]; //north
	local[1] =  tcs[0]; //east
	local[2] = -tcs[1]; //vertical
	return local;
}
double *local2tcsswizzled(double *tcs,double *local){
	tcs[0] =  local[1]; //east
	tcs[1] = -local[2]; //vertical
	tcs[2] = -local[0]; //north
	return tcs;
}
void node2pdu_entityType(int *entityKind, struct EntityType *entityType){
	//assumes node field order: kind, domain, country, category, subcategory, specific, extra
	//p.262 draft standard http://movesinstitute.org/~mcgredo/MV3500/hla/1278.1-200X%20Draft%2016%20rev%2018.pdf
	entityType->entityKind = (unsigned char) entityKind[0];
	entityType->domain = (unsigned char) entityKind[1];
	entityType->country = (unsigned short) entityKind[2];
	entityType->category = (unsigned char) entityKind[3];
	entityType->subcategory = (unsigned char) entityKind[4];
	entityType->specific = (unsigned char) entityKind[5];
	entityType->extra = (unsigned char) entityKind[6];
}
void pdu2node_entityType( struct EntityType *entityType, int *entityKind){
	//assumes node field order: kind, domain, country, category, subcategory, specific, extra
	//p.262 draft standard http://movesinstitute.org/~mcgredo/MV3500/hla/1278.1-200X%20Draft%2016%20rev%2018.pdf
	entityKind[0] = entityType->entityKind;
	entityKind[1] = entityType->domain;
	entityKind[2] = entityType->country;
	entityKind[3] = entityType->category;
	entityKind[4] = entityType->subcategory;
	entityKind[5] = entityType->specific;
	entityKind[6] = entityType->extra;
}
static int dis_event_number = 0;
int dis_next_event_number(){
	dis_event_number++;
	return dis_event_number;
}
static int dis_fire_mission_index = 0;
int dis_next_fire_mission_index(){
	dis_fire_mission_index++;
	return dis_fire_mission_index;
}
struct X3D_Node * dis_find_registered_node_by_entityid(int entityid, int sendlist, int recvlist);
struct Vector * dis_node2pdus_espdu(struct X3D_Node *node, int isHeartbeat){
	//http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/dis.html#EspduTransform
	//EspuTransform integrates the following pdus:
	//EntityStatePDU, CollisionPDU, DetonationPDU, FirePDU, CreateEntity, and RemoveEntity.
	//http://movesinstitute.org/~mcgredo/MV3500/hla/1278.1-200X%20Draft%2016%20rev%2018.pdf
	//p.332 7.2.2 espdu struct/contents
	//Q. how do create/remove work?
	struct Vector *pdus;
	struct X3D_EspduTransform * pnode = (struct X3D_EspduTransform*)node;
	pdus = newVector(struct Pdu *, 6);

	//ENTITYSTATE
	//if(pnode->_pduchange_es_articulation || pnode->_pduchange_es_deadreckoning || pnode->_pduchange_es_info || pnode->_pduchange_es_force){
	if(0) printf("es pduchange %d heartbeat %d\n",pnode->_pduchange_es,isHeartbeat);
	if(pnode->_pduchange_es || isHeartbeat){
		float xyz[3];
		struct EntityStatePdu *espdu;
		espdu = (struct EntityStatePdu*)dis_ctor(type_EntityStatePdu);
		//entity
		espdu->entityType.category = 1; //not 77
		espdu->entityID.entity = pnode->entityID;
		espdu->entityID.application = pnode->applicationID;
		espdu->entityID.site = pnode->siteID;
		//translation - assumes companion scenes will have same parent transform stack
		//(x, -z, y).
		if(pnode->__geoSystem){
			//a default geo scene is in TCS at Accra (Grenwich & equator)
			Quaternion qgc2tcs, qtcs2body, qgc2body;
			struct SFVec3d gd, gc, translate;
			struct SFVec4d rotate;
			double localxyz[3], tcsxyz[3], tcs2bodyxyz[3], world2bodyxyz[3];
			float xyza[4];
			Geosys *gs;
			gs = GEOSYS(pnode->__geoSystem);
			user2gc(gs,&pnode->geoCoords,1,&gc);
			//gc2gd(gs,&gc,1,&gd);
			//gc2tcs_transform(gs,&gd,&translate,&rotate);
			gc2tcsB_transform(gs,&gc,&translate,&rotate);
			//somehow get body/entity into world/gc - rotation and translation
			{
				//rotation
				float ypr[3];
				vrmlrot4d_to_quaternion(&qgc2tcs,rotate.c);
				vrmlrot4f_to_quaternion(&qtcs2body,pnode->rotation.c);
				quaternion_multiply(&qgc2body,&qgc2tcs,&qtcs2body);
				quaternion_to_vrmlrot4f(&qgc2body,xyza);
				//vecprint4fb("send  xyza",xyza,"\n");
				xyza[3] = -xyza[3];
				axisangle2ypr(xyza,ypr);

				espdu->entityOrientation.psi = -ypr[0];  //gimbal.js shows -yaw
				espdu->entityOrientation.theta = ypr[1];
				espdu->entityOrientation.phi = ypr[2];

			}
			{
				//translation
				struct SFVec3d tcs, world;
				float2double(tcs.c,pnode->translation.c,3);
				//tcs2gc(gs,&gd,&tcs,1,&world);
				tcs2gcB(gs,&gc,&tcs,1,&world);
				vec3d2vector3double(&espdu->entityLocation,world.c);
			}

			//send > geo > dead reckoning
			if(pnode->deadReckoning < 6){
				//first update linear V,A, angularV
				//for drmethod < 6
				//all of which are in Local/TCS for freewrl/web3d, instead of world for DIS 
				pnode->_change_count++;
				if(pnode->_change_count > 1){
					double dtime;
					float v1[3], tmp[3], a1[3];
					dtime = TickTime() - pnode->_lastp0time;
					vecscale3f(v1,vecdif3f(tmp,pnode->translation.c,pnode->_lastp0.c),1.0f/dtime);
					//pnode->_change_count = min(pnode->_change_count,2);
					if(pnode->_change_count > 2){
						//a = (v1-v0)/dt
						vecscale3f(a1,vecdif3f(tmp,v1,pnode->linearVelocity.c),1.0f/dtime);
						veccopy3f(pnode->linearAcceleration.c,a1);
						//v1 = v0 - 1/2at**2
					
					}else{
						vecset3f(pnode->linearAcceleration.c,0.0,0.0,0.0);
					}
					veccopy3f(pnode->linearVelocity.c,v1);
					{
						//update angular velocity
						Quaternion qlast,q, qinv, qdif;
						vrmlrot4f_to_quaternion(&qlast,pnode->_lastr0.c);
						vrmlrot4f_to_quaternion(&q,pnode->rotation.c);
						quaternion_inverse(&qinv,&qlast);
						quaternion_multiply(&qdif,&q,&qinv);
						quaternion_to_vrmlrot4f(&qdif,pnode->_angularVelocity.c);
						pnode->_angularVelocity.c[3] *= 1.0f/dtime;
					}
				}
			}
			veccopy3f(pnode->_lastp0.c,pnode->translation.c);
			veccopy4f(pnode->_lastr0.c,pnode->rotation.c);
			pnode->_lastp0time = TickTime();
			
			espdu->deadReckoningParameters.deadReckoningAlgorithm = pnode->deadReckoning;
			{
				float V[3], A[3];
				// in TCS aka Local
				veccopy3f(V,pnode->linearVelocity.c);
				veccopy3f(A,pnode->linearAcceleration.c);

				//convert Local/TCS linear/angular V,A to world or to Entity, depending on DR parameter
				//http://movesinstitute.org/~mcgredo/MV3500/hla/1278.1-200X%20Draft%2016%20rev%2018.pdf
				//p.333, p.329
				if(pnode->deadReckoning < 6){
					//convert our TCS/Local to world
					//Vgc = tcs2gc x Vtcs 
					Quaternion q;
					vrmlrot4d_to_quaternion(&q,rotate.c);
					quaternion_inverse(&q,&q);
					quaternion_rotation3f(V,&q,V);
					quaternion_rotation3f(A,&q,A);
				} else {
					if(0){
					//convert our TCS/Local to entity
					//Vbody = tcs2body x Vtcs 
					Quaternion q;
					vrmlrot4f_to_quaternion(&q,pnode->rotation.c);
					quaternion_inverse(&q,&q);
					quaternion_rotation3f(V,&q,V);
					quaternion_rotation3f(A,&q,A);
					}else{
					//keep entity in entity
					}
				}
				vec3f2vector3float(&espdu->entityLinearVelocity,V);
				vec3f2vector3float(&espdu->deadReckoningParameters.entityLinearAcceleration,A);

			}
			{
				//p.667 E.7.4.1.1: rotational velocity is stored as axis*angle
				//always wrt entity
				float axis[3];
				vecnormalize3f(axis,pnode->_angularVelocity.c);
				vecscale3f(axis,axis,pnode->_angularVelocity.c[3]);
				vec3f2vector3float(&espdu->deadReckoningParameters.entityAngularVelocity,axis);
			}
			//p.675 E.8.2 Use of Other Parameters for standard algorithms 1 through 9
			switch(pnode->deadReckoning){
				//fixed rotation
				case 1:
				case 2:
				case 5:
				case 6:
				case 9:
				{
					float ypr[3];
					espdu->deadReckoningParameters.otherParameters[0] = 1;
					if(pnode->__geoSystem){
						axisangle2ypr(pnode->rotation.c,ypr); //assume Transform.rotation is wrt TCS/LGS
					}else{
						vecset3f(ypr,0.0f,0.0f,0.0f); //we assume we are in local
					}
					veccopy3f((float*)&espdu->deadReckoningParameters.otherParameters[3],ypr);
				}
				break;
				//rotating
				case 3:
				case 7:
				case 8:
				{
					// p.677 E.8.2.3.2 Issuance of orientation quaternion
					// a 'squished quaternion' 
					Quaternion qglobal;
					double quat4d[4];
					float quat4f[4];
					unsigned int iquat0;
					unsigned short iquat16;
					espdu->deadReckoningParameters.otherParameters[0] = 2;
					if(pnode->__geoSystem){
						//H: W2B = W2L x L2B
						// World2body = world2local x local2body
						// where world2local is the gc2tcs (geocentric to topocentric aka local geodetic system) from geospatial
						// and local2body is the espdu.transform.(translation and rotation) (or its inverse)
						Quaternion qtcs, qlocal;
						struct SFVec3d gc, gd, translate;
						struct SFVec4d rotate;
						Geosys *gs;
						gs = GEOSYS(pnode->__geoSystem);
						user2gc(gs,&pnode->geoCoords,1,&gc);
						//gc2gd(gs,&gc,1,&gd);
						//gc2tcs_transform(gs,&gd,&translate,&rotate);
						gc2tcsB_transform(gs,&gc,&translate,&rotate);
						vrmlrot4d_to_quaternion(&qtcs,rotate.c);
						vrmlrot4f_to_quaternion(&qlocal,pnode->rotation.c);
						quaternion_multiply(&qglobal,&qlocal,&qtcs);
					}else{
						vrmlrot_to_quaternion(&qglobal,0.0,1.0,0.0,0.0); //we've already combined DR with global, so additional DR is zero
					}
					quat2double(quat4d,&qglobal); //w in last slot of quat4d
					double2float(&quat4f[1],quat4d,3);
					quat4f[0] = quat4d[3]; //now w in first slot of quat4f
					if(quat4f[0] < 0.0f)
						vecscale4f(quat4f,quat4f,-1.0f);
				
					iquat0 = (unsigned int)(quat4f[0] * 65536);
					if(quat4f[0] > 65536) iquat0 = 65535;
					iquat16 = iquat0;
					memcpy(&espdu->deadReckoningParameters.otherParameters[1],&iquat16,sizeof(short));
					memcpy(&espdu->deadReckoningParameters.otherParameters[3],&quat4f[1],3*sizeof(float));
				}
				break;
				default:
				espdu->deadReckoningParameters.otherParameters[0] = (char)0;
				break;
			}

		}else{  //geo
			//non-geo scene
			//Apr 22, 2018 we no longer use this, but keeping until benchmark against Brutzman
			//doesn't necessarily make sense to have no geoSystem or geoCoords = 0,0,0
			//but some old/existing scenes are like that, so here we handling them
			//but whether node.translation is meant to be tcs2body or global2body might make a difference?
			//we don't know because we only have freewrl for testing right now - will wait for brutzman
			if(0){
				double local2bodyxyz[3], localxyz[3];
				float2double(local2bodyxyz,pnode->translation.c,3);
				tcs2localswizzled(localxyz,local2bodyxyz);
				vec3d2vector3double(&espdu->entityLocation,localxyz);
			}else{
				espdu->entityLocation.x = pnode->translation.c[0];
				espdu->entityLocation.y = -pnode->translation.c[2]; //??? is this right?
				espdu->entityLocation.z = pnode->translation.c[1];
			}
			//rotation
			if(0){
				//theirs:
				//X PSI
				//Y THETA 
				//Z PHI
				//(x, -z, y)
				//OURS	THEIRS 	THEIRS
				//x		X=x		PSI		
				//y		Z=y		PHI
				//z		-Y=z	-THETA

				Quaternion qA;
				double ypr[3];
				float *c = pnode->rotation.c;
				vrmlrot_to_quaternion(&qA,c[0],c[1],c[2],c[3]);
				quat2euler(ypr,0,&qA);
				espdu->entityOrientation.psi = ypr[1];
				espdu->entityOrientation.theta = ypr[2];
			}
			if(1){
				float ypr[3];
				axisangle2ypr(pnode->rotation.c,ypr);
				espdu->entityOrientation.psi = -ypr[0];  //gimbal.js shows -yaw
				espdu->entityOrientation.theta = ypr[1];
				espdu->entityOrientation.phi = ypr[2];
			}
			//dead reckoning > send
			if(1){
				//first update linear V,A, angularV
				//all of which are in Local/TCS for us
				pnode->_change_count++;
				if(pnode->_change_count > 1){
					double dtime;
					float v1[3], tmp[3], a1[3];
					dtime = TickTime() - pnode->_lastp0time;
					vecscale3f(v1,vecdif3f(tmp,pnode->translation.c,pnode->_lastp0.c),1.0f/dtime);
					//pnode->_change_count = min(pnode->_change_count,2);
					if(pnode->_change_count > 2){
						//a = (v1-v0)/dt
						vecscale3f(a1,vecdif3f(tmp,v1,pnode->linearVelocity.c),1.0f/dtime);
						veccopy3f(pnode->linearAcceleration.c,a1);
						//v1 = v0 - 1/2at**2
					
					}else{
						vecset3f(pnode->linearAcceleration.c,0.0,0.0,0.0);
					}
					veccopy3f(pnode->linearVelocity.c,v1);
					{
						//update angular velocity
						Quaternion qlast,q, qinv, qdif;
						vrmlrot4f_to_quaternion(&qlast,pnode->_lastr0.c);
						vrmlrot4f_to_quaternion(&q,pnode->rotation.c);
						quaternion_inverse(&qinv,&qlast);
						quaternion_multiply(&qdif,&q,&qinv);
						quaternion_to_vrmlrot4f(&qdif,pnode->_angularVelocity.c);
						pnode->_angularVelocity.c[3] *= 1.0f/dtime;
					}
				}
				veccopy3f(pnode->_lastp0.c,pnode->translation.c);
				veccopy4f(pnode->_lastr0.c,pnode->rotation.c);
				pnode->_lastp0time = TickTime();
			}
			espdu->deadReckoningParameters.deadReckoningAlgorithm = pnode->deadReckoning;
			vec3f2vector3float(&espdu->entityLinearVelocity,pnode->linearVelocity.c);
			vec3f2vector3float(&espdu->deadReckoningParameters.entityLinearAcceleration,pnode->linearAcceleration.c);

			{
				//p.667 E.7.4.1.1: rotational velocity is stored as axis*angle
				//always wrt entity
				float axis[3];
				vecnormalize3f(axis,pnode->_angularVelocity.c);
				vecscale3f(axis,axis,pnode->_angularVelocity.c[3]);
				vec3f2vector3float(&espdu->deadReckoningParameters.entityAngularVelocity,axis);
			}
			//p.675 E.8.2 Use of Other Parameters for standard algorithms 1 through 9
			switch(pnode->deadReckoning){
				//fixed rotation
				case 1:
				case 2:
				case 5:
				case 6:
				case 9:
				{
					float ypr[3];
					espdu->deadReckoningParameters.otherParameters[0] = 1;
					if(pnode->__geoSystem){
						axisangle2ypr(pnode->rotation.c,ypr); //assume Transform.rotation is wrt TCS/LGS
					}else{
						vecset3f(ypr,0.0f,0.0f,0.0f); //we assume we are in local
					}
					veccopy3f((float*)&espdu->deadReckoningParameters.otherParameters[3],ypr);
				}
				break;
				//rotating
				case 3:
				case 7:
				case 8:
				{
					// p.677 E.8.2.3.2 Issuance of orientation quaternion
					// a 'squished quaternion' 
					Quaternion qglobal;
					double quat4d[4];
					float quat4f[4];
					unsigned int iquat0;
					unsigned short iquat16;
					espdu->deadReckoningParameters.otherParameters[0] = 2;
					if(pnode->__geoSystem){
						//H: W2B = W2L x L2B
						// World2body = world2local x local2body
						// where world2local is the gc2tcs (geocentric to topocentric aka local geodetic system) from geospatial
						// and local2body is the espdu.transform.(translation and rotation) (or its inverse)
						Quaternion qtcs, qlocal;
						struct SFVec3d gc, gd, translate;
						struct SFVec4d rotate;
						Geosys *gs;
						gs = GEOSYS(pnode->__geoSystem);
						user2gc(gs,&pnode->geoCoords,1,&gc);
						//gc2gd(gs,&gc,1,&gd);
						//gc2tcs_transform(gs,&gd,&translate,&rotate);
						gc2tcsB_transform(gs,&gc,&translate,&rotate);
						vrmlrot4d_to_quaternion(&qtcs,rotate.c);
						vrmlrot4f_to_quaternion(&qlocal,pnode->rotation.c);
						quaternion_multiply(&qglobal,&qlocal,&qtcs);
					}else{
						vrmlrot_to_quaternion(&qglobal,0.0,1.0,0.0,0.0); //we've already combined DR with global, so additional DR is zero
					}
					quat2double(quat4d,&qglobal); //w in last slot of quat4d
					double2float(&quat4f[1],quat4d,3);
					quat4f[0] = quat4d[3]; //now w in first slot of quat4f
					if(quat4f[0] < 0.0f)
						vecscale4f(quat4f,quat4f,-1.0f);
				
					iquat0 = (unsigned int)(quat4f[0] * 65536);
					if(quat4f[0] > 65536) iquat0 = 65535;
					iquat16 = iquat0;
					memcpy(&espdu->deadReckoningParameters.otherParameters[1],&iquat16,sizeof(short));
					memcpy(&espdu->deadReckoningParameters.otherParameters[3],&quat4f[1],3*sizeof(float));
				}
				break;
				default:
				espdu->deadReckoningParameters.otherParameters[0] = (char)0;
				break;
			}
		} //if geo else 
		//vecprint3fb("trans=",pnode->translation.c,"\n");
		pnode->_sent = TRUE;
		//articuation parameters
		if(pnode->articulationParameterArray.n){
			struct ArticulationParameter *ap;
			int i, np = pnode->articulationParameterArray.n;
			ap = malloc(np * sizeof(struct ArticulationParameter));
			espdu->numberOfArticulationParameters = np;
			//printf("sending %d articulation parameters:\n",np);
			for(i=0;i<np;i++){
				ap[i].parameterTypeDesignator = 0; //0 is articulated part
				ap[i].parameterType = 1029; //1024 - rudder + 5 X
				ap[i].parameterValue = pnode->articulationParameterArray.p[i];
				ap[i].partAttachedTo = 0;
				//printf("%d %f\n",i,pnode->articulationParameterArray.p[i]);
			}
			espdu->articulationParameters = (void*)ap;
		}
		node2pdu_entityType(&pnode->entityKind,&espdu->entityType);

		//...
		//printf("new espdu protocol %d type %d\n",espdu->myEntityInformationFamilyPdu.myPdu.protocolVersion,espdu->myEntityInformationFamilyPdu.myPdu.pduType);
		vector_pushBack(struct Pdu*,pdus,(struct Pdu*)espdu);
	}

	//ephemerals / expendables - no hearbeat requirements?
	//FIRE
	if(pnode->_pduchange_fire){
		struct FirePdu *fpdu;
		fpdu = (struct FirePdu *) dis_ctor(type_FirePdu);
		fpdu->myWarfareFamilyPdu.firingEntityID.entity = pnode->entityID;
		fpdu->myWarfareFamilyPdu.firingEntityID.application = pnode->applicationID;
		fpdu->myWarfareFamilyPdu.firingEntityID.site = pnode->siteID;
		//fpdu->myWarfareFamilyPdu.myPdu;
		//fpdu->myWarfareFamilyPdu.targetEntityID;
		printf("FIREPDU ");
		if(pnode->fired1 || pnode->fired2){
			pnode->firedTime = TickTime();
		}
		//copy from espdutransform node to pdu
		fpdu->burstDescriptor.fuse = pnode->fuse;
		struct X3D_EspduTransform * muni = (struct X3D_EspduTransform * )dis_find_registered_node_by_entityid(pnode->munitionEntityID,TRUE,FALSE);
		if(muni){
			fpdu->burstDescriptor.munition.category = muni->entityCategory; //get munition entity from pnode->munitionEntity, then munitionEntity.cateogory.
			fpdu->burstDescriptor.munition.country = muni->entityCountry;
			fpdu->burstDescriptor.munition.domain = muni->entityDomain;
			fpdu->burstDescriptor.munition.entityKind = muni->entityKind;
			fpdu->burstDescriptor.munition.extra = muni->entityExtra;
			fpdu->burstDescriptor.munition.specific = muni->entitySpecific;
			fpdu->burstDescriptor.munition.subcategory = muni->entitySubCategory;
		}
		fpdu->burstDescriptor.quantity = pnode->munitionQuantity;
		fpdu->burstDescriptor.rate = pnode->firingRate;
		fpdu->burstDescriptor.warhead = pnode->warhead;

		fpdu->eventID.application = 0;
		fpdu->eventID.eventNumber = pnode->eventNumber; //dis_next_event_number(); //increment in sender script node
		fpdu->eventID.site = 0; //target if known

		fpdu->fireMissionIndex = dis_next_fire_mission_index();
		{
			double loc[3];
			float2double(loc,pnode->munitionStartPoint.c,3);
			//am I supposed to convert to wworld from local here?
			//or can/should I assume that its local to Weapon Espdu here, and local to target scene's copy of Weapon Espdu?
			vec3d2vector3double(&fpdu->locationInWorldCoordinates,loc);  //is this current location, or starting location?
		}
		//unique munition entity if known
		fpdu->munitionID.application = pnode->munitionApplicationID;
		fpdu->munitionID.entity = pnode->munitionEntityID;
		fpdu->munitionID.site = pnode->munitionSiteID;

		fpdu->range = pnode->firingRange;
		{
			float delta[3];
			vecdif3f(delta,pnode->munitionEndPoint.c,pnode->munitionStartPoint.c);
			//lets say 3 seconds to deliver any munition
			vecscale3f(delta,delta,1.0f/3.0f);
			vec3f2vector3float(&fpdu->velocity,delta);
		}
		vector_pushBack(struct Pdu*,pdus,(struct Pdu*)fpdu);
	}
	//COLLISION
	if(pnode->_pduchange_collision){
		struct CollisionPdu *cpdu;
		cpdu = (struct CollisionPdu *) dis_ctor(type_CollisionPdu);
		//copy from espdutransform node to pdu
		cpdu->issuingEntityID.application = pnode->applicationID;
		cpdu->issuingEntityID.site = pnode->siteID;
		cpdu->issuingEntityID.entity = pnode->entityID;

		cpdu->collidingEntityID.entity = pnode->eventEntityID;
		cpdu->collidingEntityID.application = pnode->eventApplicationID;
		cpdu->collidingEntityID.site = pnode->eventSiteID;
		cpdu->collisionType = pnode->collisionType;
		cpdu->eventID.eventNumber = pnode->eventNumber;

		vector_pushBack(struct Pdu*,pdus,(struct Pdu*)cpdu);
	}
	//DETONATION
	if(pnode->_pduchange_detonation){
		struct DetonationPdu *dpdu;
		dpdu = (struct DetonationPdu *) dis_ctor(type_DetonationPdu);
		//copy from espdutransform node to pdu

		dpdu->myWarfareFamilyPdu.firingEntityID.entity = pnode->entityID;
		dpdu->myWarfareFamilyPdu.firingEntityID.application = pnode->applicationID;
		dpdu->myWarfareFamilyPdu.firingEntityID.site = pnode->siteID;
		//fpdu->myWarfareFamilyPdu.myPdu;
		//fpdu->myWarfareFamilyPdu.targetEntityID;

		pnode->detonateTime = TickTime();
		dpdu->detonationResult = pnode->detonationResult;
		//copy from espdutransform node to pdu
		dpdu->burstDescriptor.fuse = pnode->fuse;
		struct X3D_EspduTransform * muni = (struct X3D_EspduTransform * )dis_find_registered_node_by_entityid(pnode->munitionEntityID,TRUE,FALSE);
		if(muni){
			dpdu->burstDescriptor.munition.category = muni->entityCategory; //get munition entity from pnode->munitionEntity, then munitionEntity.cateogory.
			dpdu->burstDescriptor.munition.country = muni->entityCountry;
			dpdu->burstDescriptor.munition.domain = muni->entityDomain;
			dpdu->burstDescriptor.munition.entityKind = muni->entityKind;
			dpdu->burstDescriptor.munition.extra = muni->entityExtra;
			dpdu->burstDescriptor.munition.specific = muni->entitySpecific;
			dpdu->burstDescriptor.munition.subcategory = muni->entitySubCategory;
		}
		dpdu->burstDescriptor.quantity = pnode->munitionQuantity;
		dpdu->burstDescriptor.rate = pnode->firingRate;
		dpdu->burstDescriptor.warhead = pnode->warhead;

		dpdu->eventID.application = 0;
		dpdu->eventID.eventNumber = pnode->eventNumber; //dis_next_event_number(); //increment in sender script node
		dpdu->eventID.site = 0; //target if known

		//articuation parameters
		if(pnode->articulationParameterArray.n){
			struct ArticulationParameter *ap;
			int i, np = pnode->articulationParameterArray.n;
			ap = malloc(np * sizeof(struct ArticulationParameter));
			dpdu->numberOfArticulationParameters = np;
			//printf("sending %d articulation parameters:\n",np);
			for(i=0;i<np;i++){
				ap[i].parameterTypeDesignator = 0; //0 is articulated part
				ap[i].parameterType = 1029; //1024 - rudder + 5 X
				ap[i].parameterValue = pnode->articulationParameterArray.p[i];
				ap[i].partAttachedTo = 0;
				//printf("%d %f\n",i,pnode->articulationParameterArray.p[i]);
			}
			dpdu->articulationParameters = (void*)ap;
		}

		{
			double loc[3];
			float2double(loc,pnode->detonationLocation.c,3);
			//am I supposed to convert to wworld from local here?
			//or can/should I assume that its local to Weapon Espdu here, and local to target scene's copy of Weapon Espdu?
			vec3d2vector3double(&dpdu->locationInWorldCoordinates,loc);  
			vec3f2vector3float(&dpdu->locationInEntityCoordinates,pnode->detonationRelativeLocation.c);  
		}
		//unique munition entity if known
		dpdu->munitionID.application = pnode->munitionApplicationID;
		dpdu->munitionID.entity = pnode->munitionEntityID;
		dpdu->munitionID.site = pnode->munitionSiteID;

		{
			float delta[3];
			vecdif3f(delta,pnode->munitionEndPoint.c,pnode->munitionStartPoint.c);
			//lets say .5 seconds to detonate any munition
			vecscale3f(delta,delta,1.0f/.5f);
			vec3f2vector3float(&dpdu->velocity,delta);
		}


		vector_pushBack(struct Pdu*,pdus,(struct Pdu*)dpdu);
	}

	return pdus;

}

struct Vector * dis_node2pdus_receiver(struct X3D_Node *node, int isHeartbeat){
	struct Vector *pdus;
	struct X3D_ReceiverPdu * pnode = (struct X3D_ReceiverPdu*)node;
	pdus = newVector(struct Pdu *, 6);
	// Q. what about network sensor>
	// Q. what about _geoCoords?
	if(pnode->_pduchange_receiver){
		struct ReceiverPdu *rpdu;
		rpdu = (struct ReceiverPdu *) dis_ctor(type_ReceiverPdu);
//SFInt32  [in,out] radioID                  0            [0,65535]
//SFFloat  [in,out] receivedPower            0.0          [0,?)
//SFInt32  [in,out] receiverState            0            [0,65535]
//SFInt32  [in,out] transmitterApplicationID 1            [0,65535]
//SFInt32  [in,out] transmitterEntityID      0            [0,65535]
//SFInt32  [in,out] transmitterRadioID       0            [0,65535]
//SFInt32  [in,out] transmitterSiteID        0            [0,65535]
		rpdu->myRadioCommunicationsFamilyPdu.radioId = pnode->radioID;
		rpdu->receivedPoser = pnode->receivedPower;
		rpdu->receiverState = pnode->receiverState;
		rpdu->transmitterRadioId = pnode->transmitterRadioID;
		rpdu->transmitterEntityId.entity = pnode->transmitterEntityID;
		rpdu->transmitterEntityId.site = pnode->transmitterSiteID;
		rpdu->transmitterEntityId.application = pnode->transmitterSiteID;
		vector_pushBack(struct Pdu*,pdus,(struct Pdu*)rpdu);
	}
	return pdus;
}
struct Vector * dis_node2pdus_transmitter(struct X3D_Node *node, int isHeartbeat){
	struct Vector *pdus;
	struct X3D_TransmitterPdu * pnode = (struct X3D_TransmitterPdu*)node;
	pdus = newVector(struct Pdu *, 6);

	// Q. what about network sensor>
	// Q. what about _geoCoords?
	if(pnode->_pduchange_transmitter){
		struct TransmitterPdu *tpdu;
		tpdu = (struct TransmitterPdu *) dis_ctor(type_TransmitterPdu);

  //SFVec3f  [in,out] antennaLocation                    0 0 0        (-8,8)
  //SFInt32  [in,out] antennaPatternLength               0            [0,65535]
  //SFInt32  [in,out] antennaPatternType                 0            [0,65535]
  //SFInt32  [in,out] cryptoKeyID                        0            [0,65535]           
  //SFInt32  [in,out] cryptoSystem                       0            [0,65535]
  //SFInt32  [in,out] frequency                          0      
  //SFInt32  [in,out] inputSource                        0            [0,255]
  //SFInt32  [in,out] lengthOfModulationParameters       0            [0,255]
  //SFInt32  [in,out] modulationTypeDetail               0            [0,65535]
  //SFInt32  [in,out] modulationTypeMajor                0            [0,65535]
  //SFInt32  [in,out] modulationTypeSpreadSpectrum       0            [0,65535]
  //SFInt32  [in,out] modulationTypeSystem               0            [0,65535]
  //SFFloat  [in,out] power                              0.0          [0,8)
  //SFInt32  [in,out] radioEntityTypeCategory            0            [0,255]
  //SFInt32  [in,out] radioEntityTypeCountry             0            [0,65535]
  //SFInt32  [in,out] radioEntityTypeDomain              0            [0,255]
  //SFInt32  [in,out] radioEntityTypeKind                0            [0,255]
  //SFInt32  [in,out] radioEntityTypeNomenclature        0            [0,255]
  //SFInt32  [in,out] radioEntityTypeNomenclatureVersion 0            [0,65535]
  //SFInt32  [in,out] radioID                            0            [0,255]
  //SFVec3f  [in,out] relativeAntennaLocation            0 0 0        (-8,8)
  //SFFloat  [in,out] transmitFrequencyBandwidth         0.0          (-8,8)
  //SFInt32  [in,out] transmitState                      0            [0,255]
		{
			double loc[3];
			float2double(loc,pnode->antennaLocation.c,3);
			vec3d2vector3double(&tpdu->antennaLocation,loc);
			vec3f2vector3float(&tpdu->relativeAntennaLocation,pnode->relativeAntennaLocation.c);
		}
		tpdu->antennaPatternCount = pnode->antennaPatternLength;
		tpdu->antennaPatternType = pnode->antennaPatternType;
		tpdu->cryptoKeyId = pnode->cryptoKeyID;
		tpdu->cryptoSystem = pnode->cryptoSystem;
		tpdu->frequency = pnode->frequency;
		tpdu->inputSource = pnode->inputSource;
		tpdu->modulationType.detail = pnode->modulationTypeDetail;
		tpdu->modulationType.major = pnode->modulationTypeMajor;
		tpdu->modulationType.spreadSpectrum = pnode->modulationTypeSpreadSpectrum;
		tpdu->modulationType.system = pnode->modulationTypeSystem;
		// memcpy(tpdu->modulationParametersList, ????) we have no field for it
		tpdu->modulationParameterCount = pnode->lengthOfModulationParameters; //==0 since no field for parameters
		tpdu->power = pnode->power;
		tpdu->radioEntityType.category = pnode->radioEntityTypeCategory;
		tpdu->radioEntityType.country = pnode->radioEntityTypeCountry;
		tpdu->radioEntityType.domain = pnode->radioEntityTypeDomain;
		tpdu->radioEntityType.entityKind = pnode->radioEntityTypeKind;
		tpdu->radioEntityType.nomenclature = pnode->radioEntityTypeNomenclature;
		tpdu->radioEntityType.nomenclatureVersion = pnode->radioEntityTypeNomenclatureVersion;
		tpdu->myRadioCommunicationsFamilyPdu.radioId = pnode->radioID;
		tpdu->transmitFrequencyBandwidth = pnode->transmitFrequencyBandwidth;
		tpdu->transmitState = pnode->transmitState;

		vector_pushBack(struct Pdu*,pdus,(struct Pdu*)tpdu);
	}
	return pdus;
}
#define ONE_INT32_PER_SIGNAL_DATA_BYTE TRUE
struct Vector * dis_node2pdus_signal(struct X3D_Node *node, int isHeartbeat){
	struct Vector *pdus;
	struct X3D_SignalPdu * pnode = (struct X3D_SignalPdu*)node;
	pdus = newVector(struct Pdu *, 6);

	// Q. what about network sensor>
	// Q. what about _geoCoords?
	if(pnode->_pduchange_signal){
		struct SignalPdu *spdu;
		spdu = (struct SignalPdu *) dis_ctor(type_SignalPdu);

  //MFInt32  [in,out] data               []           [0,255]                  
  //SFInt32  [in,out] dataLength         0            [0,65535]
  //SFInt32  [in,out] encodingScheme     0            [0,65535]
  //SFInt32  [in,out] radioID            0            [0,65535]
  //SFInt32  [in,out] sampleRate         0            [0,65535]
  //SFInt32  [in,out] samples            0            [0,65535]
  //SFInt32  [in,out] tdlType            0            [0,65535]
		spdu->data = realloc(spdu->data,pnode->dataLength*sizeof(unsigned char));
  		if(ONE_INT32_PER_SIGNAL_DATA_BYTE){
			int k;
			unsigned char *cdata = (unsigned char *)spdu->data;
			for(k=0;k<spdu->dataLength;k++){
				cdata[k] = (unsigned char)((unsigned int)pnode->data.p[k] % 256);
			}
		}else{
			memcpy(spdu->data,pnode->data.p,pnode->dataLength);
			spdu->dataLength = pnode->dataLength;
		}
		spdu->encodingScheme = pnode->encodingScheme;
		spdu->sampleRate = pnode->sampleRate;
		spdu->samples = pnode->samples;
		spdu->tdlType = pnode->tdlType;
		spdu->myRadioCommunicationsFamilyPdu.radioId = pnode->radioID;
		spdu->myRadioCommunicationsFamilyPdu.entityId.entity = pnode->entityID;
		spdu->myRadioCommunicationsFamilyPdu.entityId.site = pnode->siteID;
		spdu->myRadioCommunicationsFamilyPdu.entityId.application = pnode->applicationID;
		vector_pushBack(struct Pdu*,pdus,(struct Pdu*)spdu);
	}
	return pdus;
}

// for incoming pdus, we tag them as we handle them
// in pdu->padding
enum {
	TAG_UNCLAIMED = 0,
	TAG_SAME_PROGRAM = 1,
	TAG_ESPDU = 2,
	TAG_ENTITY_MANAGER = 3,
	TAG_RECEIVER = 4,
	TAG_TRANSMITTER = 5,
	TAG_SIGNAL = 6,
	TAG_AVATAR = 7,
	TAG_SENSOR = 8,
};
struct X3D_Node *dis_find_or_create_espdu_by_category(int kind, int domain, int country, int category, int subcategory, int specific, int extra);
int dis_pdus2node_espdu(struct X3D_Node *node, struct Vector *pdus){
	int i, ihit;
	struct Pdu* pdu;
	struct X3D_EspduTransform * pnode = (struct X3D_EspduTransform*)node;

	ihit = 0;
	if(!pdus) return ihit;
	for(i=0;i<pdus->n;i++)
	{
		pdu = vector_get(struct Pdu*,pdus,i);
		if(pdu->padding == TAG_UNCLAIMED)
		switch(pdu->pduType){
			case PDU_ENTITY_STATE:
			{
				//ENTITYSTATE
				struct EntityStatePdu *espdu;
				espdu = (struct EntityStatePdu*)pdu;
				// 2018.pdf 6.2.80.3 Application Number implied an instance number, 
				// so should not be same unless loopback testing
				//if (espdu->entityID.application == pnode->applicationID
				//	&& espdu->entityID.site == pnode->siteID) {
				if (espdu->entityID.application == fwl_get_DISapplication()
					&& espdu->entityID.site == fwl_get_DISsite()) {
						pdu->padding = TAG_SAME_PROGRAM;
					break;
				}
				int avatar = FALSE;
				if (espdu->entityType.category == 77) avatar = TRUE;
				//	break; //its an avatar, handled elsewhere
				if(espdu->entityID.entity != pnode->entityID) break;
				ihit++;
				pdu->padding = avatar ? TAG_AVATAR : TAG_ESPDU;
				pnode->_change++; //mark node changed
				pnode->timestamp = TickTime();

				if(pnode->__geoSystem){
					Quaternion qgc2tcs, qtcs2body, qgc2body;
					struct SFVec3d gd, gc, translate;
					struct SFVec4d rotate;
					double localxyz[3], tcsxyz[3], tcs2bodyxyz[3], world2bodyxyz[3];
					float xyza[4];
					Geosys *gs;
					gs = GEOSYS(pnode->__geoSystem);
					user2gc(gs,&pnode->geoCoords,1,&gc);
					//gc2gd(gs,&gc,1,&gd);
					//gc2tcs_transform(gs,&gd,&translate,&rotate);
					gc2tcsB_transform(gs,&gc,&translate,&rotate);
					//somehow get body/entity into world/gc - rotation and translation
					{
						//rotation
						//assumption (Apr 2018 don't know how Xj3d does it, here's dug9's guess):
						// pdu is world2body
						// espdutransform.rotation = local2body
						// - where local is TCS Topocentric Coord System as described for GeoLocation
						// - and world is GC
						// local2body = world2local.inverse x world2body
						// for freewrl world2local is gc2tcs
						Quaternion qtcs2gc, q;
						float ypr[3], xyza[4];
						ypr[0] = -espdu->entityOrientation.psi;
						ypr[1] = espdu->entityOrientation.theta;
						ypr[2] = espdu->entityOrientation.phi;
						ypr2axisangle(ypr,xyza);
						xyza[3] = -xyza[3];
						//vecprint4fb("recv xyza",xyza,"\n");

						vrmlrot4f_to_quaternion(&qgc2body,xyza);
						vrmlrot4d_to_quaternion(&qgc2tcs,rotate.c);
						quaternion_inverse(&qtcs2gc,&qgc2tcs);
						quaternion_multiply(&qtcs2body,&qtcs2gc,&qgc2body);
						quaternion_set(&q,&qtcs2body);
						quaternion_to_vrmlrot4f(&q,pnode->rotation.c);
					}
					{
						//translation - dug9 debate: could do it one of 2 ways
						enum transmethod {
							TRANS_ZERO = 1,
							TRANS_LOCATION_MINUS_GEOCOORD = 2
						};
						//static int transmethod = TRANS_LOCATION_MINUS_GEOCOORD; 
						static int transmethod = TRANS_ZERO; 

						vector3double2vec3d(world2bodyxyz,&espdu->entityLocation);
						if(transmethod == TRANS_LOCATION_MINUS_GEOCOORD){
							//METHOD 1: translation = Location - geoCoords
							struct SFVec3d world, tcs;
							veccopyd(world.c,world2bodyxyz);
							//gc2tcs(gs,&gd,&world,1,&tcs);
							gc2tcsB(gs,&gc,&world,1,&tcs);
							double2float(pnode->translation.c,tcs.c,3);
						}else{
							//TRANS_ZERO
							//METHOD 2: geoCoords = Location; translation = 000
							// x smoothing doesn't work if done in translation / tcs space
							struct SFVec3d world, tcs2, tcs1;
							double deltatcs[3];
							float deltap[3];
							static int want_smoothing = 1;
							if(want_smoothing){
								//gc2tcs(gs,&gd,&gc,1,&tcs1);
								gc2tcsB(gs,&gc,&gc,1,&tcs1);
								veccopyd(world.c,world2bodyxyz);
								gc2tcs(gs,&gd,&world,1,&tcs2);
								gc2tcsB(gs,&gc,&world,1,&tcs2);
								vecdifd(deltatcs,tcs1.c,tcs2.c);
								double2float(deltap,deltatcs,3);
								vecadd3f(pnode->_p0.c,pnode->_p0.c,deltap);
							}
							gc2user(gs,&world,1,&pnode->geoCoords);
							//node->_change++;
							vecset3f(pnode->translation.c,0.0f,0.0f,0.0f);

						}
					}
					// recv geo dead reckoning
					pnode->deadReckoning = espdu->deadReckoningParameters.deadReckoningAlgorithm;
					{
						float V[3], A[3];
						// in entity or world, depending on drmethod
						vector3float2vec3f(A,&espdu->deadReckoningParameters.entityLinearAcceleration);
						vector3float2vec3f(V,&espdu->entityLinearVelocity);

						//convert linear/angular V,A from world or Entity, to local
						//http://movesinstitute.org/~mcgredo/MV3500/hla/1278.1-200X%20Draft%2016%20rev%2018.pdf
						//p.333, p.329
						if(pnode->deadReckoning < 6){
							//convert world to TCS/Local 
							//vtcs = gc2tcs x Vworld
							Quaternion q;
							vrmlrot4d_to_quaternion(&q,rotate.c);
							quaternion_rotation3f(V,&q,V);
							quaternion_rotation3f(A,&q,A);
						} else {
							if(0){
							//convert entity to TCS/Local
							//Vtcs = body2tcs x Vbody
							Quaternion q;
							vrmlrot4f_to_quaternion(&q,pnode->rotation.c);
							quaternion_rotation3f(V,&q,V);
							quaternion_rotation3f(A,&q,A);
							}else{
							//keep entity in entity
							}
						}

						veccopy3f(pnode->linearAcceleration.c,A);
						veccopy3f(pnode->linearVelocity.c,V);

					}
					{
						//p.667 E.7.4.1.1: rotational velocity is stored as axis*angle, in entity space
						float axis[3], angle;
						vector3float2vec3f(axis,&espdu->deadReckoningParameters.entityAngularVelocity);
						angle = veclength3f(axis);
						vecnormalize3f(pnode->_angularVelocity.c,axis);
						pnode->_angularVelocity.c[3] = angle;
					}
					if (disverbose() && pdu->padding == TAG_AVATAR) {
						printf("Avatar S%2d A%2d T %lf\r", espdu->entityID.site, espdu->entityID.application,TickTime());
					}
				}else{
					//non-geosystem scene. Apr 22, 2018 we aren't using this now
					// -- everything goes through geosystem code above
					// -- but keeping this until we benchmark against Brutzman
					//translation - assumes companion scenes will have same parent transform stack
					//(x, -z, y).
					
					pnode->translation.c[0] = espdu->entityLocation.x;
					pnode->translation.c[1] = espdu->entityLocation.z;
					pnode->translation.c[2] = -espdu->entityLocation.y; 
					//rotation
					if(0){
						Quaternion qA;
						float ypr[3];
						double r[4];
						float *c = pnode->rotation.c;
						ypr[0] = espdu->entityOrientation.phi;
						ypr[1] = espdu->entityOrientation.psi;
						ypr[2] = espdu->entityOrientation.theta;
						euler2quat(&qA,ypr[0],ypr[1],ypr[2]);
						//quaternion_normalize(&qA);
						//vrmlrot_to_quaternion(&qA,c[0],c[1],c[2],c[3]);
						quaternion_to_vrmlrot(&qA,&r[0],&r[1],&r[2],&r[3]);
						c[0] = (float)r[0];
						c[1] = (float)r[1];
						c[2] = (float)r[2];
						c[3] = (float)r[3];
					}
					if(1){
						float ypr[3];
						ypr[0] = -espdu->entityOrientation.psi;  //gimbal.js shows -yaw
						ypr[1] = espdu->entityOrientation.theta;
						ypr[2] = espdu->entityOrientation.phi;
						ypr2axisangle(ypr,pnode->rotation.c);
					}
					// dead reckoning
					pnode->deadReckoning = espdu->deadReckoningParameters.deadReckoningAlgorithm;
					vector3float2vec3f(pnode->linearAcceleration.c,&espdu->deadReckoningParameters.entityLinearAcceleration);
					vector3float2vec3f(pnode->linearVelocity.c,&espdu->entityLinearVelocity);
					{
						//p.667 E.7.4.1.1: rotational velocity is stored as axis*angle
						float axis[3], angle;
						vector3float2vec3f(axis,&espdu->deadReckoningParameters.entityAngularVelocity);
						angle = veclength3f(axis);
						vecnormalize3f(pnode->_angularVelocity.c,axis);
						pnode->_angularVelocity.c[3] = angle;
					}

				}
				//articuation parameters
				pnode->articulationParameterArray.n = espdu->numberOfArticulationParameters;
				//printf("recv art count %d\n",espdu->numberOfArticulationParameters);
				if(pnode->articulationParameterArray.n){
					struct ArticulationParameter *ap;
					float *pp;
					int i, np = pnode->articulationParameterArray.n;
					ap = espdu->articulationParameters;
					pp = malloc(np * sizeof(float));
					//printf("received %d articulation parameters:\n",np);
					for(i=0;i<np;i++){
						//ap[i].parameterTypeDesignator = 0; //0 is articulated part
						//ap[i].parameterType = 1029; //1024 - rudder + 5 X
						pp[i] = (float)ap[i].parameterValue;
						//printf("%d %f\n",i,pp[i]);
						//ap[i].partAttachedTo = 0;
						switch(i){
							case 0: pnode->articulationParameterValue0_changed = pp[i]; break;
							case 1: pnode->articulationParameterValue1_changed = pp[i]; break;
							case 2: pnode->articulationParameterValue2_changed = pp[i]; break;
							case 3: pnode->articulationParameterValue3_changed = pp[i]; break;
							case 4: pnode->articulationParameterValue4_changed = pp[i]; break;
							case 5: pnode->articulationParameterValue5_changed = pp[i]; break;
							case 6: pnode->articulationParameterValue6_changed = pp[i]; break;
							case 7: pnode->articulationParameterValue7_changed = pp[i]; break;
							default:
							break;
						}
					}
					if(pnode->articulationParameterArray.p) free(pnode->articulationParameterArray.p);
					pnode->articulationParameterArray.p = pp;
					//done in generic mark_changed_fields //MARK_EVENT(X3D_NODE(pnode),offsetof(struct X3D_EspduTransform,articulationParameterArray));
				}
				pdu2node_entityType(&espdu->entityType,&pnode->entityKind);
				pnode->_pduchange_es = TRUE;
				if(espdu->entityAppearance | 1 << 20){
					//http://movesinstitute.org/~mcgredo/MV3500/hla/1278.1-200X%20Draft%2016%20rev%2018.pdf
					//p.50 no dead reckoning if isFrozen bit is set, bit 21 of entityAppearance
					//(why can't they just leave dead reckoning parameters 0, and run through formula? H: specs written in 1990s for 80386 processors)
					//pnode->_isFrozen = TRUE; //pduchange_es = FALSE;
				}

				//...
			}
			break;
			case PDU_FIRE:
			{
				//FIRE
				struct FirePdu *fpdu;
				fpdu = (struct FirePdu*)pdu;
				if(fpdu->myWarfareFamilyPdu.firingEntityID.application != pnode->applicationID) break;
				if(fpdu->myWarfareFamilyPdu.firingEntityID.site != pnode->siteID) break;
				if(fpdu->myWarfareFamilyPdu.firingEntityID.entity != pnode->entityID) break;

				//fpdu->myWarfareFamilyPdu.myPdu;
				//fpdu->myWarfareFamilyPdu.targetEntityID;

				
				ihit++;
				pnode->_change++; //mark node changed
				pnode->timestamp = TickTime();
				
				//if(pnode->fired1 || pnode->fired2){
					pnode->firedTime = TickTime();
				//}
				pnode->fired1 = TRUE;
				//copy from espdutransform node to pdu
				pnode->fuse = fpdu->burstDescriptor.fuse;
				//pnode->munitionEntityID = fpdu->burstDescriptor.munition.
				//the following doesn't work in target scene
				struct X3D_EspduTransform * muni;
				//  kind, domain, country, category, subcategory, specific, extra
				muni = (struct X3D_EspduTransform * )dis_find_or_create_espdu_by_category(
					fpdu->burstDescriptor.munition.entityKind,
					fpdu->burstDescriptor.munition.domain,
					fpdu->burstDescriptor.munition.country,
					fpdu->burstDescriptor.munition.category,
					fpdu->burstDescriptor.munition.subcategory,
					fpdu->burstDescriptor.munition.specific,
					fpdu->burstDescriptor.munition.extra
					);
				if(!muni) printf("no muni\n");
				else {
					printf("got muni\n");
					pnode->munitionEntityID = muni->entityID;
					pnode->munitionSiteID = muni->siteID;
					pnode->munitionApplicationID = muni->applicationID;
				}
				pnode->munitionQuantity = fpdu->burstDescriptor.quantity;
				pnode->firingRate = fpdu->burstDescriptor.rate;
				pnode->warhead = fpdu->burstDescriptor.warhead;

				//fpdu->eventID.application = 0;
				pnode->eventNumber = fpdu->eventID.eventNumber; //dis_next_event_number(); //increment in sender script node
				//fpdu->eventID.site = 0; //target if known

				pnode->fireMissionIndex = fpdu->fireMissionIndex;
				{
					double loc[3];
					//am I supposed to convert to wworld from local here?
					//or can/should I assume that its local to Weapon Espdu here, and local to target scene's copy of Weapon Espdu?
					vector3double2vec3d(loc,&fpdu->locationInWorldCoordinates);  //is this current location, or starting location?
					double2float(pnode->munitionStartPoint.c,loc,3);
				}
				//unique munition entity if known
				pnode->munitionApplicationID = fpdu->munitionID.application;
				pnode->munitionEntityID = fpdu->munitionID.entity;
				pnode->munitionSiteID = fpdu->munitionID.site;

				//fpdu->myWarfareFamilyPdu.firingEntityID;
				//fpdu->myWarfareFamilyPdu.myPdu;
				//fpdu->myWarfareFamilyPdu.targetEntityID;
				pnode->firingRange = fpdu->range;
				{
					float delta[3];
					vector3float2vec3f(delta,&fpdu->velocity);
					//lets say 3 seconds to deliver any munition
					vecscale3f(delta,delta,3.0f/1.0f);
					vecadd3f(pnode->munitionEndPoint.c,pnode->munitionStartPoint.c,delta);
				}
				pnode->_pduchange_fire = TRUE;
			}
			break;
			case PDU_COLLISION:
			{
				//COLLISION
				struct CollisionPdu *cpdu;
				cpdu = (struct CollisionPdu *)pdu;

				if(cpdu->issuingEntityID.application != pnode->applicationID) break;
				if(cpdu->issuingEntityID.site != pnode->siteID) break;
				if(cpdu->issuingEntityID.entity != pnode->entityID) break;
				pnode->eventEntityID = cpdu->collidingEntityID.entity;
				pnode->eventApplicationID = cpdu->collidingEntityID.application;
				pnode->eventSiteID = cpdu->collidingEntityID.site;
				pnode->collisionType = cpdu->collisionType;
				if(pnode->collisionType) pnode->isCollided = TRUE;
				else pnode->isCollided = FALSE;
				//cpdu->eventID;
				//cpdu->location;
				//cpdu->mass;
				//cpdu->myEntityInformationFamilyPdu.myPdu.exerciseID;
				//cpdu->velocity;
				pnode->_pduchange_collision = TRUE;
			}
			break;
			case PDU_DETONATION:
			{
				//DETONATION
				struct DetonationPdu *dpdu;
				dpdu = (struct DetonationPdu*)pdu;
				if(dpdu->myWarfareFamilyPdu.firingEntityID.application != pnode->applicationID) break;
				if(dpdu->myWarfareFamilyPdu.firingEntityID.site != pnode->siteID) break;
				if(dpdu->myWarfareFamilyPdu.firingEntityID.entity != pnode->entityID) break;

				ihit++;
				pnode->_change++; //mark node changed
				pnode->timestamp = TickTime();

				pnode->detonateTime = TickTime();
				pnode->fuse = dpdu->burstDescriptor.fuse;
				//pnode->munitionEntityID = fpdu->burstDescriptor.munition.
				//the following doesn't work in target scene
				struct X3D_EspduTransform * muni;
				//  kind, domain, country, category, subcategory, specific, extra
				muni = (struct X3D_EspduTransform * )dis_find_or_create_espdu_by_category(
					dpdu->burstDescriptor.munition.entityKind,
					dpdu->burstDescriptor.munition.domain,
					dpdu->burstDescriptor.munition.country,
					dpdu->burstDescriptor.munition.category,
					dpdu->burstDescriptor.munition.subcategory,
					dpdu->burstDescriptor.munition.specific,
					dpdu->burstDescriptor.munition.extra
					);
				if(!muni) printf("no muni\n");
				else {
					printf("got muni\n");
					pnode->munitionEntityID = muni->entityID;
					pnode->munitionSiteID = muni->siteID;
					pnode->munitionApplicationID = muni->applicationID;
				}
				pnode->munitionQuantity = dpdu->burstDescriptor.quantity;
				pnode->firingRate = dpdu->burstDescriptor.rate;
				pnode->warhead = dpdu->burstDescriptor.warhead;

				//fpdu->eventID.application = 0;
				pnode->eventNumber = dpdu->eventID.eventNumber; //dis_next_event_number(); //increment in sender script node
				//unique munition entity if known
				pnode->munitionApplicationID = dpdu->munitionID.application;
				pnode->munitionEntityID = dpdu->munitionID.entity;
				pnode->munitionSiteID = dpdu->munitionID.site;
				{
					double loc[3];
					vector3double2vec3d(loc,&dpdu->locationInWorldCoordinates);
					double2float(pnode->detonationLocation.c,loc,3);
					vector3float2vec3f(pnode->detonationRelativeLocation.c,&dpdu->locationInEntityCoordinates);
				}
				pnode->detonationResult = dpdu->detonationResult;

				{
					float delta[3];
					vector3float2vec3f(delta,&dpdu->velocity);
					//lets say .5 seconds to explode a munition
					vecscale3f(delta,delta,.5f/1.0f);
					veccopy3f(pnode->munitionStartPoint.c,pnode->detonationRelativeLocation.c);
					vecadd3f(pnode->munitionEndPoint.c,pnode->munitionStartPoint.c,delta);
				}
				//articuation parameters
				pnode->articulationParameterArray.n = dpdu->numberOfArticulationParameters;
				//printf("recv art count %d\n",espdu->numberOfArticulationParameters);
				if(pnode->articulationParameterArray.n){
					struct ArticulationParameter *ap;
					float *pp;
					int i, np = pnode->articulationParameterArray.n;
					ap = dpdu->articulationParameters;
					pp = malloc(np * sizeof(float));
					//printf("received %d articulation parameters:\n",np);
					for(i=0;i<np;i++){
						//ap[i].parameterTypeDesignator = 0; //0 is articulated part
						//ap[i].parameterType = 1029; //1024 - rudder + 5 X
						pp[i] = (float)ap[i].parameterValue;
						//printf("%d %f\n",i,pp[i]);
						//ap[i].partAttachedTo = 0;
						switch(i){
							case 0: pnode->articulationParameterValue0_changed = pp[i]; break;
							case 1: pnode->articulationParameterValue1_changed = pp[i]; break;
							case 2: pnode->articulationParameterValue2_changed = pp[i]; break;
							case 3: pnode->articulationParameterValue3_changed = pp[i]; break;
							case 4: pnode->articulationParameterValue4_changed = pp[i]; break;
							case 5: pnode->articulationParameterValue5_changed = pp[i]; break;
							case 6: pnode->articulationParameterValue6_changed = pp[i]; break;
							case 7: pnode->articulationParameterValue7_changed = pp[i]; break;
							default:
							break;
						}
					}
					if(pnode->articulationParameterArray.p) free(pnode->articulationParameterArray.p);
					pnode->articulationParameterArray.p = pp;
					//done in generic mark_changed_fields //MARK_EVENT(X3D_NODE(pnode),offsetof(struct X3D_EspduTransform,articulationParameterArray));
				}
				pnode->_pduchange_detonation = TRUE;

			}
			break;
			default:
				break;
		}
	}
	return ihit;
}

int dis_pdus2node_receiver(struct X3D_Node *node, struct Vector *pdus){
	int i, ihit;
	struct Pdu* pdu;
	struct X3D_ReceiverPdu * pnode = (struct X3D_ReceiverPdu*)node;

	ihit = 0;
	if(!pdus) return ihit;
	for(i=0;i<pdus->n;i++)
	{
		pdu = vector_get(struct Pdu*,pdus,i);
		if(pdu->padding == TAG_UNCLAIMED)
		switch(pdu->pduType){
			case PDU_RECEIVER:
			{
				struct ReceiverPdu *rpdu;
				rpdu = (struct ReceiverPdu*)pdu;

				if(pnode->radioID != rpdu->myRadioCommunicationsFamilyPdu.radioId) break;
				ihit++;
				pdu->padding = TAG_RECEIVER;
				pnode->_change++; //mark node changed
				pnode->timestamp = TickTime();

				pnode->receivedPower = rpdu->receivedPoser; //spelling Poser / Power
				pnode->receiverState = rpdu->receiverState;
				pnode->transmitterRadioID = rpdu->transmitterRadioId;
				pnode->transmitterEntityID = rpdu->transmitterEntityId.entity;
				pnode->transmitterSiteID = rpdu->transmitterEntityId.site;
				pnode->transmitterSiteID = rpdu->transmitterEntityId.application;
				pnode->_pduchange_receiver = TRUE;
			}
			break;
			default:
				break;
		}
	}
	return ihit;
}
int dis_pdus2node_transmitter(struct X3D_Node *node, struct Vector *pdus){
	int i, ihit;
	struct Pdu* pdu;
	struct X3D_TransmitterPdu * pnode = (struct X3D_TransmitterPdu*)node;

	ihit = 0;
	if(!pdus) return ihit;
	for(i=0;i<pdus->n;i++)
	{
		pdu = vector_get(struct Pdu*,pdus,i);
		if(pdu->padding == TAG_UNCLAIMED)
		switch(pdu->pduType){
			case PDU_TRANSMITTER:
			{
				struct TransmitterPdu *tpdu;
				tpdu = (struct TransmitterPdu*)pdu;

				if(pnode->radioID != tpdu->myRadioCommunicationsFamilyPdu.radioId) break;
				if(tpdu->myRadioCommunicationsFamilyPdu.entityId.entity != pnode->entityID) break;
				//if(tpdu->myRadioCommunicationsFamilyPdu.entityId.site != pnode->siteID) break;
				//site.application should not be the same unless loopback testing
				//if(tpdu->myRadioCommunicationsFamilyPdu.entityId.application == pnode->applicationID) break;

				ihit++;
				pdu->padding = TAG_TRANSMITTER;
				pnode->_change++; //mark node changed
				pnode->timestamp = TickTime();
				{
					double loc[3];
					vector3double2vec3d(loc,&tpdu->antennaLocation);
					double2float(pnode->antennaLocation.c,loc,3);
					vector3float2vec3f(pnode->relativeAntennaLocation.c,&tpdu->relativeAntennaLocation);
				}
				pnode->antennaPatternLength = tpdu->antennaPatternCount;
				pnode->antennaPatternType = tpdu->antennaPatternType;
				pnode->cryptoKeyID = tpdu->cryptoKeyId;
				pnode->cryptoSystem = tpdu->cryptoSystem;
				pnode->frequency = tpdu->frequency;
				pnode->inputSource = tpdu->inputSource;
				pnode->modulationTypeDetail = tpdu->modulationType.detail;
				pnode->modulationTypeMajor = tpdu->modulationType.major;
				pnode->modulationTypeSpreadSpectrum = tpdu->modulationType.spreadSpectrum;
				pnode->modulationTypeSystem = tpdu->modulationType.system;
				// memcpy(tpdu->modulationParametersList, ????) we have no field for it
				pnode->lengthOfModulationParameters = tpdu->modulationParameterCount; //==0 since no field for parameters
				pnode->power = tpdu->power;
				pnode->radioEntityTypeCategory = tpdu->radioEntityType.category;
				pnode->radioEntityTypeCountry = tpdu->radioEntityType.country;
				pnode->radioEntityTypeDomain = tpdu->radioEntityType.domain;
				pnode->radioEntityTypeKind = tpdu->radioEntityType.entityKind;
				pnode->radioEntityTypeNomenclature = tpdu->radioEntityType.nomenclature;
				pnode->radioEntityTypeNomenclatureVersion = tpdu->radioEntityType.nomenclatureVersion;
				pnode->radioID = tpdu->myRadioCommunicationsFamilyPdu.radioId;
				pnode->transmitFrequencyBandwidth = tpdu->transmitFrequencyBandwidth;
				pnode->transmitState = tpdu->transmitState;
				pnode->_pduchange_transmitter = TRUE;

			}
			break;
			default:
				break;
		}
	}
	return ihit;
}
// #define ONE_INT32_PER_SIGNAL_DATA_BYTE TRUE
int dis_pdus2node_signal(struct X3D_Node *node, struct Vector *pdus){
	int i, ihit;
	struct Pdu* pdu;
	struct X3D_SignalPdu * pnode = (struct X3D_SignalPdu*)node;

	ihit = 0;
	if(!pdus) return ihit;
	for(i=0;i<pdus->n;i++)
	{
		pdu = vector_get(struct Pdu*,pdus,i);
		if(pdu->padding == TAG_UNCLAIMED)
		switch(pdu->pduType){
			case PDU_SIGNAL:
			{
				struct SignalPdu *spdu;
				spdu = (struct SignalPdu*)pdu;

				if(pnode->radioID != spdu->myRadioCommunicationsFamilyPdu.radioId) break;
				if(spdu->myRadioCommunicationsFamilyPdu.entityId.entity != pnode->entityID) break;
				//site won't necessarily be the same
				//if(spdu->myRadioCommunicationsFamilyPdu.entityId.site != pnode->siteID) break;
				//site.application should not be the same unless loopback testing
				//if(spdu->myRadioCommunicationsFamilyPdu.entityId.application == pnode->applicationID) break;

				ihit++;
				pdu->padding = TAG_SIGNAL;
				pnode->_change++; //mark node changed
				pnode->timestamp = TickTime();
				if(ONE_INT32_PER_SIGNAL_DATA_BYTE){
					int k;
					unsigned char *cdata = (unsigned char *)spdu->data;
					pnode->data.p = realloc(pnode->data.p,spdu->dataLength*sizeof(int));
					for(k=0;k<spdu->dataLength;k++){
						pnode->data.p[k] = (int)(cdata[k]);
					}
				}else{
					memcpy(pnode->data.p,spdu->data,pnode->dataLength);
					pnode->data.n = (spdu->dataLength + 4) / 4;
				}
				pnode->dataLength = spdu->dataLength;
				pnode->encodingScheme = spdu->encodingScheme;
				pnode->sampleRate = spdu->sampleRate;
				pnode->samples = spdu->samples;
				pnode->tdlType = spdu->tdlType;
				pnode->_pduchange_signal = TRUE;
			}
			break;
			default:
				break;
		}
	}
	return ihit;
}



// Simulation Management PDUs relate to the DISEntityManager node
// http://movesinstitute.org/~mcgredo/MV3500/hla/1278.1-200X%20Draft%2016%20rev%2018.pdf x dead link see tests/28_DIS for .pdf copy
//5.6 Simulation management p.85
//6.2.82 Simulation Management PDU Header record p.311
//- its an abstract type
//- the pdutype burried in the standard header part is the implied ACTION. ie create, or remove.
//Table 114 p.313:
//PDU 				Reference 	Originating ID 	Receiving ID
//Create Entity 	5.6.5.2 	Simulation ID 	Entity ID or Special Create Entity Identifier
//Remove Entity 	5.6.5.3 	Simulation ID 	Entity ID
//Table 12 p.87
//Table 3 p.31 - IDs, including specials: All Simulations, no particular node: ALL_SITES 65535, ALL_APPLIC = 65535,  RefID 0
//5.6.5.2 Create Entity PDU p.88 


struct Vector * dis_node2pdus_sm(struct X3D_Node *node, int isHeartbeat){

	struct Vector *pdus;
	struct X3D_DISEntityManager * pnode = (struct X3D_DISEntityManager*)node;
	pdus = newVector(struct Pdu *, 6);

	//ENTITYSTATE
	//if(pnode->_pduchange_es_articulation || pnode->_pduchange_es_deadreckoning || pnode->_pduchange_es_info || pnode->_pduchange_es_force){
	printf("em pduchange create %d remove %d heartbeat %d ticktime %lf\n",pnode->_pduchange_create, pnode->_pduchange_remove, isHeartbeat,TickTime());
	if(isHeartbeat){
		//lets say someone joins the exercise late.
		//how do they get synched up?
	}
	{
		struct SimulationManagementPdu *simanpdu;
	}
	//CREATE
	if(pnode->_pduchange_create){
		struct CreateEntityPdu *crpdu;
		crpdu = (struct CreateEntityPdu *) dis_ctor(type_CreateEntityPdu);
		//copy from espdutransform node to pdu
		crpdu->mySimulationManagementFamilyPdu.originatingEntityID.entity = pnode->entityID;
		//crpdu->mySimulationManagementFamilyPdu.receivingEntityID = ALL_SITES; ???
		//crpdu->requestID = ??
		///crpdu->mySimulationManagementFamilyPdu.myPdu.
		vector_pushBack(struct Pdu*,pdus,(struct Pdu*)crpdu);
	}
	//REMOVE
	if(pnode->_pduchange_remove){
		struct RemoveEntityPdu *rmpdu;
		rmpdu = (struct RemoveEntityPdu *) dis_ctor(type_RemoveEntityPdu);
		//copy from espdutransform node to pdu
		vector_pushBack(struct Pdu*,pdus,(struct Pdu*)rmpdu);
	}
	return pdus;

}
int dis_pdus2node_sm(struct X3D_Node *node, struct Vector *pdus){
	int i, ihit;
	struct Pdu* pdu;
	struct X3D_DISEntityManager * pnode = (struct X3D_DISEntityManager*)node;

	ihit = 0;
	if(!pdus) return ihit;
	for(i=0;i<pdus->n;i++)
	{
		pdu = vector_get(struct Pdu*,pdus,i);
		switch(pdu->pduType){
			case PDU_CREATE_ENTITY:
			{
				//CREATE
				struct CreateEntityPdu *crpdu;
				//crpdu->mySimulationManagementFamilyPdu.myPdu.
				printf("hi from pdu2node create_entity\n");
				pdu->padding = TAG_ENTITY_MANAGER;
				ihit++;
				pnode->_pduchange_create = TRUE;
			}
			break;
			case PDU_REMOVE_ENTITY:
			{
				//REMOVE
				struct RemoveEntityPdu *rmpdu;
				printf("hi from pdu2node remove_entity\n");
				pdu->padding = TAG_ENTITY_MANAGER;
				ihit++;
				pnode->_pduchange_remove = TRUE;
			}
			break;

			default:
				break;
		}
	}
	return ihit;
}
void dis_set_node_lasttime(struct X3D_Node *node, double lasttime){
	//4 nodes have the same field order for common fields, can be cast to Espdu 
	switch(node->_nodeType){
		case NODE_ReceiverPdu:
		case NODE_TransmitterPdu:
		case NODE_SignalPdu:
		case NODE_EspduTransform:
		case NODE_DISEntityManager:
		{
			struct X3D_EspduTransform *pnode = (struct X3D_EspduTransform*)node;
			pnode->_lasttime = lasttime;
		}
		break;
		break;
	}
}

int dis_pdus2newnode(struct dis_socket *dsock, struct X3D_DISEntityManager *pnode, struct Vector * pdus){
	int ihit = 0;
	if(pnode){
		int i;
		// http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/dis.html#DISEntityManager
		// https://github.com/open-dis/DISTutorial/blob/master/EntityDiscovery.md
		// entity discovery happening here
		struct Pdu* pdu;
		for(i=0;i<pdus->n;i++) {
			pdu = vector_get(struct Pdu*,pdus,i);
			if(pdu->padding == TAG_UNCLAIMED)
			switch(pdu->pduType){
				case PDU_ENTITY_STATE:
				case PDU_RECEIVER:
				case PDU_TRANSMITTER:
				case PDU_SIGNAL:
				case PDU_ENTITY_STATE_UPDATE:
				{
					int j, already_done;
					int entityID, siteID, applicationID;
					struct EntityStatePdu *espdu;
					struct X3D_EspduTransform* et;
					espdu = (struct EntityStatePdu*)pdu;
					
					//don't send to yourself
					//if (pnode->applicationID == espdu->entityID.application &&
					//	pnode->siteID == espdu->entityID.site) {
					if (espdu->entityID.application == fwl_get_DISapplication() &&
						espdu->entityID.site == fwl_get_DISsite()) {
						pdu->padding = TAG_SAME_PROGRAM;
						//ihit++;
						break;
					}
					//skip if we already got this entity and are just awaiting creation
					already_done = FALSE;
					//printf("addEntities.n = %d\n", pnode->addEntities.n);
					//printf("espdu entity %d app %d site %d\n", espdu->entityID.entity, espdu->entityID.application, espdu->entityID.site);
					for(j=0;j<pnode->addEntities.n;j++){
						struct X3D_Node *candi = (struct X3D_Node*)pnode->addEntities.p[j];
						if(candi->_nodeType == NODE_DISEntityTypeMapping){
							struct X3D_DISEntityTypeMapping *et = (struct X3D_DISEntityTypeMapping *)candi;
							//already_done = FALSE;
						}else if(candi->_nodeType == NODE_EspduTransform) {
							// || candi->_nodeType == NODE_ReceiverPdu 
							// || candi->_nodeType == NODE_TransmitterPdu || candi->_nodeType == NODE_SignalPdu){
							//else if radio etc
							struct X3D_EspduTransform *et = (struct X3D_EspduTransform *)candi;
							if(et->entityID == espdu->entityID.entity &&
								et->applicationID == espdu->entityID.application &&
								et->siteID == espdu->entityID.site) already_done = TRUE;
							//printf("addEntities[%d] entity %d app %d site %d already %d\n", 
							//	j,et->entityID, et->applicationID, et->siteID, already_done);
							if(already_done) break;
						}

					}
					if (already_done) {
						//printf("already done\n");
						break;
					}
					//printf("not already done\n");
					pdu->padding = TAG_ENTITY_MANAGER;
					ihit++;
					//we'll use an EspduTransform struct just as a temp struct, not to register.
					// -for the purpose of communicating with whatever can create a local copy
					//  of a discovered entity.
					// right now, that's our EntityManager node.
					et = createNewX3DNode0(NODE_EspduTransform); //the 0 creator which does not register the node
					int nodetype = 0;
					switch(pdu->pduType){
						case PDU_ENTITY_STATE: nodetype = NODE_EspduTransform; break;
						case PDU_RECEIVER: nodetype = NODE_ReceiverPdu; break;
						case PDU_TRANSMITTER: nodetype = NODE_TransmitterPdu; break;
						case PDU_SIGNAL: nodetype = NODE_SignalPdu; break;
						case PDU_ENTITY_STATE_UPDATE: nodetype = NODE_EspduTransform; break;
						default: break;
					}
					//copy world coordinates as approx GC, in case < earths radius / 2 (earths core) test later, we use GC instead of default GD,WE
					vector3double2vec3d(et->geoCoords.c,&espdu->entityLocation);

					et->_nodeType = nodetype;
					et->applicationID = espdu->entityID.application;
					et->siteID = espdu->entityID.site;
					et->entityID = espdu->entityID.entity;
					et->address = newASCIIString(dsock->address);
					et->port = dsock->port - fwl_get_testset();
					et->multicastRelayHost = newASCIIString(dsock->multicastRelayHost);
					et->multicastRelayPort = dsock->multicastRelayPort;
					et->entityCategory = espdu->entityType.category;
					et->entityCountry = espdu->entityType.country;
					et->entityDomain = espdu->entityType.domain;
					et->entityKind = espdu->entityType.entityKind;
					et->entityExtra = espdu->entityType.extra;
					et->entitySpecific = espdu->entityType.specific;
					et->entitySubCategory = espdu->entityType.subcategory;
					
					{
						void * pp = pnode->addEntities.p;
						pnode->addEntities.p = realloc(pp,sizeof(struct X3D_Node*)*upper_power_of_two(pnode->addEntities.n + 1));
						pnode->addEntities.p[pnode->addEntities.n] = (struct X3D_Node*)et;
						pnode->addEntities.n++;
						// >> do I need pnode->_pduchange_create = TRUE;
					}
					// ?? do I need MARK_EVENT(X3D_NODE(pnode),offsetof (struct X3D_DISEntityManager,  addEntities));
					//will get mapped and instanced as geom during entityManager scenegraph visit and compile
					// >> pnode->_change ++;
					//ihit = 1;
				}
				break;
				default:
				break;
			}
		}
	}
	return ihit;
}
int dis_entity_retire(struct X3D_DISEntityManager *pnode, struct X3D_Node *node){
	//we only retire the entities that were created by 'entity_discovery'
	int iret = 0;
	if(node->_nodeType == NODE_EspduTransform){
		if(pnode && pnode->_nodeType == NODE_DISEntityManager){
			int i;
			static int ADD = 1, REMOVE = 2;
			iret = -1;
			for(i=0;i<pnode->entities.n;i++){
				if(pnode->entities.p[i] == node){
					//yes - created by entity discovery
					AddRemoveChildren(X3D_NODE(pnode),  &pnode->entities, (struct X3D_Node * *)&node, 1, REMOVE,__FILE__,__LINE__);
					AddRemoveChildren(X3D_NODE(pnode),  &pnode->removedEntities, (struct X3D_Node * *)&node, 1, ADD,__FILE__,__LINE__);

					iret = 1;
					break;
				}
			}
		}
	}
	return iret;
}
struct Vector * dis_node2pdus(struct X3D_Node *node, int isHeartbeat){
	struct Vector *pdus = NULL;
	switch(node->_nodeType){
		case NODE_EspduTransform:
			pdus = dis_node2pdus_espdu(node, isHeartbeat);
			break;
		case NODE_DISEntityManager:
			pdus = dis_node2pdus_sm(node,isHeartbeat);
			break;
		case NODE_ReceiverPdu:
			pdus = dis_node2pdus_receiver(node,isHeartbeat);
			break;
		case NODE_TransmitterPdu:
			pdus = dis_node2pdus_transmitter(node,isHeartbeat);
			break;
		case NODE_SignalPdu:
			pdus = dis_node2pdus_signal(node,isHeartbeat);
			break;
		break;
	}
	return pdus;
}
static struct Vector *sockets_send = NULL;
static struct Vector *sockets_recv = NULL;

struct X3D_Node * dis_find_registered_node_by_entityid(int entityid, int sendlist, int recvlist){
	int i,j;
	struct X3D_Node *node, *pnode = NULL;
	if(sendlist)
	for(i=0;i<sockets_send->n;i++){
		struct dis_socket *dsock = vector_get_ptr(struct dis_socket,sockets_send,i);
		if(dsock->registered){
			for(j=0;j<dsock->registered->n;j++){
				int ihit;
				struct X3D_Node *node = vector_get(struct X3D_Node*,dsock->registered,j);
				if(node->_nodeType == NODE_EspduTransform){
					struct X3D_EspduTransform *espdu = (struct X3D_EspduTransform *)node;
					if(espdu->entityID == entityid){
						pnode = node;
						break;
					}
				}
			}
		}
	}
	if(recvlist)
	for(i=0;i<sockets_recv->n;i++){
		struct dis_socket *dsock = vector_get_ptr(struct dis_socket,sockets_recv,i);
		if(dsock->registered){
			for(j=0;j<dsock->registered->n;j++){
				int ihit;
				struct X3D_Node *node = vector_get(struct X3D_Node*,dsock->registered,j);
				if(node->_nodeType == NODE_EspduTransform){
					struct X3D_EspduTransform *espdu = (struct X3D_EspduTransform *)node;
					if(espdu->entityID == entityid){
						pnode = node;
						break;
					}
				}
			}
		}
	}
	return pnode;
}


				//  kind, domain, country, category, subcategory, specific, extra
struct X3D_Node *dis_find_or_create_espdu_by_category(int kind, int domain, int country, int category, int subcategory, int specific, int extra){
	int i,j,k, ibest, iscore;
	struct X3D_EspduTransform *best = NULL;
	ibest = -1;
	iscore = 0;
	//check EM already-instanced list
	for(i=0;i<sockets_recv->n;i++){
		struct dis_socket *dsock = vector_get_ptr(struct dis_socket,sockets_recv,i);
		if(dsock->registered){
			for(j=0;j<dsock->registered->n;j++){
				int ihit;
				struct X3D_Node *node = vector_get(struct X3D_Node*,dsock->registered,j);
				if(node->_nodeType == NODE_DISEntityManager){
					struct X3D_DISEntityManager *em = (struct X3D_DISEntityManager *)node;
					if(em->entities.n){
						for(k=0;k<em->entities.n;k++){
							struct X3D_EspduTransform *bnode = (struct X3D_EspduTransform *)em->entities.p[k];
							if(bnode->_nodeType == NODE_EspduTransform){
								int jscore = 0;
								if(domain == bnode->entityDomain) jscore++;
								if(category == bnode->entityCategory) jscore++;
								if(country == bnode->entityCountry) jscore++;
								if(kind == bnode->entityKind) jscore++;
								if(extra == bnode->entityExtra) jscore++;
								if(subcategory == bnode->entitySubCategory) jscore++;
								if(specific == bnode->entitySpecific) jscore++;
								if(jscore > iscore){
									iscore = jscore;
									ibest = i;
									best = bnode;
								}

							}
						}
					}
				}
			}
		}
	}
	return (struct X3D_Node*)best;
}


unsigned char buf2[32768];

void dis_get_node_lasttime(struct X3D_Node *node, double *lasttime, double *readInterval, double *writeInterval){
	//4 nodes have the same field order for common fields, can be cast to Espdu 
	switch(node->_nodeType){
		case NODE_ReceiverPdu:
		case NODE_TransmitterPdu:
		case NODE_SignalPdu:
		case NODE_EspduTransform:
		case NODE_DISEntityManager:
		{
			struct X3D_EspduTransform *pnode = (struct X3D_EspduTransform*)node;
			*lasttime = pnode->_lasttime;
			*writeInterval = pnode->writeInterval;
			*readInterval = pnode->readInterval;
		}
		break;
		default:
		break;
	}
}
int node_only_transform_changed(struct X3D_Node *node){
	int changed, onlytransform = FALSE;
	changed = 0;
	if(	node->_nodeType == NODE_EspduTransform)
	{
		struct X3D_EspduTransform *pnode = (struct X3D_EspduTransform *)node;
		changed += pnode->_pduchange_es ? 1 : 0;
		changed += pnode->_pduchange_collision ? 2:0;
		changed += pnode->_pduchange_fire ? 4:0;
		changed += pnode->_pduchange_detonation ? 8:0;
	}
	onlytransform = changed == 1;
	return onlytransform;
}


int node_pdus_changed_by_scene(struct X3D_Node *node){
	int changed = FALSE;
	switch(node->_nodeType){
		case NODE_EspduTransform:
			{
			struct X3D_EspduTransform *pnode = (struct X3D_EspduTransform *)node;
			changed = pnode->_pduchange_es;
			changed |= pnode->_pduchange_collision;
			changed |= pnode->_pduchange_fire;
			changed |= pnode->_pduchange_detonation;
			}
			break;
		case NODE_DISEntityManager:
			{
			struct X3D_DISEntityManager *pnode = (struct X3D_DISEntityManager *)node;
			changed = pnode->_pduchange_create;
			changed |= pnode->_pduchange_remove;
			}
			break;
		case NODE_TransmitterPdu:
			{
			struct X3D_TransmitterPdu *pnode = (struct X3D_TransmitterPdu *)node;
			changed = pnode->_pduchange_transmitter;
			}
			break;
		case NODE_SignalPdu:
			{
			struct X3D_SignalPdu *pnode = (struct X3D_SignalPdu *)node;
			changed = pnode->_pduchange_signal;
			}
			break;
		case NODE_ReceiverPdu:
			{
			struct X3D_ReceiverPdu *pnode = (struct X3D_ReceiverPdu *)node;
			changed = pnode->_pduchange_receiver;
			}
			break;
		default:
			break;
	}
	return changed;
}
void reset_node_pduchanged(struct X3D_Node *node){
	switch(node->_nodeType){
		case NODE_EspduTransform:
			{
			struct X3D_EspduTransform *pnode = (struct X3D_EspduTransform *)node;
			pnode->_pduchange_es = FALSE;
			pnode->_pduchange_collision = FALSE;
			pnode->_pduchange_fire = FALSE;
			pnode->_pduchange_detonation = FALSE;
			}
			break;
		case NODE_DISEntityManager:
			{
			struct X3D_DISEntityManager *pnode = (struct X3D_DISEntityManager *)node;
			pnode->_pduchange_em_info = FALSE;
			pnode->_pduchange_create = FALSE;
			pnode->_pduchange_remove = FALSE;
			}
			break;
		case NODE_TransmitterPdu:
			{
			struct X3D_TransmitterPdu *pnode = (struct X3D_TransmitterPdu *)node;
			pnode->_pduchange_transmitter = FALSE;
			}
			break;
		case NODE_SignalPdu:
			{
			struct X3D_SignalPdu *pnode = (struct X3D_SignalPdu *)node;
			pnode->_pduchange_signal = FALSE;
			}
			break;
		case NODE_ReceiverPdu:
			{
			struct X3D_ReceiverPdu *pnode = (struct X3D_ReceiverPdu *)node;
			pnode->_pduchange_receiver = FALSE;
			}
			break;
		default:
			break;
	}
}
//in socketutils.c:
void socket_open(struct dis_socket *dsock);
int sockwrite(SOCKET s, const char *buf, int len);
int sockread(SOCKET s, const char *buf, int len);
int sockrecvfrom(struct dis_socket *dsock, const char *buf, int len);
int socksendto(struct dis_socket *dsock, const char *buf, int len);

// https://stackoverflow.com/questions/2351087/what-is-the-best-32bit-hash-function-for-short-strings-tag-names 
// hash: compute hash value of string 
#define MULTIPLIER 37
unsigned int hash37(const char* str)
{
	unsigned int h;
	unsigned char* p;

	h = 0;
	for (p = (unsigned char*)str; *p != '\0'; p++)
		h = MULTIPLIER * h + *p;
	return h; // or, h % ARRAY_SIZE; in our case we want it spread over long int 4B so no/rare collisions
}
const char* getNodeName(struct X3D_Node* node);

void dis_recv_sensor(int sensorIndex, int ev, int butStatus2, int status, float* posn3, float* norm3);
struct dis_sensor {
	int fromNode, dataNode;
	int ev, butStatus2;
	int status, padding;
	float posn3[3], norm3[3];
};
static struct Vector* sensor_send_queue = NULL; //reset .n to 0 after pdu2buf
static int dis_verbose = FALSE; // TRUE; //just for a receiver, not for sender
int disverbose() {
	return dis_verbose;
}
struct Vector* dis_sensors2pdus() {
// 2023 multiplayer experiment: sensor event sharing
	//converts queued sensor events into a CommentPdu for sending
	struct Vector* pdus = NULL;
	if (sensor_send_queue && vectorSize(sensor_send_queue)) {
		int nevents = vectorSize(sensor_send_queue); //garbage collect after send
		pdus = newVector(struct Pdu*, 1);
		struct CommentPdu* cpdu;
		cpdu = (struct CommentPdu*)dis_ctor(type_CommentPdu); //garbage collect after send
		vector_pushBack(struct Pdu*, pdus, (struct Pdu*)cpdu);
		//entity
		cpdu->mySimulationManagementFamilyPdu.originatingEntityID.entity = 33; //can be a code for sensors
		cpdu->mySimulationManagementFamilyPdu.originatingEntityID.application = fwl_get_DISapplication();
		cpdu->mySimulationManagementFamilyPdu.originatingEntityID.site = fwl_get_DISsite();
		struct VariableDatum* vd;
		vd = malloc(nevents * sizeof(struct VariableDatum)); //free this after pdu2buff
		for (int i = 0; i < nevents; i++) {
			struct dis_sensor* ds = vector_get_ptr(struct dis_sensor, sensor_send_queue, i);
			vd[i].variableDatumID = i;
			vd[i].variableDatumLength = 6;
			vd[i].variableDatums = (void*)ds; //don't free this
		}
		cpdu->variableDatums = vd;
		cpdu->numberOfVariableDatumRecords = nevents;
		//printf("send+");
	}
	return pdus;
}
void clear_sensor_queue() {
	if(sensor_send_queue) sensor_send_queue->n = 0;
}
void dis_send_sensor(struct X3D_Node* fromNode, struct X3D_Node* dataNode, int ev, int butStatus2, 
	int status, float* posn3, float* norm3) {
	//receives sensor event information from mainloop sendSensorEvents on event
	// and queues for sensor2pdu
	// called from Mainloop.c SendSensorEvents about line 6901
	if (allow_DIS) {
		//enqueue for sending on next dis_send_loop
		if (!sensor_send_queue) sensor_send_queue = newStack(struct dis_sensor);
		struct dis_sensor ds;
		ds.ev = ev;
		ds.butStatus2 = butStatus2;
		ds.status = status;
		veccopy3f(ds.posn3, posn3);
		veccopy3f(ds.norm3, norm3);
		//node addresses may be different in different app instances
		//so we rely on DEF name
		//but DEF name can be a medium long string, not good for pdu transmission
		//so we convert to 4 byte int with a hash function
		int OK = TRUE;
		const char* def = getNodeName(fromNode);
		if (!def) {
			printf("No DEF name for from nodetype %s", stringNodeType(fromNode->_nodeType));
			OK = FALSE;
		}else
			ds.fromNode = hash37(def == NULL ? "" : def);
		//printf("send_fromnode def %s has %d ", def, ds.fromNode);
		def = getNodeName(dataNode);
		if (!def) {
			printf("No DEF name for data nodetype %s", stringNodeType(fromNode->_nodeType));
			OK = FALSE;
		}else
			ds.dataNode = hash37(def);
		//printf("send_datanode def %s has %d \n", def, ds.dataNode);
		if(OK)
			stack_push(struct dis_sensor, sensor_send_queue, ds);
	}
}

//struct Vector* sensors = NULL; //2023 multiplayer
//void dis_registerSensor(struct X3D_Node* sensor) {
//	//2023 multiplayer
//	//call this from somewhere we handle sensor events
//	if (!sensors) sensors = newVector(struct X3D_Node*, 10);
//	for (int i = 0; i < sensors->n; i++)
//		if (sensor == vector_get(struct X3D_Node*, sensors, i)) return; //already registered
//	vector_pushBack(struct X3D_Node*, sensors, sensor);
//}
int getSensorCount();
void getSensor(int k, struct X3D_Node** fromnode, struct X3D_Node** datanode);

int dis_pdus2sensors(struct Vector* pdus) {
	//2023 multiplayer
	//one sensor update per pdu, or as many as we like?
	//we need DEF or ID that's consistent across application instances
	//- how about a hash, good for going one way, can't go back
	//- so will need a list of registered sensors to compare hash(DEF) with .entity
	int i, ihit;
	struct Pdu* pdu;

	ihit = 0;
	if (!pdus || !pdus->n) return ihit;
	for (i = 0; i < pdus->n; i++)
	{
		pdu = vector_get(struct Pdu*, pdus, i);
		if (pdu->padding == TAG_UNCLAIMED)
			switch (pdu->pduType) {
			case PDU_COMMENT:
			{
				struct CommentPdu* cpdu;
				cpdu = (struct CommentPdu*)pdu;
				//don't loopback
				if (cpdu->mySimulationManagementFamilyPdu.originatingEntityID.application == fwl_get_DISapplication()
					&& cpdu->mySimulationManagementFamilyPdu.originatingEntityID.site == fwl_get_DISsite()) {
					pdu->padding = TAG_SAME_PROGRAM;
					//if (disverbose()) printf("SNDR ");
					ihit++;
					break;
				}
				if (disverbose()) {
					printf("%s ", "COM");
					printf("A%2d ", cpdu->mySimulationManagementFamilyPdu.originatingEntityID.application);
					printf("S%2d ", cpdu->mySimulationManagementFamilyPdu.originatingEntityID.site);
				}

				//find matching sensor
				struct VariableDatum* vr = cpdu->variableDatums;
				if (disverbose()) printf("ND%2d ", cpdu->numberOfVariableDatumRecords);
				//printf("vd count %d\n", cpdu->numberOfVariableDatumRecords);
				for (int j = 0; j < cpdu->numberOfVariableDatumRecords; j++) {
					struct dis_sensor* ds = (struct dis_sensor*)vr[j].variableDatums;
					//printf("ds-fromnode %d ds-datanode %d\n", ds->fromNode, ds->dataNode);

					int nsensor = getSensorCount();
					//printf("nsensor %d\n", nsensor);
					if (disverbose()) printf("NS%2d ", nsensor);
					for (int k = 0; k < nsensor; k++) {
						struct X3D_Node* fromnode, * datanode;
						getSensor(k, &fromnode, &datanode);
						//rather than sending and receiving null terminted DEF strings, we'll use 32 bit int hash values
						const char *deff, *defd;
						deff = getNodeName(fromnode);
						int fromNode, dataNode, OK;
						OK = TRUE;
						if (!deff) OK = FALSE;
						else fromNode = hash37(deff);
						//printf("recv fromnode def %s hash %d ", def, fromNode);
						defd = getNodeName(datanode);
						if (!defd) OK = FALSE;
						else dataNode = hash37(defd);

						//printf("recv datanode def %s hash %d\n", def, dataNode);
						int match = OK && ds->fromNode == fromNode && ds->dataNode == dataNode;
						if (match) {
							dis_recv_sensor(k, ds->ev, ds->butStatus2, ds->status, ds->posn3, ds->norm3);
							//printf("recvmatch+");
							if(disverbose()) printf("F %s D %s B%d S%d ", deff, defd,ds->butStatus2,ds->status);
							break;
						}
					}
				}
				pdu->padding = TAG_SENSOR;
				ihit++;
				//printf("recv+");
				if (disverbose()) printf("\n");
				break;
			}
			default:
				break;
			}
	}
	return ihit;
}
static double last_avatar_position[3] = { 0,0,0 };
static double last_avatar_orientation[4] = { 0,0,0,0 };
static double avatar_writeInterval = 4.0;
struct Vector* dis_avatar2pdus() {
	// 2023 multiplayer experiment: sensor event sharing
	struct Vector* pdus = NULL;
	if (1) {
		//assuming DIS update loop is called from scene root level, 
		//then modelview should be (at least last frame's) view matrix
		//viewmatrix vs vp.position/.orientation: viewmatrix is viewpoint agnostic and in world coords.
		double viewMatrix[16], matinv[16], xyza[4], pointd[3], diff[4];
		float xyzaf[4], ypr[3];
		viewer_getview(viewMatrix);
		vecsetd(pointd, 0.0, 0.0, 0.0);
		matinverseAFFINE(matinv, viewMatrix);

		transformAFFINEd(pointd, pointd, matinv);
		//vecscaled(pointd, pointd, -1.0);
		AFFINEmatrix2axisangled(xyza, viewMatrix);
		int changed = veclengthd(vecdifd(diff, pointd, last_avatar_position)) > .001 ? 1 : 0;
		changed = changed || veclengthd(vecdifd(diff, xyza, last_avatar_orientation)) > .001 ? 1 : 0;
		changed = changed || abs(xyza[3] - last_avatar_orientation[3] > .001) ? 1 : 0;
		static double last_time = 0.0;
		if (last_time == 0.0) last_time = TickTime() - avatar_writeInterval;
		double this_time = TickTime();
		int heartbeat = FALSE;
		if (this_time - last_time > avatar_writeInterval) heartbeat = TRUE;
		if (!changed && ! heartbeat) {
			return pdus;
		}
		last_time = this_time;

		veccopyd(last_avatar_position, pointd);
		veccopyd(last_avatar_orientation, xyza);
		last_avatar_orientation[3] = xyza[3];
		pdus = newVector(struct Pdu*, 6);
		struct EntityStatePdu* espdu;
		espdu = (struct EntityStatePdu*)dis_ctor(type_EntityStatePdu);
		static int icount = 0;
		if(0) printf("Pdu->type = %d i %d\n", ((struct Pdu*)(espdu))->pduType,icount);
		icount++;
		//entity
		espdu->entityType.category = 77; //SPECIAL CATEGORY 77 FOR AVATARS
		espdu->entityID.entity = fwl_get_DISapplication(); // application 1:1 avatar, entity = f(application)
		espdu->entityID.application = fwl_get_DISapplication();
		espdu->entityType.specific = fwl_get_DISapplication();
		espdu->entityID.site = fwl_get_DISsite();
		if(0) printf("kind %d domain %d country %d category %d sub %d specific %d extra %d\n ",
			espdu->entityType.entityKind, espdu->entityType.domain, espdu->entityType.country,
			espdu->entityType.category, espdu->entityType.subcategory,
			espdu->entityType.specific, espdu->entityType.extra);

		espdu->entityLocation.x = pointd[0];
		espdu->entityLocation.y = pointd[1];
		espdu->entityLocation.z = pointd[2];
		double2float(xyzaf, xyza, 4);
		xyzaf[3] = -xyzaf[3];
		axisangle2ypr(xyzaf, ypr);
		espdu->entityOrientation.psi = -ypr[0];  //gimbal.js shows -yaw
		espdu->entityOrientation.theta = ypr[1];
		espdu->entityOrientation.phi = ypr[2];
		//printf("send xyz %lf %lf %lf ypr %f %f %f\n", pointd[0], pointd[1], pointd[2], ypr[0], ypr[1], ypr[2]);
		stack_push(struct Pdu*, pdus, (struct Pdu*)espdu);
	}

	return pdus;
}
int write_rtp(unsigned char *buf, struct X3D_Node *node);
void dis_sendloop(){
	double thistime;
	int i,j, nbytes, nb;
	if(!sockets_send || sockets_send->n == 0) return;
	thistime = TickTime();
	for(i=0;i<sockets_send->n;i++){
		struct dis_socket *dsock = vector_get_ptr(struct dis_socket,sockets_send,i);
		nbytes = 0;
		struct Vector* pdus;

		if(dsock->registered){
			for(j=0;j<dsock->registered->n;j++){
				//options:
				//a. each node maintains its own pdus every frame on update/compile, and are merely sent here
				//b. on send in here, a function is called to pdu-ize a node before marshaling it
				//c. like a and b: each node has its own list of pdus for mem, and are updated in here just before send
				double lasttime, dtime, readInterval, writeInterval, isHeartbeat;
				struct X3D_Node *node = vector_get(struct X3D_Node*,dsock->registered,j);
				//printf("registered node type %s\n",stringNodeType(node->_nodeType));
				dis_get_node_lasttime(node,&lasttime,&readInterval,&writeInterval);
				if(writeInterval == 0.0) continue; //sentinal value 0 means don't write
				// finer granularity send decision: a)heartbeat, b)on-change except DR, c) DR dead-reckoning-threshold exceded
				// Q. where's our c)dead-reckoning-threshold?
				dtime = thistime - lasttime;
				isHeartbeat = dtime > writeInterval ? TRUE: FALSE; //a)heartbeat: skip for a while more
				if(!isHeartbeat && !node_pdus_changed_by_scene(node)) continue; //b)on-change: no pdus changed since last send, DIS ettiquette says don't send if no change
				//if(0) //moved to node _compile/_prep/_child
				//if(!isHeartbeat && node_only_transform_changed(node)){
				//	if(transform_within_DeadReckoningTolerance(node,dtime)) continue;
				//}
				printf(".\n");
				lasttime = thistime;
				dsock->lasttime = thistime; //last time something was sent, not needed
				if(j==0) {
					nb = write_rtp(&buf2[nbytes],node);
					nbytes += nb;
				}

				//option b.
				pdus = dis_node2pdus(node,isHeartbeat);
				if(isHeartbeat)
					dis_set_node_lasttime(node,lasttime);

				if(pdus && pdus->n) {
					struct Pdu* pdu = vector_get(struct Pdu*,pdus,0);
					//printf("in dis_sendloop pdu protocol %d pdutype %d\n",pdu->protocolVersion,pdu->pduType);
				}
				nb = dis_write_stream(&buf2[nbytes],pdus);
				if(0){
					//debug
					printf("sendloop >>>>\n");
					print_stream(&buf2[nbytes], nb);
					printf("<<<< sendloop\n");
				}
				nbytes += nb;
				reset_node_pduchanged(node);
			}
		}
		pdus = dis_sensors2pdus();
		nb = dis_write_stream(&buf2[nbytes], pdus);
		clear_sensor_queue();
		nbytes += nb;
		pdus = dis_avatar2pdus();
		nb = dis_write_stream(&buf2[nbytes], pdus);
		nbytes += nb;
		if (nbytes) socksendto(dsock, buf2, nbytes);
		// Q. where is the garbage collection for pdua vector?
		// I think the ctor/dtor for pdus works on the pdus themselves. 
		// But not the pdu vector list I pass around
	}
}
/*	RTP Real-time Transport Protocol
	optional header that can be on incoming, or put on outgoing
	https://en.wikipedia.org/wiki/Real-time_Transport_Protocol
	https://calhoun.nps.edu/bitstream/handle/10945/9147/virtualrealitytr00afon.pdf
	appendix F, G show DIS header settings and source code for DIS X3D
		Version = 2 (default)
		M marker = 0
		CC csrc_count = 0
		P padding = 0
		X extension_bit = 0 (no extension header used)
		PT payloadType = 111
*/
struct rtp_header {
	unsigned char toprow[2];
	unsigned short sequence;
	unsigned int timestamp;
	unsigned int ssrc;
};
unsigned char * rtp_strip_header(unsigned char *buf, int *heard){
	unsigned char *carat = buf;
	//DIS protocoVersion is 6 (1998) or 7 (2009/2012)
	//so if the first byte is bigger than that it might be RTP
	*heard = FALSE;
	if(carat[0] > 127) {
		//its an RTP header - strip
		struct rtp_header rtph;
		int cc, i, x;
		*heard = TRUE;
		memcpy(&rtph.toprow,carat,2);
		carat += 2;
		memcpy(&rtph.sequence,carat,2);
		carat += 2;
		memcpy(&rtph.timestamp,carat,4);
		carat += 4;
		memcpy(&rtph.ssrc,carat,4);
		carat += 4;
		cc = rtph.toprow[0] << 4 >> 4;
		x = rtph.toprow[0] & 1 << 4;
		for(i=0;i<cc;i++){
			//skip scrc identifiers
			carat += 4;
		}
		if(x){
			//not implemented: extension header skipping
		}
	}
	//in the future, we could do something fancy with the sequence number, like skip stale packets.
	return carat;
}
unsigned char * rtp_add_header(unsigned char *buf, unsigned int timestamp){
	struct rtp_header rtph;
	unsigned char PayloadType;
	unsigned char *carat = buf;
	PayloadType = 111;
	memset(&rtph,0,sizeof(struct rtp_header));
	rtph.toprow[0] = 2 << 7 | 0 << 6 | 0 << 5 | 0;
	rtph.toprow[1] = 0 << 7 | PayloadType;
	rtph.sequence = htons(0);
	rtph.timestamp = htonl(timestamp);
	rtph.ssrc = 0;
	memcpy(carat,&rtph.toprow,2);
	carat += 2;
	memcpy(carat,&rtph.sequence,2);
	carat += 2;
	memcpy(carat,&rtph.timestamp,4);
	carat += 4;
	memcpy(carat,&rtph.ssrc,4);
	carat += 4;
	return carat;
}
int write_rtp(unsigned char *buf, struct X3D_Node *node){
	int nb = 0;
	int rtue = FALSE;
	switch(node->_nodeType){
		case NODE_EspduTransform:
			rtue = ((struct X3D_EspduTransform *)node)->rtpHeaderExpected;
			break;
		case NODE_DISEntityManager:
			rtue = ((struct X3D_DISEntityManager *)node)->rtpHeaderExpected;
			break;
		case NODE_ReceiverPdu:
			rtue = ((struct X3D_ReceiverPdu *)node)->rtpHeaderExpected;
			break;
		case NODE_TransmitterPdu:
			rtue = ((struct X3D_TransmitterPdu *)node)->rtpHeaderExpected;
			break;
		case NODE_SignalPdu:
			rtue = ((struct X3D_SignalPdu *)node)->rtpHeaderExpected;
			break;
		default: 
			break;
	}
	if(rtue) {
		unsigned char *carat;
		unsigned int hours, timestamp;
		TickTime2DISTime(TickTime(),1,&hours,&timestamp);
		carat =  rtp_add_header(buf,timestamp);
		nb = carat - buf;
	}
	return nb;
}
void set_rtp_heard(struct X3D_Node *node){
	//would be nice if we had mulitple inheritance techniques for extracting common interfaces 
	// dis = interface(node,type_DIS)
	// dis->isRtpHeaderHeard = TRUE;
	switch(node->_nodeType){
		case NODE_EspduTransform:
			((struct X3D_EspduTransform *)node)->isRtpHeaderHeard = TRUE;
			break;
		case NODE_DISEntityManager:
			((struct X3D_DISEntityManager *)node)->isRtpHeaderHeard = TRUE;
			break;
		case NODE_ReceiverPdu:
			((struct X3D_ReceiverPdu *)node)->isRtpHeaderHeard = TRUE;
			break;
		case NODE_TransmitterPdu:
			((struct X3D_TransmitterPdu *)node)->isRtpHeaderHeard = TRUE;
			break;
		case NODE_SignalPdu:
			((struct X3D_SignalPdu *)node)->isRtpHeaderHeard = TRUE;
			break;
		default: 
			break;
	}
}
void dis_set_isActive(struct X3D_Node*node, int ival){
	switch(node->_nodeType){
		case NODE_EspduTransform:
			{
				struct X3D_EspduTransform* pnode = (struct X3D_EspduTransform*)node;
				if(pnode->isActive != ival){
					pnode->isActive = ival;
					MARK_EVENT(node,offsetof(struct X3D_EspduTransform,isActive));
				}
			}
			break;
		case NODE_DISEntityManager:
			{
				struct X3D_DISEntityManager* pnode = (struct X3D_DISEntityManager*)node;
				if(pnode->isActive != ival){
					pnode->isActive = ival;
					MARK_EVENT(node,offsetof(struct X3D_DISEntityManager,isActive));
				}
			}
			break;
		case NODE_ReceiverPdu:
			{
				struct X3D_ReceiverPdu* pnode = (struct X3D_ReceiverPdu*)node;
				if(pnode->isActive != ival){
					pnode->isActive = ival;
					MARK_EVENT(node,offsetof(struct X3D_ReceiverPdu,isActive));
				}
			}
			break;
		case NODE_TransmitterPdu:
			{
				struct X3D_TransmitterPdu* pnode = (struct X3D_TransmitterPdu*)node;
				if(pnode->isActive != ival){
					pnode->isActive = ival;
					MARK_EVENT(node,offsetof(struct X3D_TransmitterPdu,isActive));
				}
			}
			break;
		case NODE_SignalPdu:
			{
				struct X3D_SignalPdu* pnode = (struct X3D_SignalPdu*)node;
				if(pnode->isActive != ival){
					pnode->isActive = ival;
					MARK_EVENT(node,offsetof(struct X3D_SignalPdu,isActive));
				}
			}
			break;
		default: 
			break;
	}
}

void dis_set_isNetworkMode(struct X3D_Node*node, int networkMode){
	int	isStandAlone, isNetworkReader, isNetworkWriter;
	isStandAlone = isNetworkReader = isNetworkWriter = 0;
	switch(networkMode){
		case 0: isStandAlone = TRUE; break;
		case 1: isNetworkReader = TRUE; break;
		case 2: isNetworkWriter = TRUE; break;
		default: break;
	}
	switch(node->_nodeType){
		case NODE_EspduTransform:
			{
				struct X3D_EspduTransform* pnode = (struct X3D_EspduTransform*)node;
				if(pnode->isStandAlone != isStandAlone){
					pnode->isStandAlone = isStandAlone;
					MARK_EVENT(node,offsetof(struct X3D_EspduTransform,isStandAlone));
				}
				if(pnode->isNetworkReader != isNetworkReader){
					pnode->isNetworkReader = isNetworkReader;
					MARK_EVENT(node,offsetof(struct X3D_EspduTransform,isNetworkReader));
				}
				if(pnode->isNetworkWriter != isNetworkWriter){
					pnode->isNetworkWriter = isNetworkWriter;
					MARK_EVENT(node,offsetof(struct X3D_EspduTransform,isNetworkWriter));
				}
			}
			break;
		case NODE_DISEntityManager:
			{
				struct X3D_DISEntityManager* pnode = (struct X3D_DISEntityManager*)node;
				if(pnode->isStandAlone != isStandAlone){
					pnode->isStandAlone = isStandAlone;
					MARK_EVENT(node,offsetof(struct X3D_DISEntityManager,isStandAlone));
				}
				if(pnode->isNetworkReader != isNetworkReader){
					pnode->isNetworkReader = isNetworkReader;
					MARK_EVENT(node,offsetof(struct X3D_DISEntityManager,isNetworkReader));
				}
				if(pnode->isNetworkWriter != isNetworkWriter){
					pnode->isNetworkWriter = isNetworkWriter;
					MARK_EVENT(node,offsetof(struct X3D_DISEntityManager,isNetworkWriter));
				}
			}
			break;
		case NODE_ReceiverPdu:
			{
				struct X3D_ReceiverPdu* pnode = (struct X3D_ReceiverPdu*)node;
				if(pnode->isStandAlone != isStandAlone){
					pnode->isStandAlone = isStandAlone;
					MARK_EVENT(node,offsetof(struct X3D_EspduTransform,isStandAlone));
				}
				if(pnode->isNetworkReader != isNetworkReader){
					pnode->isNetworkReader = isNetworkReader;
					MARK_EVENT(node,offsetof(struct X3D_EspduTransform,isNetworkReader));
				}
				if(pnode->isNetworkWriter != isNetworkWriter){
					pnode->isNetworkWriter = isNetworkWriter;
					MARK_EVENT(node,offsetof(struct X3D_EspduTransform,isNetworkWriter));
				}
			}
			break;
		case NODE_TransmitterPdu:
			{
				struct X3D_TransmitterPdu* pnode = (struct X3D_TransmitterPdu*)node;
				if(pnode->isStandAlone != isStandAlone){
					pnode->isStandAlone = isStandAlone;
					MARK_EVENT(node,offsetof(struct X3D_EspduTransform,isStandAlone));
				}
				if(pnode->isNetworkReader != isNetworkReader){
					pnode->isNetworkReader = isNetworkReader;
					MARK_EVENT(node,offsetof(struct X3D_EspduTransform,isNetworkReader));
				}
				if(pnode->isNetworkWriter != isNetworkWriter){
					pnode->isNetworkWriter = isNetworkWriter;
					MARK_EVENT(node,offsetof(struct X3D_EspduTransform,isNetworkWriter));
				}
			}
			break;
		case NODE_SignalPdu:
			{
				struct X3D_SignalPdu* pnode = (struct X3D_SignalPdu*)node;
				if(pnode->isStandAlone != isStandAlone){
					pnode->isStandAlone = isStandAlone;
					MARK_EVENT(node,offsetof(struct X3D_EspduTransform,isStandAlone));
				}
				if(pnode->isNetworkReader != isNetworkReader){
					pnode->isNetworkReader = isNetworkReader;
					MARK_EVENT(node,offsetof(struct X3D_EspduTransform,isNetworkReader));
				}
				if(pnode->isNetworkWriter != isNetworkWriter){
					pnode->isNetworkWriter = isNetworkWriter;
					MARK_EVENT(node,offsetof(struct X3D_EspduTransform,isNetworkWriter));
				}
			}
			break;
		default: 
			break;
	}
}


int dis_read_stream(unsigned char * datastream, int streamsize, struct Vector *pdus, int *heard) 
{ 
	int pdutype, bytesread;
	unsigned char *carat, *carat2;
	static char pdubuffer[10000];
	unsigned char *pdubuf;
	struct Pdu* pdu;
	bytesread = 0;
	carat = &datastream[0];
	carat = rtp_strip_header(carat,heard);
	while(bytesread < streamsize){
		int i, distype, nbytes;
		if(0) for(i=0;i<210;i+=10){
			int j;
			printf("%d\t",i);
			for(j=0;j<10;j++){
				printf("%5d",(int)carat[i+j]);
			}
			printf("\n");
		}
		pdutype = (int)(carat[2]);
		distype = pduToDis(pdutype);
		//printf("pdu type=%d distype=%d",(int)pdutype, distype);
		
		pdubuf = dis_ctor(distype);
		carat2 = dis_unmarshal(carat,pdubuf,distype);
		nbytes = (carat2 - carat);
		pdu = (struct Pdu*)pdubuf;
		pdu->padding = TAG_UNCLAIMED;
		//printf("un-marshed version %d pdutype= %d\n",pdu->protocolVersion,pdu->pduType);
		vector_pushBack(struct Pdu*,pdus,pdu);
		//printf("unmarshed bits %d bytes %d\n",nbytes*8,nbytes);

		if(0){
			//try marshalling, then compare bytestreams
			unsigned char buf3[32000];
			unsigned char *carat3;
			int b3size;
			carat3 = dis_marshal(buf3,pdubuf,distype);
			b3size = carat3 - buf3;
			printf("marshed bits %d bytes %d\n",b3size*8,b3size);

			if(memcmp(carat,buf3,b3size) == 0)
				printf("bravo\n");
			else{
				printf("youch\n");
				for(i=0;i<210;i+=10){
					int j;
					printf("%d\t",i);
					for(j=0;j<10;j++){
						printf("%5d",(int)buf3[i+j]);
					}
					printf("\n");
				}
			}

		}


		if(0) if(pdutype == 1){
			int n;
			struct EntityStatePdu* p = (struct EntityStatePdu*)pdubuf;
			printf("loc %lf %lf %lf  rot %f %f %f\n",
				p->entityLocation.x,p->entityLocation.y,p->entityLocation.z,
				p->entityOrientation.psi,p->entityOrientation.theta,p->entityOrientation.phi);
#ifdef DIS2012
			n = p->numberOfVariableParameters;
#else //DIS1998
			n = p->numberOfArticulationParameters;
#endif
			if(n){
			  //unsigned char recordType; 
			  ///** Variable parameter data fields. Two doubles minus one byte */
			  //double variableParameterFields1; 
			  ///** Variable parameter data fields.  */
			  //unsigned int variableParameterFields2; 
			  ///** Variable parameter data fields.  */
			  //unsigned short variableParameterFields3; 
			  ///** Variable parameter data fields.  */
			  //unsigned char variableParameterFields4; 
				//struct VariableParameter *v;

#ifdef DIS2012
				struct ArticulatedParts *v;
				v = (struct ArticulatedParts*)p->variableParameters;
			   printf("v address = %p\n",v);
				//v = *vlist;
			   for(i=0;i<n;i++){
				   printf("%d %d %d %d %d %lf\n",
					   i,
					   (int)v[i].recordType,
					   (int)v[i].changeIndicator,
					   (int)v[i].partAttachedTo,
					   (int)v[i].parameterType,
					   v[i].parameterValue
					);
			   }
#else //DIS1998
				struct ArticulationParameter *v;
				v = (struct ArticulationParameter*)p->articulationParameters;
			   printf("v address = %p\n",v);
				//v = *vlist;
			   for(i=0;i<n;i++){
				   printf("%d %d %d %d %d %lf\n",
					   i,
					   (int)v[i].parameterTypeDesignator,
					   (int)v[i].changeIndicator,
					   (int)v[i].partAttachedTo,
					   (int)v[i].parameterType,
					   v[i].parameterValue
					);
			   }
#endif 
			}
		}
		//dis_dtor(pdubuf,distype);
		bytesread += nbytes;
		carat = carat2;
		//printf("bytes left = %d - %d = %d\n",streamsize, (int)(carat - datastream), streamsize - (int)(carat-datastream));
		//printf("\n");
		//if(0) for(i=0;i<npdus;i++){
		//	if(registeredPdus[i]->pduType == pdutype){
		//		//I think there should be more filtering here
		//		//for example is it the right target IP + port + entityID?
		//		memcpy(registeredPdus[i],pdubuffer,nbytes);
		//		break;
		//	}
		//}
	}
	return 0; //maybe an error number will be returned here in future
}

int dis_write_stream(unsigned char * datastream, struct Vector *pdus) 
{ 
	//missing maxsize on buffer
	int i, nbytes;
	unsigned char *carat;
	carat = datastream;
	nbytes = 0;
	if(pdus && pdus->n){
		for(i=0;i<pdus->n;i++){
			struct Pdu *pdu = vector_get(struct Pdu*,pdus,i);
			//printf("dis_wrt_str protocol %d pdutype %d\n",pdu->protocolVersion,pdu->pduType);
			int distype = pduToDis(pdu->pduType);
			carat = dis_marshal(carat,(unsigned char*)pdu,distype);
			//printf("pdu %d wrote %d bytes\n",i,nbytes);
		}
		//*streamsize = nbytes;
		nbytes = (int)(carat - datastream);
	}
	return nbytes;
}

struct X3D_Node* findNodeByName(char* defname) {
	//its weird we don't have a function for this already, 
	//  some relating to parser, and some relating to EAI, 
	//  but we want something thats parser-agnostic and will go through context->defnames.
	struct X3D_Node* node, *root;
	struct X3D_Proto* context;
	node = NULL;
	root = rootNode();
	context = X3D_PROTO(root);
	struct brotoDefpair def;
	if (context->__DEFnames) {
		int ndefs = vectorSize(context->__DEFnames);
		for (int i = 0; i < ndefs; i++) {
			def = vector_get(struct brotoDefpair, context->__DEFnames, i);
			//printf("%x %x %s\n",node,def.node,def.name);
			if (!strcmp(def.name,defname)) {
				node = def.node;
				break;
			}
		}
	}
	return node;
}
/* using regular pdu2espdu
struct Vector* avatars = NULL; //2023 multiplayer
int dis_pdus2avatars(struct Vector* pdus) {
	//2023 multiplayer
	//update avatars of other players, which may involve
	// adding an avatar, when a new player joins, with a certain appearance
	// removing an avatar (heartbeat or time since last update > participation_threshold_time ie 5 minutes)
	// updating avatar pose in world coords
	// updating avatar articulation, such as walking, standing, reaching - could there be flag combos?
	int i, ihit;
	struct Pdu* pdu;

	ihit = 0;
	if (!pdus) return ihit;
	for (i = 0; i < pdus->n; i++)
	{
		pdu = vector_get(struct Pdu*, pdus, i);
		if(pdu->padding == TAG_UNCLAIMED)
		switch (pdu->pduType) {
		case PDU_ENTITY_STATE:
		{
			//ENTITYSTATE category 77 avatar
			//we assume ENTITY_STATE pdus with category 77 are avatar updates

			struct EntityStatePdu* espdu;
			espdu = (struct EntityStatePdu*)pdu;
			if (espdu->entityID.application == fwl_get_DISapplication()
				&& espdu->entityID.site == fwl_get_DISsite()) {
				pdu->padding = TAG_SAME_PROGRAM;
				ihit++;
				break;
			}
			if (espdu->entityType.category != 77) {
				// not avatar, remains unclaimed
				break;
			}
			static struct X3D_Group* avatar_group = NULL;
			if(!avatar_group) avatar_group = (struct X3D_Group*)findNodeByName("AvatarHolder");
			if (!avatar_group) {
				printf("your scene needs a Group DEF AvatarHolder\n");
				break;
			}
			struct Multi_Node* avatars = &avatar_group->children;
			struct X3D_EspduTransform* pnode, * tnode;
			pnode = NULL;
			for (int j = 0; j < avatars->n; j++) {
				// 2018.pdf 6.2.80.3 Application Number implied an instance number, 
				// so should not be same unless loopback testing
				struct X3D_EspduTransform* tnode = (struct X3D_EspduTransform*)avatars->p[j];
				int OK = TRUE;
				//when sending avatars, we set entityID = sending programID
				if (espdu->entityID.entity != tnode->entityID) OK = FALSE;
				if (OK) {
					pnode = tnode;
					//still TAG_UNCLAIMED
					break;
				}
			}
			if (!pnode) break;
			ihit++;
			pdu->padding = TAG_AVATAR;
			pnode->_change++; //mark node changed
			pnode->timestamp = TickTime();

			if (pnode->__geoSystem) {
				Quaternion qgc2tcs, qtcs2body, qgc2body;
				struct SFVec3d gd, gc, translate;
				struct SFVec4d rotate;
				double localxyz[3], tcsxyz[3], tcs2bodyxyz[3], world2bodyxyz[3];
				float xyza[4];
				Geosys* gs;
				gs = GEOSYS(pnode->__geoSystem);
				user2gc(gs, &pnode->geoCoords, 1, &gc);
				//gc2gd(gs,&gc,1,&gd);
				//gc2tcs_transform(gs,&gd,&translate,&rotate);
				gc2tcsB_transform(gs, &gc, &translate, &rotate);
				//somehow get body/entity into world/gc - rotation and translation
				{
					//rotation
					//assumption (Apr 2018 don't know how Xj3d does it, here's dug9's guess):
					// pdu is world2body
					// espdutransform.rotation = local2body
					// - where local is TCS Topocentric Coord System as described for GeoLocation
					// - and world is GC
					// local2body = world2local.inverse x world2body
					// for freewrl world2local is gc2tcs
					Quaternion qtcs2gc, q;
					float ypr[3], xyza[4];
					ypr[0] = -espdu->entityOrientation.psi;
					ypr[1] = espdu->entityOrientation.theta;
					ypr[2] = espdu->entityOrientation.phi;
					ypr2axisangle(ypr, xyza);
					xyza[3] = -xyza[3];
					//vecprint4fb("recv xyza",xyza,"\n");

					vrmlrot4f_to_quaternion(&qgc2body, xyza);
					vrmlrot4d_to_quaternion(&qgc2tcs, rotate.c);
					quaternion_inverse(&qtcs2gc, &qgc2tcs);
					quaternion_multiply(&qtcs2body, &qtcs2gc, &qgc2body);
					quaternion_set(&q, &qtcs2body);
					quaternion_to_vrmlrot4f(&q, pnode->rotation.c);
				}
				{
					//translation - dug9 debate: could do it one of 2 ways
					enum transmethod {
						TRANS_ZERO = 1,
						TRANS_LOCATION_MINUS_GEOCOORD = 2
					};
					//static int transmethod = TRANS_LOCATION_MINUS_GEOCOORD; 
					static int transmethod = TRANS_ZERO;

					vector3double2vec3d(world2bodyxyz, &espdu->entityLocation);
					if (transmethod == TRANS_LOCATION_MINUS_GEOCOORD) {
						//METHOD 1: translation = Location - geoCoords
						struct SFVec3d world, tcs;
						veccopyd(world.c, world2bodyxyz);
						//gc2tcs(gs,&gd,&world,1,&tcs);
						gc2tcsB(gs, &gc, &world, 1, &tcs);
						double2float(pnode->translation.c, tcs.c, 3);
					}
					else {
						//TRANS_ZERO
						//METHOD 2: geoCoords = Location; translation = 000
						// x smoothing doesn't work if done in translation / tcs space
						struct SFVec3d world, tcs2, tcs1;
						double deltatcs[3];
						float deltap[3];
						static int want_smoothing = 1;
						if (want_smoothing) {
							//gc2tcs(gs,&gd,&gc,1,&tcs1);
							gc2tcsB(gs, &gc, &gc, 1, &tcs1);
							veccopyd(world.c, world2bodyxyz);
							gc2tcs(gs, &gd, &world, 1, &tcs2);
							gc2tcsB(gs, &gc, &world, 1, &tcs2);
							vecdifd(deltatcs, tcs1.c, tcs2.c);
							double2float(deltap, deltatcs, 3);
							vecadd3f(pnode->_p0.c, pnode->_p0.c, deltap);
						}
						gc2user(gs, &world, 1, &pnode->geoCoords);
						//node->_change++;
						vecset3f(pnode->translation.c, 0.0f, 0.0f, 0.0f);

					}
				}
				// recv geo dead reckoning
				{
					float V[3], A[3];
					// in entity or world, depending on drmethod
					vector3float2vec3f(V, &espdu->entityLinearVelocity);
					veccopy3f(pnode->linearVelocity.c, V);

				}

			}
			else {
				//non-geosystem scene. Apr 22, 2018 we aren't using this now
				// -- everything goes through geosystem code above
				// -- but keeping this until we benchmark against Brutzman
				//translation - assumes companion scenes will have same parent transform stack
				//(x, -z, y).
				pnode->translation.c[0] = espdu->entityLocation.x;
				pnode->translation.c[1] = espdu->entityLocation.z;
				pnode->translation.c[2] = -espdu->entityLocation.y;
				//rotation
				if (0) {
					Quaternion qA;
					float ypr[3];
					double r[4];
					float* c = pnode->rotation.c;
					ypr[0] = espdu->entityOrientation.phi;
					ypr[1] = espdu->entityOrientation.psi;
					ypr[2] = espdu->entityOrientation.theta;
					euler2quat(&qA, ypr[0], ypr[1], ypr[2]);
					//quaternion_normalize(&qA);
					//vrmlrot_to_quaternion(&qA,c[0],c[1],c[2],c[3]);
					quaternion_to_vrmlrot(&qA, &r[0], &r[1], &r[2], &r[3]);
					c[0] = (float)r[0];
					c[1] = (float)r[1];
					c[2] = (float)r[2];
					c[3] = (float)r[3];
				}
				if (1) {
					float ypr[3];
					ypr[0] = -espdu->entityOrientation.psi;  //gimbal.js shows -yaw
					ypr[1] = espdu->entityOrientation.theta;
					ypr[2] = espdu->entityOrientation.phi;
					ypr2axisangle(ypr, pnode->rotation.c);
				}
				// dead reckoning
				vector3float2vec3f(pnode->linearVelocity.c, &espdu->entityLinearVelocity);

			}
			//articuation parameters
			pnode->articulationParameterArray.n = espdu->numberOfArticulationParameters;
			//printf("recv art count %d\n",espdu->numberOfArticulationParameters);
			if (pnode->articulationParameterArray.n) {
				struct ArticulationParameter* ap;
				float* pp;
				int i, np = pnode->articulationParameterArray.n;
				ap = espdu->articulationParameters;
				pp = malloc(np * sizeof(float));
				//printf("received %d articulation parameters:\n",np);
				for (i = 0; i < np; i++) {
					//ap[i].parameterTypeDesignator = 0; //0 is articulated part
					//ap[i].parameterType = 1029; //1024 - rudder + 5 X
					pp[i] = (float)ap[i].parameterValue;
					//printf("%d %f\n",i,pp[i]);
					//ap[i].partAttachedTo = 0;
					switch (i) {
					case 0: pnode->articulationParameterValue0_changed = pp[i]; break;
					case 1: pnode->articulationParameterValue1_changed = pp[i]; break;
					case 2: pnode->articulationParameterValue2_changed = pp[i]; break;
					case 3: pnode->articulationParameterValue3_changed = pp[i]; break;
					case 4: pnode->articulationParameterValue4_changed = pp[i]; break;
					case 5: pnode->articulationParameterValue5_changed = pp[i]; break;
					case 6: pnode->articulationParameterValue6_changed = pp[i]; break;
					case 7: pnode->articulationParameterValue7_changed = pp[i]; break;
					default:
						break;
					}
				}
				if (pnode->articulationParameterArray.p) free(pnode->articulationParameterArray.p);
				pnode->articulationParameterArray.p = pp;
				//done in generic mark_changed_fields //MARK_EVENT(X3D_NODE(pnode),offsetof(struct X3D_EspduTransform,articulationParameterArray));
			}
			pnode->_pduchange_es = TRUE;
			if (espdu->entityAppearance | 1 << 20) {
				//http://movesinstitute.org/~mcgredo/MV3500/hla/1278.1-200X%20Draft%2016%20rev%2018.pdf
				//p.50 no dead reckoning if isFrozen bit is set, bit 21 of entityAppearance
				//(why can't they just leave dead reckoning parameters 0, and run through formula? H: specs written in 1990s for 80386 processors)
				//pnode->_isFrozen = TRUE; //pduchange_es = FALSE;
			}
		
		}
		}

	}
	return ihit;
}
*/

static double lasttime;
static char buf[32768];
static struct Vector *pdus = NULL;

void dis_recvloop(){
	//there are a few ways to do non-blocking recv
	//1. ioctlsocket non-blocking - set socket to not block
	//2. select() - select itself blocks, so should have its own thread
	//3. PEEK flag in recvfrom
	//Oct 24, 2017 choice: 1.
	// - because we aren't doing a separate thread yet, so 1 or 3, and 3 worked when tried first
	int i,j,nbytes, more, heard;
	static int count = 0;
	double thistime, dtime;
	if(!sockets_recv || sockets_recv->n == 0) return;
	thistime = TickTime();
	dtime = thistime - lasttime;
	if(!pdus) pdus = newVector(struct Pdu*,20);

	//since not select()ing we have to check all sockets (if readInterval?)
	//for 'Entity Discovery' at least one DIS node needs to be in scene, with IP/port to check
	for(i=0;i<sockets_recv->n;i++){
		struct dis_socket *dsock = vector_get_ptr(struct dis_socket,sockets_recv,i);
		//things may have built up in the input socket, so we loop till flushed
		do{
			struct X3D_DISEntityManager* sockem = NULL;
			heard = FALSE;
			more = FALSE;
			nbytes = sockrecvfrom(dsock,buf,32000);
			if(nbytes > 0){
				int nhit = 0;
				more = TRUE;
				dsock->lasttime = thistime;
				//printf("sock read nbytes = %d\n",nbytes);
				//free last round
				for(j=0;j<pdus->n;j++){
					struct Pdu* pdu = vector_get(struct Pdu*,pdus,j);
					dis_dtor((unsigned char *)pdu,pduToDis(pdu->pduType));
				}
				pdus->n = 0;
				dis_read_stream(buf,nbytes,pdus,&heard);
				for (int j = 0; j < pdus->n; j++) {
					struct Pdu* pdu = vector_get(struct Pdu*, pdus, j);
					pdu->padding = TAG_UNCLAIMED; //0
				}
				//print some stuff to the console, to prove we got a state update
				//printf("hallelluha %d\n",count++);
				//check pdus against all nodes registered on the port
				// in case the message is for an existing node

				int ihit;
				if(dsock->registered){
					//printf("dsock.registered.n %d\n", dsock->registered->n);
					for(j=0;j<dsock->registered->n;j++){
						struct X3D_Node *node = vector_get(struct X3D_Node*,dsock->registered,j);
						//check site and application ID
						//distribute to registered nodes by entityID
						ihit = 0;
						switch(node->_nodeType){
							case NODE_EspduTransform:
								ihit = dis_pdus2node_espdu(node, pdus);
								break;
							case NODE_DISEntityManager:
								sockem = (struct X3D_DISEntityManager*) node;
								ihit = dis_pdus2node_sm(node, pdus);
								break;
							case NODE_ReceiverPdu:
								ihit = dis_pdus2node_receiver(node, pdus);
								break;
							case NODE_TransmitterPdu:
								ihit = dis_pdus2node_transmitter(node, pdus);
								break;
							case NODE_SignalPdu:
								ihit = dis_pdus2node_signal(node, pdus);
								break;
							default:
								break;
						}
						if(ihit){
							if(heard) set_rtp_heard(node);
							dis_set_isActive(node,TRUE);
							dis_set_node_lasttime(node,thistime);
							nhit += ihit;
						}
					}
				}
				//2023 multiplayer>>
				ihit = dis_pdus2sensors(pdus);
				nhit += ihit;
				//<<2023 multiplayer
				int counts[9] = { 0,0,0,0,0,0,0,0,0 };
				for (int j = 0; j < pdus->n; j++) {
					struct Pdu* pdu = vector_get(struct Pdu*, pdus, j);
					counts[pdu->padding]++;
				}
				//TAG_UNCLAIMED = 0
				if(0) printf("counts U%d S%d E%d M%d r%d t%d s%d A%d S%d\n",
					counts[0], counts[1], counts[2], counts[3], counts[4], counts[5], counts[6], counts[7], counts[8]);
				if(nhit < pdus->n){
					// any 'left-over' pdus might be 'entity discovery' candidates
					if(0) printf("leftovers %d avatarhits %d pdus %d",pdus->n - nhit, ihit, pdus->n);
					nhit = dis_pdus2newnode(dsock,sockem, pdus);
					if(0) printf(" %d used\n",nhit);
				}
			}
		}while(more);
		if(dsock->registered){
#define RETIRE_TIME 300.0 //5 MINUTES?
			//check if any node listeners have gone inactive
			//2023 multiplayer sensors don't go stale. avatars do
			struct X3D_DISEntityManager* sockem = NULL;
			for(j=0;j<dsock->registered->n;j++){
				struct X3D_Node *node = vector_get(struct X3D_Node*,dsock->registered,j);
				if(node->_nodeType == NODE_DISEntityManager){
					sockem = (struct X3D_DISEntityManager*)node;
				}
				if(sockem) break;
			}
			for (j = 0; j < dsock->registered->n; j++) {
				//update isActive
				struct X3D_Node* node = vector_get(struct X3D_Node*, dsock->registered, j);
				if (node->_nodeType != NODE_DISEntityManager) {
					double readinterval, writeinterval, lasttime;
					dis_get_node_lasttime(node, &lasttime, &readinterval, &writeinterval);
					//printf("%d %s %lf %lf %lf %lf setting isActive false\n", j, stringNodeType(node->_nodeType), readinterval, writeinterval, lasttime, thistime);
					double timeout = RETIRE_TIME;
					if (node->_nodeType == NODE_EspduTransform) {
						struct X3D_EspduTransform* espdu = (struct X3D_EspduTransform*)node;
						if (!espdu->isNetworkWriter) {
							timeout = 5.0;
						}
					}
					if (thistime - lasttime > timeout) {
						//5 second rule: if a node recvs nothing for 5 seconds, turn isActive to FALSE.
						dis_set_isActive(node, FALSE);
					}
					//if its been several (?) heartbeat increments since we last heard from an entity
					// the DIS specs talk about removing (opposite of adding by 'entity discovery')
					if (thistime - lasttime > (timeout * 3)) {
						//if in entitymanager state.entities, removeChildren
						int ihit = 0;
						if (sockem && node->_nodeType == NODE_EspduTransform) {
							ihit = dis_entity_retire(sockem, node);
						}
						if (ihit == 1) {
							printf(" retired one\n");
							//printf("thisttime %lf lasttime %lf\n",thistime,lasttime);
						}
						//if(ihit == -1) printf(" cetiree not in EM list\n");
					}
				}
			}
			if(sockem && sockem->removedEntities.n) {
				//printf("removedEntities.n=%d\n",node->removedEntities.n);
				MARK_EVENT(X3D_NODE(sockem),offsetof(struct X3D_DISEntityManager,removedEntities));
			}

		}
	}

}

void dis_open_socket(struct dis_socket* dsock){
	if(dsock->multicastRelayHost && strlen(dsock->multicastRelayHost)){
		printf("[%s]\n",dsock->multicastRelayHost);
		printf("\n");
		//direct socket
	}else{
		socket_open(dsock);
	}

}
void * dis_register(struct X3D_Node* node,char *address,int applicationID,int entityID,char *multicastRelayHost,
		int multicastRelayPort,
		char *networkMode, int port,double readInterval,int rtpHeaderExpected,int siteID,double writeInterval){
	void *preg; //something to store in the node, to say which socket its registerd in
	int inetworkmode = 0;
	int sport = port + fwl_get_testset();
	if(!strcmp(networkMode,"standAlone")) inetworkmode = 0;
	else if(!strcmp(networkMode,"networkReader")) inetworkmode = 1;
	else if(!strcmp(networkMode,"networkWriter")) inetworkmode = 2;
	preg = NULL;
	if(inetworkmode == 0) preg = NULL; //not joined/registered to any socket
	if(inetworkmode == 1){
		int i, j, ifound;
		struct dis_socket *dsock;
		if(!sockets_recv) sockets_recv = newVector(struct dis_socket,10);
		//if theres a (networkMode,address,port) tuple already registered, then we join
		ifound = -1;
		dsock = NULL;
		for(i=0;i<sockets_recv->n;i++){
			dsock = vector_get_ptr(struct dis_socket,sockets_recv,i);
			if(!strcmp(dsock->address,address) && dsock->port == sport){
				//if(dsock->registered){
				//	for(j=0;j<dsock->registered->n;j++){
				//		we assume its not registered
				//	}
				//}
				ifound = i;
				break;
			}
		}
		if(ifound == -1){
			//create a socket
			struct dis_socket asock;
			memset(&asock,0,sizeof(struct dis_socket));
			vector_pushBack(struct dis_socket,sockets_recv,asock);
			dsock = vector_get_ptr(struct dis_socket,sockets_recv,sockets_recv->n-1);
			dsock->address = address;
			dsock->port = sport;
			dsock->multicastRelayHost = multicastRelayHost;
			dsock->multicastRelayPort = multicastRelayPort;
			dsock->idir = inetworkmode;
			//open port
			dis_open_socket(dsock);
		}
		//join socket
		if(!dsock->registered) dsock->registered = newVector(struct X3D_Node*,20);
		vector_pushBack(struct X3D_Node*,dsock->registered,node);
		preg = (void*)dsock;
	}
	if(inetworkmode==2){
		int i, j, ifound;
		struct dis_socket *dsock;
		if(!sockets_send) sockets_send = newVector(struct dis_socket,10);
		//if theres a (networkMode,address,port) tuple already registered, then we join
		ifound = -1;
		dsock = NULL;
		for(i=0;i<sockets_send->n;i++){
			dsock = vector_get_ptr(struct dis_socket,sockets_send,i);
			if(!strcmp(dsock->address,address) && dsock->port == sport){
				//if(dsock->registered){
				//	for(j=0;j<dsock->registered->n;j++){
				//		we assume its not registered
				//	}
				//}
				ifound = i;
				break;
			}
		}
		if(ifound == -1){
			//create a socket
			struct dis_socket asock;
			memset(&asock,0,sizeof(struct dis_socket));
			vector_pushBack(struct dis_socket,sockets_send,asock);
			dsock = vector_get_ptr(struct dis_socket,sockets_send,sockets_send->n-1);
			dsock->address = address;
			dsock->port = sport;
			dsock->multicastRelayHost = multicastRelayHost;
			dsock->multicastRelayPort = multicastRelayPort;
			dsock->idir = inetworkmode;
			//open port
			dis_open_socket(dsock);
		}
		//join socket
		if(!dsock->registered) dsock->registered = newVector(struct X3D_Node*,20);
		vector_pushBack(struct X3D_Node*,dsock->registered,node);
		preg = (void*)dsock;
	}
	dis_set_isNetworkMode(node, inetworkmode);
	return preg;
}
void dis_unregister(struct dis_socket * dsock, struct X3D_Node* node){
	//unregister / un-join node from socket
	int j;
	if(dsock->registered){
		for(j=0;j<dsock->registered->n;j++){
			if(vector_get(struct X3D_Node*,dsock->registered,j)==node){
				vector_remove_elem(struct X3D_Node*,dsock->registered,j);
				break;
			}
		}
	}
}
int dis_check_socket_change(struct dis_socket* dsock,char *address, int port,
		char *multicastRelayHost, 	int multicastRelayPort, char *networkMode){
	//do the node fields still jive/match with the socket it joined?
	//(a weakness of the freewrl architecture: there's no per-field change flag
	//so if a node with a zillion fields is flagged as changed, we have to 
	//re-'compile' the whole node, or save old values in _old fields on the node for comparison,
	//or (more normally) MARK_EVENT(node,offset) which runs through lists of registered routes.
	//here we are comparing a few fields with 'what they must have been when registered')
	//IDEA: save a duplicate of the node in _oldNode field, so can compare 1:1 with any field

	int inetworkmode = 0;
	int sport = port + fwl_get_testset();
	if(!strcmp(networkMode,"standAlone")) inetworkmode = 0;
	else if(!strcmp(networkMode,"networkReader")) inetworkmode = 1;
	else if(!strcmp(networkMode,"networkWriter")) inetworkmode = 2;
	if(inetworkmode == 0 && dsock == NULL) return FALSE; //not registered, and no need to register
	if(dsock == NULL) return TRUE; //need to register
	if(inetworkmode != dsock->idir) return TRUE; //change of direction
	if(strcmp(address,dsock->address)) return TRUE; //change of address
	if(sport != dsock->port) return TRUE; //change of port
	if(strcmp(multicastRelayHost,dsock->multicastRelayHost)) return TRUE; 
	if(multicastRelayPort != dsock->multicastRelayPort) return TRUE;

	return FALSE;
}
// freewrl problem: _changed flag is per-node
// x but its bad DIS ettiquette to resend pdus that haven't changed
// to detect per-pdu changes:
// 1. during node_compile
// 1.a do once: create node->_oldstate and register for disposal, copy node to _oldstate 
// 1.b compare _oldState and node
//		- compare fields per-pdu, and flag per-pdu
//  common flags:
//  _pduchange_networksensor
//	per-pdu flags:
//	espdu
//	_pduchange_deadreckoning 
//	_pduchange_articulationparameters
//	_pduchange_collision
//	_pduchange_fire
//	_pduchange_detonation
//	recieverpdu
//	_pduchange_receiver
//	signalpdu
//	_pduchange_signal
//	transmitterpdu
//	_pduchange_transmitter
// 1.c copy node fields to _oldState
// 1.d mark node compiled
// 2. in dis_sendloop only send pdus that changed
void shallow_copy_node(struct X3D_Node *copy, struct X3D_Node *original )
{
	//we just want to copy the public fields, not our private _ fields
	// which include things like _pduchange_espdutransform etc
	// same for later when we compare, just the public fields
	const int *offset;
	unsigned char *src, *dest;
	src = (unsigned char *)original;
	dest = (unsigned char *)copy;

	offset = NODE_OFFSETS[original->_nodeType];
	while(offset[0] > -1){
		if(offset[4] > 0){
			//offset[4] is the specs attribute, and if its a private field ie _name then it should have 0
			// we just want the public fields here
			union anyVrml *anysrc, *anydest;
			anysrc = (union anyVrml*)(src + offset[1]);
			anydest = (union anyVrml*)(dest + offset[1]);
			shallow_copy_field(offset[2],anysrc,anydest);
		}
		offset += 6;
	};
}
int shallow_compare_field(int typeIndex, union anyVrml* source, union anyVrml* dest)
{
	int i, isize, has_changed;
	int sftype, isMF;
	struct Multi_Node *mfs,*mfd;
	has_changed = FALSE;

	isMF = typeIndex % 2;
	sftype = typeIndex - isMF;
	//from EAI_C_CommonFunctions.c
	//isize = returnElementLength(sftype) * returnElementRowSize(sftype);
	isize = sizeofSForMF(sftype);
	if(isMF)
	{
		int nele;
		char *ps, *pd;
		mfs = (struct Multi_Node*)source;
		mfd = (struct Multi_Node*)dest;
		//self assignment is no-op
		if(mfs->n != mfd->n){
			has_changed = TRUE;
		}else{
			ps = (char *)mfs->p;
			pd = (char *)mfd->p;
			for(i=0;i<mfs->n;i++)
			{
				has_changed = shallow_compare_field(sftype,(union anyVrml*)ps,(union anyVrml*)pd);
				ps += isize;
				pd += isize;
			}
		}
	}else{ 
		//isSF
		switch(typeIndex)
		{
			case FIELDTYPE_SFString:
				{
					//go deep, same as copy_field
					struct Uni_String **ss, **sd;
					if(source != dest){
						has_changed = TRUE;
					}else{
						ss = (struct Uni_String **)source;
						sd = (struct Uni_String **)dest;
						if(*ss && *sd){
							has_changed = memcmp(*sd,*ss,sizeof(struct Uni_String)) ? TRUE : FALSE;
						}
					}
				}
				break;
			default:
				//memcpy(dest,source,sizeof(union anyVrml));
				has_changed = memcmp(dest,source,isize) ? TRUE : FALSE;
				break;
		}
	}
	return has_changed;
} //return copy_field

int shallow_compare_node_fields(struct X3D_Node *node, struct X3D_Node *old, const int *PFIELDS){
	const int *fname, *offset;
	unsigned char *src, *dest;
	int k, has_changed;

	src = (unsigned char *)old;
	dest = (unsigned char *)node;
	fname = PFIELDS;
	k = 0;
	has_changed = 0;
	while(fname[k] > -1){
		offset = NODE_OFFSETS[node->_nodeType];
		while(offset[0] > -1){
			if(offset[0] == fname[k]){
				union anyVrml *anysrc, *anydest;
				anysrc = (union anyVrml*)(src + offset[1]);
				anydest = (union anyVrml*)(dest + offset[1]);
				has_changed += shallow_compare_field(offset[2],anysrc,anydest);
				break;
			}
			offset += 6;
		};
		k++;
	};
	return has_changed ? TRUE : FALSE;
}

int mark_changed_node_fields(struct X3D_Node *node, struct X3D_Node *old, const int *PFIELDS){
	const int *fname, *offset;
	unsigned char *src, *dest;
	int k, count;

	src = (unsigned char *)old;
	dest = (unsigned char *)node;
	fname = PFIELDS;
	k = 0;
	count = 0;
	while(fname[k] > -1){
		offset = NODE_OFFSETS[node->_nodeType];
		while(offset[0] > -1){
			if(offset[0] == fname[k]){
				union anyVrml *anysrc, *anydest;
				anysrc = (union anyVrml*)(src + offset[1]);
				anydest = (union anyVrml*)(dest + offset[1]);
				if(shallow_compare_field(offset[2],anysrc,anydest)){
					MARK_EVENT(node,offset[1]);
					count++;
				}
				break;
			}
			offset += 6;
		};
		k++;
	};
	return count;
}

//here are some per-pdu lists of public fields, useful for detecting per-pdu field changes

const int FIELDS_networksensor [] = {
	FIELDNAMES_enabled,
	FIELDNAMES_isActive,
	FIELDNAMES_timestamp,
	FIELDNAMES_address,
	FIELDNAMES_port,
	FIELDNAMES_multicastRelayHost,
	FIELDNAMES_multicastRelayPort,
	FIELDNAMES_networkMode,
	FIELDNAMES_isNetworkReader,
	FIELDNAMES_isNetworkWriter,
	FIELDNAMES_isStandAlone,
	FIELDNAMES_readInterval,
	FIELDNAMES_writeInterval,
	FIELDNAMES_rtpHeaderExpected,
	FIELDNAMES_isRtpHeaderHeard,
	//FIELDNAMES__registered, //not the private fields
	//FIELDNAMES__dsock,
	//FIELDNAMES__lasttime,
	-1,
};

const int FIELDS_entity [] = {
	FIELDNAMES_entityID,
	FIELDNAMES_applicationID,
	FIELDNAMES_siteID, 
	-1,
};

const int FIELDS_geo [] = {	
	FIELDNAMES_geoSystem, 
	FIELDNAMES_geoCoords,
	-1,
};
const int FIELDS_geosys [] = {	
	FIELDNAMES_geoSystem, 
	-1,
};
const int FIELDS_geocoord [] = {	
	FIELDNAMES_geoCoords,
	-1,
};


const int FIELDS_em_info [] = {	
	FIELDNAMES_entityCategory,
	FIELDNAMES_entityCountry,
	FIELDNAMES_entityDomain,
	FIELDNAMES_entityExtra,
	FIELDNAMES_entityKind,
	FIELDNAMES_entitySpecific,
	FIELDNAMES_entitySubCategory,
	-1,
};
const int FIELDS_create [] = {	
	FIELDNAMES_addedEntities,
	-1,
};
const int FIELDS_remove [] = {	
	FIELDNAMES_removedEntities,
	-1,
};

const int FIELDS_es_force [] = {
	FIELDNAMES_forceID,
	//FIELDNAMES_marking,
	-1,
};

const int FIELDS_es_transform [] = {
	FIELDNAMES_center,
	FIELDNAMES_children,
	FIELDNAMES_rotation,
	FIELDNAMES_scale,
	FIELDNAMES_scaleOrientation,
	FIELDNAMES_translation,
	//FIELDNAMES_bboxCenter,
	//FIELDNAMES_bboxSize,
	-1,
};


const int FIELDS_es_deadreckoning [] = {	
	FIELDNAMES_deadReckoning,
	FIELDNAMES_linearVelocity,
	FIELDNAMES_linearAcceleration,
	-1,
};

const int FIELDS_es_articulation [] = {	
	FIELDNAMES_set_articulationParameterValue0,
	FIELDNAMES_set_articulationParameterValue1,
	FIELDNAMES_set_articulationParameterValue2,
	FIELDNAMES_set_articulationParameterValue3,
	FIELDNAMES_set_articulationParameterValue4,
	FIELDNAMES_set_articulationParameterValue5,
	FIELDNAMES_set_articulationParameterValue6,
	FIELDNAMES_set_articulationParameterValue7,
	FIELDNAMES_articulationParameterCount,
	FIELDNAMES_articulationParameterDesignatorArray,
	FIELDNAMES_articulationParameterChangeIndicatorArr,
	FIELDNAMES_articulationParameterIdPartAttachedToAr,
	FIELDNAMES_articulationParameterTypeArray,
	FIELDNAMES_articulationParameterArray,
	FIELDNAMES_articulationParameterValue0_changed,
	FIELDNAMES_articulationParameterValue1_changed,
	FIELDNAMES_articulationParameterValue2_changed,
	FIELDNAMES_articulationParameterValue3_changed,
	FIELDNAMES_articulationParameterValue4_changed,
	FIELDNAMES_articulationParameterValue5_changed,
	FIELDNAMES_articulationParameterValue6_changed,
	FIELDNAMES_articulationParameterValue7_changed,
	-1,
};

const int FIELDS_collision [] = {	
	FIELDNAMES_collisionType,
	FIELDNAMES_collideTime,
	FIELDNAMES_isCollided,
	-1,
};

const int FIELDS_events [] = {	
	FIELDNAMES_eventEntityID,
	FIELDNAMES_eventApplicationID,
	FIELDNAMES_eventSiteID,
	FIELDNAMES_eventNumber,
	-1,
};

const int FIELDS_fire [] = {	
	FIELDNAMES_fired1,
	FIELDNAMES_fired2,
	FIELDNAMES_fireMissionIndex,
	FIELDNAMES_firingRange,
	FIELDNAMES_firedTime,
	-1,
};

const int FIELDS_detonation [] = {	
	FIELDNAMES_detonationLocation,
	FIELDNAMES_detonationRelativeLocation,
	FIELDNAMES_detonationResult,
	FIELDNAMES_detonateTime,
	FIELDNAMES_isDetonated,
	-1,
};

const int FIELDS_munition [] = {	
	FIELDNAMES_munitionEntityID,
	FIELDNAMES_munitionApplicationID,
	FIELDNAMES_munitionSiteID,
	FIELDNAMES_munitionStartPoint,
	FIELDNAMES_munitionEndPoint,
	FIELDNAMES_munitionQuantity,
	-1,
};
const int FIELDS_rate [] = {	
	FIELDNAMES_firingRate,
	FIELDNAMES_fuse,
	FIELDNAMES_warhead,
	-1,
};

const int FIELDS_receiver [] = {	
	FIELDNAMES_radioID,
	FIELDNAMES_whichGeometry,
	FIELDNAMES_receiverState,
	FIELDNAMES_receivedPower,
	FIELDNAMES_transmitterEntityID,
	FIELDNAMES_transmitterApplicationID,
	FIELDNAMES_transmitterSiteID,
	FIELDNAMES_transmitterRadioID,
	-1,
};

const int FIELDS_signal [] = {	
	FIELDNAMES_radioID,
	FIELDNAMES_whichGeometry,
	FIELDNAMES_data,
	FIELDNAMES_dataLength,
	FIELDNAMES_encodingScheme,
	FIELDNAMES_sampleRate,
	FIELDNAMES_samples,
	FIELDNAMES_tdlType,
	-1,
};

const int FIELDS_transmitter [] = {	
	FIELDNAMES_radioID,
	FIELDNAMES_whichGeometry,
	FIELDNAMES_radioEntityTypeCategory,
	FIELDNAMES_radioEntityTypeCountry,
	FIELDNAMES_radioEntityTypeDomain,
	FIELDNAMES_radioEntityTypeKind,
	FIELDNAMES_radioEntityTypeNomenclature,
	FIELDNAMES_radioEntityTypeNomenclatureVersion,
	FIELDNAMES_antennaLocation,
	FIELDNAMES_antennaPatternLength,
	FIELDNAMES_antennaPatternType,
	FIELDNAMES_relativeAntennaLocation,
	FIELDNAMES_inputSource,
	FIELDNAMES_transmitState,
	FIELDNAMES_power,
	FIELDNAMES_frequency,
	FIELDNAMES_transmitFrequencyBandwidth,
	FIELDNAMES_lengthOfModulationParameters,
	FIELDNAMES_modulationTypeDetail,
	FIELDNAMES_modulationTypeMajor,
	FIELDNAMES_modulationTypeMajor,
	FIELDNAMES_modulationTypeSpreadSpectrum,
	FIELDNAMES_modulationTypeSystem,
	FIELDNAMES_cryptoSystem,
	FIELDNAMES_cryptoKeyID,
	-1,
};

void compile_DIS_network(struct X3D_EspduTransform *node){
	if(node->_oldState == NULL){
		//change detection 
		//later we'll copy the entire node after we detect any changed fields
		struct X3D_Node *old;
		old = createNewX3DNode0(node->_nodeType);
		//shallow_copy_node(old,X3D_NODE(node));
		node->_oldState = old; //I think one underscore means dispose
	}
	if(!node->_registered){
		void *psock;
		node->address->strptr = strdup( fwl_get_DISaddress() ? fwl_get_DISaddress() : node->address->strptr );
		//if (!strcmp(node->address->strptr, "localhost")) {
		//	//node->address->strptr = "127.0.0.1";
		//	struct hostent* hp = gethostbyname(node->address->strptr);
		//	printf("official host name %s\n", hp->h_name);
		//}
		node->multicastRelayHost->strptr = strdup("");
		node->port = fwl_get_DISport()? fwl_get_DISport():node->port;
		node->siteID = fwl_get_DISsite()? fwl_get_DISsite():node->siteID ;
		node->applicationID = fwl_get_DISapplication() ? fwl_get_DISapplication() : node->applicationID;
		//printf("address %s port %d site %d app %d\n", node->address->strptr, node->port, node->siteID, node->applicationID);
		psock = dis_register(X3D_NODE(node),
			node->address->strptr,
			node->applicationID,
			node->entityID, node->multicastRelayHost->strptr, node->multicastRelayPort,
			node->networkMode->strptr, node->port, 
			node->readInterval, node->rtpHeaderExpected, 
			node->siteID, 
			node->writeInterval);
		node->_registered = TRUE;
		node->_dsock = psock;
	}
	if(node->_registered){
		//almost every field is [in,out] so can be changed at runtime
		//IDEA: save duplicate of nodetype in _oldnode field
		if(shallow_compare_node_fields(X3D_NODE(node),node->_oldState,FIELDS_networksensor)){
			int changed;
			changed = dis_check_socket_change((struct dis_socket*)node->_dsock,node->address->strptr, node->port,
					node->multicastRelayHost->strptr,node->multicastRelayPort,	node->networkMode->strptr);
			if(changed){
				dis_unregister((struct dis_socket*)node->_dsock,X3D_NODE(node));
				node->_registered = FALSE;
				node->_dsock = NULL;
			}
		}
	}
}
void compile_DIS_geo(struct X3D_EspduTransform *node){
	//Apr 2018 interpretation of geoSystem/geoCoords for DIS:
	//- world2body = world2tcs + tcs2body where tcs2body == translation
	// Scene
	//  geoCoords used like GeoLocation, to convert ordinary nodes to geospatial 
	//   transform using DIS
	//    children
	if(TRUE){
	//if(veclengthd(node->geoCoords.c) != 0.0){
		if(!node->__geoSystem || shallow_compare_node_fields(X3D_NODE(node),node->_oldState,FIELDS_geosys)){
			compile_geoSystem(X3D_NODE(node),node->_nodeType,&node->geoSystem,&node->__geoSystem);
			update_origin(GEOSYS(node->__geoSystem), X3D_NODE(node), &node->geoCoords, NULL);
		}
	}
}
void compile_DIS_common(struct X3D_EspduTransform *node){
	//INITIALIZE_EXTENT;
	compile_DIS_network(node);
	compile_DIS_geo(node);
}
void compile_DIS_common_OLD(struct X3D_EspduTransform *node){
	if(node->_oldState == NULL){
		//change detection 
		//later we'll copy the entire node after we detect any changed fields
		struct X3D_Node *old;
		old = createNewX3DNode0(node->_nodeType);
		//shallow_copy_node(old,X3D_NODE(node));
		node->_oldState = old; //I think one underscore means dispose
	}
	if(!node->_registered){
		void *psock;
		psock = dis_register(X3D_NODE(node),node->address->strptr,node->applicationID,node->entityID,node->multicastRelayHost->strptr,
		node->multicastRelayPort,
		node->networkMode->strptr, node->port,node->readInterval,node->rtpHeaderExpected,node->siteID,node->writeInterval);
		node->_registered = TRUE;
		node->_dsock = psock;
	}
	if(node->_registered){
		//almost every field is [in,out] so can be changed at runtime
		//IDEA: save duplicate of nodetype in _oldnode field
		if(shallow_compare_node_fields(X3D_NODE(node),node->_oldState,FIELDS_networksensor)){
			int changed;
			changed = dis_check_socket_change((struct dis_socket*)node->_dsock,node->address->strptr, node->port,
					node->multicastRelayHost->strptr,node->multicastRelayPort,	node->networkMode->strptr);
			if(changed){
				dis_unregister((struct dis_socket*)node->_dsock,X3D_NODE(node));
				node->_registered = FALSE;
				node->_dsock = NULL;
			}
		}
	}
	//Apr 2018 interpretation of geoSystem/geoCoords for DIS:
	//- world2body = world2tcs + tcs2body where tcs2body == translation
	// Scene
	//  geoCoords used like GeoLocation, to convert ordinary nodes to geospatial 
	//   transform using DIS
	//    children
	if(TRUE){
	//if(veclengthd(node->geoCoords.c) != 0.0){
		if(!node->__geoSystem || shallow_compare_node_fields(X3D_NODE(node),node->_oldState,FIELDS_geosys)){
			compile_geoSystem(X3D_NODE(node),node->_nodeType,&node->geoSystem,&node->__geoSystem);
			update_origin(GEOSYS(node->__geoSystem), X3D_NODE(node), &node->geoCoords, NULL);
		}
	}
}
// >> RADIO
void compile_TransmitterPdu0(struct X3D_TransmitterPdu *node){
	if(node->isNetworkReader){
		mark_changed_node_fields(X3D_NODE(node), node->_oldState, FIELDS_transmitter);
	}else if(node->isNetworkWriter){
		if(shallow_compare_node_fields(X3D_NODE(node),node->_oldState,FIELDS_transmitter)){
			node->_pduchange_transmitter = TRUE;
		}
	}
	freeMallocedNodeFields(node->_oldState);
	shallow_copy_node(node->_oldState,X3D_NODE(node));
}
void compile_SignalPdu0(struct X3D_SignalPdu *node){
	if(node->isNetworkReader){
		mark_changed_node_fields(X3D_NODE(node), node->_oldState, FIELDS_signal);
	}else if(node->isNetworkWriter){
		if(shallow_compare_node_fields(X3D_NODE(node),node->_oldState,FIELDS_signal)){
			node->_pduchange_signal = TRUE;
		}
	}
	freeMallocedNodeFields(node->_oldState);
	shallow_copy_node(node->_oldState,X3D_NODE(node));
}
void compile_ReceiverPdu0(struct X3D_ReceiverPdu *node){
	if(node->isNetworkReader){
		mark_changed_node_fields(X3D_NODE(node), node->_oldState, FIELDS_receiver);
	}else if(node->isNetworkWriter){
		if(shallow_compare_node_fields(X3D_NODE(node),node->_oldState,FIELDS_receiver)){
			node->_pduchange_receiver = TRUE;
		}
	}
	freeMallocedNodeFields(node->_oldState);
	shallow_copy_node(node->_oldState,X3D_NODE(node));
}
// << RADIO

void compile_EspduTransform0(struct X3D_EspduTransform *node){
	//we use the same _pduchange flags and _oldState for both receiving and sending
	// but could be split if needed
	if(node->isNetworkReader){
		if(node->_pduchange_es){
			mark_changed_node_fields(X3D_NODE(node), node->_oldState, FIELDS_em_info);
			mark_changed_node_fields(X3D_NODE(node), node->_oldState, FIELDS_es_force);
			mark_changed_node_fields(X3D_NODE(node), node->_oldState, FIELDS_es_deadreckoning);
			mark_changed_node_fields(X3D_NODE(node), node->_oldState, FIELDS_es_articulation);
			//mark_changed_node_fields(X3D_NODE(node), node->_oldState, FIELDS_es_transform);
		}
		if(node->_pduchange_collision){
			mark_changed_node_fields(X3D_NODE(node), node->_oldState, FIELDS_collision);
		}
		if(node->_pduchange_fire){
			mark_changed_node_fields(X3D_NODE(node), node->_oldState, FIELDS_fire);
		}
		if(node->_pduchange_fire || node->_pduchange_collision){
			//mark_changed_node_fields(X3D_NODE(node), node->_oldState, FIELDS_events);
		}
		if(node->_pduchange_detonation){
			mark_changed_node_fields(X3D_NODE(node), node->_oldState, FIELDS_detonation);
		}
		if(node->_pduchange_fire || node->_pduchange_detonation){
			mark_changed_node_fields(X3D_NODE(node), node->_oldState, FIELDS_munition);
			mark_changed_node_fields(X3D_NODE(node), node->_oldState, FIELDS_rate);
		}

		reset_node_pduchanged(X3D_NODE(node));

	}else if(node->isNetworkWriter){
		int es_info, es_force, es_deadreckoning, es_articulation;
		es_info = es_force = es_deadreckoning = es_articulation = FALSE;
		if(shallow_compare_node_fields(X3D_NODE(node),node->_oldState,FIELDS_em_info)){
			es_info = TRUE;
		}
		if(shallow_compare_node_fields(X3D_NODE(node),node->_oldState,FIELDS_es_force)){
			es_force = TRUE;
		}
		if(shallow_compare_node_fields(X3D_NODE(node),node->_oldState,FIELDS_es_deadreckoning)){
			es_deadreckoning = FALSE; //we'll do it elsewhere TRUE;
		}
		if(shallow_compare_node_fields(X3D_NODE(node),node->_oldState,FIELDS_es_articulation)){
			int i,n;
			struct X3D_EspduTransform *old = (struct X3D_EspduTransform *)node->_oldState;
			es_articulation = TRUE;
			node->articulationParameterArray.p = realloc(node->articulationParameterArray.p,16*sizeof(float));
			n = node->articulationParameterArray.n;
			for(i=0;i<8;i++){
				switch(i){
					case 0: if(node->set_articulationParameterValue0 != old->set_articulationParameterValue0) 
						node->articulationParameterArray.p[i] = node->set_articulationParameterValue0; n=max(n,i); break;
					case 1: if(node->set_articulationParameterValue0 != old->set_articulationParameterValue1) 
						node->articulationParameterArray.p[i] = node->set_articulationParameterValue1; n=max(n,i); break;
					case 2: if(node->set_articulationParameterValue0 != old->set_articulationParameterValue2) 
						node->articulationParameterArray.p[i] = node->set_articulationParameterValue2; n=max(n,i); break;
					case 3: if(node->set_articulationParameterValue0 != old->set_articulationParameterValue3) 
						node->articulationParameterArray.p[i] = node->set_articulationParameterValue3; n=max(n,i); break;
					case 4: if(node->set_articulationParameterValue0 != old->set_articulationParameterValue4) 
						node->articulationParameterArray.p[i] = node->set_articulationParameterValue4; n=max(n,i); break;
					case 5: if(node->set_articulationParameterValue0 != old->set_articulationParameterValue5) 
						node->articulationParameterArray.p[i] = node->set_articulationParameterValue5; n=max(n,i); break;
					case 6: if(node->set_articulationParameterValue0 != old->set_articulationParameterValue6) 
						node->articulationParameterArray.p[i] = node->set_articulationParameterValue6; n=max(n,i); break;
					case 7: if(node->set_articulationParameterValue0 != old->set_articulationParameterValue7) 
						node->articulationParameterArray.p[i] = node->set_articulationParameterValue7; n=max(n,i); break;
					default:
					break;
				}
			}
			node->articulationParameterCount = node->articulationParameterArray.n;
		}
		//printf("es %d inf %d for %d dr %d art %d\n ",node->_pduchange_es,es_info,es_force,es_deadreckoning,es_articulation);
		node->_pduchange_es = node->_pduchange_es || es_info || es_force || es_deadreckoning || es_articulation ? TRUE : FALSE;
		if(shallow_compare_node_fields(X3D_NODE(node),node->_oldState,FIELDS_collision)){
			node->_pduchange_collision = TRUE;
		}
		if(shallow_compare_node_fields(X3D_NODE(node),node->_oldState,FIELDS_events)){
			//node->_pduchange_collision = TRUE;
			//node->_pduchange_fire = TRUE;
		}
		if(shallow_compare_node_fields(X3D_NODE(node),node->_oldState,FIELDS_fire)){
			node->_pduchange_fire = TRUE;
		}
		if(shallow_compare_node_fields(X3D_NODE(node),node->_oldState,FIELDS_detonation)){
			node->_pduchange_detonation = TRUE;
		}
		if(shallow_compare_node_fields(X3D_NODE(node),node->_oldState,FIELDS_munition)){
			node->_pduchange_fire = TRUE;
			node->_pduchange_detonation = TRUE;
		}
		if(shallow_compare_node_fields(X3D_NODE(node),node->_oldState,FIELDS_rate)){
			node->_pduchange_fire = TRUE;
			node->_pduchange_detonation = TRUE;
		}
	}
	freeMallocedNodeFields(node->_oldState);
	shallow_copy_node(node->_oldState,X3D_NODE(node));

}

void compile_DISEntityManager0(struct X3D_DISEntityManager *node){
	//we use the same _pduchange flags and _oldState for both receiving and sending
	// but could be split if needed
	if(node->isNetworkReader){
		if(node->_pduchange_em_info){
			mark_changed_node_fields(X3D_NODE(node), node->_oldState, FIELDS_em_info);
		}
		if(node->_pduchange_create){
			if(node->addedEntities.n > 0)
				mark_changed_node_fields(X3D_NODE(node), node->_oldState, FIELDS_create);
		}
		if(node->_pduchange_remove){
			if(node->removedEntities.n > 0)
				mark_changed_node_fields(X3D_NODE(node), node->_oldState, FIELDS_remove);
		}
		reset_node_pduchanged(X3D_NODE(node));

	}else if(node->isNetworkWriter){
		if(shallow_compare_node_fields(X3D_NODE(node),node->_oldState,FIELDS_em_info)){
			node->_pduchange_em_info = TRUE;
		}
		if(shallow_compare_node_fields(X3D_NODE(node),node->_oldState,FIELDS_create)){
			if(node->addedEntities.n > 0)
				node->_pduchange_create = TRUE;
		}
		if(shallow_compare_node_fields(X3D_NODE(node),node->_oldState,FIELDS_remove)){
			if(node->removedEntities.n > 0)
				node->_pduchange_remove = TRUE;
		}
	}
	freeMallocedNodeFields(node->_oldState);
	shallow_copy_node(node->_oldState,X3D_NODE(node));
}





// http://movesinstitute.org/~mcgredo/MV3500/hla/1278.1-200X%20Draft%2016%20rev%2018.pdf
// p.663 DR formula naming:
//   rotation: fixed (F), rotating (R)
//   order of position function:  rate of position (P) (first order), rate of velocity (V) (second order)
//   coords: world coordinates (W)  body axis coordinates (B)
// p.665 dead reckoning formulas
enum {
STATIC  = 1,
DRM_FPW = 2,
DRM_RPW = 3,
DRM_RVW = 4,
DRM_FVW = 5,  //P = P0 + V0*dt + 1/2*A*dt^2  in world coords
DRM_FPB = 6,
DRM_RPB = 7,
DRM_RVB = 8,
DRM_FVB = 9,  //P = P0 + (local2world)x(V0b*dt + 1/2*Ab*dt^2) convert to world after computing in local/entity/b=body space
};
void dead_reckon(int drmethod, double dtime, float *p1, float *R1xyza, float *p0, float *v0, float *a0, float *R0xyza, float *RVxyza){
	// freewrl: when we say world here, we mean TCS.
	// TCS == topocentric coordinate system, aka LGS Local Geodetic System, see Geospatial component, GeoLocation
	// LCS == local coordinate system - see Geospatial component, precision requirements, == TCS of geoOrigin / autoOrigin
	// DIS Local ~= web3d TCS, except with axes swizzled (DIS -Z up, X north, X3D Y up, -Z north)
	// we convert DR parameters in world system to/from web3d geo TCS system during pdu2node / node2pdu
	// so all below formula world coords are in TCS 
	// any DR (dead reckoning) parameters in DIS-Local system are swizzled to/from web3d TCS convention in node2pdu and pdu2node
	// drmethod 1 - 9
	// dtime - time in seconds since last frame ie .01
	// p1 - output new location (TCS)
	// R1xyza - output new orientation (body2tcs)
	// p0 - location on last frame
	// v0 - linear velocity set on last send/recv
	// a0 - linear acceleration set on last send/recv
	// R0xyza - orientation on last frame Rbw
	// RVxyza - angular velocity, in entity/body
	switch(drmethod){
		//world coords
		case STATIC: //1
			veccopy3f(p1,p0);
			veccopy4f(R1xyza,R0xyza);
			break;
		case DRM_FPW: //2
			{
				float tmp[3];
				//update position
				// P = P0 + V0*dt
				vecadd3f(p1,p0,vecscale3f(tmp,v0,dtime));
				veccopy4f(R1xyza,R0xyza);
			}
			break;
		case DRM_RPW: //3

			{
				float tmp[3];
				Quaternion qv, q1, q0;
				//update position
				// P = P0 + V0*dt
				vecadd3f(p1,p0,vecscale3f(tmp,v0,dtime));
				//update rotation
				// Rwb1 = DR(dt) * Rwb0
				vrmlrot4f_to_quaternion(&q0,R0xyza);
				vrmlrot_to_quaternion(&qv,RVxyza[0],RVxyza[1],RVxyza[2],RVxyza[3]*dtime);
				quaternion_multiply(&q1,&q0,&qv);
				quaternion_to_vrmlrot4f(&q1,R1xyza);

			}
			break;
		case DRM_RVW: //4
			{
				//update position
				//P = P0 + V0*dt + 1/2*A*dt^2  in world coords
				float tmp3[3],tmp2[3],tmp1[3];
				Quaternion qv, q1, q0;

				vecadd3f(p1,p0,vecadd3f(tmp3,vecscale3f(tmp2,v0,dtime),vecscale3f(tmp1,a0,.5f*dtime*dtime)));
				//update rotation
				// Rwb1 = DR(dt) * Rwb0
				vrmlrot4f_to_quaternion(&q0,R0xyza);
				vrmlrot_to_quaternion(&qv,RVxyza[0],RVxyza[1],RVxyza[2],RVxyza[3]*dtime);
				quaternion_multiply(&q1,&q0,&qv);
				quaternion_to_vrmlrot4f(&q1,R1xyza);

			}
			break;
		case DRM_FVW: //5

			{
				//F=fixed rotation, V = 2nd order, W=world coords
				//E.7.2.2 p.666
				//update position
				//P = P0 + V0*dt + 1/2*A*dt^2  in world coords
				float tmp3[3],tmp2[3],tmp1[3];
				vecadd3f(p1,p0,vecadd3f(tmp3,vecscale3f(tmp2,v0,dtime),vecscale3f(tmp1,a0,.5f*dtime*dtime)));
				//update rotation
				veccopy4f(R1xyza,R0xyza);
			}
			break;
		
		//body/entity > A,V in body coords
		case DRM_FPB: //6
			{
				Quaternion qv, qa, q1, qbw;
				float deltap[3], att[3], tmp[3], tmp1[3], tmp2[3], tmp3[3];

				vrmlrot_to_quaternion(&qv,RVxyza[0],RVxyza[1],RVxyza[2],RVxyza[3]*dtime);
				vrmlrot4f_to_quaternion(&qbw,R0xyza);
				vecscale3f(tmp3,v0,dtime);
				quaternion_rotation3f(deltap,&qv,tmp3);
				quaternion_rotation3f(tmp2,&qbw,deltap); //world2body
				vecadd3f(p1,p0,tmp2);

				//update rotation
				veccopy4f(R1xyza,R0xyza);

			}
			break;
		case DRM_RPB: //7
			{
				Quaternion qv, qa, q1, qbw;
				float deltap[3], att[3], tmp[3], tmp1[3], tmp2[3], tmp3[3];

				vrmlrot_to_quaternion(&qv,RVxyza[0],RVxyza[1],RVxyza[2],RVxyza[3]*dtime);
				vrmlrot4f_to_quaternion(&qbw,R0xyza);
				vecscale3f(tmp3,v0,dtime);
				quaternion_rotation3f(deltap,&qv,tmp3);
				quaternion_rotation3f(tmp2,&qbw,deltap); //world2body
				vecadd3f(p1,p0,tmp2);

				//update rotation 
				// Rwb1 = DR(dt) * Rwb0
				quaternion_multiply(&q1,&qbw,&qv);
				quaternion_to_vrmlrot4f(&q1,R1xyza);

			}
			break;
		case DRM_RVB: //8
			{
				//p.669
				//I think I see 2 problems with the formula they give:
				//1. their R1, R2 formula divide by |w|^n and when |w| is 0, that's divide by zero 
				//   - should produce Identity matrix when |w| is zero
				//2. P = P0 + Rbw*(R1*Vb + R2*Ab)
				//  problem: when R1, R2 are Identity (when |w| 0), it doesn't look like V0*t + 1/2*A*t^2
				//	should be:
				//	P = P0 + Rbw*(R1*Vb*dt + R2*.5*Ab*dt*dt)
				// proposed simplification:
				// Rbb = Rv*dt (and maybe + Ra*.5*t^2) where bb means body pose update with dt
				// P = P0 + Rbw x Rbb(Vb*dt + Ab*.5*dt*dt)
				Quaternion qv, qa, q1, qbw;
				float deltap[3], att[3], tmp[3], tmp1[3], tmp2[3], tmp3[3];

				vrmlrot_to_quaternion(&qv,RVxyza[0],RVxyza[1],RVxyza[2],RVxyza[3]*dtime);
				vrmlrot4f_to_quaternion(&qbw,R0xyza);
				vecadd3f(tmp3,vecscale3f(tmp2,v0,dtime),vecscale3f(tmp1,a0,.5f*dtime*dtime));
				quaternion_rotation3f(deltap,&qv,tmp3);
				quaternion_rotation3f(tmp2,&qbw,deltap); //world2body
				vecadd3f(p1,p0,tmp2);

				//update rotation 
				// Rwb1 = DR(dt) * Rwb0
				quaternion_multiply(&q1,&qbw,&qv);
				quaternion_to_vrmlrot4f(&q1,R1xyza);

			}
			break;
		case DRM_FVB: //9
			{
				//P = P0 + (local2world)x(V0b*dt + 1/2*Ab*dt^2) convert to world after computing in local/entity/b=body space
				Quaternion qv, qa, q1, qbw;
				float deltap[3], att[3], tmp[3], tmp1[3], tmp2[3], tmp3[3];

				vrmlrot_to_quaternion(&qv,RVxyza[0],RVxyza[1],RVxyza[2],RVxyza[3]*dtime);
				vrmlrot4f_to_quaternion(&qbw,R0xyza);
				vecadd3f(tmp3,vecscale3f(tmp2,v0,dtime),vecscale3f(tmp1,a0,.5f*dtime*dtime));
				quaternion_rotation3f(deltap,&qv,tmp3);
				quaternion_rotation3f(tmp2,&qbw,deltap); //world2body
				vecadd3f(p1,p0,tmp2);

				//update rotation
				veccopy4f(R1xyza,R0xyza);  //no update for 9

			}
			break;
		
		default:
			//update translation
			veccopy3f(p1,p0);
			//update rotation
			veccopy4f(R1xyza,R0xyza);
			break;
	}

}
#define DRA_POS_THRSH 1.5
#define DRA_ORIENT_THRSH .175 //RADIANS about 10 degrees

int transform_within_DeadReckoningTolerance1(struct X3D_EspduTransform *node){
	int withintol = TRUE;
	float p0[3], gap[3];
	struct X3D_EspduTransform *oldstate = (struct X3D_EspduTransform *)node->_oldState;
	veccopy3f(p0,node->_p0.c); //oldstate->translation.c);

	vecdif3f(gap,p0,node->translation.c);
	if(veclength3f(gap) > DRA_POS_THRSH) 
		withintol = FALSE;
	return withintol;
}

void compile_EspduTransform1 (struct X3D_EspduTransform *node) { 
	if(node->isNetworkReader){
		if(node->_pduchange_es){
			mark_changed_node_fields(X3D_NODE(node), node->_oldState, FIELDS_es_transform);
			mark_changed_node_fields(X3D_NODE(node), node->_oldState, FIELDS_geo);
		}
	}else if(node->isNetworkWriter){
		if(shallow_compare_node_fields(X3D_NODE(node),node->_oldState,FIELDS_es_transform) 
		|| shallow_compare_node_fields(X3D_NODE(node),node->_oldState,FIELDS_geo)) {
			//node->_pduchange_es = TRUE;
			if(!transform_within_DeadReckoningTolerance1(node)) {
				node->_pduchange_es = TRUE;
			}
		}
	}
	//whether its reader, writer or standalone, we need to compile if it changed state
	if(shallow_compare_node_fields(X3D_NODE(node),node->_oldState,FIELDS_es_transform)){

		INITIALIZE_EXTENT;

		/* printf ("changed Transform for node %u\n",node); */
		node->__do_center = verify_translate ((GLfloat *)node->center.c);
		node->__do_trans = verify_translate ((GLfloat *)node->translation.c);
		node->__do_scale = verify_scale ((GLfloat *)node->scale.c);
		node->__do_rotation = verify_rotate ((GLfloat *)node->rotation.c);
		node->__do_scaleO = verify_rotate ((GLfloat *)node->scaleOrientation.c);

		node->__do_anything = (node->__do_center ||
				node->__do_trans ||
				node->__do_scale ||
				node->__do_rotation ||
				node->__do_scaleO);

		REINITIALIZE_SORTED_NODES_FIELD(node->children,node->_sortedChildren);
		MARK_NODE_COMPILED
	}

}
void compile_EspduTransform (struct X3D_EspduTransform *node) { 
	compile_DIS_common(node);  // must be first in case need to initialize _oldState
	compile_EspduTransform1(node);
	compile_EspduTransform0(node); //must be last to re-copy node to oldstate
	MARK_NODE_COMPILED
}

void espdu_update_by_dead_reckoning (struct X3D_EspduTransform *node) {
	int drmethod, wasTransmitted;
	float p1[3],v1[3],a1[3], RVxyza[4], R0xyza[4], R1xyza[4];
	float p0[3],v0[3],a0[3];
	double dtime;
	static int smoothing_frames = 230; //frame count, at 60fps would be 4 seconds, ideally this would be a smoothing time in seconds
	static int want_smoothing = 1; //0;

	
	wasTransmitted = FALSE;
	if(node->isNetworkReader){
		if(node->_lasttime == 0.0) 
			return;
		if(node->_pduchange_es){
			//node start or node received
			node->_change_count++;
			wasTransmitted = TRUE;
			if(want_smoothing && node->_change_count){
				vecdif3f(node->_smoothingDelta.c,node->translation.c,node->_p0.c);
				node->_smoothingCount = smoothing_frames;
				if(node->_change_count > 1) 
					node->_smoothingCount = 0;
			}
			veccopy3f(node->_p0.c,node->translation.c);
			veccopy4f(node->_r0.c,node->rotation.c);
		}
		veccopy3f(p0,node->_p0.c);
		//veccopy3f(p0,node->translation.c);
		veccopy3f(v0,node->linearVelocity.c);
		veccopy3f(a0,node->linearAcceleration.c);
		veccopy4f(RVxyza,node->_angularVelocity.c);
		veccopy4f(R0xyza,node->_r0.c);

	}
	if(node->isStandAlone){
		veccopy3f(p0,node->translation.c);
		veccopy3f(v0,node->linearVelocity.c);
		veccopy3f(a0,node->linearAcceleration.c);
		veccopy4f(RVxyza,node->_angularVelocity.c);
		veccopy4f(R0xyza,node->rotation.c);
	}
	if(node->isNetworkWriter){
		if(node->_sent){
			//to be fair, only use what you send
			wasTransmitted = TRUE;
			node->_sent = FALSE;
			veccopy3f(node->_p0.c,node->translation.c);
			veccopy4f(node->_r0.c,node->rotation.c);
		}
		veccopy3f(p0,node->_p0.c);
		veccopy3f(v0,node->linearVelocity.c);
		veccopy3f(a0,node->linearAcceleration.c);
		veccopy4f(RVxyza,node->_angularVelocity.c);
		veccopy4f(R0xyza,node->_r0.c);
	}
	if(node->_lastframetime == 0.0)
		veccopy3f(node->_p0.c,p0);
	if(node->_lastframetime > 0.0){
		dtime = TickTime() - node->_lastframetime; //lastime();
		drmethod = node->deadReckoning;
		//if(drmethod)
		//	if(!node->__geoSystem) drmethod = DRM_FVW; //if no geocoords, we'll assume transform is already in world coords
		dead_reckon(drmethod, dtime, p1, R1xyza, p0, v0, a0, R0xyza, RVxyza);
		veccopy3f(node->_p0.c,p1);
		veccopy4f(node->_r0.c,R1xyza);
		MARK_EVENT(X3D_NODE(node),offsetof(struct X3D_EspduTransform,_p0));
	}
	node->_lastframetime = TickTime();
	if(node->isNetworkReader){
		//update translation based on DR
		if(want_smoothing){
			//E.9 Smoothing p.678
			//just done on the receiver/isNetworkReader
			float psmooth[3], pzero[3], alpha;
			int n, i;
			node->_change++;
			i = node->_smoothingCount;
			n = smoothing_frames;
			alpha = 1.0f - (float)min(i,n)/(float)n;
			vecset3f(pzero,0.0f,0.0f,0.0f);
			veclerp3f(psmooth,pzero,node->_smoothingDelta.c,alpha);
			vecdif3f(node->translation.c,node->_p0.c,psmooth);
			node->_smoothingCount++;
		}else{
			veccopy3f(node->translation.c,node->_p0.c);
		}
		veccopy4f(node->rotation.c,node->_r0.c);
	}
}

/* do transforms, calculate the distance */
void prep_EspduTransform (struct X3D_EspduTransform *node) {
	if(node->isNetworkReader) espdu_update_by_dead_reckoning(node);
	COMPILE_IF_REQUIRED
	if(node->__geoSystem) 
		geoprep(GEOSYS(node->__geoSystem),&node->geoCoords); //prep_EspduTransform0(node); //has render_vp filter
	if(!node->isNetworkReader) espdu_update_by_dead_reckoning(node);
	/* rendering the viewpoint means doing the inverse transformations in reverse order (while poping stack),
		* so we do nothing here in that case -ncoder */


	/* printf ("prep_Transform, render_hier vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */

	/* do we have any geometry visible, and are we doing anything with geometry? */
	OCCLUSIONTEST

	if(!renderstate()->render_vp) {
		/* do we actually have any thing to rotate/translate/scale?? */
		push_transform_local_identity();

		if (node->__do_anything) {

			FW_GL_PUSH_MATRIX();
			FW_GL_PUSH_MATRIX(); //this is to get us a separate 4x4 matrix just for the stuff here
			FW_GL_LOAD_IDENTITY(); // .. wehich we will save for child_Transform to propagate its bbox up to its extent

			/* TRANSLATION */
			if (node->__do_trans)
				FW_GL_TRANSLATE_F(node->translation.c[0],node->translation.c[1],node->translation.c[2]);

			/* CENTER */
			if (node->__do_center)
				FW_GL_TRANSLATE_F(node->center.c[0],node->center.c[1],node->center.c[2]);

			/* ROTATION */
			if (node->__do_rotation) {
				FW_GL_ROTATE_RADIANS(node->rotation.c[3], node->rotation.c[0],node->rotation.c[1],node->rotation.c[2]);
			}

			/* SCALEORIENTATION */
			if (node->__do_scaleO) {
				FW_GL_ROTATE_RADIANS(node->scaleOrientation.c[3], node->scaleOrientation.c[0], node->scaleOrientation.c[1],node->scaleOrientation.c[2]);
			}


			/* SCALE */
			if (node->__do_scale)
				FW_GL_SCALE_F(node->scale.c[0],node->scale.c[1],node->scale.c[2]);

			/* REVERSE SCALE ORIENTATION */
			if (node->__do_scaleO)
				FW_GL_ROTATE_RADIANS(-node->scaleOrientation.c[3], node->scaleOrientation.c[0], node->scaleOrientation.c[1],node->scaleOrientation.c[2]);

			/* REVERSE CENTER */
			if (node->__do_center)
				FW_GL_TRANSLATE_F(-node->center.c[0],-node->center.c[1],-node->center.c[2]);

			{
				double mat[16];

				FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX,mat); //we got our local transform saved
				FW_GL_POP_MATRIX();
				FW_GL_TRANSFORM_D(mat); //now apply the above to prep for child_Tranform
				reset_transform_local(mat);
			}

		} 
	}

}


void fin_EspduTransform (struct X3D_EspduTransform *node) {
	OCCLUSIONTEST

	if(!renderstate()->render_vp) {
		pop_transform_local();
		if (node->__do_anything) {
			FW_GL_POP_MATRIX();
		}
	} else {
		/*Rendering the viewpoint only means finding it, and calculating the reverse WorldView matrix.*/
		if((node->_renderFlags & VF_Viewpoint) == VF_Viewpoint) {
			FW_GL_TRANSLATE_F(((node->center).c[0]),((node->center).c[1]),((node->center).c[2])
			);
			FW_GL_ROTATE_RADIANS(((node->scaleOrientation).c[3]),((node->scaleOrientation).c[0]),((node->scaleOrientation).c[1]),((node->scaleOrientation).c[2])
			);
			FW_GL_SCALE_F((float)1.0/(((node->scale).c[0])),(float)1.0/(((node->scale).c[1])),(float)1.0/(((node->scale).c[2]))
			);
			FW_GL_ROTATE_RADIANS(-(((node->scaleOrientation).c[3])),((node->scaleOrientation).c[0]),((node->scaleOrientation).c[1]),((node->scaleOrientation).c[2])
			);
			FW_GL_ROTATE_RADIANS(-(((node->rotation).c[3])),((node->rotation).c[0]),((node->rotation).c[1]),((node->rotation).c[2])
			);
			FW_GL_TRANSLATE_F(-(((node->center).c[0])),-(((node->center).c[1])),-(((node->center).c[2]))
			);
			FW_GL_TRANSLATE_F(-(((node->translation).c[0])),-(((node->translation).c[1])),-(((node->translation).c[2]))
			);
		}
	}
	if(node->__geoSystem) 
		geofin(GEOSYS(node->__geoSystem),&node->geoCoords); //has vp_render filters //fin_EspduTransform0(node);

} 
void render_munitions(struct X3D_EspduTransform *node){
	//I have no ideas. something about quantity, velocity, start/end or startpoint
	//a) update locations based on time and trajectory - like partical physics
	//b) render each munition instance
	if(!renderstate()->render_vp) {

		if(node->fired1){
			int i;
			struct X3D_EspduTransform * mnode;
			static int eventNumber = 0;
			if(node->eventNumber > eventNumber){
				node->firedTime = TickTime();
				eventNumber = node->eventNumber;
			}
			double dtime =  TickTime() - (double)node->munitionQuantity/(double)max(1,node->firingRate) - node->firedTime ;
			if(dtime > 5.0) return; //already finished
			mnode = (struct X3D_EspduTransform*)dis_find_registered_node_by_entityid(node->munitionEntityID,TRUE,TRUE);
			if(mnode){
				for(i=0;i<node->munitionQuantity;i++){
					//how about a 1 second gap between burst pals
					dtime = max(0.0,TickTime() - (double)i/(double)max(1,node->firingRate)  - node->firedTime);
					dtime = min(5.0,dtime);
					float delta[3], velocity[3], progress[3], loc[3];
					vecdif3f(delta,node->munitionEndPoint.c,node->munitionStartPoint.c);
					vecscale3f(velocity,delta,1.0f/3.0f);
					vecscale3f(progress,velocity,(float)dtime);
					if(veclength3f(progress) > veclength3f(delta)) {
						// detonate or whatever you do when munition reaches target
						veccopy3f(loc,node->munitionEndPoint.c);
						if(i==(node->munitionQuantity-1)){
							node->fired1 = FALSE; //last munition in burst hit target
							node->detonateTime = TickTime();
						}
					} else {
						vecadd3f(loc,node->munitionStartPoint.c,progress);
						// render munition instance
					}
					FW_GL_PUSH_MATRIX();
					FW_GL_TRANSLATE_F(loc[0],loc[1],loc[2]);
					//static int k = 0;
					//if(k++ % 120 == 0) 
					//	printf("%lf %lf %lf\n",loc[0],loc[1],loc[2]);
					//strip espdu wrapper (otherwise we have geoLocation wrapping geoLocation - double geo transform
					normalChildren(mnode->children);
					FW_GL_POP_MATRIX();
				}
			}
		}
	}
}
void render_detonation(struct X3D_EspduTransform *node){
	//I have no ideas. something about quantity, velocity, start/end or startpoint
	//a) update locations based on time and trajectory - like partical physics
	//b) render each munition instance
	if(!renderstate()->render_vp) {
		double dtime = TickTime() - node->detonateTime;
		if( dtime > 0.0 && dtime < .5){
			int i;
			struct X3D_EspduTransform * mnode;
			mnode = (struct X3D_EspduTransform*)dis_find_registered_node_by_entityid(node->munitionEntityID,TRUE,TRUE);
			if(mnode){
				for(i=0;i<node->munitionQuantity;i++){
					float loc[3], fscale;
					//how about a 1 second gap between burst pals
					veccopy3f(loc,node->detonationRelativeLocation.c);
					FW_GL_PUSH_MATRIX();
					FW_GL_TRANSLATE_F(loc[0],loc[1],loc[2]);
					fscale = dtime * 10.0f;
					FW_GL_SCALE_F(fscale,fscale,fscale);
					normalChildren(mnode->children);
					FW_GL_POP_MATRIX();
				}
			}
		}
	}
}
void dis_register_collide(struct X3D_Node* node,double *transform);
void child_EspduTransform (struct X3D_EspduTransform *node) {
	CHILDREN_COUNT
	OCCLUSIONTEST

	RETURN_FROM_CHILD_IF_NOT_FOR_ME

	/* any children at all? */
	if (nc==0) return;
	{
		double modelviewMatrix[16];
		FW_GL_GETDOUBLEV(GL_MODELVIEW_MATRIX, modelviewMatrix);
		dis_register_collide(X3D_NODE(node),modelviewMatrix);
	}
	prep_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);

	prep_BBox((struct BBoxFields*)&node->bboxCenter);

	normalChildren(node->_sortedChildren);
	//render munitions
	render_munitions(node);
	//render detonations
	render_detonation(node);
	//render collisions

	fin_BBox((struct X3D_Node*)node,(struct BBoxFields*)&node->bboxCenter,TRUE);

	fin_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);
}


// >> RADIO
// first parts of radio node structs ynchronized so as to match Espdu struct so
// the 3 radio nodes can be cast to EspduTransform for common field handling
void compile_TransmitterPdu (struct X3D_TransmitterPdu *node) { 
	compile_DIS_common((struct X3D_EspduTransform *)node); //assumes transform padding in transmitter node
	compile_TransmitterPdu0(node);
	MARK_NODE_COMPILED
}
void compile_SignalPdu (struct X3D_SignalPdu *node) { 
	compile_DIS_common((struct X3D_EspduTransform *)node); //assumes transform padding in signal node
	compile_SignalPdu0(node);
	MARK_NODE_COMPILED
}
void compile_ReceiverPdu (struct X3D_ReceiverPdu *node) { 
	compile_DIS_common((struct X3D_EspduTransform *)node); //assumes transform padding in receiver node
	compile_ReceiverPdu0(node);
	MARK_NODE_COMPILED
}

void child_TransmitterPdu (struct X3D_TransmitterPdu *node) { 
	COMPILE_IF_REQUIRED
	geoprep(GEOSYS(node->__geoSystem),&node->geoCoords);
	//do stuff
	geofin(GEOSYS(node->__geoSystem),&node->geoCoords);
}
void child_SignalPdu (struct X3D_SignalPdu *node) { 
	COMPILE_IF_REQUIRED
	geoprep(GEOSYS(node->__geoSystem),&node->geoCoords);
	//do stuff
	geofin(GEOSYS(node->__geoSystem),&node->geoCoords);
}
void child_ReceiverPdu (struct X3D_ReceiverPdu *node) { 
	COMPILE_IF_REQUIRED
	geoprep(GEOSYS(node->__geoSystem),&node->geoCoords);
	//do stuff
	geofin(GEOSYS(node->__geoSystem),&node->geoCoords);
}

//<< RADIO

void print_entitymapping(struct X3D_DISEntityTypeMapping *anode){
	ConsoleMessage("domain %d category %d country %d kind %d extra %d subcat %d spec %d\n",
	anode->domain, anode->category,anode->country, anode->kind, anode->extra, anode->subcategory, anode->specific);
}
void compile_DISEntityManager(struct X3D_DISEntityManager *node){
	compile_DIS_network((struct X3D_EspduTransform *)node);
	compile_DISEntityManager0(node);
	MARK_NODE_COMPILED
}
static int app_entity_last_id = 0;
int newEntityID(){
//for current app instance
	app_entity_last_id++;
	return app_entity_last_id;
}
#define GEOEL_WE_A	(double)6378137
void child_DISEntityManager(struct X3D_DISEntityManager *node){
	//Problem: web3d doesn't have a sender entitymanager. So its dependant on other (unknown) ?commercial? programs.
	//Solution 1: modify DISEntityManager to have networkMode='networkWriter' 
	// and an MFnode initializeOnly field of EntityTypeMapping nodes
	//Solution 2: 'entity discovery' by listening -> entity manager for creation
	// - done in the pdu receive loop, if there are 'leftover pdus' they are
	//   examined as candidates for entity discovery, and sent here via .addEntities
	static int ADD = 1, REMOVE = 2;
	COMPILE_IF_REQUIRED
	//like add remove children in opengl utils
	if(node->addEntities.n){
		int i,j;
		struct Multi_Node* mfn = &node->entities;
		node->addedEntities.n = 0;
		for(j=0;j<node->addEntities.n;j++){
			int ibest,iscore,jscore;
			int entityID, applicationID, siteID;
			int port, multicastRelayPort;
			struct Uni_String *address, *networkMode, *multicastRelayHost;
			struct X3D_Node *candi;
			struct X3D_DISEntityTypeMapping *best;
			int use_GC = FALSE;

			// = (struct X3D_DISEntityTypeMapping *)node->addEntities.p[j];
			ibest = -1;
			iscore = 0;
			best = NULL;
			candi = node->addEntities.p[j];
			if(candi->_nodeType == NODE_DISEntityTypeMapping)
			{
				//problem: the DISEntityTypeMapping doesn't have a field for entityID
				//	- that's OK when we are just told to create-and-own a new entity
				//		-we can assign our siteID, applicationID and increment our entityID count for entityID
				//  x but not for entity discovery
				//  * so we added some fields _entityID,_applicationID,_siteID for copying from sniffed pdu
				//      and sending to the .addEntities list
				//print_entitymapping(anode);
				struct X3D_DISEntityTypeMapping *anode = (struct X3D_DISEntityTypeMapping *)node->addEntities.p[j];
				struct Multi_Node* mapping = &node->mapping;
				if (mapping->n == 0) mapping = &node->children;
				for(i=0;i<mapping->n;i++){
					struct X3D_DISEntityTypeMapping *bnode = (struct X3D_DISEntityTypeMapping *)mapping->p[i];
					//printf("compare %d",i);
					//print_entitymapping(bnode);
					jscore = 0;
					if (!bnode->kind || anode->kind == bnode->kind) jscore++;
					if (!bnode->domain || anode->domain == bnode->domain) jscore++;
					if (!bnode->country || anode->country == bnode->country) jscore++;
					if (!bnode->category || anode->category == bnode->category) jscore++;
					if (!bnode->subcategory || anode->subcategory == bnode->subcategory) jscore++;
					if (!bnode->specific || anode->specific == bnode->specific) jscore++;
					if (!bnode->extra || anode->extra == bnode->extra) jscore++;
					if(jscore > iscore){
						iscore = jscore;
						ibest = i;
						best = bnode;
					}
				}
				printf("\niscore %d ibest %d best.url %s\n", iscore, ibest, best->url.p[0]->strptr);
				if(ibest > -1){
					applicationID = node->applicationID;
					siteID = node->siteID;
					entityID = newEntityID(); //anode->_entityID;
					address = node->address;
					port = node->port;
					networkMode = newASCIIString ("networkWriter"); //if we're ordered to create, usually that also means own
					multicastRelayHost = node->multicastRelayHost;
					multicastRelayPort = node->multicastRelayPort;

				}
			} else if(candi->_nodeType == NODE_EspduTransform) {
				// || candi->_nodeType == NODE_ReceiverPdu 
				//	|| candi->_nodeType == NODE_TransmitterPdu || candi->_nodeType == NODE_SignalPdu){
				//this comes from 'entity discovery' from leftovver pdus
				struct X3D_EspduTransform *anode = (struct X3D_EspduTransform *)node->addEntities.p[j];
				for(i=0;i<node->mapping.n;i++){
					struct X3D_DISEntityTypeMapping *bnode = (struct X3D_DISEntityTypeMapping *)node->mapping.p[i];
					//printf("compare %d",i);
					//print_entitymapping(bnode);
					jscore = 0;
					if (!bnode->kind || anode->entityKind == bnode->kind) jscore++;
					if (!bnode->domain || anode->entityDomain == bnode->domain) jscore++;
					if (!bnode->country || anode->entityCountry == bnode->country) jscore++;
					if (!bnode->category || anode->entityCategory == bnode->category) jscore++;
					if (!bnode->subcategory || anode->entitySubCategory == bnode->subcategory) jscore++;
					if (!bnode->specific || anode->entitySpecific == bnode->specific) jscore++;
					if(!bnode->extra || anode->entityExtra == bnode->extra) jscore++;
					if(jscore > iscore){
						iscore = jscore;
						ibest = i;
						best = bnode;
					}
				}
				//printf("etm  %d %d %d %d %d %d %d\n", best->kind, best->domain, best->country, best->category, 
				//	best->subcategory, best->specific, best->extra);
				//printf("es   %d %d %d %d %d %d %d\n", anode->entityKind, anode->entityDomain, anode->entityCountry, 
				//	anode->entityCategory, anode->entitySubCategory, anode->entitySpecific, anode->entityExtra);
				//printf("\niscore %d ibest %d best.url %s\n", iscore, ibest, best->url.p[0]->strptr);
				if(ibest > -1){
					applicationID = anode->applicationID;
					siteID = anode->siteID;
					entityID = anode->entityID;
					address = anode->address;
					port = anode->port;
					networkMode = newASCIIString ("networkReader"); //if we discovered entity by its heartbeats, then we're reading
					multicastRelayHost = anode->multicastRelayHost;
					multicastRelayPort = anode->multicastRelayPort;
					if(veclengthd(anode->geoCoords.c) < GEOEL_WE_A/2.0) use_GC = TRUE; //earths core GD,WE doesn't work well here 
				}
			}

			if(ibest > -1){
				int isgroup = 0;
				//printf("ibest = %d iscore= %d url=%s\n",ibest,iscore,best->url.p[0]->strptr);
				//if (best->_child == NULL) {
					struct X3D_Inline * iline;
					struct X3D_EspduTransform *espdu;
					//struct X3D_Group *grp;
					iline = createNewX3DNode(NODE_Inline); //this assigns a parent resource using parsing thread methods, which is wrong for rendering thread
					//resource_item_t *pres = iline->_parentResource;
					iline->_parentResource = X3D_PROTO(node->_executionContext)->_parentResource; //for rendering-thread creation of inlines, use the parent context's parentResource
					//if(isgroup){
					//	grp = createNewX3DNode(NODE_Group);
					//}else{
						//this is 'normal' according to specs we are supposed to generate espdus
						espdu = createNewX3DNode(NODE_EspduTransform);
						if(use_GC) {
							espdu->geoSystem.p[0] = newASCIIString("GC");
							espdu->geoSystem.n = 1;
						}
						//populate entity fields - so it starts swallowing the heartbeat and update pdus of the entity
						espdu->enabled = TRUE;
						espdu->isActive = TRUE;
						espdu->entityID = entityID;
						espdu->applicationID = applicationID;
						espdu->siteID = siteID;
						espdu->port = port;
						espdu->address = address;
						espdu->multicastRelayHost = multicastRelayHost;
						espdu->multicastRelayPort = multicastRelayPort;
						espdu->networkMode = networkMode;
						dis_set_node_lasttime(X3D_NODE(espdu),TickTime());

						/*
						void *dis_register(struct X3D_Node* node,char *address,int applicationID,int entityID,char *multicastRelayHost,
								int multicastRelayPort,
								char *networkMode, int port,double readInterval,int rtpHeaderExpected,int siteID,double writeInterval)
						*/
						dis_register(X3D_NODE(espdu),address->strptr,applicationID,entityID,multicastRelayHost->strptr,multicastRelayPort,
							networkMode->strptr,port,5.0,FALSE,siteID,5.0);
					//}
					//if(best->_executionContext){
						add_node_to_broto_context(X3D_PROTO(node->_executionContext),X3D_NODE(iline));
						//if(isgroup)
						//	add_node_to_broto_context(X3D_PROTO(node->_executionContext),X3D_NODE(grp));
						//else
							add_node_to_broto_context(X3D_PROTO(node->_executionContext),X3D_NODE(espdu));
					//}
					//best->_child = isgroup ? X3D_NODE(grp) : X3D_NODE(espdu);

					//ADD_PARENT(X3D_NODE(best->_child), X3D_NODE(best));
					//if(isgroup)
					//	AddRemoveChildren(X3D_NODE(grp),  &grp->children, (struct X3D_Node * *)&iline, 1, ADD,__FILE__,__LINE__);
					//else
						AddRemoveChildren(X3D_NODE(espdu),  &espdu->children, (struct X3D_Node * *)&iline, 1, ADD,__FILE__,__LINE__);
					/* copy over the URL from parent */
					shallow_copy_field(FIELDTYPE_MFString,(union anyVrml*)&best->url,(union anyVrml*)&iline->url);
					iline->load = TRUE;
				//}

				AddRemoveChildren(X3D_NODE(node),  mfn, (struct X3D_Node * *)&espdu, 1, ADD,__FILE__,__LINE__);
				//AddRemoveChildren(X3D_NODE(node),  &node->addedEntities, (struct X3D_Node * *)&best->_child, 1, ADD,__FILE__,__LINE__);
				AddRemoveChildren(X3D_NODE(node),  &node->addedEntities, (struct X3D_Node * *)&espdu, 1, ADD,__FILE__,__LINE__);

			}

		}
		if(node->addedEntities.n) MARK_EVENT(X3D_NODE(node),offsetof(struct X3D_DISEntityManager,addedEntities));
		node->addEntities.n = 0;
		FREE_IF_NZ(node->addEntities.p);
	}
	if(node->removeEntities.n){
		int i,j;
		struct Multi_Node* mfn = &node->entities;
		node->removedEntities.n = 0;
		for(j=0;j<node->removeEntities.n;j++){
			if(node->removeEntities.p[j]->_nodeType == NODE_DISEntityTypeMapping){
				int ibest,iscore,jscore;
				struct X3D_DISEntityTypeMapping *best, *anode = (struct X3D_DISEntityTypeMapping *)node->removeEntities.p[j];
				ibest = -1;
				iscore = 0;
				best = NULL;
				for(i=0;i<node->mapping.n;i++){
					if(node->mapping.p[i]->_nodeType == NODE_DISEntityTypeMapping){
						struct X3D_DISEntityTypeMapping *bnode = (struct X3D_DISEntityTypeMapping *)node->mapping.p[i];
						jscore = 0;
						if(anode->domain == bnode->domain) jscore++;
						if(anode->category == bnode->category) jscore++;
						if(anode->country == bnode->country) jscore++;
						if(anode->kind == bnode->kind) jscore++;
						if(anode->extra == bnode->extra) jscore++;
						if(anode->subcategory == bnode->subcategory) jscore++;
						if(anode->specific == bnode->specific) jscore++;
						if(jscore > iscore){
							iscore = jscore;
							ibest = i;
							best = bnode;
						}
					}
				}
				if(ibest > -1){
					//printf("remove: ibest = %d iscore= %d url=%s\n",ibest,iscore,best->url.p[0]->strptr);
					AddRemoveChildren(X3D_NODE(node),  mfn, (struct X3D_Node * *)&best, 1, REMOVE,__FILE__,__LINE__);
					if(best->_child)
						AddRemoveChildren(X3D_NODE(node),  &node->removedEntities, (struct X3D_Node * *)&best->_child, 1, ADD,__FILE__,__LINE__);
				}
				//else
				//	printf("remove: no match found\n");
			}
		}
		if(node->removedEntities.n) {
			//printf("removedEntities.n=%d\n",node->removedEntities.n);
			MARK_EVENT(X3D_NODE(node),offsetof(struct X3D_DISEntityManager,removedEntities));
		}
		node->removeEntities.n = 0;
	}
}
Stack *dis_collide_stack = NULL;
void dis_collide(){
	int i,j;
	if(dis_collide_stack){
		for(i=0;i<dis_collide_stack->n;i++){
			int ihit;
			float ee[6];
			double mvmInverse[16], m2m[16];
			struct X3D_EspduTransform *espdu;

			usehit *uhit = vector_get_ptr(usehit,dis_collide_stack,i);
			espdu = (struct X3D_EspduTransform*)uhit->node;
			if(espdu->isNetworkReader) continue; //do only OWNED/isWriter,isNeutral, listen for the rest
			ihit = 0;
			//invert matrix
			matinverseAFFINE(mvmInverse,uhit->mvm);
			extent6f_copy(ee,uhit->node->_extent);
			for(j=0;j<dis_collide_stack->n;j++){
				if(j != i){
					float eeb[6],eeba[6],eaXb[6];
					usehit *uhitb = vector_get_ptr(usehit,dis_collide_stack,j);
					extent6f_copy(eeb,uhitb->node->_extent);
					if(extent6f_isSet(eeb)){
						//multiply matrices
						matmultiplyAFFINE(m2m,mvmInverse,uhitb->mvm);
						//convert B extent to A-space
						extent6f_mattransform4d(eeba,eeb,m2m);
						//compare extents
						extent6f_intersect_extent6f(eaXb,ee,eeba);
						if(extent6f_isSet(eaXb)){
							//they overlap/intersect/collide
							//extent6f_printf(ee); printf("ee  \n"); 
							//extent6f_printf(eeb); printf("eeb \n"); 
							//extent6f_printf(eeba); printf("eeba\n"); 
							//extent6f_printf(eaXb); printf("eaXb\n");
							//we'll just change A, and just for its collistion with B
							struct X3D_EspduTransform *espdub = (struct X3D_EspduTransform*)uhitb->node;
							if(espdu->isCollided == FALSE){
								espdu->collideTime = TickTime();
								espdu->eventNumber = dis_next_event_number();
							}
							espdu->collisionType = 33;
							espdu->isCollided = TRUE;
							espdu->eventSiteID = espdub->siteID;
							espdu->eventApplicationID = espdub->applicationID;
							espdu->eventEntityID = espdub->entityID;
							ihit++;
							//H we automatically do this during node compile:
							MARK_EVENT(X3D_NODE(espdu),offsetof(struct X3D_EspduTransform,isCollided));
							break;
						}
					}
				}
			}
			if(ihit == 0) {
				if(espdu->isCollided){
					espdu->isCollided = FALSE;
					espdu->collideTime = 0.0;
					espdu->collisionType = 0;
					espdu->eventSiteID = 0;
					espdu->eventApplicationID = 0;
					espdu->eventEntityID = 0;
					MARK_EVENT(X3D_NODE(espdu),offsetof(struct X3D_EspduTransform,isCollided));
				}
			}
		}
	}
}
void dis_clear_collide(){
	if(dis_collide_stack) dis_collide_stack->n = 0;
}
void dis_register_collide(struct X3D_Node* node,double *transform){
	//call from child_espduTransform 
	int i, ifound;
	if(!dis_collide_stack) dis_collide_stack = newStack(usehit);
	ifound = -1;
	for(i=0;i<dis_collide_stack->n;i++){
		usehit *uhit = vector_get_ptr(usehit,dis_collide_stack,i);
		if(uhit->node == node){
			ifound = i;
			break;
		}
	}
	if(ifound == -1){
		usehit uhit;
		uhit.node = node;
		memcpy(uhit.mvm,transform,16*sizeof(double));
		uhit.userdata = NULL;
		vector_pushBack(usehit,dis_collide_stack,uhit);
	}

}
void dis_initialize() {
	//this is for the 2023 experimental multiplayer sensor synchronization and avatar update methods
	//it relies on freewrl commandline parameter values for DIS, rather than in-scene DIS node field values
	static int once = 0;
	if (!once) {
		// load default dis_socket
		int sport = fwl_get_DISport() + fwl_get_testset();
		char* address = strdup(fwl_get_DISaddress());
		int site = fwl_get_DISsite();
		int application = fwl_get_DISapplication();

		struct dis_socket dsock, * psock;
		//create a recv socket
		if (!sockets_recv) sockets_recv = newVector(struct dis_socket, 10);
		memset(&dsock, 0, sizeof(struct dis_socket));
		vector_pushBack(struct dis_socket, sockets_recv, dsock);
		psock = vector_get_ptr(struct dis_socket, sockets_recv, sockets_recv->n - 1);
		psock->address = address;
		psock->port = sport;
		psock->multicastRelayHost = strdup("");// multicastRelayHost;
		psock->multicastRelayPort = 0; // multicastRelayPort;
		psock->idir = 1;
		//open port
		dis_open_socket(psock);

		//create a send socket
		if (!sockets_send) sockets_send = newVector(struct dis_socket, 10);
		memset(&dsock, 0, sizeof(struct dis_socket));
		vector_pushBack(struct dis_socket, sockets_send, dsock);
		psock = vector_get_ptr(struct dis_socket, sockets_send, sockets_send->n - 1);
		psock->address = address;
		psock->port = sport;
		psock->multicastRelayHost = strdup("");// multicastRelayHost;
		psock->multicastRelayPort = 0;// multicastRelayPort;
		psock->idir = 2;
		//open port
		dis_open_socket(psock);
		once = 1;
	}
}
#else //WITH_DIS

void compile_DISEntityManager(struct X3D_DISEntityManager *node){}
void child_DISEntityManager(struct X3D_DISEntityManager *node){}
void compile_TransmitterPdu(struct X3D_TransmitterPdu *node){}
void child_TransmitterPdu(struct X3D_TransmitterPdu *node){}
void compile_SignalPdu(struct X3D_SignalPdu *node){}
void child_SignalPdu(struct X3D_SignalPdu *node){}
void compile_ReceiverPdu(struct X3D_ReceiverPdu *node){}
void child_ReceiverPdu(struct X3D_ReceiverPdu *node){}
void compile_EspduTransform (struct X3D_EspduTransform *node) {}
void prep_EspduTransform(struct X3D_EspduTransform *node){}
void fin_EspduTransform(struct X3D_EspduTransform *node){}
void child_EspduTransform(struct X3D_EspduTransform *node){}
void dis_initialize() {}
#endif //WITH_DIS

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
	if(allow_DIS){
#ifdef WITH_DIS
		//printf("yo from fwl_sendreceive_DIS\n");
		dis_initialize();
		dis_collide();
		dis_sendloop();
		dis_recvloop();
		dis_clear_collide();
#endif //WITH_DIS
	}
}
//
//#ifdef WITH_DIS
//#include "../DIS/DIS.c"
//#endif //WITH_DIS
