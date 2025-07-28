#pragma once

#include <sys/epoll.h>

#include "Channel.h"

class Epoll {
public:
    Epoll();
    void updateChannel(Channel* channel);
    std::vector<Channel*> loop();
private:
    int epoll_fd;
    struct epoll_event events[1024];
};