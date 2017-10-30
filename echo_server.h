#ifndef PROXY_SERVER_ECHO_SERVER_H
#define PROXY_SERVER_ECHO_SERVER_H


#include <iostream>

struct echo_server {
    //Starts server - listening to port and outputs to another
    void start_echo_server(uint16_t port);

private:
    const int REQUEST_QUEUE_SIZE = 16;
    const int EPOLL_QUEUE_LEN = 16;
    const int MAX_EVENTS_GRABBED_PER_TICK = 16;

    int create_socket(uint16_t port);
    int create_epoll(int socket_fd);
    void accept_new_client(int socket_fd, int epoll_fd);
    void handle_client(int client_fd);
};


#endif //PROXY_SERVER_ECHO_SERVER_H
