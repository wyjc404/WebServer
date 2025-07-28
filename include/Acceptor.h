#pragma once

#include "EventLoop.h"
#include "Socket.h"

class Acceptor {
public:
    Acceptor(EventLoop* loop);

    void setAcceptCallback(std::function<void(Socket*)> cb);

private:
    Socket* socket;
    EventLoop* loop;

    std::function<void()> callback;
};