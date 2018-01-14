/*
 * UDPSocket.cpp
 *
 *  Created on: 29/12/2017
 *      Author: eliseu
 */

#include "UDPSocket.h"

UDPSocket::UDPSocket() {
	// TODO Auto-generated constructor stub
	addrlen=sizeof(addr);

    /* create what looks like an ordinary UDP socket */
    if ((UDPSocket::sock=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
	  perror("socket");
	  exit(1);
    }


}

void UDPSocket::SetMulticast(){
	 u_int yes=1;            /*** MODIFICATION TO ORIGINAL */

	/**** MODIFICATION TO ORIGINAL */
	    /* allow multiple sockets to use the same PORT number */
	    if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) {
	       perror("Reusing ADDR failed");
	       exit(1);
	       }
	/*** END OF MODIFICATION TO ORIGINAL */
}

void UDPSocket::SetJoinGroup(char* group){
    /* use setsockopt() to request that the kernel join a multicast group */
    mreq.imr_multiaddr.s_addr=inet_addr(group);
    mreq.imr_interface.s_addr=htonl(INADDR_ANY);
    if (setsockopt(sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0) {
	  perror("setsockopt");
	  exit(1);
    }
}

void UDPSocket::Bind(){
    /* bind to receive address */
    if (bind(sock,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
	  perror("bind");
	  exit(1);
    }
}

void UDPSocket::SetNonBloking(){
	unsigned long nonblocking_long = 1;
	int sts = ioctl(sock, FIONBIO, &nonblocking_long);

	if (sts < 0) {
		perror("ioctl() failed");
		close(sock);
		exit(-1);
	}
      /*
    int opts;
    opts = fcntl ( sock, F_SETFL,O_NONBLOCK );
    if(opts < 0){
    	perror("fcntl");
    	exit(1);
    }*/

}

void UDPSocket::CreateSendSock(int port, char* group){
    /* set up destination address */
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr(group);
    addr.sin_port=htons(port);
}

void UDPSocket::SendDatagram(char* message){
  	if (sendto(sock,message,MSG_SIZE,0,(struct sockaddr *) &addr, sizeof(addr)) < 0) {
       perror("sendto");
       exit(1);
  	}
}

void UDPSocket::CreateReceiveSock(int port, char* group){
	SetMulticast();
    /* set up destination address */
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY); /* N.B.: differs from sender */
    addr.sin_port=htons(port);
	Bind();
	//SetNonBloking();
	SetJoinGroup(group);

	fd_set socks;
	struct timeval tv;
	FD_SET(sock, &socks);
	tv.tv_sec = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
	    perror("Error");
	}
}

void UDPSocket::ReceiveDatagram(char * msgbuf) {

	if (recvfrom(sock, msgbuf, MSG_SIZE, 0, (struct sockaddr *) &addr, &addrlen) < 0) {
		//timeout reached
		n_timeouts++;
		//printf("Timeout number: %d\n", n_timeouts);
	}else {
		n_timeouts = 0;
	}

}


