#include <iostream>
#include <fstream>
#include <string>
#include <getopt.h>
#include "base64.h"

using namespace std;

#define MIN_PORT 0
#define MAX_PORT 65536

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

        static void parse_arguments(Arguments *arguments, int argc, char *argv[]);
        static void parse_commands(Arguments *arguments, int argc, char *argv[]);
        static string read_file(bool delete_file);
        void print_help();
        void print_usage(char* argv[0]);

    /**
     * Arguments constructor
     */
    Arguments(){
        this->address = "127.0.0.1";
        this->port = 32323;
    }
};
