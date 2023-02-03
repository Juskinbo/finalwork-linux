// 服务器端，用于接收客户端信息
#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#define PORT 12345
#define Max 10 // 最大连接数,也就是聊天室最高在线人数
#define MAXSIZE 4096
#define TRUE 1
#define FALSE 0
int fdt[Max] = {0};
char message[MAXSIZE];
// 发送消息
void SendToClient(int fd, char *buf, int Size)
{
    int i;
    int e;
    for (i = 0; i < Max; i++)
    {
        // 给其他在线用户发送消息
        if ((fdt[i] != 0) && (fdt[i] != fd))
            send(fdt[i], buf, Size, 0);
    }
    bzero(buf, sizeof(buf));
}
// 子线程函数
void *pthread_service(void *sfd)
{
    int fd = *(int *)sfd;
    while (TRUE)
    {
        int i;
        int numbytes;
        numbytes = recv(fd, message, MAXSIZE, 0);
        if (numbytes <= 0)
        {
            for (i = 0; i < Max; i++)
            {
                if (fd == fdt[i])
                {
                    fdt[i] = 0;
                }
            }
            printf("客户端 %d 已退出\n", fd);
            break;
        }
        printf("来自客户端 %d 的信息: \n", fd);
        printf("用户%s\n", message);
        // 开始转发
        SendToClient(fd, message, numbytes);
        bzero(message, MAXSIZE);
    }
    close(fd);
    pthread_exit(0);
}
int main()
{
    int listenfd, connectfd;
    struct sockaddr_in server;
    struct sockaddr_in client;
    int sin_size;
    sin_size = sizeof(struct sockaddr_in);
    int number = 0;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("创建 socket 失败");
        exit(1);
    }
    bzero(&server, sizeof(server)); // 防止内存出错
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listenfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("bind() 错误");
        exit(1);
    }
    if (listen(listenfd, 1) == -1)
    {
        perror("listen() 错误\n");
        exit(1);
    }
    printf("等待连接...\n");
    while (TRUE)
    {
        if ((connectfd = accept(listenfd, (struct sockaddr *)&client, &sin_size)) == -1)
        {
            perror("accept() 错误\n");
            exit(1);
        }
        if (number >= Max)
        {
            printf("同时在线人数已达上限\n");
            close(connectfd);
        }
        int i;
        for (i = 0; i < Max; i++)
        {
            if (fdt[i] == 0)
            {
                fdt[i] = connectfd;
                break;
            }
        }
        pthread_t tid;
        pthread_create(&tid, NULL, (void *)pthread_service, &connectfd);
        pthread_detach(tid);
        number = number + 1;
    }
    close(listenfd);
    return 0;
}
