// TCP Server

#include <cstdlib>
#include <unistd.h>

#include "server.h"
using namespace std;

int main(int argc, char **argv) {

	char* port = argv[1];

	int sockfd;

	Server server(port);
	sockfd = server.open_socket();
	server.bind_socket(sockfd);
	server.listen_socket(sockfd);
	s = server.accept_connection(sockfd);

	close(sockfd);

	return 0;
}
