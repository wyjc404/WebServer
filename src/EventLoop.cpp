#include <EventLoop.h>

EventLoop::EventLoop() : epoll(new Epoll()) {
}

EventLoop::~EventLoop() {
    delete epoll;
}

void EventLoop::loop() {
    while(true) {
        auto channels = epoll->loop();
        for(auto channel : channels) {
            channel->handle();
        }
    }
}

void EventLoop::updateChannel(Channel* channel) {
    epoll->updateChannel(channel);
}