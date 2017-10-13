// TCP Server Function Definition

#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <strings.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "server.h"
using namespace std;

Server::Server(char *port, int buf_sz = 4096) {

	// create server address struct
	bzero((char *) &sin, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(atoi(port));
	client_addr_size = sizeof(client_addr);

	this->in_buf_sz = buf_sz;
	this->in_buffer = (char *)malloc(this->in_buf_sz);
}

Server::Server(const int port, int buf_sz = 4096) {

	// create server address struct
	bzero((char *) &sin, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);
	client_addr_size = sizeof(client_addr);
	
	this->in_buf_sz = buf_sz;
	this->in_buffer = (char *)malloc(this->in_buf_sz);
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

void Server::send_data(const char *buffer) {

	int len = strlen(buffer);
	if(write(data_socket, buffer, len) == -1) {
		perror("write() failed");
		exit(1);
	}
}

void Server::receive_data(char *buffer) {

	bzero(buffer, sizeof(buffer));

	if(read(data_socket, (void *) &buffer, sizeof(buffer)) == -1) {
		perror("read() failed");
		exit(1);
	}
}

// obtained help from https://stackoverflow.com/questions/11952898/c-send-and-receive-file
void Server::download_file() {
	
	int16_t file_name_len;
	char *data = (char*)&file_name_len;
	receive_data(data);
	file_name_len = ntohs(file_name_len);
		
	string ack = "ack";
	send_data(ack.c_str());

	receive_data(this->in_buffer);
	string filename = rstrip(string(in_buffer));

	struct stat st;
	int file_exists, file_size;

	if((file_exists = access(filename.c_str(), F_OK)) == -1) {
		int32_t no_file = htonl(file_exists);
		char *no_f = (char*)&no_file;
		send_data(no_f);
		return;
	}

    if(stat(filename.c_str(), &st) == 0) {
		file_size = st.st_size;
		int32_t f_sz = htonl(file_size);
		char *file_sz = (char*)&f_sz;
		send_data(file_sz);
	}
	else {
		perror("stat() failed");
		exit(1);
	}

	off_t offset = 0;
	int sent_bytes, remaining_data = file_size;
	int fd = open(filename.c_str(), O_RDONLY);
	while(((sent_bytes = sendfile(data_socket, fd, &offset, 4096)) > 0) && (remaining_data > 0)) {
		remaining_data -= sent_bytes;
	}
}

void Server::upload_file() {

	int16_t file_name_len;
	char *data = (char*)&file_name_len;
	receive_data(data);
	file_name_len = ntohs(file_name_len);
		
	string ack = "ack";
	send_data(ack.c_str());

	receive_data(this->in_buffer);
	string filename = rstrip(string(in_buffer));

	ack = "1";
	send_data(ack.c_str());

	// Receive file size
	int32_t fs;
	char* f_sz = (char*) &fs;
	receive_data(f_sz);
	int file_size = ntohl(fs);

	// Open file to be written
	FILE* receivedFile = fopen(filename.c_str(), "w");
	if (receivedFile == NULL) {
		cout << "Failed to open file: " <<  strerror(errno) << endl;
		return;
	}

	// Receive file and write to disk (obtained help from StackOverflow post titled "c send and receive file")
	int remainingData = file_size;
	int len;
	char buffer[4096];
	struct timeval start, end;

	gettimeofday(&start, NULL);
	while((remainingData > 0) && ((len = recv(data_socket, buffer, 4096, 0)) > 0)) {
		fwrite(buffer, sizeof(char), len, receivedFile);
		remainingData -= len;
	}
	gettimeofday(&end, NULL);
	int32_t throughput = htonl(calculate_throughput(file_size, start, end));

	char *tp = (char*)&throughput;
	send_data(tp);

	fclose(receivedFile);

}

void Server::change_directory() {

	int16_t dir_path_len;
	char *data = (char*)&dir_path_len;
	receive_data(data);
	dir_path_len = ntohs(dir_path_len);
		
	string ack = "ack";
	send_data(ack.c_str());

	receive_data(this->in_buffer);
	string dir_path = rstrip(string(in_buffer));

	struct stat sb; 
	int16_t resp_num;
	char *resp = (char*)&resp_num;

	if(stat(dir_path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
		if(chdir(dir_path.c_str()) == -1) {
			resp_num = htons(-1);
			send_data(resp);
		} else {
			resp_num = htons(1);
			send_data(resp);
		}
	} else {
		resp_num = htons(-2);
		send_data(resp);
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
	
	response = rstrip(response);
	int32_t data_len = htonl(response.length());
	char *d_len = (char*)&data_len;
	send_data(d_len);
	
	string ack = "";
	while(ack == "") {
		receive_data(this->in_buffer);
		ack = rstrip(string(in_buffer));
	}

	this->send_data(response.c_str());
}

void Server::make_directory() {
	
	int16_t dir_path_len;
	char *data = (char*)&dir_path_len;
	receive_data(data);
	dir_path_len = ntohs(dir_path_len);
		
	string ack = "ack";
	send_data(ack.c_str());

	receive_data(this->in_buffer);
	string dir_path = rstrip(string(in_buffer));	

    struct stat sb;
	int16_t resp_num;
	char *resp = (char*)&resp_num;
	
	if (stat(dir_path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
		resp_num = htons(-2);
		send_data(resp);
	} else {
		if(mkdir(dir_path.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1) {
			fprintf(stderr, "mkdir() failed");
			resp_num = htons(-1);
			send_data(resp);
		} else {
			resp_num = htons(1);
			send_data(resp);
		}
	}
}

void Server::remove_directory() {
	
	int16_t dir_path_len;
	char *data = (char*)&dir_path_len;
	receive_data(data);
	dir_path_len = ntohs(dir_path_len);
		
	string ack = "ack";
	send_data(ack.c_str());

	receive_data(this->in_buffer);
	string dir_path = rstrip(string(in_buffer));	
	
    struct stat sb;
	int16_t resp_num;
	char *resp = (char*)&resp_num;

	if (stat(dir_path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
		resp_num = htons(1);
		send_data(resp);
	} else {
		resp_num = htons(-1);
		send_data(resp);
		return;
	}

	receive_data(in_buffer);
	string confirmation = rstrip(string(in_buffer));
	if(confirmation == "Yes") {
		if(rmdir(dir_path.c_str()) == -1) {
			resp_num = htons(-1);
			send_data(resp);
		} else {
			resp_num = htons(1);
			send_data(resp);
		}
	}
}


void Server::quit() {

	close(data_socket);
	accept_connection();
}

void Server::parse_and_execute(string command) {

	// trim trailing whitespace
	command = rstrip(command);

	if(command.compare("DWLD") == 0) {
		download_file();
	}
	else if(command.compare("UPLD") == 0) {
		upload_file();
	}
	else if(command.compare("LIST") == 0) {
		this->list_directory_contents();
	}
	else if(command.compare("MDIR") == 0) {
		this->make_directory();
	}
	else if(command.compare("RDIR") == 0) {
		this->remove_directory();
	}
	else if(command.compare("CDIR") == 0) {
		this->change_directory();
	}
	else if(command.compare("DELF") == 0) {
		this->delete_file();
	}
	else if(command.compare("QUIT") == 0) {
		this->quit();
	}
	else {
		cout << "Invalid command: " << command << endl;
	}
}

void Server::delete_file() {
	
	int16_t file_path_len;
	char *data = (char*)&file_path_len;
	receive_data(data);
	file_path_len = ntohs(file_path_len);
		
	string ack = "ack";
	send_data(ack.c_str());

	receive_data(this->in_buffer);
	string file_path = rstrip(string(in_buffer));	
	
    struct stat sb;
	int16_t resp_num;
	char *resp = (char*)&resp_num;
	
	if (stat(file_path.c_str(), &sb) == 0 && S_ISREG(sb.st_mode)) {
		resp_num = htons(1);
		send_data(resp);
	} else {
		resp_num = htons(-1);
		send_data(resp);
		return;
	}

	receive_data(in_buffer);
	string confirmation = rstrip(string(in_buffer));
	if(confirmation == "Yes") {
		if(unlink(file_path.c_str()) == -1) {
			resp_num = htons(-1);
			send_data(resp);
		} else {
			resp_num = htons(1);
			send_data(resp);
		}
	}
}

int Server::calculate_throughput(int size, struct timeval start, struct timeval end) {

	return 8*size/(((end.tv_sec * 1000000) + end.tv_usec) - ((start.tv_sec * 1000000) + start.tv_usec));
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

void Server::split_msg(string msg, string &s1, string &s2) {

	string::size_type pos;
	pos=msg.find(' ',0);
	if(pos != string::npos) {
		s1 = msg.substr(0,pos);
		s2 = msg.substr(pos+1);
	}
}
