/*************************************************************************
	> File Name: rzhttpd.c
	> Author: renzheng 
	> Mail: renz@koal.com
	> Created Time: Sat Feb  3 19:35:08 2018
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define BACKLOG 5                 //how many pending connections queue will hold

int startup(u_short *port);
void accept_request(void *);

int main()
{
    int server_sock = -1;
    u_short port = 4000;
    int client_sock = -1;
    struct sockaddr_in client_name;
    socklen_t client_name_len = sizeof(client_name);

    server_sock = startup(&port);//传入port的地址，为了便于当port不可用时修改port
    printf("httpd running on port %d\n", port);

    while(1)
    {
        client_sock = accept(server_sock, (struct sockaddr *)&client_name, &client_name_len);
        if( client_sock == -1 ) {
            perror("accept");
        }
        accept_request(&client_sock);
    }

    close(server_sock);
    return 0;
}

/*
 * 初始化http服务：建立套接字
 *                 绑定端口
 *                 进行监听
 */
int startup(u_short *port)//port的格式
{
    int httpd = 0;
    int on = 1;
    struct sockaddr_in name;
    /*
     * #include <sys/types.h>
     * #include <sys/socket.h>
     * int socket(int domain, int type, int protocol);
     * domain:
     *     Name                 Purpose                          Man page
     *     PF_UNIX, PF_LOCAL    Local communication              unix(7)
     *     PF_INET              IPv4 Internet protocols          ip(7)
     *     PF_INET6             IPv6 Internet protocols
     *     PF_IPX               IPX - Novell protocols
     *     PF_NETLINK           Kernel user interface device      netlink(7)
     *     PF_X25               ITU-T X.25 / ISO-8208 protocol    x25(7)
     *     PF_AX25              Amateur radio AX.25 protocol
     *     PF_ATMPVC            Access to raw ATM PVCs
     *     PF_APPLETALK         Appletalk                         ddp(7)
     *     PF_PACKET            Low level packet interface        packet(7)
     *
     * type:
     *     SOCK_STREAM:Provides sequenced, reliable, two-way, connection-based byte streams.An out-of-band data transmission mechanism may be supported.
     */
    if( (httpd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    //设置套接字选项
    if( ( setsockopt(httpd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) ) == -1 ) {
        perror("setsockopt failed");
        exit(1);
    }
    
    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_port = htons(*port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    /*
     * int bind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen);
     * struct sockaddr_in {
     *     short             sin_family;    //e.g. AF_INET
     *     unsigned short    sin_port;      //e.g. htons(3490)
     *     struct in_addr    sin_addr;      //see struct in_addr, below
     *     char              sin_zero[8];   //zero this if you want to
     * };
     *
     * struct in_addr {
     *     unsigned long s_addr;    //load with inet_aton()
     * };
     */
    if( (bind(httpd, (struct sockaddr *)&name, sizeof(name)) ) == -1 ) {
        perror("bind");
        exit(1);
    }

    /*
     * listen
     */
     if( ( listen(httpd, BACKLOG) ) == -1 ) {
         perror("listen");
         exit(1);
     }

    return(httpd);
}

void accept_request(void *arg)
{
    int client = (intptr_t)arg;
    char buf[1024];
    size_t numchars;

    numchars = get_line(client, buf, sizeof(buf));
}
