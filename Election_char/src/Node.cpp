/*
 * Node.cpp
 *
 *  Created on: 25/12/2017
 *      Author: eliseu
 */

#include "Node.h"

namespace std {

Node::Node(int myID, int leaderID, int leader_distance,char* group, int port) {
	// TODO Auto-generated constructor stub
	myheight.myID = myID;
	myheight.leaderID = leaderID;
	myheight.leader_distance = leader_distance;
	Node::group = group;
	Node::port = port;

	SendSock.CreateSendSock(port, group);
	RecieveSock.CreateReceiveSock(port, group);

	// configure the reference time
	referenceTime.tm_hour = 0;
	referenceTime.tm_min = 0;
	referenceTime.tm_sec = 0;
	referenceTime.tm_year = 118; // 2018
	referenceTime.tm_mon = 0;   // dezembro
	referenceTime.tm_mday = 9;  // day 26
}

void Node::update(height height_j) {
	height my_old_height = myheight;
	cout << "HEIGHT_j: " ;
	PrintHeight(height_j);
	//insert in map
	neighbors[height_j.myID] = height_j;

	if ((height_j.leader_electionTS == myheight.leader_electionTS)
			&& (height_j.leaderID == myheight.leaderID)  && (height_j.leader_distance >= myheight.leader_distance)) {

		if ((Sink(height_j) == 1)) {

			if (CompareRefHeights() == 1) {
				// Neighbors have the same ref levels
				// init_electionTS, search_nodeID, search_deadend

				if ((myheight.init_electionTS > 0)
						&& (myheight.search_deadend == 0)) {
					// Atingui o deadend e reflete para os outros nos essa situacao
					ReflectRefLevel();

				} else if ((myheight.init_electionTS > 0)
						&& (myheight.search_deadend == 1)
						&& (myheight.search_nodeID == myheight.myID)) {
					// Percorreu todos os nos, logo pode eleger-se lider
					ElectSelf();

				} else if((myheight.init_electionTS == 0)
						|| ((myheight.init_electionTS > 0)
								&& (myheight.search_deadend == 1)
								&& (myheight.search_nodeID != myheight.myID))){
					// (init_electionTS == 0) or (search_nodeID != myID)
					// inicia uma pesquisa
					StartRefLevel();
				}

			} else {

				PropagateLargestRefLevel();
			}
		}
	} else {
		// têm lideres diferentes, tem de escolher entre manter ou mudar
		AdoptLPifPriority(height_j);
	}

	//my_old_height != myheight
	if ((CompareHeight(my_old_height, myheight) == 0) && (first_c == 1)) {
		char msgbuf[MSG_SIZE];
		CreateMessage(msgbuf, myheight, "u ");
		SendSock.SendDatagram(msgbuf);
	}
	cout << "MY_HEIGHT: " ;
	PrintHeight(myheight);
}

void Node::CreateMessage(char * msgbuf, height height_j, string cmd){
	string m = cmd + to_string(height_j.myID) +" "+ to_string(height_j.init_electionTS) +" "+ to_string(height_j.leaderID) +" "+
			to_string(height_j.leader_distance) +" "+ to_string(height_j.leader_electionTS) +" "+ to_string(height_j.search_deadend) +" "+
			to_string(height_j.search_nodeID);
	strcpy(msgbuf, m.c_str());
}

void Node::PrintHeight(height height_j){
	cout << height_j.init_electionTS << " "
			<< height_j.search_deadend << " "
			<< height_j.search_deadend << " "
			<< height_j.leader_distance << " "
			<< height_j.leader_electionTS << " "
			<< height_j.leaderID << " "
			<< height_j.myID << endl;
}

void Node::LinkDown(int j_ID){
	neighbors.erase(j_ID);
	cout << "Link Down." << endl;

	// neighbors doesn't have elements
	if(neighbors.empty()){

		ElectSelf();

	}else if((Sink(myheight)==1)){
		if (CompareRefHeights() == 1) {
			StartRefLevel();
			// send update
			char msgbuf[MSG_SIZE];
			CreateMessage(msgbuf, myheight, "u ");
			SendSock.SendDatagram(msgbuf);
		}
	}

}

void Node::LinkUp(height height_j){
	// send update
	cout << "Link UP." << endl;
	first_c = 0;
	update(height_j);
	first_c = 1;

	char msgbuf[MSG_SIZE];
	CreateMessage(msgbuf, myheight, "u ");
	SendSock.SendDatagram(msgbuf);
}

/*
 * Commpara valores dos pesos
 */
int Node::CompareHeight(height h1,height h2){
	if(h1.init_electionTS != h2.init_electionTS){
		//cout << "Different start election timestamp." << endl;
		return 0;
	}
	if(h1.leaderID != h2.leaderID){
		//cout << "Different leader ID." << endl;
		return 0;
	}
	if(h1.leader_distance != h2.leader_distance){
		//cout << "Different leader distance." << endl;
		return 0;
	}
	if(h1.leader_electionTS != h2.leader_electionTS){
		//cout << "Different election timestamp." << endl;
		return 0;
	}
	if(h1.myID != h2.myID){
		//cout << "Different node ID." << endl;
		return 0;
	}
	if(h1.search_deadend != h2.search_deadend){
		//cout << "Different deadend value." << endl;
		return 0;
	}
	if(h1.search_nodeID != h2.search_nodeID){
		//cout << "Different search node ID." << endl;
		return 0;
	}

	//cout << "The heights are the same." << endl;
	return 1;
}

int Node::CompareRefHeights(){
	map<int,height>::iterator ite;
	for (ite = neighbors.begin(); ite != neighbors.end(); ++ite) {
		if(ite->second.search_nodeID != myheight.search_nodeID){
			//cout << "Different search node ID." << endl;
			return 0;
		}
		if(ite->second.search_deadend != myheight.search_deadend){
			//cout << "Different search deadend value." << endl;
			return 0;
		}
		if(ite->second.init_electionTS != myheight.init_electionTS){
			//cout << "Different inicial election TS." << endl;
			return 0;
		}
	}

	return 1;
}

int Node::CompareLPHeights(){
	map<int,height>::iterator ite;
	for (ite = neighbors.begin(); ite != neighbors.end(); ++ite) {
		if(ite->second.leaderID != myheight.leaderID){
			//cout << "Different leader ID." << endl;
			return 0;
		}
		if(ite->second.leader_electionTS != myheight.leader_electionTS){
			//cout << "Different leader election TS." << endl;
			return 0;
		}
	}
	return 1;
}

int Node::Sink(height height_j){


	if(CompareLPHeights() == 0){
		return 0;
	}
	if(myheight.leaderID == myheight.myID){
		//cout << "This node is the current leader." << endl;
		return 0;
	}

	// check the minimal distance
	map<int, height>::iterator ite;
	int minDistance = neighbors.begin()->second.leader_distance;
	for (ite = neighbors.begin(); ite != neighbors.end(); ++ite) {

		if ((ite->second.leader_distance < minDistance) && (height_j.myID != ite->first)) {

			minDistance = ite->second.leader_distance;
		}
	}
	if (height_j.leader_distance > minDistance ||
			(myheight.leader_distance > minDistance && height_j.myID == myheight.leaderID)){
		//cout << "This node doesn't have the minimal distance to leader." << endl;
		return 0;
	}


	// check the minimal leader election
	int minInitSearch = neighbors.begin()->second.leader_electionTS;
	for (ite = neighbors.begin(); ite != neighbors.end(); ++ite) {
		if ((ite->second.leader_electionTS > minInitSearch))  {
			minInitSearch = ite->second.leader_electionTS;
		}
	}
	if (myheight.leader_electionTS > minInitSearch) {
		//cout << "This node doesn't have the minimal search timestamp." << endl;
		return 0;
	}



	// check the minimal id that started search
	int minSearchNode = neighbors.begin()->second.search_nodeID;
	for (ite = neighbors.begin(); ite != neighbors.end(); ++ite) {
		if ((ite->second.search_nodeID > minSearchNode) && (height_j.myID != ite->first)) {
			minSearchNode = ite->second.search_nodeID;
		}
	}
	if (height_j.search_nodeID > minSearchNode) {
		//cout << "This node doesn't have the minimal search node id." << endl;
		return 0;
	}


	// check the minimal deadend
	int minDeadend = neighbors.begin()->second.search_deadend;
	for (ite = neighbors.begin(); ite != neighbors.end(); ++ite) {
		if ((ite->second.search_deadend > minDeadend)) {
			minDeadend = ite->second.search_deadend;
		}
	}
	if (height_j.search_deadend > minDeadend) {
		//cout << "This node doesn't have the minimal value of deadend." << endl;
		return 0;
	}

	// check the minimal node
	int minNodeID = neighbors.begin()->second.myID;
	for (ite = neighbors.begin(); ite != neighbors.end(); ++ite) {
		if ((ite->second.myID > minNodeID)) {
			minNodeID = ite->second.myID;
		}
	}
	if (height_j.myID > minNodeID) {
		//cout << "This node doesn't have the minimal value of deadend." << endl;
		return 0;
	}

	return 1;
}

void Node::ReflectRefLevel(void){
	cout << "Node " << myheight.myID << " it's a deadend." << endl;
	//cout << "Reflect to the other nodes." << endl;
	myheight.search_deadend = 1;
	myheight.leader_distance = 0;
}

void Node::StartRefLevel(void){
	 start = chrono::system_clock::now();

	cout << "Node " << myheight.myID << " start a search." << endl;
	myheight.search_nodeID = myheight.myID;
	myheight.search_deadend = 0;
	myheight.leader_distance = 0;

	//get the current time difference
	time_t timer;
	time(&timer);
	int time = difftime(timer,mktime(&referenceTime));
	myheight.init_electionTS = time;
}

void Node::PropagateLargestRefLevel(void){
	// search for the max Ref levels
	cout << "Propagate the ref levels" << endl;

	map<int,height>::iterator ite;
	int maxInitElectionTS = neighbors.begin()->second.init_electionTS;
	int maxID = neighbors.begin()->second.myID;

	for (ite = neighbors.begin(); ite != neighbors.end(); ++ite) {
		if(ite->second.init_electionTS > maxInitElectionTS){
			maxInitElectionTS = ite->second.init_electionTS;
			maxID =ite->first;
		}
	}

	myheight.init_electionTS = neighbors[maxID].init_electionTS;
	myheight.search_deadend = neighbors[maxID].search_deadend;
	myheight.search_nodeID = neighbors[maxID].search_nodeID;

	// search minimal distance
	int minID = neighbors.begin()->second.myID;
	int minDistance = neighbors.begin()->second.leader_distance;
	for (ite = neighbors.begin(); ite != neighbors.end(); ++ite){

		if((ite->second.search_nodeID == myheight.search_nodeID) &&
				(ite->second.search_deadend == myheight.search_deadend) &&
				(ite->second.init_electionTS == myheight.init_electionTS)){

			if(ite->second.leader_distance < minDistance){
				minID = ite->first;
				minDistance = ite->second.leader_distance;
			}
		}
	}
/*
	myheight.init_electionTS = neighbors[minID].init_electionTS;
	myheight.search_deadend = neighbors[minID].search_deadend;
	myheight.search_nodeID = neighbors[minID].search_nodeID;
*/
	myheight.leader_distance = (neighbors[minID].leader_distance) - 1;
}

void Node::AdoptLPifPriority(height height_j){

	/*
	// search minimal distance
	map<int,height>::iterator ite;
	int minID = neighbors.begin()->second.myID;
	int minDistance = neighbors.begin()->second.leader_distance;
	for (ite = neighbors.begin(); ite != neighbors.end(); ++ite) {

		if (ite->second.leader_distance < minDistance) {
			minID = ite->first;
			minDistance = ite->second.leader_distance;
		}
	}*/

	if((height_j.leader_electionTS < myheight.leader_electionTS) ||
			((height_j.leader_electionTS == myheight.leader_electionTS) && (height_j.leaderID < myheight.leaderID)) ||
			((height_j.leader_electionTS == myheight.leader_electionTS) && (height_j.leaderID == myheight.leaderID) && (height_j.leader_distance < myheight.leader_distance))){

		myheight.init_electionTS = height_j.init_electionTS;
		myheight.leaderID = height_j.leaderID;
		myheight.leader_distance = (height_j.leader_distance) + 1;
		myheight.leader_electionTS = height_j.leader_electionTS;
		myheight.search_deadend = height_j.search_deadend;
		myheight.search_nodeID = height_j.search_nodeID;
		cout << "Adopt LP." << endl;
		//cout << "Height was changed." << endl;
	}
}

void Node::ElectSelf(void){
	cout << "Node " << myheight.myID << " elect self as leader." << endl;
	myheight.search_nodeID = 0;
	myheight.search_deadend = 0;
	myheight.init_electionTS = 0;
	myheight.leader_distance = 0;
	myheight.leaderID = myheight.myID;

	//get the current time difference
	time_t timer;
	time(&timer);
	int time = difftime(timer,mktime(&referenceTime));
	myheight.leader_electionTS = -time;

	chrono::time_point<chrono::system_clock> end = chrono::system_clock::now();

	cout << "Delta t2-t1: "
	              << std::chrono::duration_cast<chrono::milliseconds>(end-start).count()
	              << " milliseconds" << endl;
}

} /* namespace std */
