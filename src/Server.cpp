#include "Server.h"

#include <cstdio>
#include <cstring>

Server::Server() {
    int size = std::thread::hardware_concurrency();
    main_loop = new EventLoop();
    for(int i = 0; i < size; i++) {
        event_loop_vec.push_back(new EventLoop());
    }
    pool = new thread_pool(size);
    for(auto& loop : event_loop_vec) {
        std::function<void()> task = std::bind(&EventLoop::loop, loop);
        pool->submit(task);
    }

    acceptor = new Acceptor(main_loop);
    std::function<void(Socket*)> callback = std::bind(&Server::accept, this, std::placeholders::_1);
    acceptor->setAcceptCallback(callback);
}

void Server::start() {
    main_loop->loop();
}

Server& Server::onConnect(std::function<void(int)> cb) {
    connect_callback = cb;
    return *this;
}
Server& Server::onReceive(std::function<void(Connection*)> cb) {
    receive_callback = cb;
    return *this;
}

void Server::accept(Socket* socket) {
    int clnt_sockfd = socket->accept();
    connect_callback(clnt_sockfd);
    int random = clnt_sockfd % event_loop_vec.size();
    
    Connection* new_connection = new Connection(clnt_sockfd, event_loop_vec[random]);
    std::function<void()> delete_callback = std::bind(&Server::deleteConnectionCallback, this, clnt_sockfd);
    
    new_connection->setReceiveCallback(receive_callback);
    new_connection->setDeleteCallback(delete_callback);
    connection_map[clnt_sockfd] = new_connection;
}

void Server::deleteConnectionCallback(int fd) {
    auto& connection = connection_map[fd];
    connection_map.erase(fd);
    delete connection;
}