
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


static int allow_DIS = 0;
void fwl_init_DIS(){
	//from commandline --DIS or -D
	allow_DIS = 1;
}
int fwl_get_allow_DIS(){
	return allow_DIS;
}
void fwl_set_allow_DIS(int allow){
	allow_DIS = allow ? 1 : 0;
}

#ifdef WITH_DIS

/* DIS - Distributed Interactive Simulation communication
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
	float yaw, pitch, roll, x,y,z,a;
	x = xyza[0]; y = xyza[1], z=xyza[2], a=xyza[3];
	yaw = atan2(y,x);
	pitch = atan(z);
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
	float yaw, pitch, roll, x,y,z,a;
	yaw = ypr[0];
	pitch = ypr[1];
	roll = ypr[2];
	a = roll;
	x = cos(pitch)*cos(yaw);
	y = cos(pitch)*sin(yaw);
	z = sin(pitch); //or sqrt(1.0 - (x*x + y*y))
	xyza[0] = x;
	xyza[1] = y;
	xyza[2] = z;
	xyza[3] = a;
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

struct Vector * dis_node2pdus_espdu(struct X3D_Node *node){
	struct Vector *pdus;
	struct EntityStatePdu *espdu;
	struct CollisionPdu *cpdu;
	struct FirePdu *fpdu;
	struct X3D_EspduTransform * pnode = (struct X3D_EspduTransform*)node;
	espdu = (struct EntityStatePdu*)dis_ctor(type_EntityStatePdu);
	//fpdu = dis_ctor(pduToDis(type_FirePdu));
	//cpdu = dis_ctor(pduToDis(type_CollisionPdu));
	pdus = newVector(struct Pdu *, 4);
	//ENTITYSTATE
	//entity
	espdu->entityID.entity = pnode->entityID;
	espdu->entityID.application = pnode->applicationID;
	espdu->entityID.site = pnode->siteID;
	//translation - assumes companion scenes will have same parent transform stack
	//(x, -z, y).
	espdu->entityLocation.x = pnode->translation.c[0];
	espdu->entityLocation.y = -pnode->translation.c[2]; //??? is this right?
	espdu->entityLocation.z = pnode->translation.c[1];
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
	//...
	printf("new espdu protocol %d type %d\n",espdu->myEntityInformationFamilyPdu.myPdu.protocolVersion,espdu->myEntityInformationFamilyPdu.myPdu.pduType);
	vector_pushBack(struct Pdu*,pdus,(struct Pdu*)espdu);
	//FIRE
	//COLLISION
	//...
	return pdus;

}
int dis_pdus2node_espdu(struct X3D_Node *node, struct Vector *pdus){
	int i, ihit;
	struct Pdu* pdu;
	struct EntityStatePdu *espdu;
	struct CollisionPdu *cpdu;
	struct FirePdu *fpdu;
	struct X3D_EspduTransform * pnode = (struct X3D_EspduTransform*)node;

	ihit = 0;
	if(!pdus) return ihit;
	for(i=0;i<pdus->n;i++)
	{
		pdu = vector_get(struct Pdu*,pdus,i);
		switch(pdu->pduType){
			case PDU_ENTITY_STATE:
			{
				//ENTITYSTATE
				espdu = (struct EntityStatePdu*)pdu;
				if(espdu->entityID.application != pnode->applicationID) break;
				if(espdu->entityID.site != pnode->siteID) break;
				if(espdu->entityID.entity != pnode->entityID) break;
				ihit++;
				pnode->_change++; //mark node changed
				//translation - assumes companion scenes will have same parent transform stack
				//(x, -z, y).
				pnode->translation.c[0] = espdu->entityLocation.x;
				pnode->translation.c[1] = espdu->entityLocation.z;
				pnode->translation.c[2] = -espdu->entityLocation.y; 
				pnode->timestamp = TickTime();
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
				//articuation parameters
				pnode->articulationParameterArray.n = espdu->numberOfArticulationParameters;
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
						pp[i] = ap[i].parameterValue;
						//printf("%d %f\n",i,pp[i]);
						//ap[i].partAttachedTo = 0;
					}
					if(pnode->articulationParameterArray.p) free(pnode->articulationParameterArray.p);
					pnode->articulationParameterArray.p = pp;
					MARK_EVENT(X3D_NODE(pnode),offsetof(struct X3D_EspduTransform,articulationParameterArray));
				}

				//...
			}
			break;
			case PDU_FIRE:
			//FIRE
			break;
			case PDU_COLLISION:
			//COLLISION
			break;
			//...
			default:
				break;
		}
	}
	return ihit;
}
struct Vector * dis_node2pdus(struct X3D_Node *node){
	struct Vector *pdus = NULL;
	switch(node->_nodeType){
		case NODE_EspduTransform:
			pdus = dis_node2pdus_espdu(node);
			break;
		case NODE_ReceiverPdu:
		case NODE_TransmitterPdu:
		case NODE_SignalPdu:
		break;
	}
	return pdus;
}
static struct Vector *sockets_send = NULL;
static struct Vector *sockets_recv = NULL;

unsigned char buf2[32767];

void dis_get_node_lasttime(struct X3D_Node *node, double *lasttime, double *readInterval, double *writeInterval){
	switch(node->_nodeType){
		case NODE_EspduTransform:
		{
			struct X3D_EspduTransform *pnode = (struct X3D_EspduTransform*)node;
			*lasttime = pnode->_lasttime;
			*writeInterval = pnode->writeInterval;
			*readInterval = pnode->readInterval;
		}
		break;
		case NODE_ReceiverPdu:
		case NODE_TransmitterPdu:
		case NODE_SignalPdu:
		break;
	}
}
void dis_set_node_lasttime(struct X3D_Node *node, double lasttime){
	switch(node->_nodeType){
		case NODE_EspduTransform:
		{
			struct X3D_EspduTransform *pnode = (struct X3D_EspduTransform*)node;
			pnode->_lasttime = lasttime;
		}
		break;
		case NODE_ReceiverPdu:
		case NODE_TransmitterPdu:
		case NODE_SignalPdu:
		break;
	}
}


void dis_sendloop(){
	double thistime;
	int i,j, nbytes, nb;
	if(!sockets_send || sockets_send->n == 0) return;
	thistime = TickTime();
	for(i=0;i<sockets_send->n;i++){
		struct dis_socket *dsock = vector_get_ptr(struct dis_socket,sockets_send,i);
		if(dsock->registered){
			nbytes = 0;
			for(j=0;j<dsock->registered->n;j++){
				//options:
				//a. each node maintains its own pdus every frame on update/compile, and are merely sent here
				//b. on send in here, a function is called to pdu-ize a node before marshaling it
				//c. like a and b: each node has its own list of pdus for mem, and are updated in here just before send
				double lasttime, readInterval, writeInterval;
				struct Vector *pdus;
				struct X3D_Node *node = vector_get(struct X3D_Node*,dsock->registered,j);
				dis_get_node_lasttime(node,&lasttime,&readInterval,&writeInterval);
				if(writeInterval == 0.0) continue; //sentinal value 0 means don't write
				if(thistime - lasttime < writeInterval) continue; //skip for a while more
				lasttime = thistime;
				dsock->lasttime = thistime; //last time something was sent, not needed
				dis_set_node_lasttime(node,lasttime);
				if(j==0) {
					nb = write_rtp(&buf2[nbytes],node);
					nbytes += nb;
				}

				//option b.
				pdus = dis_node2pdus(node);
				if(pdus && pdus->n) {
					struct Pdu* pdu = vector_get(struct Pdu*,pdus,0);
					printf("in dis_sendloop pdu protocol %d pdutype %d\n",pdu->protocolVersion,pdu->pduType);
				}
				nb = dis_write_stream(&buf2[nbytes],pdus);
				if(0){
					//debug
					printf("sendloop >>>>\n");
					print_stream(&buf2[nbytes], nb);
					printf("<<<< sendloop\n");
				}
				nbytes += nb;
			}
			if(nbytes) socksendto(dsock,buf2,nbytes);
		}
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
		printf("pdu type=%d distype=%d",(int)pdutype, distype);
		
		pdubuf = dis_ctor(distype);
		carat2 = dis_unmarshal(carat,pdubuf,distype);
		nbytes = (carat2 - carat);
		pdu = (struct Pdu*)pdubuf;
		printf("un-marshed version %d pdutype= %d\n",pdu->protocolVersion,pdu->pduType);
		vector_pushBack(struct Pdu*,pdus,pdu);
		printf("unmarshed bits %d bytes %d\n",nbytes*8,nbytes);

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
		printf("bytes left = %d - %d = %d\n",streamsize, (int)(carat - datastream), streamsize - (int)(carat-datastream));
		printf("\n");
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
			printf("dis_wrt_str protocol %d pdutype %d\n",pdu->protocolVersion,pdu->pduType);
			int distype = pduToDis(pdu->pduType);
			carat = dis_marshal(carat,(unsigned char*)pdu,distype);
			//printf("pdu %d wrote %d bytes\n",i,nbytes);
		}
		//*streamsize = nbytes;
		nbytes = (int)(carat - datastream);
	}
	return nbytes;
}


//in socketutils.c:
void socket_open(struct dis_socket *dsock);
int sockwrite(SOCKET s, const char *buf, int len);
int sockread(SOCKET s, const char *buf, int len);
int sockrecvfrom(struct dis_socket *dsock, const char *buf, int len);
int socksendto(struct dis_socket *dsock, const char *buf, int len);

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
	double thistime, dtime;
	if(!sockets_recv || sockets_recv->n == 0) return;
	thistime = TickTime();
	dtime = thistime - lasttime;
	if(!pdus) pdus = newVector(struct Pdu*,20);

	//since not select()ing we have to check all sockets (if readInterval?)
	for(i=0;i<sockets_recv->n;i++){
		struct dis_socket *dsock = vector_get_ptr(struct dis_socket,sockets_recv,i);
		//things may have built up in the input socket, so we loop till flushed
		do{
			heard = FALSE;
			more = FALSE;
			nbytes = sockrecvfrom(dsock,buf,32000);
			if(nbytes > 0){
				more = TRUE;
				dsock->lasttime = thistime;
				printf("sock read nbytes = %d\n",nbytes);
				//free last round
				for(j=0;j<pdus->n;j++){
					struct Pdu* pdu = vector_get(struct Pdu*,pdus,j);
					dis_dtor((unsigned char *)pdu,pduToDis(pdu->pduType));
				}
				pdus->n = 0;
				dis_read_stream(buf,nbytes,pdus,&heard);
				//print some stuff to the console, to prove we got a state update
				printf("hallelluha\n");
				if(dsock->registered){
					for(j=0;j<dsock->registered->n;j++){
						int ihit;
						struct X3D_Node *node = vector_get(struct X3D_Node*,dsock->registered,j);
						//check site and application ID
						//distribute to registered nodes by entityID
						ihit = dis_pdus2node_espdu(node, pdus);
						if(ihit){
							if(heard) set_rtp_heard(node);
							dis_set_isActive(node,TRUE);
							dis_set_node_lasttime(node,thistime);
						}
					}
				}
			}
		}while(more);
		if(dsock->registered){
			//check if any node listeners have gone inactive
			for(j=0;j<dsock->registered->n;j++){
				//update isActive
				double readinterval, writeinterval, lasttime;
				struct X3D_Node *node = vector_get(struct X3D_Node*,dsock->registered,j);
				dis_get_node_lasttime(node,&lasttime,&readinterval,&writeinterval);
				if(thistime - lasttime > 5.0)   //5 second rule: if a node recvs nothing for 5 seconds, turn isActive to FALSE.
					dis_set_isActive(node,FALSE);
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
void *dis_register(struct X3D_Node* node,char *address,int applicationID,int entityID,char *multicastRelayHost,
		int multicastRelayPort,
		char *networkMode, int port,double readInterval,int rtpHeaderExpected,int siteID,double writeInterval){
	void *preg; //something to store in the node, to say which socket its registerd in
	int inetworkmode = 0;
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
			if(!strcmp(dsock->address,address) && dsock->port == port){
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
			dsock->port = port;
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
			if(!strcmp(dsock->address,address) && dsock->port == port){
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
			dsock->port = port;
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
	if(!strcmp(networkMode,"standAlone")) inetworkmode = 0;
	else if(!strcmp(networkMode,"networkReader")) inetworkmode = 1;
	else if(!strcmp(networkMode,"networkWriter")) inetworkmode = 2;
	if(inetworkmode == 0 && dsock == NULL) return FALSE; //not registered, and no need to register
	if(dsock == NULL) return TRUE; //need to register
	if(inetworkmode != dsock->idir) return TRUE; //change of direction
	if(strcmp(address,dsock->address)) return TRUE; //change of address
	if(port != dsock->port) return TRUE; //change of port
	if(strcmp(multicastRelayHost,dsock->multicastRelayHost)) return TRUE; 
	if(multicastRelayPort != dsock->multicastRelayPort) return TRUE;

	return FALSE;
}

void compile_DIS_common(struct X3D_EspduTransform *node){
	if(node->_registered){
		//almost every field is [in,out] so can be changed at runtime
		//IDEA: save duplicate of nodetype in _oldnode field
		int changed;
		changed = dis_check_socket_change((struct dis_socket*)node->_dsock,node->address->strptr, node->port,
				node->multicastRelayHost->strptr,node->multicastRelayPort,	node->networkMode->strptr);
		if(changed){
			dis_unregister((struct dis_socket*)node->_dsock,X3D_NODE(node));
			node->_registered = FALSE;
			node->_dsock = NULL;
		}
	}
	if(!node->_registered){
		void *psock;
		psock = dis_register(X3D_NODE(node),node->address->strptr,node->applicationID,node->entityID,node->multicastRelayHost->strptr,
		node->multicastRelayPort,
		node->networkMode->strptr, node->port,node->readInterval,node->rtpHeaderExpected,node->siteID,node->writeInterval);
		node->_registered = TRUE;
		node->_dsock = psock;
	}
	//Mar 2018 interpretation of geoSystem/geoCoords for DIS:
	//- specs say DIS coords are (x,-z,y) cartesian, they can never be geospatial like GD (lat,lon)
	//- and specs say children of espdu are translated by DIS coordinates in X3D order
	//- therefore geoCoordinates must apply to the local scene, and aren't transmitted to/from other DIS participants
	// Scene
	//  geoCoords used like GeoLocation, to convert ordinary nodes to geospatial 
	//   transform using DIS
	//    children
	// like we had wrapped espduTransform with a GeoLocation node
	compile_geoSystem(X3D_NODE(node),node->_nodeType,&node->geoSystem,&node->__geoSystem);
	update_origin(GEOSYS(&node->__geoSystem), X3D_NODE(node), &node->geoCoords, NULL);
}
void compile_TransmitterPdu0(struct X3D_TransmitterPdu *node){
}
void compile_SignalPdu0(struct X3D_SignalPdu *node){
}
void compile_ReceiverPdu0(struct X3D_ReceiverPdu *node){
}
void compile_EspduTransform0(struct X3D_EspduTransform *node){
	node->articulationParameterCount = node->articulationParameterArray.n;
}
void prep_EspduTransform0(struct X3D_EspduTransform *node){
	if(!renderstate()->render_vp) {
		geoprep(GEOSYS(node->__geoSystem),&node->geoCoords);
		/* did either we or the Viewpoint move since last time? */
		RECORD_DISTANCE
		if(renderstate()->render_boxes) extent6f_draw(node->_extent);
	}

}
void fin_EspduTransform0(struct X3D_EspduTransform *node){
	geofin(GEOSYS(node->__geoSystem),&node->geoCoords);
}

#else //WITH_DIS
void compile_DIS_common(struct X3D_EspduTransform *node){}
void compile_TransmitterPdu0(struct X3D_TransmitterPdu *node){}
void compile_SignalPdu0(struct X3D_SignalPdu *node){}
void compile_ReceiverPdu0(struct X3D_ReceiverPdu *node){}
void compile_EspduTransform0(struct X3D_EspduTransform *node){}
void prep_EspduTransform0(struct X3D_EspduTransform *node){}
void fin_EspduTransform0(struct X3D_EspduTransform *node){}
#endif //WITH_DIS


void compile_EspduTransform1 (struct X3D_EspduTransform *node) { 
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
void compile_EspduTransform (struct X3D_EspduTransform *node) { 
	compile_DIS_common(node);
	compile_EspduTransform0(node);
	compile_EspduTransform1(node);
	MARK_NODE_COMPILED
}

/* do transforms, calculate the distance */
void prep_EspduTransform (struct X3D_EspduTransform *node) {

	COMPILE_IF_REQUIRED
	prep_EspduTransform0(node);
	/* rendering the viewpoint means doing the inverse transformations in reverse order (while poping stack),
		* so we do nothing here in that case -ncoder */

	/* printf ("prep_Transform, render_hier vp %d geom %d light %d sens %d blend %d prox %d col %d\n",
	render_vp,render_geom,render_light,render_sensitive,render_blend,render_proximity,render_collision); */

	/* do we have any geometry visible, and are we doing anything with geometry? */
	OCCLUSIONTEST

	if(!renderstate()->render_vp) {
		/* do we actually have any thing to rotate/translate/scale?? */
		if (node->__do_anything) {

			FW_GL_PUSH_MATRIX();

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
		} 

		RECORD_DISTANCE

	}
}


void fin_EspduTransform (struct X3D_EspduTransform *node) {
	OCCLUSIONTEST

	if(!renderstate()->render_vp) {
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
	fin_EspduTransform0(node);
} 
void child_EspduTransform (struct X3D_EspduTransform *node) {
	//LOCAL_LIGHT_SAVE
	CHILDREN_COUNT
	OCCLUSIONTEST

	RETURN_FROM_CHILD_IF_NOT_FOR_ME

	/* any children at all? */
	if (nc==0) return;

	//if(node->__sibAffectors.n)
	//	printf("have transform sibaffectors\n");
	prep_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);


	//profile_start("local_light_kids");
	/* do we have a local light for a child? */
//	LOCAL_LIGHT_CHILDREN(node->_sortedChildren);
	//profile_end("local_light_kids");
	/* now, just render the non-directionalLight children */

	/* printf ("Transform %d, flags %d, render_sensitive %d\n",
			node,node->_renderFlags,render_sensitive); */

	#ifdef CHILDVERBOSE
		printf ("transform - doing normalChildren\n");
	#endif

	normalChildren(node->_sortedChildren);

	#ifdef CHILDVERBOSE
		printf ("transform - done normalChildren\n");
	#endif

//	LOCAL_LIGHT_OFF
	fin_sibAffectors((struct X3D_Node*)node,&node->__sibAffectors);
}


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
		dis_sendloop();
		dis_recvloop();
#endif //WITH_DIS
	}
}
//
//#ifdef WITH_DIS
//#include "../DIS/DIS.c"
//#endif //WITH_DIS
