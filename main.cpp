#include <iostream>
#include "echo_server.h"

int main() {
    std::cout << "Hello, World!" << std::endl;

    echo_server server;
    server.start_echo_server(8883);

    return 0;
}