#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>

#include <errno.h>
#include <assert.h>
#include <iostream>
#include <string.h>

#include "echo_server.h"

#define LOG(a) std::cout<<(a)<<'\n'

int echo_server::create_socket(uint16_t port){
    //http://www.codenet.ru/progr/cpp/Sockets.php
    //Для создания сокета типа stream с протоколом TCP, обеспечивающим коммуникационную поддержку, вызов функции socket должен быть следующим:
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd == -1){
        LOG("Failed creating socket, error:");
        LOG(std::string(strerror(errno)));
        assert(false);
    }
    LOG("Socket created:");
    LOG(socket_fd);

    //bind socket to port and addresses
    sockaddr_in addr;
    addr.sin_family = AF_INET; //using ipv4
    addr.sin_port = htons(port); //select port
    addr.sin_addr.s_addr = htonl(INADDR_ANY); //listen to all ips

    if (bind(socket_fd, (sockaddr *)&addr, sizeof(sockaddr)) != 0)
    {
        LOG("Failed binding socket, error:");
        LOG(std::string(strerror(errno)));
        assert (false);
    }
    LOG("Socket binded to:");
    LOG(port);

    //add listening to socket
    if (listen(socket_fd,REQUEST_QUEUE_SIZE)){
        LOG("Failed setting listen to socket, error:");
        LOG(std::string(strerror(errno)));
        assert(false);
    }
    LOG("Added listening");
    return socket_fd;
}

int echo_server::create_epoll(int socket_fd) {
    //https://kovyrin.net/2006/04/13/epoll-asynchronous-network-programming/

    int epoll_fd = epoll_create(EPOLL_QUEUE_LEN);

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = socket_fd;
    if (epoll_ctl(epoll_fd,EPOLL_CTL_ADD,socket_fd,&ev)==-1){
        LOG("Failed creating epoll, error:");
        LOG(std::string(strerror(errno)));
        assert(false);
    }
    LOG("Epoll on "+ std::to_string(socket_fd) + "socket created:");
    LOG(epoll_fd);
    return epoll_fd;
}

void echo_server::start_echo_server(uint16_t port) {

    int socket_fd = create_socket(port);
    int epoll_fd = create_epoll(socket_fd);

    epoll_event events[MAX_EVENTS_GRABBED_PER_TICK];
    while(true){
        int event_amount = epoll_wait(epoll_fd, events,MAX_EVENTS_GRABBED_PER_TICK,-1);
        if (event_amount == -1) {
            LOG("Failed waiting events, error:");
            LOG(std::string(strerror(errno)));
            assert(false);
        }
        LOG("Got " + std::to_string(event_amount) + " while waiting:");
        for (int i=0; i<event_amount;i++){
            if (events[i].data.fd == socket_fd){//got new client
                LOG("Got new client");
                accept_new_client(socket_fd,epoll_fd);
            }else{
                LOG("Got new info from " + std::to_string(events[i].data.fd));
                handle_client(events[i].data.fd);
            }
        }
    }


}