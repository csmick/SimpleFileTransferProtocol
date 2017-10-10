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

void Client::send_messages(string s) {
	int len = s.length();
	if(send(this->sockfd, s.c_str(), len, 0) == -1) {
		fprintf(stderr, "tcp-client: send() failed");
		close_socket();
		exit(1);
	}
};

string Client::receive_data() {

	char in_buffer[4096];
	if(read(this->sockfd, (void *) &in_buffer, 4096) == -1) {
		perror("read() failed");
		exit(1);
	}
	return c_to_cpp_string(in_buffer);
}

void Client::start() {
	
	printf("Welcome to Simple FTP client\n");
	this->print_usage();

	cout << endl << endl;
	cout << "> ";
	string command;
	while(cin >> command) { // loop until the input is finished
	
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
	
	// Send download request
	string message = "DWLD";
	this->send_messages(message);
	message = to_string(filename.length()) + " " + filename;
	this->send_messages(message);

	// TODO: Receive file from server and save to disk

}

void Client::upload() {

	// Prompt user for filename
	cout << "Please enter the name of the file you would like to upload:" << endl;
	cout << "--> ";

	string filename = "";
	cin >> filename;

	// Send the intent to upload	
	this->send_messages("UPLD");
	string message = to_string(filename.length()) + " " + filename;
	this->send_messages(message);

	// TODO: Upload file to server

}

void Client::delete_file() {

	// Prompt user for filename
	cout << "Please enter the name of the file you would like to delete:" << endl;
	cout << "--> ";
	string filename = "";
	cin >> filename;

	// Send the intent to delete
	this->send_messages("DELF");
	this->send_messages(filename.length() + " " + filename);

	// Acknowledgement from server
	int ack = stoi(this->receive_data());

	if(ack != 1) {
		cout << "The file does not exist on server" << endl << endl;
		return;
	}

	string deleteConf;
	while(deleteConf != "Yes" && deleteConf != "No") {
		cout << "Are you sure you want to delete " << filename << "? (Yes, No): ";
	
		// Read in the user's answer
		cin >> deleteConf;

		if(deleteConf == "Yes" || deleteConf == "No") {
			this->send_messages(deleteConf);
			break;
		}
	}

	if(deleteConf == "Yes") {
		
	}
	else {
		cout << "Delete abandoned by user!" << endl << endl;
	}
}

void Client::list() {

	this->send_messages("LIST");

	string numBytesString = this->receive_data();
	int bytes = stoi(numBytesString);

	// Loop until all bytes from the file listing are read
	int i = 0;
	while(i < bytes) {
		string listingPart = this->receive_data();
		cout << listingPart;
		i += listingPart.length();
	}

	cout << endl << endl;

}

void Client::make_dir() {}
void Client::remove_dir() {}
void Client::change_dir() {}
void Client::close_socket() {

	this->send_messages("QUIT");
	close(this->sockfd);

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
