#include <Acceptor.h>

Acceptor::Acceptor(EventLoop* loop) : loop(loop){

    const char* ip = "127.0.0.1";
    int port = 8888;

    socket = new Socket();
    InetAddress serv_addr(ip, port);
    socket->bind(serv_addr.getAddr());
    socket->listen();
}

void Acceptor::setAcceptCallback(std::function<void(Socket*)> cb) {
    callback = std::bind(cb, socket);
    Channel* channel = new Channel(socket->getFd());
    channel->setCallback(callback);
    loop->updateChannel(channel);
}