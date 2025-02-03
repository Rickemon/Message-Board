#include <iostream>

#include "TCPClient.h"
#include <vector>
#include <thread>
#include "barrier.h"
#include "threadPool.h"
#include <shared_mutex>

#define DEFAULT_PORT 12345
std::vector <std::vector <int>> numOfRequests;
int seconds = 10;
auto end = std::chrono::system_clock::now() + std::chrono::milliseconds(10000);

std::shared_mutex mutex;
int totalNumOfRequests = 0;
std::vector <std::string> readRequestTypes = {"READ","LIST","COUNT"};

std::string writeRequestGenerater()
{

	std::string request = "POST";
	std::string topic = "topic" + std::to_string(rand() % 10);
	std::string message = "abcdefghijk";

	return (request + "@" + topic + "#" + message);





}


std::string readRequestGenerater()
{
	int index = rand() % 3;
	std::string request = readRequestTypes[index];
	std::string topic = "topic" + std::to_string(rand() % 10);
	int ID = rand() % 100;
	switch (index) {
	case 0:
		
		return (request + "@" + topic + "#" + std::to_string(ID));
		break;
	case 1:
		return request;
		break;
	case 2:
		return (request + "@" + topic);
		break;
	}



	return"";
}

void sendRequest(std::string request, TCPClient client)
{

		std::cout << "String sent: " << request << std::endl;
		std::cout << "Bytes sent: " << request.size() << std::endl;
		std::string reply = client.send(request);

		std::cout << "String returned: " << reply << std::endl;
		std::cout << "Bytes received: " << reply.size() << std::endl;
	

}

void writer()
{	
	TCPClient client("127.0.0.1", DEFAULT_PORT);
	client.OpenConnection();
	int count = 0;
	double timeSpan;
	std::chrono::high_resolution_clock::time_point endTime;
	std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();
	do {
		client.send(writeRequestGenerater());
		endTime = std::chrono::high_resolution_clock::now();
		timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();

		if (timeSpan < seconds) {
			count++;
		}
	} while (timeSpan < seconds);
	std::cout << count << std::endl;
	while (!mutex.try_lock()){}
	totalNumOfRequests = totalNumOfRequests + count;
	mutex.unlock();

	std::cout << count << std::endl;
}

void reader()
{
	TCPClient client("127.0.0.1", DEFAULT_PORT);
	client.OpenConnection();
	int count = 0;
	double timeSpan;
	std::chrono::high_resolution_clock::time_point endTime;
	std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();
	
	do {
		std::chrono::high_resolution_clock::time_point s = std::chrono::high_resolution_clock::now();
		client.send(readRequestGenerater());
		std::chrono::high_resolution_clock::time_point Time = std::chrono::high_resolution_clock::now();

		double TimeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(Time - s).count();
		std::cout << TimeSpan << std::endl;
		endTime = std::chrono::high_resolution_clock::now();
		timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();

		if (timeSpan < seconds) {
			count++;
		}
	} while (timeSpan < seconds);
	std::cout << count << std::endl;
	while (!mutex.try_lock()) {}
	totalNumOfRequests = totalNumOfRequests + count;
	mutex.unlock();
	std::cout << count << std::endl;

}






int main()
{
	// Validate the parameters int argc, char** argv
	//if (argc != 2) {
	//	printf("usage: %s server-name|IP-address\n", argv[0]);
	//	return 1;
	//}
	int numOfWriters = 5;
	int numOfReaders = 5;
	//ThreadPool poolOfWriters(numOfWriters);
	//ThreadPool poolOfReaders(numOfReaders);


	std::string request;
	std::vector<std::thread> readers;
	std::vector<std::thread> writers;
	int i;

	for (i = 0; i < numOfReaders; i++)
		readers.emplace_back(reader);

	for (i = 0; i < numOfWriters; i++)
		writers.emplace_back(writer);
	for (auto& reader : readers)
		reader.join();

	for (auto& writer : writers)
		writer.join();
	TCPClient client("127.0.0.1", DEFAULT_PORT);
	client.OpenConnection();
	client.send("EXIT");
	std::cout << totalNumOfRequests << std::endl;

	std::cout << totalNumOfRequests << std::endl;
	return 0;
}


