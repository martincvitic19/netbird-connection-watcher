#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define TCP_PORT 8080

int main(void)
{
    int server_fd, client_fd;
    struct sockaddr_in addr;
    char buf[512];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) 
    {
        perror("socket");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(TCP_PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        return 1;
    }

    listen(server_fd, 1);

    printf("Listening on http://127.0.0.1:%d\n", TCP_PORT);

    while (1) 
    {
        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0)
            continue;

        read(client_fd, buf, sizeof(buf));

        dprintf(client_fd,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Connection: close\r\n"
                "\r\n");

        FILE *fp = popen("netbird status -d", "r");
        if (fp) 
        {
            while (fgets(buf, sizeof(buf), fp))
            {
                write(client_fd, buf, strlen(buf));
            }
            pclose(fp);
        }

        close(client_fd);
    }

    return 0;
}