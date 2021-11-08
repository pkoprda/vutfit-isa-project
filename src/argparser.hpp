#include <iostream>
#include <string>
#include <getopt.h>

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

        static Arguments *parse_arguments(int argc, char *argv[]);
        static Arguments *parse_commands(Arguments *arguments, int argc, char *argv[]);
        bool isNumber(const string& str);
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
