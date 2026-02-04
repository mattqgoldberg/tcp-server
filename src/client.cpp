#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

constexpr int max_size { 100 };

int main() {

	int status {};
	struct addrinfo hints {};
	struct addrinfo *servinfo {};
	struct addrinfo *p {};

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;


	if ((status = getaddrinfo("127.0.0.1", "3490", &hints, &servinfo) != 0)) {
		fprintf(stderr, "gai error: %s\n", gai_strerror(status));
		exit(1);
	}

	int sock {};
	int yes {1};

	for(p = servinfo; p != NULL; p = p->ai_next) {
		
		//Create socket
		if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("error creating socket");
			continue;
		}

		// Allow for socket reuse (for restarting server quickly)
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (connect(sock, p->ai_addr, p->ai_addrlen) == -1) {
			perror("error connecting");
			close(sock);
			continue;
		}

		break;
	}


	if (p == NULL) {
		std::cerr << "Failed to bind socket\n";
		exit(1);
	}

	freeaddrinfo(servinfo);

	char buf[max_size];
	int bytes_recv {};
	if ((bytes_recv = recv(sock, buf, max_size-1, 0)) == -1) {
		perror("error recv");
		exit(1);
	}

	buf[bytes_recv] = '\0';

	printf("server says: %s\n", buf);

	close(sock);


	return 0;
}

