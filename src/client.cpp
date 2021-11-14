#include "client.hpp"


void Client::connect_to_server(Arguments *arguments, Client *client){
    int status;
    struct addrinfo hints, *res, *p;
    char ipstr[INET6_ADDRSTRLEN];
    memset(&hints, 0, sizeof(hints));
    // assign IP, PORT
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    string port_str = to_string(arguments->port);
    if((status = getaddrinfo((arguments->address).c_str(), port_str.c_str(), &hints, &res)) != 0){
        error_exit("getaddrinfo error: %s", gai_strerror(status));
    }

    for(p = res; p != NULL; p = p->ai_next){
        if(p->ai_family == AF_INET){ // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            inet_ntop(p->ai_family, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
        } else{ // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            inet_ntop(p->ai_family, &(ipv6->sin6_addr), ipstr, sizeof(ipstr));
        }
    }

    client->sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(client->sockfd == -1){
        error_exit("socket error: could not create socket");
    }
    if(connect(client->sockfd, res->ai_addr, res->ai_addrlen) == -1){
        error_exit("connection error: could not connect");
    }

    freeaddrinfo(res);
}

string Client::send_message(Arguments *arguments, Client *client){
    char buffer[arguments->target_args.length()];
    char *command = &*arguments->target_args.begin();       // convert string to char*
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, command);
    send(client->sockfd, buffer, sizeof(buffer), 0);
    char buf_out[MAXDATASIZE];
    bzero(buf_out, sizeof(buf_out));
    int numbytes = recv(client->sockfd, buf_out, sizeof(buf_out), 0);
    buf_out[numbytes] = '\0';
    if(close(client->sockfd) == -1){
        error_exit("error when closing socket");
    }
    return string(buf_out);

}

string Client::print_response(Arguments *arguments, string buffer){
    string response = "";
    string status;
    if(arguments->target != "logout"){
        status = buffer.substr(1, 3);
        status.erase(status.find_last_not_of(' ') + 1);
    }

    if(arguments->target == "register"){
        response = status == "ok" ? "SUCCESS: registered user " + arguments->target_user : "ERROR: user already registered";
    } else if(arguments->target == "login"){
        if(status == "ok"){
            response = "SUCCESS: user logged in";
            string user_hash = buffer.substr(22);
            user_hash.erase(user_hash.length() - 2);
            ofstream tokenfile;
            tokenfile.open("login-token");
            tokenfile << user_hash;
            tokenfile.close();
        } else if(regex_match(buffer, regex("(.*)(incorrect password)(.*)"))){
            response = "ERROR: incorrect password";
        } else{
            response = "ERROR: unknown user";
        }
    } else if(arguments->target == "logout"){
       response = "SUCCESS: logged out";
    } else if(arguments->target == "list"){
        response = "SUCCESS:";
        if(buffer.length() == EMPTY_LIST_LENGTH){
            return response;
        }
        string list_messages = buffer.substr(6, -1);
        list_messages.erase(list_messages.length() - 4);
        string delimiter = ") (";
        size_t pos = 0;
        string token;
        while((pos = list_messages.find(delimiter)) != string::npos){
            token = list_messages.substr(0, pos);
            response += '\n' + token.substr(0, token.find(' ')) + ":\n" +
                        "  From: " + token.substr(token.find(' ') + 2, token.find("\" \"") - 3) + '\n' +
                        "  Subject: " + token.substr(token.find("\" \"") + 3, -1);
            list_messages.erase(0, pos + delimiter.length());
        }
        token = list_messages;
        response += '\n' + token.substr(0, token.find(' ')) + ":\n" +
                    "  From: " + token.substr(token.find(' ') + 2, token.find("\" \"") - 3) + '\n' +
                    "  Subject: " + token.substr(token.find("\" \"") + 3, -1);
    } else if(arguments->target == "send"){
        response = status == "ok" ? "SUCCESS: message sent" : "ERROR: unknown recipient";
    } else if(arguments->target == "fetch"){
        if(status == "ok"){
            response = "SUCCESS:\n\n";
            string message = buffer.substr(6, -1);
            message.erase(message.length() - 3);
            response += "From: " + message.substr(0, message.find("\" \"")) + '\n';
            message = message.substr(message.find("\" \"") + 3, -1);
            response += "Subject: " + message.substr(0, message.find("\" \"")) + "\n\n";
            response += message.substr(message.find("\" \"") + 3, -1);
            return response;
        } else{
            response = "ERROR: message id not found";
        }
    }
    return response + '\n';
}


int main(int argc, char* argv[]){
    Arguments *arguments = new Arguments();
    arguments->parse_arguments(arguments, argc, argv);
    Client *client = new Client();
    client->connect_to_server(arguments, client);
    arguments->parse_commands(arguments, argc, argv);
    string buffer = client->send_message(arguments, client);
    string response = client->print_response(arguments, buffer);
    cout << response;
    return 0;
}
