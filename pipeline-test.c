#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_PORT 8080
#define SERVER_IP "127.0.0.1" // Example.com IP

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[1024];
    int bytes_received;

    // 创建socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Cannot create socket");
        return 1;
    }

    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // 连接到服务器
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed");
        close(sockfd);
        return 1;
    }

    // 准备HTTP请求
    char *http_requests = "GET /test/test.html HTTP/1.1\r\nHost: example.com\r\n\r\n"
                          "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n"
                          "GET /test/test.html HTTP/1.1\r\nHost: example.com\r\n\r\n";

    // 发送HTTP请求
    if (send(sockfd, http_requests, strlen(http_requests), 0) < 0) {
        perror("Send failed");
        close(sockfd);
        return 1;
    }

    // // 发送HTTP请求
    // if (send(sockfd, http_request2, strlen(http_request2), 0) < 0) {
    //     perror("Send failed");
    //     close(sockfd);
    //     return 1;
    // }

    // 接收响应
    while ((bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        printf("bytes_received: %d\n", bytes_received);
        buffer[bytes_received] = '\0'; // 确保字符串结束
        printf("%s", buffer);
    }

    if (bytes_received < 0) {
        perror("Recv failed");
        close(sockfd);
        return 1;
    }

    // 关闭socket
    close(sockfd);
    return 0;
}
