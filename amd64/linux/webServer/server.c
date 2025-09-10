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

#define check_error(retval, msg)                                 \
    do                                                           \
    {                                                            \
        if ((retval) == -1)                                      \
        {                                                        \
            fprintf(stderr, "%s: %s\n", (msg), strerror(errno)); \
            return 1;                                            \
        }                                                        \
    } while (0)

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {AF_INET, htons(PORT), 0};
    int retval = bind(sockfd, &addr, sizeof(addr));
    check_error(retval, "Bind failed");
    retval = listen(sockfd, BACKLOG);
    check_error(retval, "Listen failed");
    printf("Server listening on port %d\n", PORT);
    for (;;)
    {
        int clientfd = accept(sockfd, NULL, NULL);
        check_error(clientfd, "Accept failed");
        printf("Accepted a connection\n");
        char buffer[BUFFER_SIZE] = {0};
        int bytes_received = recv(clientfd, buffer, BUFFER_SIZE - 1, 0);
        check_error(bytes_received, "Receive failed");
        char *request = buffer + 5; // Skip "GET /"
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
            send(clientfd, not_found_response, strlen(not_found_response), 0);
            fprintf(stderr, "File not found: %s\n", request);
            close(clientfd);
            continue;
        }
        const char *ok_response = "HTTP/1.1 200 OK\r\n\r\n";
        send(clientfd, ok_response, strlen(ok_response), 0);
        sendfile(clientfd, filefd, NULL, FILE_BUFFER_SIZE);
        close(filefd);
        close(clientfd);
        printf("Served resource: %s\n", request);
    }
    return 0;
}