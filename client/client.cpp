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
	return c_to_cpp_string(in_buffer);
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

	// TODO: Receive file from server and save to disk

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

	// TODO: Upload file to server

}

void Client::delete_file() {

	// Prompt user for filename
	cout << "Please enter the name of the file you would like to delete:" << endl;
	cout << "--> ";
	string filename = "";
	cin >> filename;
	filename = rstrip(filename);

	// Send the intent to delete
	this->send_message("DELF");
	this->send_message(filename.length() + " " + filename);

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
		deleteConf = rstrip(deleteConf);

		if(deleteConf == "Yes" || deleteConf == "No") {
			this->send_message(deleteConf);
			break;
		}
	}

	if(deleteConf == "Yes") {
		// TODO: receive confirmation message		
	}
	else {
		cout << "Delete abandoned by user!" << endl << endl;
	}
}

void Client::list() {

	this->send_message("LIST");
	
	cout << "receiving list::msg" << endl;	
	string msg = this->receive_data();
	cout << "list::msg: <<" << msg << ">>" << endl;

	string numBytesString, listing_part;

	this->split_msg(msg, numBytesString, listing_part);

	cout << listing_part << endl;

	int bytes = stoi(numBytesString);

	// Loop until all bytes from the file listing are read
	int i = listing_part.length();
	while(i < bytes) {
		cout << "receiving listing_part" << endl;	
		listing_part = this->receive_data();
		cout << listing_part;
		i += listing_part.length();
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

	// Send the intent to upload	
	string message = to_string(dir_name.length()) + " " + dir_name;
	this->send_message(message);
	
	cout << "receiving res" << endl;	
	string res = receive_data();
	cout << "res: <<" << res << ">>" << endl;
	if(res == "-2") {
		cout << "The directory already exists on server" << endl;
	} else if(res == "-1") {
		cout << "Error in making directory" << endl;
	} else if(res == "1"){
		cout << "The directory was successfully made" << endl;
	} else {
		cout << "Error with MDIR" << endl;
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
	
	cout << "receiving dir_exists" << endl;	
	string dir_exists = receive_data();
	cout << "dir_exists: <<" << dir_exists << ">>" << endl;
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
			cout << "receiving result dir_exists" << endl;	
			string result = receive_data();
			cout << "result: <<" << result << ">>" << endl;
			if(result == "-1") {
				cout << "Failed to delete directory" << endl;
			} else if(result == "1"){
				cout << "Directory deleted" << endl;
			} else {
				cout << "Error with RDIR" << endl;
			}
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

	// Send the directory
	this->send_message(to_string(dir.length()) + " " + dir);

	// Retrieve the status
	int status = stoi(this->receive_data());

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

