#include <Server.h>

int main() {
    Server server;
    server
    .onConnect([](int fd) {
        printf("New connect: %d\n", fd);
    })
    .onReceive([](Connection* connection) {
        connection->setNoBlocking();
        connection->Read();
        
        if(connection->state == Connection::DISCONNECT) 
            return connection->disconnect();
        
        auto buffer = connection->getBuffer();
        printf("Message from %d: %s\n", connection->getConnectSocket()->getFd(), buffer->getBuffer());
        connection->Write(buffer->getBuffer());
    })
    .start();
}