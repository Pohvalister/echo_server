#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>

#include <cerrno>
#include <cassert>
#include <iostream>
#include <cstring>

#include "echo_server.h"

#define LOG(a) std::cout<<(a)<<'\n'
#define FLOG(a) std::cout<<"\n--\n"<<(a)<<", error:\n"<<'['<<std::string(strerror(errno))<<']'<<"\n--\n\n"
#define ELOG std::cout<<"_________\n\n"

int echo_server::create_socket(uint16_t port) {
    //Для создания сокета типа stream с протоколом TCP, обеспечивающим коммуникационную поддержку, вызов функции socket должен быть следующим:
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd == -1) {
        FLOG("Failed creating socket");
        assert(false);
    }
    LOG("Socket created:" + std::to_string(socket_fd));

    //bind socket to port and addresses
    sockaddr_in addr;
    addr.sin_family = AF_INET; //using ipv4
    addr.sin_port = htons(port); //select port
    addr.sin_addr.s_addr = htonl(INADDR_ANY); //listen to all ips

    if (bind(socket_fd, (sockaddr *) &addr, sizeof(sockaddr)) != 0) {
        FLOG("Failed binding socket");
        assert (false);
    }
    LOG("Socket binded to port:"+ std::to_string(port));

    //add listening to socket
    if (listen(socket_fd, REQUEST_QUEUE_SIZE)) {
        FLOG("Failed setting listen to socket");
        assert(false);
    }
    LOG("Added listening");
    ELOG;
    return socket_fd;
}

int echo_server::create_epoll(int socket_fd) {

    int epoll_fd = epoll_create(EPOLL_QUEUE_LEN);

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = socket_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev) == -1) {
        FLOG("Failed creating epoll");
        assert(false);
    }
    LOG("Epoll on socket " + std::to_string(socket_fd) + " created:" + std::to_string(epoll_fd));
    ELOG;
    return epoll_fd;
}

void echo_server::accept_new_client(int socket_fd, int epoll_fd) {
    int client_socket_fd = accept(socket_fd, nullptr, nullptr);
    if (client_socket_fd == -1) {
        FLOG("Failed getting new client");
        return;
    }

    epoll_event client_ready;
    client_ready.events = EPOLLIN | EPOLLET;
    client_ready.data.fd = client_socket_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket_fd, &client_ready)) {
        FLOG("Failed adding new client");
        return;
    }

    LOG("Client with " + std::to_string(client_socket_fd) + " socket created");
    ELOG;
}

void echo_server::handle_client(int client_fd) {
    const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];

    ssize_t data_length = read(client_fd, buffer, BUFFER_SIZE);
    if (data_length == -1) {
        FLOG("Failed reading from client");//REINTR!!
        return;
    }
    if (data_length == 0) {
        LOG("Client on socket " + std::to_string(client_fd) + " closed connection");
        return;
    }

    ssize_t sent_length = 0;
    while (sent_length != data_length) {
        ssize_t writen = write(client_fd, buffer, (size_t) (data_length - sent_length));
        if (writen == -1) {
            FLOG("Failed writing to client");
            return;
        }
        sent_length += writen;
    }
    LOG("Data from client on socket " + std::to_string(client_fd) + " was returned");
    ELOG;
}

void echo_server::start_echo_server(uint16_t port) {

    int socket_fd = create_socket(port);
    int epoll_fd = create_epoll(socket_fd);

    epoll_event events[MAX_EVENTS_GRABBED_PER_TICK];
    while (true) {
        int event_amount = epoll_wait(epoll_fd, events, MAX_EVENTS_GRABBED_PER_TICK, -1);
        if (event_amount == -1) {
            FLOG("Failed waiting events");
            assert(false);
        }
        LOG("Got " + std::to_string(event_amount) + " events while waiting:");
        for (int i = 0; i < event_amount; i++) {
            if (events[i].data.fd == socket_fd) {//got new client
                LOG(std::to_string(i+1) + ") Got new client");
                accept_new_client(socket_fd, epoll_fd);
            } else {
                LOG(std::to_string(i+1) + ") Got new info from " + std::to_string(events[i].data.fd));
                handle_client(events[i].data.fd);
            }
        }
    }
}