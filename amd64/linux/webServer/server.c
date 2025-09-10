#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/sendfile.h>

#define PORT 8080
#define BACKLOG 10
#define BUFFER_SIZE 1024
#define FILE_BUFFER_SIZE 4096

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        fprintf(stderr, "Socket creation failed: %s\n", strerror(errno));
        return 1;
    }
    struct sockaddr_in addr = {AF_INET, htons(PORT), 0};
    int retval = bind(sockfd, &addr, sizeof(addr));
    if (retval == -1)
    {
        fprintf(stderr, "Bind failed: %s\n", strerror(errno));
        close(sockfd);
        return 1;
    }
    retval = listen(sockfd, BACKLOG);
    if (retval == -1)
    {
        fprintf(stderr, "Listen failed: %s\n", strerror(errno));
        close(sockfd);
        return 1;
    }
    printf("Server listening on port %d\n", PORT);
    for (;;)
    {
        int clientfd = accept(sockfd, NULL, NULL);
        if (clientfd == -1)
        {
            fprintf(stderr, "Accept failed: %s\n", strerror(errno));
            close(sockfd);
            return 1;
        }
        printf("Accepted a connection\n");
        char buffer[BUFFER_SIZE] = {0};
        int bytes_received = recv(clientfd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received == -1)
        {
            fprintf(stderr, "Receive failed: %s\n", strerror(errno));
            close(clientfd);
            close(sockfd);
            return 1;
        }
        char *request = buffer + 5;
        if (strncmp(buffer, "GET /", 5) != 0)
        {
            fprintf(stderr, "Unsupported request: %s\n", buffer);
            close(clientfd);
            continue;
        }
        char *end = strchr(request, ' ');
        if (end == NULL)
        {
            fprintf(stderr, "Received malformed request: %s\n", buffer);
            close(clientfd);
            continue;
        }
        *end = '\0';
        printf("Requested resource: %s\n", request);
        int filefd = open(request, O_RDONLY);
        if (filefd == -1)
        {
            const char *not_found_response = "HTTP/1.1 404 Not Found\r\n\r\n";
            retval = send(clientfd, not_found_response, strlen(not_found_response), 0);
            if (retval == -1)
            {
                fprintf(stderr, "Send failed: %s\n", strerror(errno));
                close(clientfd);
                close(sockfd);
                return 1;
            }
            fprintf(stderr, "File not found: %s\n", request);
            close(clientfd);
            continue;
        }
        const char *ok_response = "HTTP/1.1 200 OK\r\n\r\n";
        retval = send(clientfd, ok_response, strlen(ok_response), 0);
        if (retval == -1)
        {
            fprintf(stderr, "Send failed: %s\n", strerror(errno));
            close(filefd);
            close(clientfd);
            close(sockfd);
            return 1;
        }
        retval = sendfile(clientfd, filefd, NULL, FILE_BUFFER_SIZE);
        if (retval == -1)
        {
            fprintf(stderr, "Sendfile failed: %s\n", strerror(errno));
            close(filefd);
            close(clientfd);
            close(sockfd);
            return 1;
        }
        close(filefd);
        close(clientfd);
        printf("Served resource: %s\n", request);
    }
    close(sockfd);
    return 0;
}