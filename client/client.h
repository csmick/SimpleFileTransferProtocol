// TCP Client Class Definition

#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
using namespace std;

class Client {

	private:
		
		char *buf;	
		int buf_size;
		int sockfd;
		char *host;
		struct hostent *hp;
		struct sockaddr_in sin;
		socklen_t client_addr_size;
		int calculate_throughput(int size, struct timeval start, struct timeval end);
		string c_to_cpp_string(char* buf);
		void split_msg(string msg, string &s1, string &s2);
		string rstrip(string str);

	public:

		Client(char *h, const int port, int buf_size);
		
		void start();	
		void print_usage();
		
		void open_socket();
		void connect_socket();
		void send_message(const char *buf);
		void receive_data(char *buf);
	
		void download();
		void upload();
		void delete_file();
		void list();
		void make_dir();
		void remove_dir();
		void change_dir();
		void close_socket();
};
