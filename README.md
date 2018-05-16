# rzhttpd

# 1. 简介

a simple httpd server by C

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

