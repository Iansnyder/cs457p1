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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define MAX_PORT_NUM 65535

void printUsage();
bool isValidPort(const char * port);

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
        //TODO add server implementation
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