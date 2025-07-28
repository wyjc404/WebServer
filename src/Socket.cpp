#include "Socket.h"
#include "util.h"

#include <unistd.h> 
#include <cstdio>   
#include <cstring>     

#include <arpa/inet.h>   
#include <strings.h>
#include <fcntl.h> 

InetAddress::InetAddress(const char* ip, int port) {
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
}

struct sockaddr_in InetAddress::getAddr() { 
    return addr; 
}

Socket::Socket() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket create error");
}

Socket::Socket(int fd) {
    sockfd = fd;
}

Socket::~Socket() { 
    close(sockfd);
}

void Socket::bind(struct sockaddr_in serv_addr) {
    errif(::bind(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1, "socket bind error");
}

void Socket::listen() {
    errif(::listen(sockfd, SOMAXCONN) == -1, "socket listen error");
}

int Socket::accept() { 
    return ::accept(sockfd, NULL, NULL); 
}

void Socket::setNoBlocking() {  
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) | O_NONBLOCK); 
}

int Socket::getFd() { 
    return sockfd; 
}