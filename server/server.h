// TCP Server Class Definition

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

using namespace std;

#ifndef SERVER_H
#define SERVER_H

class Server {

	private:
		
		int connection_socket;
		int data_socket;
		struct sockaddr_in sin;
		struct sockaddr_in client_addr;
		socklen_t client_addr_size;
		void download_file();
		void upload_file();
		void change_directory();
		void list_directory_contents();
		void make_directory();
		void remove_directory();
		void delete_file();
		void quit();
		int calculate_throughput(int size, struct timeval start, struct timeval end);
		string c_to_cpp_string(char *c_str);
		string rstrip(string str);
		void split_msg(string msg, string &s1, string &s2);

	public:

		char *in_buffer;	
		int in_buf_sz;
		Server(char *port, int buf_sz);
		Server(const int port, int buf_sz);
		void open_socket();
		void bind_socket();
		void listen_socket();
		void accept_connection();
		void send_data(const char *buffer);
		void receive_data(char *buffer);
		void parse_and_execute(string command);

};

#endif
