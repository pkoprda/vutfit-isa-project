/**
 * @file Header file for client
 * @author Peter Koprda <xkoprd00@stud.fit.vutbr.cz>
 */

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

using namespace std;

#define MAXDATASIZE 8192        // Max length of buffer which can send through socket.
#define EMPTY_LIST_LENGTH 7     // Length of list which does not have any message.

class Client{
    public:
        /**
         * socket file descriptor
         */
        int sockfd;

        /**
         * Create socket and connect to server.
         * @param arguments Arguments object.
         * @param client Client object.
         */
        void connect_to_server(Arguments *arguments, Client *client);

        /**
         * Send message through socket, get message from socket and close socket.
         * @param arguments Arguments object.
         * @param client Client object.
         * @return Received data from server.
         */
        string send_message(Arguments *arguments, Client *client);

        /**
         * Parse response from the server.
         * @param arguments Arguments object.
         * @param buffer Received buffer from the server.
         * @return Response which will be printed on stdout.
         */
        string print_response(Arguments *arguments, string buffer);
};
