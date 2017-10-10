// TCP Client Class Definition

#include <netinet/in.h>
#include <sys/socket.h>

class Client {

	private:
		
		char *buf;	
		int buf_size;
		int sockfd;
		char *host;
		struct hostent *hp;
		struct sockaddr_in sin;
		socklen_t client_addr_size;

	public:

		Client(char *h, const int port, int buf_size);
		
		void start();	
		void print_usage();
		
		void open_socket();
		void connect_socket();
		void send_messages(char *buf1, char *buf2);
		
		void download();
		void upload();
		void delete_file();
		void list();
		void make_dir();
		void remove_dir();
		void change_dir();
		void close_socket();
};