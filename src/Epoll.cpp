#include "Epoll.h"
#include <unistd.h>

#include <cstdio>

Epoll::Epoll() {
    epoll_fd = epoll_create(1);
}

void Epoll::updateChannel(Channel* channel) {
    struct epoll_event ev;
    ev.data.ptr = channel;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, channel->getFd(), &ev);
}

std::vector<Channel*> Epoll::loop() {
    int nfds = epoll_wait(epoll_fd, events, 1024, -1);
    if (nfds < 0) {
        perror("epoll_wait error");
        return {};
    }
    
    std::vector<Channel*> result(nfds);
    for(int i = 0; i < nfds; i++) {
        result[i] = (Channel*)events[i].data.ptr;
    }
    return result;
}