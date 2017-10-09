// TCP Server Function Definition

#include <cstdlib>
#include <cstdio>
#include <strings.h>
#include <sys/types.h>

#include "server.h"
using namespace std;

Server::Server(char *port) {

	// create server address struct
	bzero((char *) &sin, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(atoi(port));
	client_addr_size = sizeof(client_addr);
}

Server::Server(const int port = 8000) {

	// create server address struct
	bzero((char *) &sin, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);
	client_addr_size = sizeof(client_addr);
}

int Server::open_socket() {
	
	int sockfd;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket() failed");
		exit(1);
	}
	return sockfd;
};

void Server::bind_socket(const int sockfd) {

	// allow for port reuse
	int opt = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(int));

	// bind socket
	if((bind(sockfd, (struct sockaddr *) &sin, sizeof(sin))) < 0) {
		perror("bind() failed");
		exit(1);
	}
};

void Server::listen_socket(const int sockfd) {
	
	if(listen(sockfd, 1) == -1) {
		perror("listen() failed");
		exit(1);
	}
}

int Server::accept_connection(const int sockfd) {
	int new_fd;
	if((new_fd = accept(sockfd, (struct sockaddr *) &client_addr, &client_addr_size)) == -1) {
		perror("accept() failed");
		exit(1);
	}
	return new_fd;
}
