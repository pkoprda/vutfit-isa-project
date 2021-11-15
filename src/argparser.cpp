/**
 * @file File for parsing command line arguments
 * @author Peter Koprda <xkoprd00@stud.fit.vutbr.cz>
 */

#include "argparser.hpp"


void Arguments::parse_options(Arguments *arguments, int argc, char *argv[]){
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
}

void Arguments::parse_commands(Arguments *arguments, int argc, char *argv[]){
    if(optind == argc){
        arguments->print_usage(&argv[0]);
    }

    argc -= optind;
    arguments->target = argv[1];
    if(arguments->target == "list"){
        if(argc != 1){
            error_exit("list");
        }
        string user_hash = read_file(false);
        arguments->target_args = '(' + arguments->target + " \"" + user_hash + "\")";
    }
    else if(arguments->target == "logout"){
        if(argc != 1){
            error_exit("logout");
        }
        string user_hash = read_file(true);
        arguments->target_args = '(' + arguments->target + " \"" + user_hash + "\")";
    } else if(arguments->target == "fetch"){
        if(argc != 2){
            error_exit("fetch <id>");
        }
        string user_hash = read_file(false);
        arguments->target_args = '(' + arguments->target + " \"" + user_hash + "\" " + string(argv[2]) + ')';
    } else if(arguments->target  == "register" || arguments->target  == "login"){
        if(argc != 3){
            error_exit("%s <username> <password>", argv[1]);
        }
        arguments->target_user = argv[2];
        arguments->target_passwd = Base64::encode(string(argv[3]));
        arguments->target_args = '(' + arguments->target + " \"" + arguments->target_user + "\" \"" + arguments->target_passwd + "\")";
    } else if(arguments->target == "send"){
        if(argc != 4){
            error_exit("send <recipient> <subject> <body>");
        }
        string user_hash = read_file(false);
        arguments->target_args = '(' + arguments->target + " \"" + user_hash + "\" \"" + string(argv[2]) + "\" \"" + string(argv[3]) + "\" \"" + string(argv[4]) + "\")";
    } else{
        error_exit("unknown command");
    }
}

string Arguments::read_file(bool delete_file){
    ifstream token_file;
    string user_hash;
    token_file.open("login-token");
    if(token_file){
        token_file >> user_hash;
        token_file.close();
        if(delete_file && remove("login-token") != 0){
            error_exit("Error deleting file");
        }
    } else{
        error_exit("Not logged in");
    }
    return user_hash;
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
