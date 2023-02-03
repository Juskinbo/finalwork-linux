// 客户端，用于登陆注册以及聊天功能的实现
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#define PORT 12345
#define MAXSIZE 4096
#define TRUE 1
#define FALSE 0
char sendbuf[MAXSIZE];
char recvbuf[MAXSIZE];
char data[100][100];
char name[100];
char password[100];
int fd;
// 用以接收信息
void *pthread_recv(void *ptr)
{
    while (TRUE)
    {
        if ((recv(fd, recvbuf, MAXSIZE, 0)) == -1)
        {
            printf("recv() error\n");
            exit(1);
        }
        printf("%s", recvbuf);
        memset(recvbuf, 0, sizeof(recvbuf));
    }
}
int main(int argc, char *argv[])
{
    int numbytes;
    char buf[MAXSIZE];
    struct hostent *hostent;
    struct sockaddr_in server;
    if (argc != 2)
    {
        printf("使用方法： %s <您的IP地址>\n", argv[0]);
        exit(1);
    }
    if ((hostent = gethostbyname(argv[1])) == NULL)
    {
        printf("gethostbyname() error\n");
        exit(1);
    }
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("socket() error\n");
        exit(1);
    }
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr = *((struct in_addr *)hostent->h_addr);
    if (connect(fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        printf("连接失败\n");
        exit(1);
    }
    printf("连接成功\n");
    char str[] = "已进入聊天室\n";
    printf("请输入用户名（若用户名不存在将进行自动创建用户）：");
    fscanf(stdin, "%s", name); // 读取一行
    printf("请输入密码：");
    fscanf(stdin, "%s", password); // 读取一行
    char tmp[10];
    fgets(tmp, sizeof(tmp), stdin);
    FILE *fp = fopen("users.txt", "r+");
    int flag1 = 0; // 为1表示登录成功
    int flag2 = 0; // 为1表示密码错误，为0表示注册
    // 奇数行是用户名
    // 偶数行是密码
    int num = 0;
    if (NULL == fp) // 判断一下文件是否成功打开
    {
        printf("open failed.\n");
        return 0;
    }
    while (!feof(fp))
    {
        fscanf(fp, "%s", data[num]);
        // printf("%s\n", data[num]);
        num++;
    }
    for (int i = 0; i < num - 1; i++)
    {
        if (strcmp(name, data[i]) == 0 && strcmp(password, data[i + 1]) == 0)
        {
            flag1 = 1; // 登录成功
        }
        else if (strcmp(name, data[i]) == 0 && strcmp(password, data[i + 1]) != 0)
        {
            flag2 = 1; // 用户名对了但是密码错了
        }
    }
    if (flag1 == 1)
    { // 登录成功
        printf("登陆成功，输入exit可退出聊天室\n");
        send(fd, name, (strlen(name)), 0);
        send(fd, str, (strlen(str)), 0);
        // 创建子线程
        pthread_t tid;
        pthread_create(&tid, NULL, pthread_recv, NULL);
        pthread_detach(tid);
        // 客户端的输入
        while (TRUE)
        {
            memset(sendbuf, 0, sizeof(sendbuf));
            fgets(sendbuf, sizeof(sendbuf), stdin);
            if (strcmp(sendbuf, "exit\n") == 0)
            {
                memset(sendbuf, 0, sizeof(sendbuf));
                printf("您已退出聊天室\n");
                send(fd, sendbuf, (strlen(sendbuf)), 0);
                break;
            }
            send(fd, name, (strlen(name)), 0);
            send(fd, ":", 1, 0);
            send(fd, sendbuf, (strlen(sendbuf)), 0);
        }
        close(fd);
    }
    else
    {
        if (flag2 == 1)
        {
            printf("密码错误\n");
            fclose(fp);
        }
        else
        {
            printf("注册成功，已帮您自动登录，输入exit可退出聊天室\n");
            // 将用户信息写入users.txt文件中
            fprintf(fp, "\n%s", name);
            fprintf(fp, "\n%s", password);
            fclose(fp);
            send(fd, name, (strlen(name)), 0);
            send(fd, str, (strlen(str)), 0);
            // 创建子线程
            pthread_t tid;
            pthread_create(&tid, NULL, pthread_recv, NULL);
            pthread_detach(tid);
            // 客户端的输入
            while (TRUE)
            {
                memset(sendbuf, 0, sizeof(sendbuf));
                fgets(sendbuf, sizeof(sendbuf), stdin);
                if (strcmp(sendbuf, "exit\n") == 0)
                {
                    memset(sendbuf, 0, sizeof(sendbuf));
                    printf("您已成功退出聊天室\n");
                    send(fd, sendbuf, (strlen(sendbuf)), 0);
                    break;
                }
                send(fd, name, (strlen(name)), 0);
                send(fd, ": ", 2, 0);
                send(fd, sendbuf, (strlen(sendbuf)), 0);
            }
            close(fd);
        }
    }
    return 0;
}
