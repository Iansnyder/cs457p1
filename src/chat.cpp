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
#include <string>
#include <iostream>
#include <unistd.h> // getopt
#include <cstdlib>

using namespace std;

int main(int argc, char* argv[]){

    int port = 0;
    string IPaddress = "";
    int c;

    while (( c = getopt(argc, argv, "+p:s:")) != -1) {
        switch (c) {
            case 'p':
                // TODO verify that port is a number and valid
                port = atoi(optarg);
                break;
            case 's':
                IPaddress = optarg;
                break;
            case '?':
                //TODO more elaborate flags error checking
                cerr << argv[0] << ": Erroneous argument encountered.\n";
                return 1;
            default:
                cerr << argv[0] << ": Something unexpected happened while parsing arguments\n";
                return 1;
        }
    }

    if (port && !IPaddress.empty()) { // we are the client
        //TODO add client implementation
        cout << "IP address: " << IPaddress << "\nPort: " << port << '\n';
    }
    else { // we are the server
        // TODO add server implementation
    }
    
    return 0;
}
