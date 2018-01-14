/*
 * Node.h
 *
 *  Created on: 25/12/2017
 *      Author: eliseu
 */

#ifndef NODE_H_
#define NODE_H_

#include "UDPSocket.h"
#include <time.h>
#include <map>
#include <iostream>
#include <iterator>
#include <chrono>

namespace std {

typedef struct height{
	int init_electionTS = 0;
	int search_nodeID = 0;
	int search_deadend = 0;
	int leader_distance;
	int leader_electionTS = 0;
	int leaderID;
	int myID;
} height;


class Node {
public:
	// attributes
	chrono::time_point<std::chrono::system_clock> start;
	height myheight;
	string group;
	int port;
	UDPSocket SendSock;
	UDPSocket RecieveSock;
	map<int,UDPSocket> connections;
	map<int,height> neighbors;
	map<int,height> forming;

	// methods
	Node(int myID, int leaderID, int leader_distance,char* host , int port);
	void update(height height_j);
	void PrintHeight(height height_j);
	void CreateMessage(char * msgbuf, height height_j, string cmd);
	void LinkDown(int j_ID);
	void LinkUp(height height_j);

private:
	// attributes
	int first_c = 1;
	struct tm referenceTime = {0};

	// methods
	int CompareHeight(height h1,height h2);
	int CompareRefHeights(void);
	int CompareLPHeights(void);
	int Sink(height height_j);
	void ReflectRefLevel(void);
	void StartRefLevel(void);
	void PropagateLargestRefLevel(void);
	void AdoptLPifPriority(height height_j);
	void ElectSelf(void);
};

} /* namespace std */

#endif /* NODE_H_ */


