/**
 * @file Main file
 * @author Peter Koprda <xkoprd00@stud.fit.vutbr.cz>
 */

#include "argparser.hpp"
#include "client.hpp"


int main(int argc, char* argv[]){
    Arguments *arguments = new Arguments();
    arguments->parse_options(arguments, argc, argv);
    Client *client = new Client();
    client->connect_to_server(arguments, client);
    arguments->parse_commands(arguments, argc, argv);
    string buffer = client->send_message(arguments, client);
    string response = client->print_response(arguments, buffer);
    cout << response;
    return 0;
}
