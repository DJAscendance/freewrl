
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


//
//#ifdef _MSC_VER
//#include <direct.h>
//WSADATA wsaData;
//static int wsa_once = 0;
//void initialize_sockets(){
//	// Initialize Winsock
//	if(!wsa_once){
//		int iResult;
//		iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
//		if (iResult != 0) {
//			printf("WSAStartup failed: %d\n", iResult);
//		}
//		wsa_once = 1;
//	}
//}
//#else
//void initialize_sockets(){}
//#endif

//A. per-frame
struct dis_socket {
	int port;
	char *address;
	SOCKET socket;
	int multicastRelayPort;
	char *multicastRelayHost;
	int idir; //0 = receive, 1 = send
	struct Vector *registered;
};
static struct Vector *sockets_send = NULL;
static struct Vector *sockets_recv = NULL;
#ifdef WITH_DIS
void dis_sendloop(){
	if(!sockets_send || sockets_send->n == 0) return;
}
void dis_recvloop(){
	if(!sockets_recv || sockets_recv->n == 0) return;

}
#endif //WITH_DIS
void socket_open(struct dis_socket *dsock);

void dis_open_socket(struct dis_socket* dsock){
	if(dsock->multicastRelayHost && strlen(dsock->multicastRelayHost)){
		printf("[%s]\n",dsock->multicastRelayHost);
		printf("\n");
		//direct socket
	}else{
		socket_open(dsock);
/*
		//multicast socket
		int nbytes, npdus, addrlen, on=1;
		SOCKET sock;
		struct sockaddr_in addr;
		struct ip_mreq mreq;

		initialize_sockets();
		if(dsock->idir == 1){
			//RECEIVE
			sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
			// FIONBIO
			setsockopt(sock, SOL_SOCKET,SO_REUSEADDR, (const char *)&on, sizeof(int));
			memset(&addr,0,sizeof(addr));
			addr.sin_family=AF_INET;
			addr.sin_addr.s_addr=htonl(INADDR_ANY); // N.B.: differs from sender 
			addr.sin_port=htons(dsock->port);
     
			// bind to receive address 
			if (bind(sock,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
				printf("bind");
				#ifdef _MSC_VER
				printf("wsagetlasterror= %d\n",WSAGetLastError());
				#endif
				//goto exit;
			}

			mreq.imr_multiaddr.s_addr=inet_addr(dsock->address);
			mreq.imr_interface.s_addr=htonl(INADDR_ANY);
			if (setsockopt(sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char*)&mreq,sizeof(mreq)) < 0) {
				printf("setsockopt ");
				#ifdef _MSC_VER
				// https://msdn.microsoft.com/en-us/library/windows/desktop/ms740668(v=vs.85).aspx
				printf("wsagetlasterror= %d\n",WSAGetLastError());
				#endif

				//goto exit;
			}
			printf("opened port\n");
			dsock->socket = sock;
		} else if(dsock->idir == 2){
			//SEND
			//http://www.tack.ch/multicast/
			SOCKET sockout;

			struct sockaddr_in saddr;
			struct in_addr iaddr;
			unsigned char ttl = 3;
			unsigned char one = 1;

			// set content of struct saddr and imreq to zero
			memset(&saddr, 0, sizeof(struct sockaddr_in));
			memset(&iaddr, 0, sizeof(struct in_addr));

			// open a UDP socket
			sockout = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP); //0
			if ( sockout < 0 ){
				printf("Error creating socket");
				#ifdef _MSC_VER
				printf( "%d\n",WSAGetLastError());
				#endif
				
			}

			saddr.sin_family = PF_INET;
			saddr.sin_port = htons(0); // Use the first free port
			saddr.sin_addr.s_addr = htonl(INADDR_ANY); // bind socket to any interface
			if(0)
			if( bind(sockout, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in)) == SOCKET_ERROR){
				printf("Error binding socket to interface");
				#ifdef _MSC_VER
				printf( "%d\n",WSAGetLastError());
				#endif
				
			}

			iaddr.s_addr = INADDR_ANY; // use DEFAULT interface

			// Set the outgoing interface to DEFAULT
			if( setsockopt(sockout, IPPROTO_IP, IP_MULTICAST_IF, (const char*) &iaddr,
				sizeof(struct in_addr)) == SOCKET_ERROR){
				printf("sockopt1 erro");
				#ifdef _MSC_VER
				printf( "%d\n",WSAGetLastError());
				// https://msdn.microsoft.com/en-us/library/windows/desktop/ms740668(v=vs.85).aspx
				#endif
			}

			// Set multicast packet TTL to 3; default TTL is 1
			if( setsockopt(sockout, IPPROTO_IP, IP_MULTICAST_TTL, &ttl,
						sizeof(unsigned char)) == SOCKET_ERROR){
				printf("sockopt2 error");
				#ifdef _MSC_VER
				printf( "%d\n",WSAGetLastError());
				#endif
			}
			// send multicast traffic to myself too
			if(  setsockopt(sockout, IPPROTO_IP, IP_MULTICAST_LOOP,
								&one, sizeof(unsigned char)) == SOCKET_ERROR){
				printf("sockopt3 error");
				#ifdef _MSC_VER
				printf( "%d\n",WSAGetLastError());
				#endif
			}
			if(1)
			if( bind(sockout, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in)) == SOCKET_ERROR){
				printf("Error binding socket to interface");
				#ifdef _MSC_VER
				printf( "%d\n",WSAGetLastError());
				#endif
			}

			// set destination multicast address
			saddr.sin_family = PF_INET;
			saddr.sin_addr.s_addr = inet_addr(dsock->address);
			saddr.sin_port = htons(dsock->port);
			if(0)
			if( bind(sockout, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in)) == SOCKET_ERROR){
				printf("Error binding socket to interface");
				#ifdef _MSC_VER
				printf( "%d\n",WSAGetLastError());
				#endif
			}

		}
	*/
	}

}
void dis_register(struct X3D_Node* node,char *address,int applicationID,int entityID,char *multicastRelayHost,
		int multicastRelayPort,
		char *networkMode, int port,int readInterval,int rtpHeaderExpected,int siteID,int writeInterval){

	int inetworkmode = 0;
	if(!strcmp(networkMode,"standAlone")) inetworkmode = 0;
	else if(!strcmp(networkMode,"networkReader")) inetworkmode = 1;
	else if(!strcmp(networkMode,"networkWriter")) inetworkmode = 2;
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

	}

}
void dis_unregister(struct X3D_Node* node){
	//unregister from both sender and receiver
	if(sockets_recv){
		int i,j, ifound;
		struct X3D_Node *nr;
		ifound = -1;
		for(i=0;i<sockets_recv->n;i++){
			struct dis_socket *dsock;
			dsock = vector_get_ptr(struct dis_socket,sockets_recv,i);
			if(dsock->registered){
				ifound = -1;
				for(j=0;j<dsock->registered->n;j++){
					if(vector_get(struct X3D_Node*,dsock->registered,j)==node){
						vector_remove_elem(struct X3D_Node*,dsock->registered,j);
						ifound = j;
						break;
					}
				}
			}
			if(ifound > -1) break;
		}
	}
	if(sockets_send){
		int i,j, ifound;
		struct X3D_Node *nr;
		ifound = -1;
		for(i=0;i<sockets_send->n;i++){
			struct dis_socket *dsock;
			dsock = vector_get_ptr(struct dis_socket,sockets_send,i);
			if(dsock->registered){
				ifound = -1;
				for(j=0;j<dsock->registered->n;j++){
					if(vector_get(struct X3D_Node*,dsock->registered,j)==node){
						vector_remove_elem(struct X3D_Node*,dsock->registered,j);
						ifound = j;
						break;
					}
				}
			}
			if(ifound > -1) break;
		}
	}

}
int dis_check_socket_change(struct X3D_Node* node,char *address,int applicationID,int entityID,char *multicastRelayHost,
		int multicastRelayPort,
		char *networkMode, int port,int readInterval,int rtpHeaderExpected,int siteID,int writeInterval){
	return FALSE;
}

void compile_EspduTransform0(struct X3D_EspduTransform *node){
#ifdef WITH_DIS
	if(node->_registered){
		//almost every field is [in,out] so can be changed at runtime
		int changed;
		changed = dis_check_socket_change(X3D_NODE(node),node->address->strptr,node->applicationID,node->entityID,node->multicastRelayHost->strptr,
		node->multicastRelayPort,
		node->networkMode->strptr, node->port,node->readInterval,node->rtpHeaderExpected,node->siteID,node->writeInterval);
		if(changed){
			dis_unregister(X3D_NODE(node));
			node->_registered = FALSE;
		}
	}
	if(!node->_registered){
		dis_register(X3D_NODE(node),node->address->strptr,node->applicationID,node->entityID,node->multicastRelayHost->strptr,
		node->multicastRelayPort,
		node->networkMode->strptr, node->port,node->readInterval,node->rtpHeaderExpected,node->siteID,node->writeInterval);
		node->_registered = TRUE;
	}
#endif //WITH_DIS	
}
void prep_EspduTransform0(struct X3D_EspduTransform *node){
}
void fin_EspduTransform0(struct X3D_EspduTransform *node){
}


void compile_EspduTransform (struct X3D_EspduTransform *node) { 
	compile_EspduTransform0(node);
	compile_Transform((struct X3D_Transform*)node);
	MARK_NODE_COMPILED
}

/* do transforms, calculate the distance */
void prep_EspduTransform (struct X3D_EspduTransform *node) {

	prep_EspduTransform0(node);
	//else standalone
	prep_Transform((struct X3D_Transform *)node);
}

void fin_EspduTransform (struct X3D_EspduTransform *node) {
	fin_EspduTransform0(node);
	fin_Transform((struct X3D_Transform*)node);
} 

void child_EspduTransform (struct X3D_EspduTransform *node) {
	child_Transform((struct X3D_Transform*)node);
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

#ifdef WITH_DIS
#include "../DIS/DIS.c"
#endif //WITH_DIS
