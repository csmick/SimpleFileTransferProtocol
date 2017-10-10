// TCP Client

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFF_SIZE 4096

#include "client.h"
using namespace std;

int main(int argc, char **argv) {
	
	if(argc != 3) {
		fprintf(stderr, "usage: tcpclient server_hostname server_port");
	}

	char *hostname = argv[1];
	int port = atoi(argv[2]);

	char *buf = (char *)malloc(BUFF_SIZE);
	bzero(buf,BUFF_SIZE);

	Client client(hostname, port, BUFF_SIZE);
	
	client.connect_socket();
	client.start();
	client.close_socket();
	
	return 0;
}
