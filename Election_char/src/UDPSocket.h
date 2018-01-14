/*
 * UDPSocket.h
 *
 *  Created on: 29/12/2017
 *      Author: eliseu
 */

#ifndef UDPSOCKET_H_
#define UDPSOCKET_H_

#include <time.h>

#include <cstring>            // For string
#include<string>
#include <sys/types.h>       // For data types
#include <sys/socket.h>      // For socket(), connect(), send(), and recv()
#include <netdb.h>           // For gethostbyname()
#include <arpa/inet.h>       // For inet_addr()
#include <unistd.h>          // For close()
#include <netinet/in.h>      // For sockaddr_in
#include<errno.h>
#include<climits>
#include <iostream>
#include <cstdlib>
#include <sys/ioctl.h>
#include <fcntl.h>

#define MSG_SIZE 30

class UDPSocket {
public:
	int sock;
	int n_timeouts = 0;

	UDPSocket();
	void CreateReceiveSock(int port,char* group);
	void SendDatagram(char* message);
	void CreateSendSock(int port, char* group);
	void ReceiveDatagram(char * msgbuf);
private:
    struct sockaddr_in addr;
    socklen_t addrlen;
	struct ip_mreq mreq;
	fd_set master_set;

	void SetNonBloking();
	void SetMulticast();
	void SetJoinGroup(char* group);
	void Bind();
};

#endif /* UDPSOCKET_H_ */
