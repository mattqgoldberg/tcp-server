#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>


const int backlog { 5 };
const int yes {1};

void sigchld_handler(int s) {
	int saved_errno = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}

int main() {

	int err {};

	struct addrinfo hints {};
	struct addrinfo *servinfo {};
	struct addrinfo *p {};
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

	int sock {};

	for(p = servinfo; p != NULL; p = p->ai_next) {
		// Create IPv4 socket
		sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (sock == -1) {
			perror("error creating socket");
			continue;
		}

		// Allow for socket reuse (for restarting server quickly)
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		//Bind socket
		err = bind(sock, p->ai_addr, p->ai_addrlen);  
		if (err == -1) {
			perror("error binding");
			continue;
		}

		break;
	}



	freeaddrinfo(servinfo);

	if (p == NULL) {
		std::cerr << "Failed to bind socket\n";
		exit(1);
	}

	//Listen for connections 
	err = listen(sock, backlog); 
	if (err == -1) {
		perror("error listening");
		exit(1);
	}

	//Handle dead fork processes
	struct sigaction sa {};
	sa.sa_handler = sigchld_handler;
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("error sigaction");
		exit(1);
	}


	while (true) {

		//Accept connections
		socklen_t addr_size {sizeof(their_addr)};
		int new_sock = accept(sock, (struct sockaddr *)&their_addr, &addr_size);
		if (new_sock == -1) {
			perror("error accepting");
			exit(1);
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
			_exit(0);
		}

		close(new_sock);
	}

	return 0;
}
