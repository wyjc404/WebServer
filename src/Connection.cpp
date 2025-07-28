#include <Connection.h>

#include <cstddef>
#include <sys/types.h>
#include <unistd.h> 
#include <errno.h> 
#include <cstdio>     
#include <cstring>   

#include <string>
#include <sstream>

std::string createMessage(const char* message) {
    std::ostringstream os;
    os << strlen(message) << '&' << message;
    return os.str();
}

std::tuple<size_t, std::string> parseMessage(char* message) {
    if (message == nullptr) {
        return {static_cast<size_t>(-1), nullptr};
    }

    char* delimiter = strchr(message, '&');
    if (delimiter == nullptr) {
        return {static_cast<size_t>(-1), nullptr};
    }
    
    *delimiter = '\0';
    
    size_t length = static_cast<size_t>(atoi(message));
    
    char* content = delimiter + 1;
    
    return {length, std::string(content)};
}

Connection::Connection(int sockfd, EventLoop* loop_) : loop(loop_) {
    socket = new Socket(sockfd);
    channel = new Channel(sockfd);
    buffer = new Buffer(1024);

    write_function[0] = [this](const char* message) { no_block_write(message); };
    write_function[1] = [this](const char* message) { block_write(message); };
    
    read_function[0] = [this] { no_block_read(); };
    read_function[1] = [this] { block_read(); };

    state = CONNECT;
}

Connection::~Connection() {
    printf("EOF, client fd %d disconnected\n", socket->getFd());

    delete socket;
    delete channel;
    delete buffer;
}

void Connection::setReceiveCallback(std::function<void(Connection*)> cb) {
    std::function<void()> callback = std::bind(cb, this);
    channel->setCallback(callback);
    loop->updateChannel(channel);
}

void Connection::Write(const char* message) {
    write_function[isBlocking](message);
}

void Connection::Read() {
    read_function[isBlocking]();   
}

Buffer* Connection::getBuffer() { 
    return buffer; 
}

Socket* Connection::getConnectSocket() { 
    return socket;
}

void Connection::setDeleteCallback(std::function<void()> cb) { 
    deleteConnectionCallback = cb; 
}

void Connection::setNoBlocking() { 
    socket->setNoBlocking(); 
    isBlocking = false; 
}

void Connection::no_block_read() {

    int fd = socket->getFd();

    auto check = [&](ssize_t bytes) {
        bool is_disconnect = false;
        if(bytes == -1) {
            if(bytes == 0) {
            // EOF，客户端正常断开连接
            printf("EOF, client disconnected\n");
            state = DISCONNECT;
            is_disconnect = true;
            }else if(errno == EINTR) {
                // 被信号中断，应该重试
                printf("continue reading\n");
            } else if(errno == EAGAIN || errno == EWOULDBLOCK) {
                // 非阻塞IO，暂时无数据可读，正常情况
                printf("finish reading once, errno: %d\n", errno);
            } else if(errno == ECONNRESET) {
                // 连接被对端重置
                printf("Connection reset by peer\n");
                state = DISCONNECT;
                is_disconnect = true;
            } else if(errno == EPIPE) {
                // 管道破裂，对端已关闭
                printf("Broken pipe\n");
                state = DISCONNECT;
                is_disconnect = true;
            } else if(errno == EBADF) {
                // 无效的文件描述符
                printf("Bad file descriptor\n");
                state = DISCONNECT;
                is_disconnect = true;
            } else {
                // 其他未知错误
                printf("Read error: %s\n", strerror(errno));
                state = DISCONNECT;
                is_disconnect = true;
            }
        }
        return is_disconnect;
    };

    buffer->clear();
    ssize_t bytes_read = read(fd, buffer->getBuffer(), buffer->getSize());
    if(check(bytes_read)) return;

    auto[to_read, receive_message] = parseMessage(buffer->getBuffer());
    to_read -= receive_message.size();

    while(to_read > 0) {
        buffer->clear();
        bytes_read = read(fd, buffer->getBuffer(), to_read);
        if(check(bytes_read)) return ;
        
        receive_message += buffer->getBuffer();
        to_read -= bytes_read;
    }

    strcpy(buffer->getBuffer(), receive_message.c_str());
}

void Connection::no_block_write(const char* message) {
    int fd = socket->getFd();
    std::string send_message = createMessage(message);
    size_t total_size = send_message.size();
    size_t sent_bytes = 0;  // 偏移量，记录已发送的字节数

    auto check = [&](ssize_t write_bytes) {
        bool disconnect = false;
        if(write_bytes < 0) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) ; 
            else disconnect = true;
            state = DISCONNECT;
        }
        return disconnect;
    };

    while(sent_bytes < total_size) {
        ssize_t write_bytes = write(fd, send_message.c_str() + sent_bytes, 
                                   total_size - sent_bytes);
        if(check(write_bytes)) return;
        if(write_bytes > 0) 
            sent_bytes += write_bytes; 
    }
}

void Connection::block_read() {
    int fd = socket->getFd();
    ssize_t read_bytes = read(fd, buffer->getBuffer(), buffer->getSize());
    
    if(read_bytes == -1) {
        printf("finish reading once, errno: %d\n", errno);
        state = DISCONNECT;
    }
    else if(read_bytes == 0) {
        printf("EOF, client fd %d disconnected\n", fd);
        state = DISCONNECT;
    }
}

void Connection::block_write(const char* message) {
    int fd = socket->getFd();
    ssize_t write_bytes = write(fd, message, strlen(message));
    if(write_bytes == -1) {
        state = DISCONNECT;
    }
}

void Connection::disconnect() {
    deleteConnectionCallback();
}