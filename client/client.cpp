// TCP Client Function Implementation

#include <stdio.h>	
#include <stdlib.h>	
#include <string.h>	
#include <sys/types.h>	
#include <sys/socket.h>	
#include <netinet/in.h>	
#include <netdb.h>	
#include <unistd.h>

#include "client.h"
using namespace std;

Client::Client(char *h, const int port, int buf_sz) {

	// malloc for buf
	this->buf = (char *) malloc(buf_sz);
	this->buf_size = buf_sz;
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

void Client::send_messages(char *buf1, char *buf2) {
	int len1 = strlen(buf1)+1, len2 = strlen(buf2)+1;
	if(send(this->sockfd, buf1, len1, 0) == -1) {
		fprintf(stderr, "tcp-client: send() failed");
		exit(1);
		close(this->sockfd);
	}
	if(send(this->sockfd, buf2, len2, 0) == -1) {
		fprintf(stderr, "tcp-client: send() failed");
		exit(1);
		close(this->sockfd);
	}
};

void Client::start() {
	
	printf("Welcome to Simple FTP client\n");
	this->print_usage();

	while(fgets(this->buf, this->buf_size, stdin)) { // loop until the input is finished
	
		buf[this->buf_size-1] = '\0';
		if(!strncmp(buf, "QUIT", 4)) {
			printf("Goodbye!");
			break;
		} else if(!strncmp(buf, "DWLD", 4)) {
			this->download();
		} else if(!strncmp(buf, "UPLD", 4)) {
			this->upload();
		} else if(!strncmp(buf, "DELF", 4)) {
			this->delete_file();
		} else if(!strncmp(buf, "LIST", 4)) {
			this->list();
		} else if(!strncmp(buf, "MDIR", 4)) {
			this->make_dir();
		} else if(!strncmp(buf, "RDIR", 4)) {
			this->remove_dir();
		} else if(!strncmp(buf, "CDIR", 4)) {
			this->change_dir();
		}

		this->print_usage();
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
	printf("Please enter the name of the file you would like to download: ");
	fgets(this->buf, this->buf_size, stdin);
	
	char *op = 0;
	strncpy(op, "DWLD", 5);;
	this->send_messages(op, buf);

}

void Client::upload() {}
void Client::delete_file() {}
void Client::list() {}
void Client::make_dir() {}
void Client::remove_dir() {}
void Client::change_dir() {}
void Client::close_socket() {}
