#include<iostream>
#include<errno.h>
#include<unistd.h>
#include<cstring>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>

#define PORT "2018"

int main(int argc, char *argv[]) {
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;
	int sockfd, new_fd;
	int getaddrinfo_result;
	int yes = 1;
	socklen_t sin_size;

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

	while (true) {
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		
		if (new_fd == -1) {
			std::cerr << "accept failed " << std::endl;
		}	

		std::cout << "connected" << std::endl;
		close(sockfd);
		send(new_fd, "Hey", 3, 0);
		close(new_fd);
		return 0;
	}
	return 0;
}


