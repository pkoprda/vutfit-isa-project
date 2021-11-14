#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "argparser.hpp"

using namespace std;

#define MAXDATASIZE 8192
#define EMPTY_LIST_LENGTH 7

class Client{
    public:
        int sockfd;

        void connect_to_server(Arguments *arguments, Client *client);
        string send_message(Arguments *arguments, Client *client);
        string print_response(Arguments *arguments, string buffer);
};
