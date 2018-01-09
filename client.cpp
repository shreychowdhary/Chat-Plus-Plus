#include "client.h"
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <future>

#define PORT "2018"

#define MAXDATASIZE 100

int main(int argc, char *argv[]) {
	struct addrinfo hints, *servinfo, *p;
	int sockfd;
	int getaddrinfo_result;
	char buf[MAXDATASIZE];
	std::string username;
	if (argc != 2) {
		std::cerr << "pass in only server ip" << std::endl;
		return 1;
	}
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((getaddrinfo_result = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		std::cerr << "getaddrinfo error: " << gai_strerror(getaddrinfo_result) << std::endl;
		return 1;
	}

	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			std::cerr << "socket create: " << strerror(errno) << std::endl;
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			std::cerr << "socket connect: " << strerror(errno) << std::endl;
			continue;
		}
		break;
	}
	
	if (p == NULL) {
		std::cerr << "client connect failed" << std::endl;
		return 2;
	}
	
	freeaddrinfo(servinfo);
	std::cout << "connected" << std::endl; 	
	std::future<std::pair<int,std::string>> rec_result = std::async(std::launch::async, receive_message, sockfd);
	std::future<void> input_result = std::async(std::launch::async, user_input, sockfd);

	while (true) {
		if (rec_result.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
			std::pair<int,std::string> message = rec_result.get();
			int numbytes = message.first;
			if (numbytes == -1) {
				std::cerr << "recv: " << strerror(errno) << std::endl;
				return 1;
			} else if (numbytes == 0) {
				return 0;
			}

			rec_result = std::async(std::launch::async, receive_message, sockfd);

			std::cout << message.second << std::endl;
		}
		if (input_result.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
			input_result = std::async(std::launch::async, user_input, sockfd);
		}
	}

	close(sockfd);
	return 0;
}

std::pair<int,std::string> receive_message(int sockfd) {
	char buf[MAXDATASIZE];
	int numbytes;
	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
		return std::make_pair(-1,"");
	}
	buf[numbytes] = '\0';
	std::string str(buf);
	return make_pair(numbytes,str);
}

void user_input(int sockfd) {
	std::string str;
	std::getline(std::cin, str);
	send(sockfd, str.c_str(), str.length(), 0);
}
