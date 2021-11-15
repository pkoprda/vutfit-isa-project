/**
 * @file Header file for argparser
 * @author Peter Koprda <xkoprd00@stud.fit.vutbr.cz>
 */

#include <iostream>
#include <fstream>
#include <string>
#include <getopt.h>
#include "base64.h"

using namespace std;

#define error_exit(message, ...) \
    fprintf(stderr, message "\n", ##__VA_ARGS__); \
    exit(EXIT_FAILURE);

class Arguments{
    public:
        string address;
        int port;
        string target;
        string target_user;
        string target_passwd;
        string target_args;

        /**
         * Parse command line options (address, port number).
         * @param arguments Arguments object.
         * @param argc Number of given arguments.
         * @param argv Array of program arguments.
         */
        static void parse_options(Arguments *arguments, int argc, char *argv[]);

        /**
         * Parse commands (register, login, list, send, fetch, logout) and their options.
         * @param arguments Arguments object.
         * @param argc Number of given arguments.
         * @param argv Array of program arguments.
         */
        static void parse_commands(Arguments *arguments, int argc, char *argv[]);

        /**
         * Read login token from file.
         * @param delete_file If true delete file after reading.
         * @return User hash.
         */
        static string read_file(bool delete_file);

        /**
         * Print help message.
         */
        void print_help();

        /**
         * Print usage message.
         * @param argv[0] Path to executable file.
         */
        void print_usage(char* argv[0]);

    /**
     * Arguments constructor.
     */
    Arguments(){
        this->address = "127.0.0.1"; // IP address or hostname (default 127.0.0.1).
        this->port = 32323;          // Port number (default 32323).
    }

    /**
     * Arguments destructor. 
     */
    ~Arguments();
};
