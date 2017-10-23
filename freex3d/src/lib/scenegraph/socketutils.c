		

/* socket bind can be over-ridden by pthread.h bind if you include that.
	which was happening for me.
	So isolating the socket stuff means I can use simpler sockets-only includes.
*/

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
int sockwrite(SOCKET s, const char *buf, int len){
	return send(s,buf,len,0);
}
int sockread(SOCKET s, const char *buf, int len){
	return recv(s,buf,len,0);
}

#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#define STRTOK_S strtok_r
int sockwrite(SOCKET s, const char *buf, int len){
	return write(s,buf,len);
}
int sockread(SOCKET s, const char *buf, int len){
	return recv(s,buf,len,0);
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
	int multicastRelayPort;
	char *multicastRelayHost;
	int idir; //0 = receive, 1 = send
	struct Vector *registered;
};

void socket_open(struct dis_socket *dsock)		
{
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
			addr.sin_addr.s_addr=htonl(INADDR_ANY); /* N.B.: differs from sender */
			addr.sin_port=htons(dsock->port);
     
			/* bind to receive address */
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
				printf("setsockopt0 ");
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
}