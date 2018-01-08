#include<iostream>
#include<unistd.h>
#include<errno.h>
#include<cstring>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <netdb.h>

#define PORT "2018"

#define MAXDATASIZE 100

int main(int argc, char *argv[]) {
	struct addrinfo hints, *servinfo, *p;
	int sockfd;
	int numbytes;
	int getaddrinfo_result;
	char buf[MAXDATASIZE];

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

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
		std::cerr << "recv: " << strerror(errno) << std::endl;
		return 1;
	}

	buf[numbytes] = '\0';
	std::cout << buf << std::endl;
	close(sockfd);
	return 0;
}
