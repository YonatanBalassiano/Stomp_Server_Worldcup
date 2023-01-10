#include "../include/StompProtocol.h" 
#include "../include/summarize.h"
typedef StompProtocol::Command Command;
typedef StompProtocol::Frame Frame;
#include <string>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <iostream>
#include "../include/json.hpp"
#include "../include/ConnectionHandler.h"
#include "../include/event.h"
#include <filesystem>
#include <unistd.h>
#include <queue>
#include <mutex>



#include <bits/stdc++.h>
using namespace std;
using json = nlohmann::json;



void receiveData(ConnectionHandler *connectionHandler,  queue<Frame> &q, mutex &mutex,summarize &summarize,int &terminate){
	StompProtocol stompProtocol;
	while (terminate !=-3){
		string frame;
		
		if (!connectionHandler -> getFrameAscii(frame,'\0')) {
				std::cout << "Disconnected. Exiting...\n" << std::endl;
				break;
			}
			cout << "FRAME ----------------------- \n" << frame << endl;
			Frame temp = stompProtocol.stringToStomp(frame);
			temp = temp.command == Command::MESSAGE ? stompProtocol.stringToStompForMessage(frame) : temp;
			if(temp.command == Command::MESSAGE){
				summarize.add(temp.headers["destination"],temp.headers["user"],temp);
			}
			else if(temp.command == Command::ERROR ){
					terminate = -2;
					lock_guard<std::mutex> lock (mutex);
					q.push(temp);
			}
			else{
				if(temp.command == Command::RECEIPT && temp.headers["receipt-id"] == std::to_string(terminate) ){
					std::cout << "terminated" << endl;
					terminate = -2;
				}
				lock_guard<std::mutex> lock (mutex);
				q.push(temp);
			}
		
	}
	std::cout << "second thread terminated" << endl;
}

void sendFrame(Frame &frame, StompProtocol &stompProtocol, ConnectionHandler *connectionHandler){
	std::string frameLine = frame.command == Command::SEND ? stompProtocol.stompToStringForReports(frame) : stompProtocol.stompToString(frame);
		if (!connectionHandler-> sendFrameAscii(frameLine,'\0')) {
			std::cout << "Disconnected. Exiting...\n" << std::endl;
		}
}


int main(int argc, char *argv[]) {
	std::cout << "Starting client" << endl;
	bool shouldStop = false;


	//STRART of main while loop
	while (!shouldStop){
		std::cout << "please Login" << endl;
			//fields for the client
		bool isConnected = false, isLogin = false;
		int idCounter = 1, receiptCounter = 1,  error_logout = -1;
		map<std::string, std::string> subscriptions, receipts;
		StompProtocol stompProtocol;
		string host = "", user = "", password = "", acceptVersion = "1.2";
		ConnectionHandler *connectionHandler = nullptr;
		short bufsize = 1024, port = 0;
		char buf[bufsize];
		queue<Frame> q;
		set<string> receiptIds;
		mutex mutex;
		summarize summarize;

	//try to connect to server
	while(!isConnected){
		std::cin.getline(buf, bufsize);
		std::string line(buf);

		if(line.substr(0,5) == "login"){
			//read the line from the terminal
			stringstream ss(line);
			string word;
			string words [4];
			int i = 0;
			while (ss >> word) {
			 	words[i] = word;
				i++;
			}

			//check for correctness of the inLine
			if(i<3){
				std::cout << "too few arguments. please try again" << endl;
				break; 
			}

			//assign argument
			host = words[1].substr(0,words[1].find(':'));
			port = (short)std::stoi(words[1].substr(words[1].find(':')+1));
			user = words[2];
			password = words[3];
			
			//connect to server
			connectionHandler = new ConnectionHandler(host, port);
			if (!connectionHandler -> connect()) {
				//socketError
				std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
				return 1;
			}
			else{
				isConnected = true;
			}
		}
		else{
			std::cerr << "Please login first";
		}
	}

	//START NEW THREAD
    std::thread receive_thread(receiveData, connectionHandler, std::ref(q),std::ref(mutex),	std::ref(summarize),std::ref(error_logout));


	//send Login frame
	while(!isLogin){
		Frame frame = stompProtocol.buildConnectFrame(acceptVersion, host, user,password);	
		unsigned int temp = q.size();
		connectionHandler->sendFrameAscii(stompProtocol.stompToString(frame), '\0');

		while (q.size() == temp){
			//do nothing
		}
		{
			lock_guard<std::mutex> lock (mutex);
			frame = q.back();
			q.pop();
		}

		std::cout << stompProtocol.stompToString(frame) << endl;

		if(frame.command == Command::CONNECTED){
			std::cout << "Login successful" << endl;
		}
		else{
			isConnected = false;
			std::cout << "Login failed" << endl;
			connectionHandler->close();
			delete connectionHandler;
			connectionHandler = nullptr;
		}
		isLogin = true;
	}

	// From here we will see the rest of the STOMP client implementation
	//can assume that the client is connected to the server and is log in to the server

	while(isLogin && isConnected){
		const short bufsize = 1024;
        std::cin.getline(buf, bufsize);
		std::string line(buf);
		Frame frame;

		bool validCommand = true;

		if(error_logout==-2){
			std::cout << "Logout from client" << endl;
			isConnected = false;
			isLogin = false;
			connectionHandler->close();
			delete connectionHandler;
			error_logout = -1;
		}

		//build the frame
		if(line.substr(0,4) == "join"){
			std::string tempId = std::to_string(idCounter++);
			subscriptions.insert({line.substr(5),tempId});
			std::string tempReceipt = std::to_string(receiptCounter++);
			receipts.insert({tempReceipt,line});
			string destination = line.substr(5);
			frame = stompProtocol.buildSubscribeFrame(destination,tempId,tempReceipt);
		}
		else if(line.substr(0,4) == "exit"){
			auto tempDesti = line.substr(5);
			std::string subId = subscriptions[tempDesti];
			subscriptions.erase(tempDesti);

			std::string tempReceipt = std::to_string(receiptCounter++);
			receipts.insert({tempReceipt,line});

			frame = stompProtocol.buildUnsubscribeFrame(subId,tempReceipt);

		}
		else if(line.substr(0,6) == "report"){
			std::string path = "client/data/" + line.substr(7);
			names_and_events events = parseEventsFile(path);
			string destination = events.team_a_name + "_" + events.team_b_name;
			for (Event event : events.events){
				frame = stompProtocol.buildSendFrame(destination , event, user);
				cout << "Sending frame: "<< "\n" << stompProtocol.stompToStringForReports(frame) << endl;	
				sendFrame(frame,stompProtocol,connectionHandler);
				std::cout << "Sent " << line.length()+1 << " bytes to server" << std::endl;			
			}
			validCommand = false;
		}
		else if(line == "logout"){
			error_logout = receiptCounter;
			std::string tempReceipt = std::to_string(receiptCounter++);
			receipts.insert({tempReceipt,line});
			frame = stompProtocol.buildDisconnectFrame(tempReceipt);
		}
		else if(line.substr(0,7) == "summary"){
			stringstream ss(line);
			string word;
			string words [4];
			int i = 0;
			while (ss >> word) {
			 	words[i] = word;
				i++;
			}
			if(i<4){
				std::cout << "too few arguments. please try again" << endl;
				break; 
			}
			std::string path = "client/data/" + words[3];

			summarize.buildTextFile(words[1],words[2],path);

			validCommand = false;
		}
		else{
			if(line != "")
				std::cerr << "Invalid command";
			validCommand = false;
		}

		if(validCommand){
			//send the frame
			sendFrame(frame,stompProtocol,connectionHandler);
			std::cout << "Sent " << line.length()+1 << " bytes to server" << std::endl;
		}

	}

	//END of main while loop
	receive_thread.detach();

	}

	return 0;
}











