#pragma once

#include "Acceptor.h"
#include "Connection.h"
#include "thread_pool.h"

class Server {
public:
    Server();
    void start() ;
    Server& onConnect(std::function<void(int)>);
    Server& onReceive(std::function<void(Connection*)>);
private:
    void accept(Socket* socket);
    void deleteConnectionCallback(int fd);
private:
    Acceptor* acceptor;
    thread_pool* pool;

    std::vector<EventLoop*> event_loop_vec;
    EventLoop* main_loop;

    std::unordered_map<int,Connection*> connection_map;

    std::function<void(int)> connect_callback;
    std::function<void(Connection*)> receive_callback;
};