#include <iostream>
#include "webserver.h"

int main(int, char**){

    std::cout << "Hello, from rtspChaosClub!\n";

    WebServer server;

    const int result = server.start();

    std::cout << "Start result: " << result << '\n';
    std::cout << "Port: " << server.port() << '\n';

    std::cin.get();

    server.stop();

    return 0;
}
