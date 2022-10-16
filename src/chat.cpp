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

void printUsage();

int main(int argc, char* argv[]){

    int port = 0;
    char * IPaddress;
    int c;

    while (( c = getopt(argc, argv, "+p:s:h")) != -1) {
        switch (c) {
            case 'p':
                // TODO verify that port is a number and valid
                port = atoi(optarg);
                break;
            case 's':
                IPaddress = optarg;
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

    if (port && IPaddress) { // we are the client
        //TODO add client implementation
        printf("IP address: %s\nPort: %d\n", IPaddress, port);
    }
    else { // we are the server
        // TODO add server implementation
    }
    
    return 0;
}

void printUsage() {
    printf("Make sure to run the client side this way:\n./chat -p [PORT NUMBER] -s [IP ADDRESS]\nMake sure to run the server side this way:\n./chat\n");
}