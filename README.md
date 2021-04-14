# rzhttpd

# 1. 简介

一个简单的httpd服务器

v1.0.0:

- 读取GET请求
- 返回index.html固定页面

# 2. 实现

## 2.1 init_net

创建套接字：

```
httpd = socket(PF_INET, SOCK_STREAM, 0)
```

初始化地址设置：

```
memset(&name, 0, sizeof(name));
name.sin_family = AF_INET;
name.sin_port = htons(*port);
name.sin_addr.s_addr = htonl(INADDR_ANY);
```

地址与套接字绑定：

```
bind(httpd, (struct sockaddr *)&name, sizeof(name))
```

接受连接请求：

```
listen(httpd, BACKLOG)
```

## 2.2 while循环

建立连接请求：

```
client_sock = accept(server_sock, (struct sockaddr *)&client_name, &client_name_len);
```

读取处理请求：

```
accept_request(client_sock);
```

### 2.2.1 解析请求(accept_request)

读取请求：

```
numchars = get_line(client, buf, sizeof(buf));
```

请求：`GET / HTTP/1.1`

获取method

判断method值为GET还是POST（未实现）

读取path

读取path中的询问字段

读取html页面：

```
serve_file(client, path);
```

### 2.2.2 读取请求(get_line)

### 2.2.3 读取页面(serve_file)

# 3. 网络编程

##### 3.1 getaddrinfo

```c
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int getaddrinfo(const char *node, const char *service,
        const struct addrinfo *hints,
        struct addrinfo **res);

void freeaddrinfo(struct addrinfo *res);

const char *gai_strerror(int errcode);


struct addrinfo {
    int              ai_flags;
    int              ai_family;
    int              ai_socktype;
    int              ai_protocol;
    socklen_t        ai_addrlen;
    struct sockaddr *ai_addr;
    char            *ai_canonname;
    struct addrinfo *ai_next;
};
```
- ai_family：为返回的地址指定地址族（AF_INET、AF_INET6、AF_UNSPEC）
- ai_socktype：指定首选的套接字类型（SOCK_STREAM、SOCK_DGRAM）
- ai_protocol：指定返回的套接字地址的协议
- ai_flags
    - 若设置为AI_PASSIVE且 getaddrinfo的node为NULL，则返回的参数可以用于bind和accept（服务端）；
    - 若未设置该参数，则返回的参数可用于connect、sendto、sendmsg（客户端）。

例如：

获得域名对应的IP地址：

```c
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;     //IPv4 或 IPv6
	hints.ai_socktype = SOCK_STREAM; //TCP
	hints.ai_flags = 0;
	hints.ai_protocol = 0;
```

