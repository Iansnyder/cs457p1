/*******************************************
* Group Name  : XXXXXX

* Member1 Name: Ian Snyder
* Member1 SIS ID: 831615373
* Member1 Login ID: isnyder

* Member2 Name: Daniel Ives
* Member2 SIS ID: XXXXXX
* Member2 Login ID: XXXXXX

* Member2 Name: Lorenzo Garcia
* Member2 SIS ID: XXXXXX
* Member2 Login ID: XXXXXX
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

void printUsage();
bool isValidPort(const char * port);
int server_main();

int main(int argc, char* argv[]){

    int port = 0;
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
                port = atoi(optarg);
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
        //TODO add client implementation
        printf("IP address: %s\nPort: %d\n", IPaddress, port);
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

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

	if ((status = getaddrinfo(NULL, SERVER_PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 1;
	} 

    int sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    // could do setsockopt here to force binding
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

    // server accept loop


    // end server running code

    free(servinfo);

    return 0;
}