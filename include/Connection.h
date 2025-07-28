#pragma once

#include "EventLoop.h"
#include "Buffer.h"
#include "Socket.h"


class Connection {
public:
    enum State {
        WAIT,
        CONNECT,
        DISCONNECT
    };
public:
    Connection(int sockfd, EventLoop* loop_);
    ~Connection();

    void setReceiveCallback(std::function<void(Connection*)> cb);
    void Write(const char* message);
    void Read() ;

    void disconnect();

    Buffer* getBuffer();
    Socket* getConnectSocket();

    void setDeleteCallback(std::function<void()> cb);
    void setNoBlocking();
public:
    State state = WAIT;
private:
    Socket* socket;
    Channel* channel;
    EventLoop* loop;

    Buffer* buffer;

    bool isBlocking = true;
    
    std::function<void()> deleteConnectionCallback;
    std::function<void(const char*)> write_function[2];
    std::function<void()> read_function[2];
private:
    void no_block_read();
    void no_block_write(const char* message) ;
    void block_read();
    void block_write(const char* message);
};