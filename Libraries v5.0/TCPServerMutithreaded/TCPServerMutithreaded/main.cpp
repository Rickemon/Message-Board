#include <iostream>
#include <algorithm>
#include <string>
#include <thread>
#include <vector>

#include "config.h"
#include "TCPServer.h"
#include "TCPClient.h"
#include <unordered_map>
#include <mutex>
#include <shared_mutex>

#define DEFAULT_PORT 12345

bool terminateServer = false;


class topic
{
private:
	std::vector<std::string> messages;
	std::shared_mutex mutex;
public:
	topic(std::string message)
	{
		messages.push_back(message);
	}
	std::string read(int index)
	{
		std::string response;
		if (index<= this->messages.size()-1) {
			if (mutex.try_lock_shared()) {
				response = this->messages[index];
				mutex.unlock_shared();
			}
			else {
				while (!mutex.try_lock_shared());
				response = this->messages[index];
				mutex.unlock_shared();
			}
		}
		else {
			response = "";
		}
		
		return response;

	}
	int post(std::string message) {
		int index;
		if (mutex.try_lock()) {
			messages.push_back(message);
			index = messages.size() - 1;
			mutex.unlock();
		}
		else {
			while (!mutex.try_lock());
			messages.push_back(message);
			index = messages.size() - 1;
			mutex.unlock();
		}

		return index;
	}
	int count() {
		int response;
		if (mutex.try_lock_shared()) {
			response = this->messages.size();
			mutex.unlock_shared();
		}
		else {
			while (!mutex.try_lock_shared());
			response = this->messages.size();
			mutex.unlock_shared();
		}

		return response;
	}


};
class MessageBoard
{
	
private:
	std::shared_mutex mutex;
	std::unordered_map<std::string, topic> messageBoard;
public:
	void parse(TCPServer* server,ReceivedSocketData& data) {
		data.reply = "";
		if (data.request == "list" || data.request == "LIST") {
			while (!mutex.try_lock_shared());
				if (messageBoard.size() > 0) {
					for (std::unordered_map<std::string, topic>::iterator kv = messageBoard.begin(); kv != messageBoard.end(); ++kv) {
						data.reply.append("@" + kv->first + "#");
					}
					mutex.unlock_shared();
					data.reply.pop_back();
					server->sendReply(data);
				}
				else {
					mutex.unlock_shared();
					data.reply = "";
					server->sendReply(data);
				}

		}
		else {
			std::string request = data.request;//save request might not be neccassary 
			std::string delimiter = "@";//all command end with an @ symbol(other than list and exit which are handled seperatly)
			int delLoc = request.find(delimiter);//saves position of delimiter for later use
			if (delLoc != -1) {//if an @ symbol has been found
				std::string token = data.request.substr(0, delLoc);//saves which command is being inputed

				if (token == "POST" || token == "post")
				{
					delimiter = "#";//# is the break point bettween topic id and message contents
					delLoc = request.find(delimiter);//saves position of delimiter for later use
					if (delLoc != -1) {// if there is a # symbol in the command
						token = request.substr(5, delLoc - 5);
						while (!mutex.try_lock_shared());
						std::unordered_map<std::string, topic>::iterator iter = messageBoard.find(token);
						mutex.unlock_shared();
						if (iter != messageBoard.end())//if the topic is already in the messageboard map
						{
							data.reply = std::to_string(iter->second.post(request.substr(delLoc + 1, request.length() - (delLoc))));// adds the message to the internal message list
						}
						else {//if topic is not already in list
							while (!mutex.try_lock());
							messageBoard.emplace(token, request.substr(delLoc + 1, request.length() - (delLoc)));//adds topic to the map and gives it it's first message
							mutex.unlock();
							data.reply = "0";
						}
						server->sendReply(data);
					}
					else {// if there is no # in the command
						data.reply = "";
						server->sendReply(data);
					}

				}
				else if (token == "COUNT" || token == "count")
				{
					token = request.substr(6, request.length() - (6));
					while (!mutex.try_lock_shared());
					std::unordered_map<std::string, topic>::iterator iter = messageBoard.find(token);
					if (iter != messageBoard.end())//if the topic is in the messageboard map
					{
						data.reply = std::to_string(iter->second.count());// adds the message to the internal message list
						mutex.unlock_shared();
					}
					else {//if topic is not in map
						mutex.unlock_shared();
						data.reply = "0";
					}
					server->sendReply(data);
				}
				else if (token == "READ" || token == "read")
				{
					delimiter = "#";//# is the break point bettween topic id and message contents
					delLoc = request.find(delimiter);//saves position of delimiter for later use
					if (delLoc != -1) {// if there is a # symbol in the command
						token = request.substr(5, delLoc - 5);
						while (!mutex.try_lock_shared());
						std::unordered_map<std::string, topic>::iterator iter = messageBoard.find(token);
						if (iter != messageBoard.end())//if the topic is in the messageboard map
						{
							data.reply = iter->second.read(stoi(request.substr(delLoc + 1, request.length() - (delLoc))));// adds the message to the internal message list
							mutex.unlock_shared();
						}
						else {//if topic is not in list
							mutex.unlock_shared();
							data.reply = "";
							
						}
						server->sendReply(data);
					}
					else {// if there is no # in the command
						mutex.unlock_shared();
						data.reply = "";
						server->sendReply(data);
					}
				}
				else
				{
					data.reply = "";
					server->sendReply(data);
				}
			}
			else {//if ther is no @ sybol in the command
				data.reply = "";
				server->sendReply(data);
			}



		}
	}
};

void parse(TCPServer* server, ReceivedSocketData&& data);

void serverThreadFunction(TCPServer* server, ReceivedSocketData&& data);
MessageBoard messageBoard;


int main()
{
	TCPServer server(DEFAULT_PORT);

	ReceivedSocketData receivedData;

	std::vector<std::thread> serverThreads;

	std::cout << "Starting server. Send \"exit\" (without quotes) to terminate." << std::endl;
	while (!terminateServer)
	{
		receivedData = server.accept();
		if (!terminateServer)
		{
			serverThreads.emplace_back(serverThreadFunction, &server, receivedData);
		}
	}

	for (auto& th : serverThreads)
		th.join();

	std::cout << "Server terminated." << std::endl;

	return 0;
}

void serverThreadFunction(TCPServer* server, ReceivedSocketData && data)
{
	unsigned int socketIndex = (unsigned int) data.ClientSocket;

	do {
		server->receiveData(data, 0);

		if (data.request != "" && data.request != "exit" && data.request != "EXIT")
		{
			messageBoard.parse(server, data);
		}
		else if (data.request == "exit" || data.request == "EXIT")
		{
			std::cout << "[" << socketIndex << "] Data received: " << data.request << std::endl;
			std::cout << "[" << socketIndex << "] Exiting... Bye bye!" << std::endl;

			data.reply = data.request;//The server responds to the EXIT request with a single line and the string TERMINATING
			server->sendReply(data);
		}
	} while (data.request != "exit" && data.request != "EXIT" && !terminateServer);

	if (!terminateServer && (data.request == "exit" || data.request == "EXIT"))
	{
		terminateServer = true;

		TCPClient tempClient(std::string("127.0.0.1"), DEFAULT_PORT);
		tempClient.OpenConnection();
		tempClient.CloseConnection();
	}

	server->closeClientSocket(data);
}