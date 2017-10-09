// TCP Server Class Definition

#include <netinet/in.h>
#include <sys/socket.h>

class Server {

	private:

		struct sockaddr_in sin;
		struct sockaddr_in client_addr;
		socklen_t client_addr_size;

	public:

		Server(char *port);
		Server(const int port);
		int open_socket();
		void bind_socket(const int sockfd);
		void listen_socket(const int sockfd);
		int accept_connection(const int sockfd);

};
