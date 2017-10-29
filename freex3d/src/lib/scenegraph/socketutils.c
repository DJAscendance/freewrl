		

/* socket bind can be over-ridden by pthread.h bind if you include that.
	which was happening for me.
	So isolating the socket stuff means I can use simpler sockets-only includes.
*/
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include "Component_DIS.h" //for dis_socket




#ifdef WIN32
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#define strdup _strdup
	#include <winsock2.h>	
	#include <ws2tcpip.h> /* for TCPIP - are we using tcp? */
	#define SHUT_RDWR SD_BOTH
	#include <windows.h>
	#define snprintf _snprintf
	//#define sscanf sscanf_s
	#define STRTOK_S strtok_s


char* ErrorString(DWORD err)
{
	//don't free the resulting string
	char * Error;
	LPTSTR s;
	if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		0,
		(LPTSTR)&s,
		0,
		NULL) == 0)
	{ // failed 
		// Unknown error code %08x (%d)
		static char * unk = "unknown error";
		Error = (char *)unk;
	} /* failed */
	else
	{ /* success */
		char* p = strchr(s, '\r');
		if(p != NULL)
		{ /* lose CRLF */
			*p = '\0';
		} /* lose CRLF */
		Error = s;
	} /* success */
	return Error;
} // ErrorString

void print_socket_error(char *message, int error){
	int lasterror = WSAGetLastError();
	printf("%s %d ",message,lasterror);
	printf(" %s \n",ErrorString(lasterror));
}

#else
	#include <sys/ioctl.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <errno.h>
	#include <netdb.h>
	#include <unistd.h>
	#define ioctlsocket ioctl
	#define SOCKET int
	#define SOCKET_ERROR SO_ERROR
	#define STRTOK_S strtok_r
void print_socket_error(char *message, int error){
	printf("%s %d \n",message,error);
}
#endif

#ifdef _MSC_VER
#include <direct.h>
#define chdir _chdir
#define strcasecmp _stricmp
WSADATA wsaData;
void initialize_sockets(){
	int iResult;
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
	}
}
#else
void initialize_sockets(){}
#endif



struct dis_socket {
	int port;
	char *address;
	SOCKET socket;
	struct sockaddr_in saddr;
	int multicastRelayPort;
	char *multicastRelayHost;
	int idir; //0 = receive, 1 = send
	struct Vector *registered;
};





#ifdef WIN32
int sockwrite(SOCKET s, const char *buf, int len){
	return send(s,buf,len,0);
}
int sockread(SOCKET s, const char *buf, int len){
	return recv(s,buf,len,0);
}

int socksendto(struct dis_socket *dsock, const char *buf, int len){
	int iret;
	if( iret = sendto(dsock->socket, buf, len, 0,
		(struct sockaddr *)&dsock->saddr, sizeof(struct sockaddr)) == SOCKET_ERROR){
		printf("sendto failed with error %d\n", WSAGetLastError());
	}
	return iret;
}

int sockrecvfrom(struct dis_socket *dsock, const char *buf, int len){
	// receive packet from socket
	int status, fromlen;
	fromlen = sizeof(struct sockaddr);
	status = recvfrom(dsock->socket, buf, len, 0, 
                     (struct sockaddr *)&dsock->saddr, &fromlen );
	// I think -1 is normal for non-blocking when no data
	// if(status < 0) print_socket_error("recvfrom ",status);
	return status;
}

#else
int sockwrite(SOCKET s, const char *buf, int len){
	return write(s,buf,len);
}
int sockread(SOCKET s, const char *buf, int len){
	return recv(s,buf,len,0);
}

int socksendto(struct dis_socket *dsock, const char *buf, int len){
        int iret;
        if( iret = sendto(dsock->socket, buf, len, 0,
                (struct sockaddr *)&dsock->saddr, sizeof(struct sockaddr)) == SOCKET_ERROR){
                printf("sendto failed with error %d\n", errno);
        }
	return iret;
}

int sockrecvfrom(struct dis_socket *dsock, const char *buf, int len){
        // receive packet from socket
        int status, fromlen;
        fromlen = sizeof(struct sockaddr);
        status = recvfrom(dsock->socket, buf, len, 0, 
                     (struct sockaddr *)&dsock->saddr, &fromlen );
        // I think -1 is normal for non-blocking when no data
        // if(status < 0) print_socket_error("recvfrom ",status);
        return status;
}

#endif



void socket_open(struct dis_socket *dsock)		
{
		//multicast socket
		int nbytes, npdus, addrlen, on=1;
		SOCKET sock;

		initialize_sockets();
		if(dsock->idir == 1){
			//struct sockaddr_in addr;
			int ier;
			struct ip_mreq mreq;
			//RECEIVE
			sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
			// FIONBIO
			setsockopt(sock, SOL_SOCKET,SO_REUSEADDR, (const char *)&on, sizeof(int));
			if( ier = ioctlsocket(sock, FIONBIO, (unsigned long *)&on) < 0)
			{
				print_socket_error("setting to nonblock failed",ier);
				close(sock);
			}
			memset(&dsock->saddr,0,sizeof(struct sockaddr_in));
			dsock->saddr.sin_family=AF_INET;
			dsock->saddr.sin_addr.s_addr=htonl(INADDR_ANY); /* N.B.: differs from sender */
			dsock->saddr.sin_port=htons(dsock->port);
     
			/* bind to receive address */
			if (bind(sock,(struct sockaddr *) &dsock->saddr,sizeof(struct sockaddr_in)) < 0) {
				printf("bind");
				#ifdef _MSC_VER
				printf("wsagetlasterror= %d\n",WSAGetLastError());
				#endif
				//goto exit;
			}
			mreq.imr_multiaddr.s_addr=inet_addr(dsock->address);
			mreq.imr_interface.s_addr=htonl(INADDR_ANY);
			if (setsockopt(sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char*)&mreq,sizeof(mreq)) < 0) {
				char *errstr;
				printf("setsockopt0 ");
				#ifdef _MSC_VER
				// https://msdn.microsoft.com/en-us/library/windows/desktop/ms740668(v=vs.85).aspx
				printf("wsagetlasterror= %d\n",WSAGetLastError());
				errstr = ErrorString(WSAGetLastError());
				printf("%s",errstr);
				#endif

				//goto exit;
			}
			printf("opened recv port\n");
			dsock->socket = sock;
		} else if(dsock->idir == 2){
			//SEND
			//http://www.tack.ch/multicast/
			SOCKET sockout;

			//struct sockaddr_in saddr;
			struct in_addr iaddr;
			unsigned char ttl = 3;
			unsigned char one = 1;

			// set content of struct saddr and imreq to zero
			memset(&dsock->saddr, 0, sizeof(struct sockaddr_in));
			memset(&iaddr, 0, sizeof(struct in_addr));

			// open a UDP socket
			sockout = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP); //0
			if ( sockout < 0 ){
				printf("Error creating socket");
				#ifdef _MSC_VER
				printf( "%d\n",WSAGetLastError());
				#endif
				
			}

			dsock->saddr.sin_family = PF_INET;
			dsock->saddr.sin_port = htons(0); // Use the first free port
			dsock->saddr.sin_addr.s_addr = htonl(INADDR_ANY); // bind socket to any interface
			if(0)
			if( bind(sockout, (struct sockaddr *)&dsock->saddr, sizeof(struct sockaddr_in)) == SOCKET_ERROR){
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
			if( bind(sockout, (struct sockaddr *)&dsock->saddr, sizeof(struct sockaddr_in)) == SOCKET_ERROR){
				printf("Error binding socket to interface");
				#ifdef _MSC_VER
				printf( "%d\n",WSAGetLastError());
				#endif
			}

			// set destination multicast address
			dsock->saddr.sin_family = PF_INET;
			dsock->saddr.sin_addr.s_addr = inet_addr(dsock->address);
			dsock->saddr.sin_port = htons(dsock->port);
			if(0)
			if( bind(sockout, (struct sockaddr *)&dsock->saddr, sizeof(struct sockaddr_in)) == SOCKET_ERROR){
				printf("Error binding socket to interface");
				#ifdef _MSC_VER
				printf( "%d\n",WSAGetLastError());
				#endif
			}
			dsock->socket = sockout;
			printf("opened send port\n");

		}
}