#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>


const int backlog { 5 };
const int yes {1};

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
		perror("error creating socket");
		exit(1);
	}

	// Allow for socket reuse (for restarting server quickly)
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
    }

	//Bind socket
	err = bind(sock, servinfo->ai_addr, servinfo->ai_addrlen);  
	if (err == -1) {
		perror("error binding");
		exit(1);
	}

	//Listen for connections 
	err = listen(sock, backlog); 
	if (err == -1) {
		perror("error listening");
		exit(1);
	}

	struct sigaction sa {};

	while (true) {

		//Accept connections
		socklen_t addr_size {sizeof(their_addr)};
		int new_sock = accept(sock, (struct sockaddr *)&their_addr, &addr_size);
		if (new_sock == -1) {
			perror("error accepting");
		}

		pid_t c_pid = fork();
		if (c_pid ==-1) {
			perror("fork");
			exit(1);
		}

		if (c_pid == 0) {
			close(sock);
			if(send(new_sock, "Hello client!\n", 14, 0) == -1) {
				perror("error sending");
			}
			close(new_sock);
			exit(0);
		}

		close(new_sock);
	}

	return 0;
}
