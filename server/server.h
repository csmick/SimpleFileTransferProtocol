// TCP Server Class Definition

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

using namespace std;

#ifndef SERVER_H
#define SERVER_H

class Server {

	private:
		
		char *in_buffer;
		string in_buffer;
		int connection_socket;
		int data_socket;
		struct sockaddr_in sin;
		struct sockaddr_in client_addr;
		socklen_t client_addr_size;
		void change_directory();
		void list_directory_contents();
		void make_directory();
		void quit();
		string c_to_cpp_string(char *c_str);
		string rstrip(string str);

	public:

		Server(char *port);
		Server(const int port);
		void open_socket();
		void bind_socket();
		void listen_socket();
		void accept_connection();
		void send_data(string buffer);
		string receive_data();
		void parse_and_execute(string command);

};

#endif
