#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

const int backlog { 5 };

int main() {

	int err {};

	struct addrinfo hints {};
	struct addrinfo *servinfo {};
	struct sockaddr_storage their_addr {};

	memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int status { getaddrinfo(NULL, "3490", &hints, &servinfo) };
	if (status != 0) {
		std::cerr << "gai error: " << gai_strerror(status) << "\n";
		exit(1);
	}

	// Create IPv4 socket
	int sock { socket(PF_INET, SOCK_STREAM, 0) };
	if (sock == -1) {
		// TODO: print proper error
		std::cerr << "Socket creation failed\n";
		exit(1);
	}

	//Bind socket TODO: error handling on bind
	err = bind(sock, servinfo->ai_addr, servinfo->ai_addrlen);  
	if (err == -1) {
		//TODO: print errno
		std::cerr << "Error binding socket\n";
		exit(1);
	}

	//Listen for connections 
	err = listen(sock, backlog); 
	if (err == -1) {
		std::cerr << "Error listening\n";
		exit(1);
	}

	//Accept connections
	socklen_t addr_size {sizeof(their_addr)};
	int new_sock = accept(sock, (struct sockaddr *)&their_addr, &addr_size);

	char const *msg = "Hello client!\n";
	int len = strlen(msg);

	int bytes_sent = send(new_sock, msg, len, 0);



	return 0;
}
