// TCP Server

#include <cstdlib>
#include <iostream>
#include <unistd.h>

#include "server.h"
using namespace std;

#define BUF_SIZE 4096

using namespace std;

int main(int argc, char **argv) {

	if(argc != 2) {
		cout << "usage: myftpd PORT" << endl;
		exit(1);
	}
	char *port = argv[1];

	string command;

	Server server(port);
	server.open_socket();
	server.bind_socket();
	server.listen_socket();
	server.accept_connection();

	while(1) {

		command = server.receive_data();
		
		cout << "Command from main is: " << command << endl;
		
		server.parse_and_execute(command);

	}
}
