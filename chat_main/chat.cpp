/*******************************************
* Group Name  : earlybirds
 
* Member1 Name: Ian Snyder
* Member1 SIS ID: 831615373
* Member1 Login ID: isnyder

* Member2 Name: Daniel Ives
* Member2 SIS ID: 832928510 
* Member2 Login ID: daives

* Member2 Name: Lorenzo Garcia
* Member2 SIS ID: 832606636
* Member2 Login ID: enzo95
********************************************/
#include <unistd.h> // getopt
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>

#define MAX_PORT_NUM 65535
#define SERVER_PORT "3360"
#define SERVER_BACKLOG_LIMIT 10

struct message {
    uint16_t version;
    uint16_t length;
    char data[140];
};

void printUsage();
bool isValidPort(const char * port);
int server_main();
int client_main(const char * server_ip, const char * server_port);
int sendMessage(int sendfd);
void createMessage(char * data, struct message * msg);
int recMessage(int fromfd);
void clearStdin();

int main(int argc, char* argv[]){

    char * port;
    char * IPaddress;
    int c;
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;

    while ((c = getopt(argc, argv, "+p:s:h")) != -1) {
        switch (c) {
            case 'p':
                if (!isValidPort(optarg)) {
                    fprintf(stderr, "%s: %s is not a valid port number.\n", argv[0], optarg);
                    return 1;
                }
                port = optarg;
                break;
            case 's':
                IPaddress = optarg;
                if (!inet_pton(AF_INET, IPaddress, &serverAddress.sin_addr)) {
                    fprintf(stderr, "%s: %s is an invalid IPv4 address.\n", argv[0], IPaddress);
                    return 1;
                }
                break;
            case 'h':
                printUsage();
                return 0;
                break;
            case '?':
                //TODO more elaborate flags error checking
                fprintf(stderr, "%s: Erroneous argument encountered.\n", argv[0]);
                return 1;
            default:
                fprintf(stderr, "%s: Something unexpected happened while parsing arguments.\n", argv[0]);
                return 1;
        }
    }

    if (port && IPaddress) { //we are the client
        printf("IP address: %s\nPort: %s\n", IPaddress, port);
        return client_main(IPaddress, port);
    }
    else { // we are the server
        return server_main();
    }
    
    return 0;
}

void printUsage() {
    printf("Make sure to run the client side this way:\n./chat -p [PORT NUMBER] -s [IP ADDRESS]\nMake sure to run the server side this way:\n./chat\n");
}


bool isValidPort(const char * port) {
    size_t length = strlen(port);
    if (length == 0) {
        return false;
    }
    for (size_t i = 0; i < length; ++i) {
        if (!isdigit(*(port+i)))
            return false;
    }
    long portVal = atoi(port); //this is only loose protection against wrapping. Entering invalid ports is undef behavior
    if (portVal < 1 || portVal > MAX_PORT_NUM) {
        return false;
    }
    return true;
}

int server_main() {
    struct addrinfo hints, *servinfo;
    int status;
    int yes = 1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, SERVER_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    } 

    int sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

    // allow program to reuse the socket if the server crashes
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("Setsockopt");
        return 1;
    }

    if (bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen)) {
        perror("Bind"); //netstat -ltp to find an in use port for testing bind errors
        return 1;
    }

    if (listen(sockfd, SERVER_BACKLOG_LIMIT)) {
        perror("Listen");
        return 1;
    }
    char ipstr[INET_ADDRSTRLEN];
    struct sockaddr_in * serverIP = (sockaddr_in *) servinfo->ai_addr;
    inet_ntop(servinfo->ai_family, &serverIP->sin_addr, ipstr, sizeof ipstr);
    printf("Welcome to Chat!\n");
    printf("Waiting for a connection on %s port %s\n", ipstr, SERVER_PORT);

    struct sockaddr_storage clientAddr;
    socklen_t sinSize = sizeof clientAddr;
    // server recv loop
    int clientSockFd = accept(sockfd, (struct sockaddr *)&clientAddr, &sinSize);
    printf("Found a friend! You receive first.\n");
    while (1) {
        if (recMessage(clientSockFd) == -1) {
            return 1;
        }
        if (sendMessage(clientSockFd) == -1) {
            return 1;
        }
    }

    // end recv loop

    free(servinfo);

    return 0;
}

int client_main(const char * server_ip, const char * server_port) {
    struct addrinfo hints, *servinfo, *p;
    int status, sockfd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(server_ip, server_port, &hints, &servinfo)) != 0)  {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }

    printf("Connecting to server...\n");

	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
    else {
        printf("Connected!\nConnected to a friend! You send first.\n");
    }

    //client send loop
    while (1) {
        if (sendMessage(sockfd) == -1) {
            return 1;
        }
        if (recMessage(sockfd) == -1) {
            return 1;
        }
    }

    return 0;
}

int sendMessage(int sockfd) {
    char user_input[141];
    memset(user_input, 0, sizeof user_input);
    int input_size = 0;
    while (1) {
        printf("You: ");
        scanf("%141[^\n]%n", user_input, &input_size);
        if (input_size > 140) {
            fprintf(stderr, "Error: Input too long.\n");
            clearStdin();
            continue;
        }

        //TODO figure out what to do if a user simply doesn't input anything and presses ENTER
        user_input[input_size] = '\0';
        break;
    }

    clearStdin();


    struct message msg;
    createMessage(user_input, &msg);
    if (send(sockfd, &msg, (input_size+4), 0) == -1) {
        perror("Send");
        return 1;
    }

    return 0;

}

void createMessage(char * data, struct message * msg) {
    msg->version = htons(457);
    msg->length = htons(strlen(data));
    strcpy(msg->data, data);
}

int recMessage(int fromfd){
    struct message msg;
    memset(msg.data, 0, sizeof(msg.data));
    ssize_t numBytesRecv;

    if ((numBytesRecv = recv(fromfd, &msg, sizeof msg, 0)) == -1) {
        perror("Receive");
        return 1;
    }

    if (numBytesRecv == 0) {
        printf("Connection closed by peer.\n");
        return -1;
        }

    msg.version = ntohs(msg.version);
    msg.length = ntohs(msg.length);

    if (msg.version != 457) {
        fprintf(stderr, "Error: Version numbers don't match.\n");
        return 0;
    }

    if (msg.length != numBytesRecv-4) {
        if (msg.length != strlen(msg.data)) { //interoperability with other chat programs that pad their packet
            fprintf(stderr, "Error: Message length doesn't match.\n");
            return 0;
        }
    }

    printf("Friend: %s\n", msg.data);

    return 0;
}

void clearStdin() {
    char c;
    while((c = getchar()) != '\n' && c != EOF) {
        //clear out any stuff leftover in the input
    }
}