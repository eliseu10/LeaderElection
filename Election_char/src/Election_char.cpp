//============================================================================
// Name        : Election.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <vector>
#include "Node.h"

#define ERROR_SPLIT 9999

using namespace std;

height ConvertMessageToHeight();
int SplitMessage();

int main(int argc, char *argv[]) {

	int myID, leaderID = 90;
	int port = 3333;
	int leader_distance = 90;
	string group = "225.0.1.";

	cout << "Enter the ID of this node: ";
	cin >> myID;

	string mygroup = group + to_string(myID);
	char cstr[mygroup.length() + 1];
	strcpy(cstr, mygroup.c_str());

	Node myNode(myID, leaderID, leader_distance, cstr, port + myID);

	myNode.connections[myNode.myheight.myID] = myNode.RecieveSock;

	cout << endl;

	int cID;
	while(true){
		cout << "Enter the ID of the node that is running\n(0 to terminate configuration): ";
		cin >> cID;

		if(cID == 0)
			break;

		string group1 = group + to_string(cID);
		char cstr[group1.length() + 1];
		strcpy(cstr, group1.c_str());

		UDPSocket socket;
		socket.CreateReceiveSock(port + cID, cstr);
		myNode.connections[cID] = socket;

		UDPSocket socketSend;
		socketSend.CreateSendSock(port + cID, cstr);
		char msgConnect[MSG_SIZE];
		myNode.CreateMessage(msgConnect, myNode.myheight, "c " + to_string(cID) + " ");
		//cout << "c " + to_string(cID) + " " << endl;
		socketSend.SendDatagram(msgConnect);
	}


	char msgAlive[MSG_SIZE];
	msgAlive[0] = 'a';
	msgAlive[1] = ' ';

	char message[MSG_SIZE];
	while (true) {
		myNode.SendSock.SendDatagram(msgAlive);

		map<int, UDPSocket>::iterator ite;
		for (ite = myNode.connections.begin(); ite != myNode.connections.end(); ++ite) {

			message[0] = '\0';

			if (ite->second.n_timeouts < 10) {


				ite->second.ReceiveDatagram(message);

				if (message[0] == 'u') {
					cout << endl;
					//cout << "---------------------" << endl;
					//cout << "Received message: " << message << endl;

					strtok(message, " ");
					height height_j = ConvertMessageToHeight();

					if ((height_j.myID != ERROR_SPLIT)
							&& (height_j.myID != myNode.myheight.myID)
							&& (myNode.connections.find(height_j.myID) != myNode.connections.end())) {
						//cout << "HEIGHT J" << endl;
						//myNode.PrintHeight(height_j);
						myNode.update(height_j);
					}
				}

				if(message[0] == 'c'){

					cout << endl;
					//cout << "---------------------" << endl;
					//cout << "Received message: " << message << endl;

					strtok(message, " ");
					char* splitString = strtok(NULL, " ");
					int value = stoi(splitString, NULL,10);

					if(value == myNode.myheight.myID){

						height height_j = ConvertMessageToHeight();

						if ((height_j.myID != ERROR_SPLIT)
								&& (height_j.myID != myNode.myheight.myID)) {
							//cout << "HEIGHT J" << endl;
							//myNode.PrintHeight(height_j);
							UDPSocket socket;

							string group1 = group + to_string(height_j.myID);
							char cstr[group1.length() + 1];
							strcpy(cstr, group1.c_str());
							socket.CreateReceiveSock(port + height_j.myID, cstr);
							myNode.connections[height_j.myID] =  socket;
							myNode.LinkUp(height_j);
						}

					}

				}

			} else {
				//ocorreu um timeout
				myNode.LinkDown(ite->first);
				myNode.connections.erase(ite->first);
				//cout << "Timeout on conn: " << myNode.connections.size() << endl;
				break;
			}

		}
	}

}

height ConvertMessageToHeight() {
	height height_j;
	height_j.myID = ERROR_SPLIT;
	int value_int;



	if ((value_int = SplitMessage()) != ERROR_SPLIT) {
		height_j.myID = value_int;
	}

	if ((value_int = SplitMessage()) != ERROR_SPLIT) {
		height_j.init_electionTS = value_int;
	}

	if ((value_int = SplitMessage()) != ERROR_SPLIT) {
		height_j.leaderID = value_int;
	}

	if ((value_int = SplitMessage()) != ERROR_SPLIT) {
		height_j.leader_distance = value_int;
	}

	if ((value_int = SplitMessage()) != ERROR_SPLIT) {
		height_j.leader_electionTS = value_int;
	}

	if ((value_int = SplitMessage()) != ERROR_SPLIT) {
		height_j.search_deadend = value_int;
	}

	if ((value_int = SplitMessage()) != ERROR_SPLIT) {
		height_j.search_nodeID = value_int;
	}

	return height_j;
}

int SplitMessage(){
	int value_int;
	char* splitString = strtok(NULL, " ");
	if(splitString != NULL){
		value_int = stoi(splitString, NULL,10);
		return value_int;
	}else{
		return ERROR_SPLIT;
	}
}

