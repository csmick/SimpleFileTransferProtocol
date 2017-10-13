// TCP Client Function Implementation

#include <iostream>
#include <stdio.h>	
#include <stdlib.h>	
#include <string.h>	
#include <sys/types.h>	
#include <sys/socket.h>	
#include <netinet/in.h>	
#include <netdb.h>	
#include <unistd.h>
#include <string>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "client.h"
using namespace std;

Client::Client(char *h, const int port, int buf_sz) {

	// create the hostent structure
	this->hp = gethostbyname(h);	
	if(!this->hp) {	
		fprintf(stderr, "tcp-client: unknown host: %s\n", h);	
		exit(1);
	}	

	// create client address struct
	bzero((char	*)&this->sin, sizeof(struct sockaddr_in));	
	this->sin.sin_family = AF_INET;	
	bcopy(this->hp->h_addr, (char *)&this->sin.sin_addr, this->hp->h_length);	
	this->sin.sin_port = htons(port);

	this->open_socket();
};

void Client::open_socket() {	
	if((this->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("tcp-client: socket() failed");
		exit(1);
	}
};

void Client::connect_socket() {
	if( connect(this->sockfd, (struct sockaddr *)&this->sin, sizeof(this->sin)) < 0) {
		perror("tcp-client: connect() failed");	
		close(this->sockfd); 
		exit(1);
	}
};

void Client::send_message(string s) {
	int len = s.length();
	if(send(this->sockfd, s.c_str(), len, 0) == -1) {
		fprintf(stderr, "tcp-client: send() failed");
		close_socket();
		exit(1);
	}
};

string Client::receive_data() {

	char in_buffer[4096];
	bzero(in_buffer, 4096);
	if(read(this->sockfd, (void *) &in_buffer, 4096) == -1) {
		perror("read() failed");
		exit(1);
	}
	return rstrip(c_to_cpp_string(in_buffer));
}

void Client::start() {
	
	printf("Welcome to Simple FTP client\n");
	this->print_usage();

	cout << endl << endl;
	cout << "> ";
	string command;
	while(cin >> command) { // loop until the input is finished
	
		command = rstrip(command);
		if(command == "QUIT") {
			printf("Goodbye!\n");
			break;
		} else if(command == "DWLD") {
			this->download();
		} else if(command == "UPLD") {
			this->upload();
		} else if(command == "DELF") {
			this->delete_file();
		} else if(command == "LIST") {
			this->list();
		} else if(command == "MDIR") {
			this->make_dir();
		} else if(command == "RDIR") {
			this->remove_dir();
		} else if(command == "CDIR") {
			this->change_dir();
		} else {
			cout << "INVALID COMMAND\n\n";
		}

		cout << endl << endl;

		this->print_usage();

		cout << endl << endl;	
		cout << "> ";
		command.clear();
	}
};

void Client::print_usage() {
	printf("OPERATIONS:\n");
	printf("\tDWLD -- download a file from the server\n");
	printf("\tUPLD -- upload a file to the server\n");
	printf("\tDELF -- delete a file from the server\n");
	printf("\tLIST -- list the contents of the current directory on the server\n");
	printf("\tMDIR -- make a directory on the server\n");
	printf("\tRDIR -- delete a directory from the server\n");
	printf("\tCDIR -- change directories on the server\n");
	printf("\tQUIT -- quit this FTP client\n");
}

void Client::download() {

	// Prompt user for filename
	cout << "Please enter the name of the file you would like to download:" << endl;
	cout << "--> ";

	string filename = "";
	cin >> filename;
	filename = rstrip(filename);
	
	// Send download request
	string message = "DWLD";
	this->send_message(message);
	message = to_string(filename.length()) + " " + filename;
	this->send_message(message);

	// Receive file size
	int fileSize = stoi(this->receive_data());

	if(fileSize < 0) {
		cout << "File \"" << filename << "\" does not exist on server" << endl;
		return;
	}
	
	// Open file to be written
	FILE* receivedFile = fopen(filename.c_str(), "w");
	if (receivedFile == NULL) {
		cout << "Failed to open file: " <<  strerror(errno) << endl;
		return;
	}

	// Receive file and write to disk (obtained help from StackOverflow post titled "c send and receive file")
	int remainingData = fileSize;
	int len;
	char buffer[4096];
	struct timeval start, end;
	
	gettimeofday(&start, NULL);
	while((remainingData > 0) && ((len = recv(this->sockfd, buffer, 4096, 0)) > 0)) {
		fwrite(buffer, sizeof(char), len, receivedFile);
		remainingData -= len;
	}
	gettimeofday(&end, NULL);
	fclose(receivedFile);
	cout << "Throughput = " << to_string(calculate_throughput(fileSize, start, end)) << "Mbps" << endl;
}

void Client::upload() {

	// Prompt user for filename
	cout << "Please enter the name of the file you would like to upload:" << endl;
	cout << "--> ";

	string filename = "";
	cin >> filename;
	filename = rstrip(filename);

	// Send the intent to upload	
	this->send_message("UPLD");
	string message = to_string(filename.length()) + " " + filename;
	this->send_message(message);

	// Receive acknowledgement from server
	string ack = "";
	while(ack == "") {
		ack = this->receive_data();
	}

	// Upload file to server
	struct stat st;
	int file_size;
	
	if(stat(filename.c_str(), &st) == 0) {
		file_size = st.st_size;
		send_message(to_string(file_size));
	}
	else {
		perror("stat() failed");
		exit(1);
	}

	off_t offset = 0;
	int sent_bytes, remaining_data = file_size;
	int fd = open(filename.c_str(), O_RDONLY);
	while(((sent_bytes = sendfile(sockfd, fd, &offset, 4096)) > 0) && (remaining_data > 0)) {
		remaining_data -= sent_bytes;
	}

	string throughput = receive_data();
	cout << "Throughput = " << throughput << "Mbps" << endl;
}

void Client::delete_file() {

	// Send the intent to delete
	this->send_message("DELF");
	
	// Prompt user for filename
	cout << "Please enter the name of the file you would like to delete:" << endl;
	cout << "--> ";
	string filename = "";
	cin >> filename;
	filename = rstrip(filename);
	
	// send the length and file to delete
	this->send_message(to_string(filename.length()) + " " + filename);

	// Acknowledgement from server
	string ack = "";
	while(ack == "") {
		ack = this->receive_data();
	}

	if(ack == "-1") {
		cout << "The file does not exist on server" << endl << endl;
		return;
	}

	string deleteConf;
	while(deleteConf != "Yes" && deleteConf != "No") {
		cout << "Are you sure you want to delete " << filename << "? (Yes/No): ";
	
		// Read in the user's answer
		cin >> deleteConf;
		deleteConf = rstrip(deleteConf);
	}

	this->send_message(deleteConf);
	
	if(deleteConf == "Yes") {
		string deleted = "";
		while(deleted == "") {
			deleted = this->receive_data();
		}
		
		if(deleted == "-1") {
			cout << "Failed to delete file" << endl;
		} else if(deleted == "1") {
			cout << "File deleted" << endl;
		}
	}
	else {
		cout << "Delete abandoned by user!" << endl << endl;
	}
}

void Client::list() {

	this->send_message("LIST");
	
	string msg = "";
	while(msg == "") {
		msg = this->receive_data();
	}

	string numBytesString, listing_part;

	this->split_msg(msg, numBytesString, listing_part);

	cout << listing_part << endl;

	int bytes = stoi(numBytesString);
	cout << "BYTES = " << bytes << endl;

	// Loop until all bytes from the file listing are read
	int bytes_received = listing_part.length();
	
	while(bytes_received < bytes) {
		cout << "BYTES_RECEIVED = " << bytes << endl;
		listing_part = this->receive_data();
		cout << listing_part;
		bytes_received += listing_part.length();
	}

	cout << endl << endl;

}

void Client::make_dir() {
	
	this->send_message("MDIR");
	
	// Prompt user for directory path
	cout << "Please enter the path of the directory you would like to create:" << endl;
	cout << "--> ";

	string dir_name = "";
	cin >> dir_name;
	dir_name = rstrip(dir_name);

	string message = to_string(dir_name.length()) + " " + dir_name;
	this->send_message(message);
	
	string res = "";
	while(res == "") {
		res = receive_data();
	}
	if(res == "-2") {
		cout << "The directory already exists on server" << endl;
	} else if(res == "-1") {
		cout << "Error in making directory" << endl;
	} else if(res == "1"){
		cout << "The directory was successfully made" << endl;
	}
}

void Client::remove_dir() {

	// Prompt user for directory path
	cout << "Please enter the path of the directory you would like to delete:" << endl;
	cout << "--> ";

	string dir_name = "";
	cin >> dir_name;
	dir_name = rstrip(dir_name);

	// Send the intent to upload	
	this->send_message("RDIR");
	string message = to_string(dir_name.length()) + " " + dir_name;
	this->send_message(message);
	
	string dir_exists = "";
	while(dir_exists == "") {
		dir_exists = receive_data();
	}
	if(dir_exists == "-1") {
		cout << "The directory does not exist on server" << endl;
	} else {
		string conf = "";
		while(conf != "Yes" && conf != "No") {
			cout << "Are you sure that you want to delete " << dir_name << "? (Yes/No) ";
			cin >> conf;
			conf = rstrip(conf);
		}
		this->send_message(conf);
		if(conf == "Yes") {
			string result = "";
			while(result == "") {
				result = receive_data();
			}
			if(result == "-1") {
				cout << "Failed to delete directory" << endl;
			} else if(result == "1"){
				cout << "Directory deleted" << endl;
			}
		} else {
			cout << "Delete abandoned by the user!" << endl;
		}
	}
}

void Client::change_dir() {

	this->send_message("CDIR");

	// Prompt user for filename
	cout << "Please enter the name of the directory to which you would like to change:" << endl;
	cout << "--> ";

	string dir = "";
	cin >> dir;
	dir = rstrip(dir);

	// Send the directory
	this->send_message(to_string(dir.length()) + " " + dir);

	// Retrieve the status
	string result = "";
	while(result == "") {
		result = receive_data();
	}
	int status = stoi(result);

	if(status == -2) {
		cout << "The directory does not exist on server" << endl;
	}
	else if (status == -1) {
		cout << "Error in changing directory" << endl;
	}
	else {
		cout << "Changed current directory" << endl;
	}

	
}
void Client::close_socket() {

	this->send_message("QUIT");
	close(this->sockfd);

}

int Client::calculate_throughput(int size, struct timeval start, struct timeval end) {

	return 8*size/(((end.tv_sec * 1000000) + end.tv_usec) - ((start.tv_sec * 1000000) + start.tv_usec));
}

// Convert a c string into a cpp string and return the cpp string
string Client::c_to_cpp_string(char* buf) {

	string str;
	if(buf == NULL) {
		str = string("");
	}
	else {
		str = string(buf);
	}

	return str;

}

void Client::split_msg(string msg, string &s1, string &s2) {

	string::size_type pos;
	pos=msg.find(' ',0);
	if(pos != string::npos) {
		s1 = msg.substr(0,pos);
		s2 = msg.substr(pos+1);
	}
}

string Client::rstrip(string str) {
	return str.substr(0, str.find_last_not_of(" \t\n") + 1);
}

