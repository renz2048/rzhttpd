#include <stdio.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>

#define PORT 56784
#define BACKLOG 5

#define SERVER_STRING "Server: rzhttpd/0.1.0\r\n"

void accept_request(int);
void cat(int , FILE *);
int get_line(int, char *, int);
void headers(int , const char *);
int init_net(u_short *);
void not_found(int);
void serve_file(int , const char *);
void unimplemented(int);

void accept_request(int client)
{
  char buf[1024];
  int numchars;
  char method[255];
  char url[255];
  char path[512];
  size_t i,j;
  struct stat st;

  numchars = get_line(client, buf, sizeof(buf));
  i = 0; j = 0;
  puts(buf);
  while(!isspace(buf[j]) && (i < sizeof(method) - 1))
  {
    method[i] = buf[i];
    i++; j++;
  }
  method[i] = '\0';

  puts(method);
  //如果不是GET和POST方法，调用unimplemented接口，表明方法不被支持
  if(strcasecmp(method, "GET") && strcasecmp(method, "POST")) {
    unimplemented(client);
    return;
  }

  i = 0;
  while(isspace(buf[j]) && (j < sizeof(buf)))
    j++;

  while(!isspace(buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf))) {
    url[i] = buf[j];
    i++; j++;
  }
  url[i] = '\0';
  puts(url);

  sprintf(path, "htdocs%s", url);
  if(path[strlen(path) - 1] == '/') {
    strcat(path, "index.html");
  }
  if(stat(path, &st) == -1) {
    while((numchars > 0) && strcmp("\n", buf))
      numchars = get_line(client, buf, sizeof(buf));
    not_found(client);
  }else{
    if( (st.st_mode & S_IFMT) == S_IFDIR) {
      strcat(path, "/index.html");
    }
  }

  serve_file(client, path);

  close(client);
}

void cat(int client, FILE *resource)
{
  char buf[1024];
  fgets(buf, sizeof(buf), resource);
  while(!feof(resource)) {
    send(client, buf, strlen(buf), 0);
    fgets(buf, sizeof(buf), resource);
  }
}

int get_line(int client, char *buf, int size)
{
  int i = 0;
  char c = '\0';
  int n;

  while((i < size - 1 ) && (c != '\n'))
  {
    //recv函数读取tcp buffer中的数据到buf中，
    //并从tcp buffer中移除已读取的数据
    n = recv(client, &c, 1, 0);
    //printf("%02X ", c);
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

  return i;
}

void headers(int client, const char *filename)
{
  char buf[1024];
  (void)filename;

  strcpy(buf, "HTTP/1.0 200 OK\r\n");
  send(client, buf, strlen(buf), 0);
  strcpy(buf, SERVER_STRING);
  send(client, buf, strlen(buf), 0);
  strcpy(buf, "Content-Type: text/html\r\n");
  send(client, buf, strlen(buf), 0);
  strcpy(buf, "\r\n");
  send(client, buf, strlen(buf), 0);
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

void not_found(int client)
{
  char buf[1024];

  sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, SERVER_STRING);
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "Content-Type: text/html\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "<BODY><P>The server cound not fulfill\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "your request because the resource specified\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "is unavailable or nonexistent.\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "</BODY></HTML>\r\n");
  send(client, buf, strlen(buf), 0);
}

void serve_file(int client, const char *filename)
{
  FILE *resource = NULL;
  int numchars = 1;
  char buf[1024];

  buf[0] = 'A'; buf[1] = '\0';
  while((numchars > 0) && strcmp("\n", buf))
    numchars = get_line(client, buf, sizeof(buf));

  resource = fopen(filename, "r");
  if(resource == NULL) {
    not_found(client);
  }else{
    headers(client, filename);
    printf("hhhhhhhhhhh");
    cat(client, resource);
  }
  fclose(resource);
}

void unimplemented(int client)
{
  char buf[1024];

  sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, SERVER_STRING);
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "Content-Type: text/html\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "<HTML><HEAD><TITLE><Method Not Implemented\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "</TITLE></HEAD>\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
  send(client, buf, strlen(buf), 0);
  sprintf(buf, "</BODY></HTML>\r\n");
  send(client, buf, strlen(buf), 0);
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
