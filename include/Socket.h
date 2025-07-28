#pragma once

#include <sys/socket.h>
#include <netinet/in.h>

class InetAddress {
public:
    InetAddress(const char* ip, int port) ;

    struct sockaddr_in getAddr();
private:
    struct sockaddr_in addr;
};

class Socket {
public:
    Socket() ;
    Socket(int fd);
    ~Socket();
    void bind(struct sockaddr_in serv_addr) ;
    void listen();
    int accept();
    void setNoBlocking();
    int getFd();
private:
    int sockfd;
};