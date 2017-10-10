// TCP Server Function Definition

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <iostream>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>

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

Server::Server(const int port) {

	// create server address struct
	bzero((char *) &sin, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);
	client_addr_size = sizeof(client_addr);
}

void Server::open_socket() {
	
	if((connection_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket() failed");
		exit(1);
	}
};

void Server::bind_socket() {

	// allow for port reuse
	int opt = 0;
	setsockopt(connection_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(int));

	// bind socket
	if((bind(connection_socket, (struct sockaddr *) &sin, sizeof(sin))) < 0) {
		perror("bind() failed");
		exit(1);
	}
};

void Server::listen_socket() {
	
	if(listen(connection_socket, 1) == -1) {
		perror("listen() failed");
		exit(1);
	}
}

void Server::accept_connection() {

	if((data_socket = accept(connection_socket, (struct sockaddr *) &client_addr, &client_addr_size)) == -1) {
		perror("accept() failed");
		exit(1);
	}
}

void Server::send_data(string buffer) {

	if(write(data_socket, buffer.c_str(), 4096) == -1) {
		perror("write() failed");
		exit(1);
	}
}

string Server::receive_data() {

	char in_buffer[4096];
	bzero(in_buffer, 4096);

	if(read(data_socket, (void *) &in_buffer, 4096) == -1) {
		perror("read() failed");
		exit(1);
	}
	return c_to_cpp_string(in_buffer);
}

void Server::change_directory() {

	string size = receive_data();
	string path = receive_data();
	path = rstrip(path);

	if(chdir(path.c_str()) == -1) {
		perror("chdir() failed");
		exit(1);
	}
}

void Server::list_directory_contents() {

	DIR *d;
	struct dirent *dir;
	struct stat file_stats;
	string response("");

	d = opendir(".");

	if(d) {
		while((dir = readdir(d)) != 0) {
			if(stat(dir->d_name, &file_stats) == 0) {
				response += ((S_ISDIR(file_stats.st_mode)) ? 'd' : '-');
				response += ((file_stats.st_mode & S_IRUSR) ? 'r' : '-');
				response += ((file_stats.st_mode & S_IWUSR) ? 'w' : '-');
				response += ((file_stats.st_mode & S_IXUSR) ? 'x' : '-');
				response += ((file_stats.st_mode & S_IRGRP) ? 'r' : '-');
				response += ((file_stats.st_mode & S_IWGRP) ? 'w' : '-');
				response += ((file_stats.st_mode & S_IXGRP) ? 'x' : '-');
				response += ((file_stats.st_mode & S_IROTH) ? 'r' : '-');
				response += ((file_stats.st_mode & S_IWOTH) ? 'w' : '-');
				response += ((file_stats.st_mode & S_IXOTH) ? 'x' : '-');
				response += (" " + c_to_cpp_string(dir->d_name) + "\n");
			}
		}
		closedir(d);
	}
	else {
		perror("opendir() failed");
		exit(1);
	}
	
	send_data(to_string(response.length()));
	send_data(response);
}

void Server::make_directory() {
	string size = receive_data();
	string dir_name = receive_data();
	
	dir_name = rstrip(dir_name);

}

void Server::quit() {

	close(data_socket);
	accept_connection();
}

void Server::parse_and_execute(string command) {

	// trim trailing whitespace
	command = rstrip(command);

	if(command.compare("DWLD") == 0) {
		printf("Download initiated\n");
	}
	else if(command.compare("UPLD") == 0) {
		printf("Upload initiated\n");
	}
	else if(command.compare("LIST") == 0) {
		printf("List initiated\n");
		list_directory_contents();
	}
	else if(command.compare("MDIR") == 0) {
		make_directory();
		printf("Make Directory initiated\n");
	}
	else if(command.compare("RDIR") == 0) {
		printf("Remove Directory initiated\n");
	}
	else if(command.compare("CDIR") == 0) {
		printf("Change Directory initiated\n");
		change_directory();
	}
	else if(command.compare("DELF") == 0) {
		printf("Delete File initiated\n");
	}
	else if(command.compare("QUIT") == 0) {
		printf("Quit initiated\n");
		quit();
	}
	else {
		cout << "Invalid command: " << command << endl;
	}
}

string Server::c_to_cpp_string(char *c_str) {
	string str;
	if(c_str == NULL) {
		str = string("");
	}
	else {
		str = string(c_str);
	}
	return str;
}

string Server::rstrip(string str) {
	return str.substr(0, str.find_last_not_of(" \t\n") + 1);
}
