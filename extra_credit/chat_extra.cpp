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
#include <poll.h>

#define MAX_PORT_NUM 65535
#define SERVER_PORT "3360"
#define SERVER_BACKLOG_LIMIT __INT_MAX__

struct message {
    uint16_t version;
    uint16_t length;
    char data[140];
};

typedef struct {
    struct pollfd *clients; 
    bool *is_connected;
    size_t used_connections;
    size_t total_connections;
} Connections;

void printUsage();
bool isValidPort(const char * port);
int server_main();
int client_main(const char * server_ip, const char * server_port);
int sendMessage(int sendfd);
void createMessage(char * data, struct message * msg);
int recMessage(int fromfd, Connections *con);
int checkForMessages(int sockfd);
void clearStdin();
void *get_in_addr(struct sockaddr *sa);
struct pollfd setup_listening_socket(int sockID);

/* The following Array segmented code was "inspired" by https://stackoverflow.com/questions/3536153/c-dynamically-growing-array */

void intialize_Array(Connections *con, size_t intitialSize){
    con->clients = (pollfd *) calloc(intitialSize, sizeof(Connections));
    con->is_connected = (bool *) calloc(intitialSize, sizeof(bool));
    con->used_connections = 0;
    con->total_connections = intitialSize;
}

void insert_Connections (Connections *con, pollfd elements){
    if(con->used_connections == con->total_connections){
        con->total_connections *= 2;
        con->clients = (pollfd *) realloc(con->clients, con->total_connections * sizeof(pollfd));
        con->is_connected = (bool *) realloc(con->is_connected, con->total_connections * sizeof(bool));
    }

    con->clients[con->used_connections] = elements;
    con->is_connected[con->used_connections] = true;
    con->used_connections++;
}

void freeArray(Connections *con){
    free(con->clients);
    free(con->is_connected);
    con->clients = NULL;
    con->used_connections = con->total_connections = 0;
}

/*End of "inspired" code*/


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
    

    Connections connections;
    intialize_Array(&connections, 10);

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

    // server recv loop
    // add server as listener with events
    insert_Connections(&connections, setup_listening_socket(sockfd));

    while(1){
        int poll_count = poll(connections.clients, connections.used_connections, -1);

        if(poll_count == -1){
            perror("poll");
            exit(1);
        }

        for(size_t i = 0; i < connections.used_connections; i++){

            if(connections.is_connected[i] == false){
                continue;
            }

            if(connections.clients[i].revents & POLLIN){
                if(connections.clients[i].fd == sockfd){
                    struct sockaddr_storage clientAddr;
                    socklen_t sinSize = sizeof clientAddr;
                    // server recv loop
                    int clientSockFd = accept(sockfd, (struct sockaddr *)&clientAddr, &sinSize); 

                    if(clientSockFd == -1){
                        perror("accept");
                    }
                    else{
                        char remoteIP[INET6_ADDRSTRLEN];
                        insert_Connections(&connections, setup_listening_socket(clientSockFd));  
                        printf("pollserver: New connection from %s on socket %d\n", inet_ntop(clientAddr.ss_family, get_in_addr((struct sockaddr*) &clientAddr), remoteIP, INET6_ADDRSTRLEN), clientSockFd);
                        
                        //send welcome message
                        char welcomeMessage[140];
                        sprintf(welcomeMessage, "Server: Welcome to Chat! Your client ID is %d", (int) connections.used_connections - 1);
                        struct message msg;
                        createMessage(welcomeMessage, &msg);
                        send(clientSockFd, &msg, sizeof(msg), 0);
                        printf("pollserver: sent welcome message to %d\n", clientSockFd);
                    }
                }
                else {
                    if (recMessage(connections.clients[i].fd, &connections) == -1) {
                        connections.is_connected[i] = false;
                        printf("pollserver: user %d disconnected\n", (int) i);
                    }

                }
            }
        }

    }

    // end recv loop

    free(servinfo);
    freeArray(&connections);

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
        printf("Connected!\n\n");
        printf("Send messages to user by typing 'ID::MESSAGE'\n");
        printf("Press enter to check for new messages.\n\n");
    }

    usleep(200000);
    checkForMessages(sockfd); //for welcome message
    printf("\n\n");
    //client loop
    while (1) {
        if (sendMessage(sockfd) == -1) {
            return 1;
        }
        usleep(200000); //in case server responds to message
        checkForMessages(sockfd);
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
        //if the user only enters a newline, scanf will return 0
        if (input_size == 0) {
            clearStdin();
            return 0;
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

int checkForMessages(int sockfd) {
    while(1) {
        
    struct message msg;
    int bytesReceived = recv(sockfd, &msg, sizeof msg, MSG_DONTWAIT);
    if (bytesReceived == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        }
        perror("Recv");
        return 1;
    }
    else if (bytesReceived == 0) {
        printf("Server disconnected.\n");
        return 1;
    }

    msg.version = ntohs(msg.version);
    msg.length = ntohs(msg.length);

    if (msg.version != 457) {
        fprintf(stderr, "Error: Tried to receive message, but version numbers don't match.\n");
        return 0;
    }

    if (msg.length != bytesReceived-4) {
        if (msg.length != strlen(msg.data)) { //interoperability with other chat programs that pad their packet with null bytes
            fprintf(stderr, "Error: Message length doesn't match.\n");
            return 0;
        }
    }

    printf("Friend: %s\n", msg.data);
    }
        return 0;
}

int recMessage(int fromfd, Connections *con){
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
        fprintf(stderr, "Error: Message length doesn't match.\n");
        return 0;
    }

    int user;

    char message [140];

    memset(message, 0, sizeof(message));

    if (sscanf(msg.data, "%d::%140[^\n]", &user, message) != 2) {
        //if the format doesn't match, the message was for the server, so print it
        printf("Friend: %s\n", msg.data);

        char thankYou[140];
        memset(thankYou, 0, sizeof(thankYou));
        sprintf(thankYou, "Server: Thank you for the message! To send to a user, make sure to write your message using ID::MESSAGE format");
        struct message msg;
        memset(msg.data, 0, sizeof(msg.data));
        createMessage(thankYou, &msg);
        if (send(fromfd, &msg, sizeof(struct message), 0) == -1) {
            perror("Send");
            return 1;
        }
        return 0;
    }

    if((size_t) user >= con->used_connections || con->is_connected[user] == false) {
        char notConnectedMsg[140];
        memset(notConnectedMsg, 0, sizeof(notConnectedMsg));
        sprintf(notConnectedMsg, "Server: User %d is not connected.", user);
        notConnectedMsg[strlen(notConnectedMsg)] = '\0';
        struct message error_msg;
        memset(error_msg.data, 0, sizeof(error_msg.data));
        createMessage(notConnectedMsg, &error_msg);
        if (send(fromfd, &error_msg, sizeof(struct message), 0) == -1) {
            perror("Send");
            return 1;
        }

        return 1;
    }
    
    struct message new_msg;
    memset(new_msg.data, 0, sizeof(new_msg.data));
    createMessage(message, &new_msg);


    if (send(con->clients[user].fd, &new_msg, sizeof new_msg, 0) == -1) {
        perror("Send");
        return 1;
    }

    return 0;
}

void clearStdin() {
    char c;
    while((c = getchar()) != '\n' && c != EOF) {
        //clear out any stuff leftover in the input
    }
}

struct pollfd setup_listening_socket(int sockID){
    struct pollfd new_Socket;
    new_Socket.fd = sockID;
    new_Socket.events = POLLIN;
    return new_Socket;
}

void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

