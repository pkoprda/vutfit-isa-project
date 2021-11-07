#include "argparser.hpp"


Arguments *Arguments::parse_arguments(int argc, char *argv[]){
    Arguments *arguments = new Arguments();
    int opt;
    int option_index = 0;
    static const char *short_options = "a:p:h";
    static const struct option long_options[] = {
        {"address", required_argument, nullptr, 'a' },
        {"port",    required_argument, nullptr, 'p' },
        {"help",    no_argument,       nullptr, 'h' },
        {nullptr,   no_argument,       nullptr,  0  }
    };

    while((opt = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1){
        switch(opt){
            case 'a':
                arguments->address = optarg;
                break;
            case 'p':
                arguments->port = stoi(optarg);
                try{
                    arguments->port = stoi(optarg);
                } catch(...){
                    error_exit("Port argument must be number between %d and %d!", MIN_PORT, MAX_PORT);
                } 
                if (arguments->port < MIN_PORT || arguments->port > MAX_PORT || !arguments->isNumber(optarg)){
                    error_exit("Port argument must be number between %d and %d!", MIN_PORT, MAX_PORT);
                }
                break;
            case 'h':
                arguments->print_help();
                exit(EXIT_SUCCESS);
            case '?':
            default:
                arguments->print_help();
                exit(EXIT_FAILURE);
        }
    }
    return arguments;
}

bool Arguments::isNumber(const string& str){
    char *ptr;
    strtol(str.c_str(), &ptr, 10);
    return *ptr == '\0';
}

void Arguments::print_help(){
    cout << "usage: client [ <option> ... ] <command> [<args>] ...\n"             << endl
         << "<option> is one of\n"                                                << endl
         << "  -a <addr>, --address <addr>"                                       << endl
         << "\tServer hostname or address to connect to"                          << endl
         << "  -p <port>, --port <port>"                                          << endl
         << "\t Server port to connect to"                                        << endl
         << "  --help, -h"                                                        << endl
         << "\tShow this help"                                                    << endl
         << "  --"                                                                << endl
         << "\tDo not treat any remaining argument as a switch (at this level)\n" << endl
         << "Multiple single-letter switches can be combined after one `-`."      << endl
         << "For example, `-h-` is the same as `-h --`."                          << endl
         << "Supported commands:"                                                 << endl
         << "  register <username> <password>"                                    << endl
         << "  login <username> <password>"                                       << endl
         << "  list"                                                              << endl
         << "  send <recipient> <subject> <body>"                                 << endl
         << "  fetch <id>"                                                        << endl
         << "  logout"                                                            << endl;
}
