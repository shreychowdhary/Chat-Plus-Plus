#include "server.h"
#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <future>
#include <vector>

#define PORT "2018"
#define MAXDATASIZE 100
int main(int argc, char *argv[]) {
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;
	int sockfd, new_fd;
	int getaddrinfo_result;
	int yes = 1;
	socklen_t sin_size;
	char buf[MAXDATASIZE];
	
	fd_set readfds;


	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((getaddrinfo_result = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		std::cout << "getaddrinfo error: " << gai_strerror(getaddrinfo_result) << std::endl;
		return 1;
	}
	
	for (p = servinfo; servinfo != NULL; servinfo = servinfo->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			std::cerr << "server: socket " << strerror(errno) << std::endl;
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			std::cerr << "setsockopt " << strerror(errno) << std::endl;
			return 1;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			std::cerr << "setsockopt " << strerror(errno) << std::endl;
			continue;
		}
		break;
	}

	freeaddrinfo(servinfo);

	if (p == NULL) {
		std::cerr << "server failed to bind " << std::endl;
		return 1;
	}

	if (listen(sockfd, 10) == -1) {
		std::cerr << "listen failed " << strerror(errno) << std::endl;
		return 1;
	}
	
	int max_sd = sockfd;
	std::vector<std::pair<int,std::string>> clients;

	while (true) {
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
		max_sd = sockfd;
		
		for (int i = 0; i < clients.size(); i++) {
			int sd = clients[i].first;
			FD_SET(sd, &readfds);
			
			if (sd > max_sd) {
				max_sd = sd;
			}
		}
		int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

		if (activity < 0) {
			std::cerr << strerror(errno) << std::endl;
		}

		if (FD_ISSET(sockfd, &readfds)) {
			socklen_t sin_size = sizeof their_addr;
			int new_socket;
			if ((new_socket = accept(sockfd, (struct sockaddr*) &their_addr, &sin_size)) < 0) {
				std::cerr << "Accept: " << strerror(errno) << std::endl;
				return 1;
			}
			send(new_socket, "Enter Name", 10, 0);
			clients.push_back(std::make_pair(new_socket,""));
		}
		
		std::vector<std::pair<int,std::string>> clients_tmp;
		std::vector<std::pair<int,std::string>> messages;
		
		for (int i = 0; i < clients.size(); i++) {
			int sd = clients[i].first;
			std::string name = clients[i].second;
			if (FD_ISSET(sd, &readfds)) {
				int numbytes = recv(sd, buf, MAXDATASIZE, 0);
				std::string message;
				if (numbytes == -1) {
					std::cerr << "Recv: " << strerror(errno) << std::endl;
					close(sd);
					continue;
				} else if (numbytes == 0) {
					message = name + " disconnected";
					messages.push_back(std::make_pair(sd, message));
					std::cout << message << std::endl;
					close(sd);
					continue;
				}
				buf[numbytes] = '\0';
				if (name == "") {
					name = buf;
					message = name + " joined.";
				} else {
					message = name + ":" + buf;
				}
				std::cout << message << std::endl;
				messages.push_back(std::make_pair(sd, message));
			}
			clients_tmp.push_back(std::make_pair(sd, name));
		}

		clients = clients_tmp;
		for (int i = 0; i < messages.size(); i++) {
			int sendfd = messages[i].first;
			std::string message = messages[i].second;
			for (int j = 0; j < clients.size(); j++) {
				if (sendfd == clients[j].first) {
					continue;
				}
				send(clients[j].first, message.c_str(), message.length(), 0);
			}
		}
	}

	close(new_fd);
	return 0;
}

