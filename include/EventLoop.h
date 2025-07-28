#pragma once

#include "Channel.h"
#include "Epoll.h"

class EventLoop {
public:
    EventLoop();
    ~EventLoop();
    void loop();
    void updateChannel(Channel* channel);
private:
    Epoll* epoll;
};