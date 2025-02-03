#include <conio.h>
#include <iostream>
#include <string>
#include <vector>

#include "RequestParser.h"

int main()
{
	std::vector<std::string> requestList =
	{
		"POST@Solar system#The dark side of the @moon is #dark.",
		"POST@#x",
		"POST@night_sky#cloudy!",
		"POST@Solar system#Mercury, Venus, Earth, Mars, Jupiter, Saturn, Neptune, Uranus",
		"LIST",
		"COUNT@Solar system",
		"COUNT@solar_system",
		"READ@#0",
		"READ@Solar system#1",
		"READ@night_sky#2",
		"READ@solar_system#0",
		"EXIT",
		"AAA"
	};

	for (std::string & request : requestList)
	{
		PostRequest post = PostRequest::parse(request);
		if (post.valid)
		{
			std::cout << "Post request: " << post.toString() << std::endl;
			std::cout << "Post topic: " << post.getTopicId() << std::endl;
			std::cout << "Post message: " << post.getMessage() << std::endl;
			continue;
		}

		ReadRequest read = ReadRequest::parse(request);
		if (read.valid)
		{
			std::cout << "Read request" << read.toString() << std::endl;
			std::cout << "Read topic: " << read.getTopicId() << std::endl;
			std::cout << "Read post id: " << read.getPostId() << std::endl;
			continue;
		}

		CountRequest count = CountRequest::parse(request);
		if (count.valid)
		{
			std::cout << "Count request: " << count.toString() << std::endl;
			std::cout << "Count topic: " << count.getTopicId() << std::endl;
			continue;
		}

		ListRequest list = ListRequest::parse(request);
		if (list.valid)
		{
			std::cout << "List request: " << list.toString() << std::endl;
			continue;
		}

		ExitRequest exitReq = ExitRequest::parse(request);
		if (exitReq.valid)
		{
			std::cout << "Exit request: " << exitReq.toString() << std::endl;
			continue;
		}

		std::cout << "Unknown request: " << request << std::endl;
		std::cout << std::endl;
	}
	_getch();

	return 0;
}
