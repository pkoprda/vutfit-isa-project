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
    bool address_opt, port_opt = address_opt = false;

    while((opt = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1){
        switch(opt){
            case 'a':
                arguments->address = optarg;
                if(address_opt){
                    error_exit("%s: only one instance of one option from (-a --address) is allowed", argv[0]);
                }
                address_opt = true;
                break;
            case 'p':
                try{
                    arguments->port = stoi(optarg);
                } catch(...){
                    error_exit("Port number is not a string");
                }
                if(port_opt){
                    error_exit("%s: only one instance of one option from (-p --port) is allowed", argv[0]);
                }
                port_opt = true;
                if (arguments->port < MIN_PORT || arguments->port > MAX_PORT || !arguments->isNumber(optarg)){
                    error_exit("Port number must be between %d and %d", MIN_PORT, MAX_PORT);
                }
                break;
            case 'h':
                arguments->print_help();
                exit(EXIT_SUCCESS);
            case '?':
                exit(EXIT_FAILURE);
            default:
                arguments->print_help();
                exit(EXIT_FAILURE);
        }
    }
    return arguments;
}

Arguments *Arguments::parse_commands(Arguments *arguments, int argc, char **argv){
    if(optind == argc){
        arguments->print_usage(&argv[0]);
    }

    argc -= optind;
    if(argv[optind] == string("list") || argv[optind] == string("logout")){
        if(argc != 1){
            error_exit("%s", argv[optind]);
        }
        arguments->target = argv[optind];
    } else if(argv[optind] == string("fetch")){
        if(argc != 2){
            error_exit("fetch <id>");
        }
        arguments->target = "fetch";
    } else if(argv[optind]  == string("register") || argv[optind]  == string("login")){
        if(argc != 3){
            error_exit("%s <username> <password>", argv[optind]);
        }
        arguments->target = argv[optind];
    } else if(argv[optind] == string("send")){
        if(argc != 4){
            error_exit("send <recipient> <subject> <body>");
        }
        arguments->target = "send";
    } else{
        error_exit("unknown command");
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
         << "\tServer port to connect to"                                         << endl
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

void Arguments::print_usage(char* argv[0]){
    error_exit("%s: expects <command> [<args>] ... on the command line, given 0 arguments", argv[0]);
}
