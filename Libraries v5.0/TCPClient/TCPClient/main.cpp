#include <iostream>

#include "TCPClient.h"
#include "barrier.hpp"

#include <vector>
#include <thread>
#include <shared_mutex>
#include <list>

#define DEFAULT_PORT 12345
std::vector <std::vector <int>> numOfRequests;
int numOfWriters = 5;
int numOfReaders = 5;
int totalNumOfRequests = 0;
int seconds = 10;


auto end = std::chrono::system_clock::now() + std::chrono::milliseconds(10000);
barrier simpleBarrier(numOfWriters+ numOfReaders);

std::shared_mutex mutex;
bool endSent= false;

std::vector <std::string> readRequestTypes = { "READ","LIST","COUNT" };

std::string writeRequestGenerater()
{

	std::string request = "POST";
	std::string topic = "topic" + std::to_string(rand());
	std::string message = "abcdefghijk";

	return (request + "@" + topic + "#" + message);
}


std::string readRequestGenerater()
{
	int random = rand();
	std::string request = readRequestTypes[random%3];
	std::string topic = "topic" + std::to_string(random );
	switch (random%3) {
	case 0:

		return (request + "@" + topic + "#" + std::to_string(random));
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



void writer(int** threadStats, int index)
{
	//std::cout << "reader Starting" << std::endl; //start of writer thread
	TCPClient client("127.0.0.1", DEFAULT_PORT);
	client.OpenConnection();
	int count = 0;
	int timeSpan = 0;//set up variables used to figure out how much time has passed
	std::chrono::high_resolution_clock::time_point endTime;
	std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();
	do {
		endTime = std::chrono::high_resolution_clock::now();
		client.send(writeRequestGenerater());//send requests to server
		timeSpan = std::chrono::duration_cast<std::chrono::duration<int>>(endTime - startTime).count();//since duration is an int the number will automatically be rounded down
		
		if (timeSpan < seconds) {
			threadStats[index][timeSpan] ++;
		}


	} while (timeSpan < seconds);//while time passed is less than time test needs to be run



	while (!mutex.try_lock()) {}
	if (!endSent)
	{
		std::cout << "Ending: ";
		endSent = true;
		std::string x = client.send("EXIT");
		std::cout << x;

	}
	client.CloseConnection();
	mutex.unlock();
	simpleBarrier.count_down_and_wait();

	
}

void reader(int** threadStats, int index)
{
	//std::cout << "reader Starting" << std::endl;
	TCPClient client("127.0.0.1", DEFAULT_PORT);
	client.OpenConnection();
	int timeSpan;
	std::chrono::high_resolution_clock::time_point endTime;
	std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

	do {
		endTime = std::chrono::high_resolution_clock::now();
		std::string rep = client.send(readRequestGenerater());
		timeSpan = std::chrono::duration_cast<std::chrono::duration<int>>(endTime - startTime).count();//since duration is an int the number will automatically be rounded down

		if (timeSpan < seconds) {
			threadStats[index][timeSpan] ++;
		}

	} while (timeSpan < seconds);

	while (!mutex.try_lock()) {}
	if (!endSent)
	{
		std::cout << "Ending: " ;
		endSent = true;
		std::string x = client.send("EXIT");
	}
	client.CloseConnection();
	mutex.unlock();
	simpleBarrier.count_down_and_wait();

}






int main()
{
	// Validate the parameters int argc, char** argv
	//if (argc != 2) {
	//	printf("usage: %s server-name|IP-address\n", argv[0]);
	//	return 1;
	//}

	int numOfThreads = numOfWriters + numOfReaders;
	int sumOfRequestsForThread;
	int sumOfRequestsForPosterThreads = 0;
	int sumOfRequestsForReaderThreads = 0;
	
	//ThreadPool poolOfWriters(numOfWriters);
	//ThreadPool poolOfReaders(numOfReaders);
	std::cout << "Starting..." << std::endl;

	std::string request;
	int** threadStats = new int* [numOfThreads];
	for (int x = 0; x < numOfThreads; x++)//preping the array used to store thread statistics
	{
		threadStats[x] = new int[seconds];
		for (int y = 0; y < seconds; y++)//preping the array used to store thread statistics
		{
			threadStats[x][y] = 0;
		}
	}


	std::vector<std::thread> threads;
	int i;


	for (i = 0; i < numOfWriters; i++) {
		threads.emplace_back(writer, threadStats, i);
		
	}

	for (i = numOfWriters; i < numOfThreads; i++) {
		threads.emplace_back(reader, threadStats, i);
	}
	for (auto& thread : threads)
		thread.join();


	





	for (int x = 0; x < numOfThreads; x++)
	{
		for (int y = 0; y < seconds; y++)
		{
			totalNumOfRequests += threadStats[x][y];
		}
	}


	for (int x = 0; x < numOfThreads; x++)
	{
		if (x< numOfWriters) {
			std::cout << "Post Thread " << x << " sent: " << std::endl;
		}
		else {
			std::cout << "Read Thread " << x - numOfWriters << " sent: " << std::endl;
		}
		sumOfRequestsForThread = 0;
		for (int y = 0; y < seconds; y++)
		{
			std::cout << "	Second " << y << ": "<< threadStats[x][y] <<" requests. " << std::endl;
			if (x < numOfWriters) {
				sumOfRequestsForPosterThreads += threadStats[x][y];
			}
			else {
				sumOfRequestsForReaderThreads += threadStats[x][y];
			}
			sumOfRequestsForThread += threadStats[x][y];
			totalNumOfRequests += threadStats[x][y];
		}
		std::cout << "		Average: " << (int)(sumOfRequestsForThread/seconds) << " requests. " << std::endl;
		std::cout << "		Runtime: " << seconds << " seconds." << std::endl;
	}

	std::cout << std::endl << "Total poster requests: " << sumOfRequestsForPosterThreads << std::endl;

	std::cout << "Average requests per poster thread: " << (int)(sumOfRequestsForPosterThreads/numOfWriters) << std::endl;

	std::cout << "Total reader requests: " << sumOfRequestsForReaderThreads << std::endl;

	std::cout << "Average requests per reader thread: " << (int)(sumOfRequestsForReaderThreads / numOfReaders) << std::endl;

	std::cout << "Total requests: " << totalNumOfRequests << std::endl;

	WSACleanup();
	return 0;
}


