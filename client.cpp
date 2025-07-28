#include <cstddef>
#include <cstdio>
#include <ostream>
#include <strings.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <cstring>
#include <unistd.h>

#define BUFFER_SIZE 1024

#include <string>
#include <sstream>

void errif(bool condition, const char *errmsg){
    if(condition){
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
}

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

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket create error");

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(8888);

    errif(connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1, "socket connect error");
    
    printf("Success to connect server\n");
    while(true){
        char buf[BUFFER_SIZE];
        bzero(&buf, sizeof(buf));
        fgets(buf,sizeof(buf), stdin);
        
        if(buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] ='\0';

        auto send_message = createMessage(buf);

        size_t to_write = send_message.size();
        
        while(to_write > 0 ) {
            ssize_t write_bytes = write(sockfd, send_message.c_str(),send_message.size());
            to_write -= send_message.size();
            printf("%s\n", send_message.c_str());
            if(to_write) send_message.erase(0,write_bytes);
        }

        bzero(&buf, sizeof(buf));
        ssize_t read_bytes = read(sockfd, buf, sizeof(buf));

        auto[to_read, receive_message] = parseMessage(buf);
        to_read -= receive_message.size();

        while(to_read > 0) {
            bzero(buf, sizeof(buf));
            read_bytes = read(sockfd, buf, sizeof(buf));
            receive_message += buf;
            to_read -= read_bytes;
        }

        if(read_bytes > 0){
            printf("message from server: %s\n", receive_message.c_str());
        }else if(read_bytes == 0){
            printf("server socket disconnected!\n");
            break;
        }else if(read_bytes == -1){
            close(sockfd);
            errif(true, "socket read error");
        }
    }
    close(sockfd);
    return 0;
}
