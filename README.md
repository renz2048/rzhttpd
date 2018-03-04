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
