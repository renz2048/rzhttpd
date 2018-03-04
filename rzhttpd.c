#define PORT 56784

void accept_request(int client)
{
  char buf[1024];
  init numchars;
  int i = 0;
  char c = '\0';
  int size = 0;

  size = sizeof(buf);

  //numchars = get_line(client, buf, sizeof(buf));
  while((i < size - 1 ) && (c != '\n'))
  {
    //recv函数读取tcp buffer中的数据到buf中，
    //并从tcp buffer中移除已读取的数据
    n = recv(client, &c, 1, 0);
    printf("%02X\n", c);
    if(n > 0) {
      if( c == '\r' ) {//当读到\r时，检测下一个符号是不是\n
        n = recv(client, &c, 1, MSG_PEEK);
        if((n > 0) && (c == '\n')) {
          recv(client, &c, 1, 0);//如果是\n,读取出\n放入buf中
        }else{
          c = '\n';
        }
      }
      buf[i] = c;
      i++;
    }else{
      c = '\n';
    }
  }
  buf[i] = '\0';

  i = 0; j = 0;
  while(!isspace(buf[i]) && (i < sizeof(method) - 1))
  {
    method[i] = buf[i];
    i++;
  }
  i = 1;
  method[i] = '\0';
}

int init_net(u_short *port)
{
  int httpd = 0;
  struct sockaddr_in name;

  if( (httpd = socket(PF_INET, SOCK_STREAM, 0)) == -1 ) {
    perror("socket");
    exit(1);
  }

  memset(&name, 0, sizeof(name));
  name.sin_family = AF_INET;
  name.sin_port = htons(*port);
  name.sin_addr.s_addr = htonl(INADDR_ANY);

  if( (bind(httpd, (struct sockaddr *)&name, sizeof(name))) == -1 ) {
    perror("bind");
    exit(1);
  }

  if( (listen(httpd, BACKLOG)) == -1 ) {
    perror("listen");
    exit(1);
  }

  return httpd;
}

int main()
{
  int server_sock = -1;
  int client_sock = -1;
  u_short port = PORT;
  struct sockaddr_in client_name;
  socklen_t client_name_len = sizeof(client_name);
  server_sock = init_net(&port);
  printf("httpd running on port %d\n",port);

  while(1)
  {
    client_sock = accept(server_sock, (struct sockaddr *)&client_name, &client_name_len);
    if( client_sock == -1 ) {
      perror("accept");
      exit(1);
    }
    accept_request(client_sock);
  }

  close(server_sock);
  return 0;
}
