# rzhttpd
a simple httpd server by C
# 1.init_net()初始化网络
```
int init_net(u_short *port)
{
  int httpd = 0;
  struct sockaddr_in name;

  if( (httpd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  memset(&name, 0, sizeof(name));
  name.sin_family = AF_INET;
  name.sin_port = htons(*port);
  name.sin_addr.s_addr = htonl(INADDR_ANY);

  if( (bind(httpd, (struct sockaddr *)&name, sizeof(name)) ) == -1) {
    perror("bind");
    exit(1);
  }

  if( (listen(httpd, BACKLOG)) == -1) {
    perror("listen");
    exit(1);
  }

  return httpd;
}
```

# 2. accept
与客户端建立连接
```
while(1)
{
  client_sock = accept(server_sock, (struct sockaddr *)&client_name, &client_name_len);
  if(client_sock == -1) {
    perror("accept");
    exit(1);
  }

  //接受请求accept_requset
}
```
# 3. 接受请求accept_requset
```
accept_request()
{
  
}
