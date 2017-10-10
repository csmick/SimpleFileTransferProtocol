#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#define MAX_PENDING 5
#define BUFF_SIZE 4096
#define MAX_KEY 15

void encrypt(char *buffer, char *key, int buflen);
char *getTimestamp();

int main(int argc, char *argv[]) {

	// check if the user gave the correct number of command line arguments
	if (argc != 3) {
		fprintf(stderr, "usage:\n    ./udpServer portNumber encryptionKey\n");
		exit(1);
	}
	
	struct timeval now;
	char hours[3], minutes[3], seconds[3], microseconds[7], time_buf[16];
	struct sockaddr_in sin, client_addr;
	struct hostent *hostp; /* client host info */
	char buf[BUFF_SIZE], key[MAX_KEY];
	int addr_len, serverPort, errCheck, s, optval, numBytes, encryptedLen;

	
	serverPort = atoi(argv[1]); // assign port based on the command line argument

	// check that the encryption key is the correct length
	if(strlen(argv[2]) < 10 || strlen(argv[2]) > 15 ) {
		fprintf(stderr, "Invalid key length, please enter a key between 10 and 15 characters\n");
		exit(1);
	} else {
		strncpy(key, argv[2], MAX_KEY);
	}

	// build the address structure 
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY; // Use the default IP address of server
	sin.sin_port = htons(serverPort);
	
	// create a socket
	errCheck = (s = socket(PF_INET, SOCK_DGRAM, 0)); 
	if(errCheck < 0) {
		fprintf(stderr, "udpserver: socket error");
		exit(1);
	}

	// allow for the port to be reused immediately after closing
	optval = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));
	
	// bind the socket with the port
	errCheck = bind(s, (struct sockaddr *)&sin, sizeof(sin));
	if(errCheck < 0) {
		fprintf(stderr, "udpserver: bind error");
		exit(1);
	}
	
	addr_len = sizeof (client_addr);

	while (1){
		// receive the greeting from the client
		numBytes = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &addr_len);
		if(errCheck == -1) {
			fprintf(stderr, "udpserver: error receiving greeting\n");
			exit(1);
		}
		// printf("Server Recived: %s\n", buf);
		
		// send the key to the client as a response
		errCheck = sendto(s, key, strlen(key), 0, (struct sockaddr *) &client_addr, addr_len);
		if(errCheck == -1) {
			fprintf(stderr, "udpserver: error sending key\n");
			exit(1);
		}
		
		// receive the message back from the client
		numBytes = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &addr_len);
		if(errCheck == -1) {
			fprintf(stderr, "udpserver: error receiving message\n");
			exit(1);
		}
		//printf("Server Recived: %s\n", buf);
		
		// build the timestamp by getting the time in seconds since the epoch, then use modulo to
		// calculate minutes, hours, and seconds of the current day.
		gettimeofday(&now, NULL);
		
		strncpy(time_buf, "", 1);
		sprintf(microseconds, "%d", (int)now.tv_usec);
		sprintf(seconds, "%d", (int)now.tv_sec%60);
		sprintf(minutes, "%d", (int)(now.tv_sec/60)%60);
		sprintf(hours, "%d", (int)(now.tv_sec/3600)%24-4);
	
		// calculate the current hour
		while(strlen(hours) < 2) {
			char temp[3];
			strncpy(temp, hours, 3);
			strncpy(hours, "0", 3);
			strcat(hours, temp);
		}
		// printf("hours: %s\n", hours);

		// calculate the current minutes
		while(strlen(minutes) < 2) {
			char temp[3];
			strncpy(temp, minutes, 3);
			strncpy(minutes, "0", 3);
			strcat(minutes, temp);
		}
		// printf("minutes: %s\n", minutes);

		// calculate the current seconds
		while(strlen(seconds) < 2) {
			char temp[3];
			strncpy(temp, seconds, 3);
			strncpy(seconds, "0", 3);
			strcat(seconds, temp);
		}
		// printf("seconds: %s\n", seconds);

		while(strlen(microseconds) < 6) {
			char temp[7];
			strncpy(temp, microseconds, 7);
			strncpy(microseconds, "0", 7);
			strcat(microseconds, temp);
		}
		// printf("microseconds: %s\n", microseconds);

		// build the timestamp with the calculated times
		strcat(time_buf, hours);
		strcat(time_buf, ":");
		strcat(time_buf, minutes);
		strcat(time_buf, ":");
		strcat(time_buf, seconds);
		strcat(time_buf, ".");
		strcat(time_buf, microseconds);		
		
		// printf("time_buf: %s\n", time_buf);
		
		strcat(buf, time_buf);
		encryptedLen = strlen(buf);
		encrypt(buf, key, encryptedLen); 
		
		// send the encrypted message with the timestamp back
		errCheck = sendto(s, buf, encryptedLen, 0, (struct sockaddr *) &client_addr, addr_len);
		if(errCheck == -1) {
			fprintf(stderr, "udpserver: error sending key\n");
			exit(1);
		}
		
		// clear the buffet
		bzero((char*)&buf, sizeof(buf));
	}
	
	// close the socket
	close(s);
}

// encrypt by XORing the buffer with the key, loop through the key if it is not long enough
void encrypt(char *buffer, char *key, int buflen) {
	int i, keyLen = strlen(key);
	for(i = 0; i < buflen; i++) {
		buffer[i] = buffer[i] ^ key[i%keyLen];
	}
}
