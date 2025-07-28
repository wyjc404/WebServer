#include <Channel.h>

Channel::Channel(int fd_) : fd(fd_) {} 
void Channel::setCallback(std::function<void()> callback_) {
    this->callback = callback_;
}
int Channel::getFd() { return fd; }

void Channel::handle() {
    callback();
}