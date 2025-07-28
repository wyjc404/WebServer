#pragma once

#include <functional>

class Channel {
public:
    Channel(int fd_) ;
    void setCallback(std::function<void()> callback_);
    int getFd();

    void handle();

private:
    int fd;
    std::function<void()> callback;
};