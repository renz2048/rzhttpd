/*************************************************************************
	> File Name: rzhttpd.c
	> Author: renzheng 
	> Mail: renz@koal.com
	> Created Time: Sat Feb  3 19:35:08 2018
 ************************************************************************/

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

#define ISspace(x) isspace((int)(x))

#define BACKLOG 5                 //how many pending connections queue will hold
#define SERVER_STRING "Server: jdbhttpd/0.1.0\r\n"


void accept_request(int);
void cat(int, FILE *);
void execute_cgi(int, const char *, const char *, const char *);
int get_line(int, char *, int);
void headers(int, const char *);
void not_found(int);
void serve_file(int, const char *);
int startup(u_short *);
void unimplemented(int);


//void accept_request(int client);
//void cat(int client, FILE *resource);
//void execute_cgi(int client, const char *path, const char *method, const char *query_string);
//int get_line(int sock, char *buf, int size);
//void headers(int client, const char *filename);
//void not_found(int client);
//void serve_file(int client, const char *filename);
//int startup(u_short *port);
//void unimplemented(int client);
/*
 * 初始化http服务：建立套接字
 *                 绑定端口
 *                 进行监听
 */
int startup(u_short *port)//port的格式
{
  int httpd = 0;
  //int on = 1;
  struct sockaddr_in name;
  /*
   * socket
   */
  if( (httpd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  //设置套接字选项
  //if( ( setsockopt(httpd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) ) == -1 ) {
  //  perror("setsockopt failed");
  //  exit(1);
  //}
  
  memset(&name, 0, sizeof(name));
  name.sin_family = AF_INET;
  name.sin_port = htons(*port);
  name.sin_addr.s_addr = htonl(INADDR_ANY);

  /*
   * bind
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

void accept_request(int client)
{
  char buf[1024];
  int numchars;
  char method[255];
  char url[255];
  char path[255];
  size_t i, j;
  struct stat st;
  int cgi = 0;    /*becomes true if server decides this is a CGI program*/ 
  char *query_string = NULL;

  numchars = get_line(client, buf, sizeof(buf));
  i = 0; j = 0;

  /*
   * GET / HTTP/1.1CRLF
   */

  //取出GET放入method
  while (!ISspace(buf[j]) && (i < sizeof(method) - 1)) {
    method[i] = buf[j];
    i++; j++;
  }
  method[i] = '\0';
  
  //如果不是GET方法和POST方法，调用unimplemented接口，表明方法不被支持
  if (strcasecmp(method, "GET") && strcasecmp(method, "POST")) {
    unimplemented(client);
    return;
  }

  if (strcasecmp(method, "POST") == 0)
    cgi = 1;

  i = 0;
  while (ISspace(buf[j]) && (j < sizeof(buf)))
    j++;

  while (!ISspace(buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf))) {
    url[i] = buf[j];
    i++; j++;
  }
  url[i] = '\0';

  if (strcasecmp(method, "GET") == 0) {
    query_string = url;
    while ((*query_string != '?') && (*query_string != '\0'))
      query_string++;
    if (*query_string == '?') {
      cgi = 1;
      *query_string = '\0';
      query_string++;
    }
  }

  sprintf(path, "htdocs%s", url);
  if (path[strlen(path) - 1] == '/')
    strcat(path, "index.html");
puts(path);
  if (stat(path, &st) == -1) {
    while ((numchars > 0) && strcmp("\n",buf)) /* read & discard headers */
      numchars = get_line(client, buf, sizeof(buf));
    not_found(client);
  }else{
    if ((st.st_mode & S_IFMT) == S_IFDIR)
      strcat(path, "/index.html");
    if ((st.st_mode & S_IXUSR) ||
        (st.st_mode & S_IXGRP) ||
        (st.st_mode & S_IXOTH)    )
      cgi = 1;
printf("%d\n",cgi);

    if (!cgi)
      serve_file(client, path);
    else
      execute_cgi(client, path, method, query_string);
  }

  close(client);
}

void cat(int client, FILE *resource)
{
  char buf[1024];
  fgets(buf, sizeof(buf), resource);
  while (!feof(resource)) {
printf("cat\n");
    send(client, buf, strlen(buf), 0);
    fgets(buf, sizeof(buf), resource);
  }
}

void execute_cgi(int client, const char *path, const char *method, const char *query_string)
{
  //TODO
  (void)client;
  (void)path;
  (void)method;
  (void)query_string;
}

/*
 * Return the informational HTTP headers about a file.
 */
void headers(int client, const char *filename)
{
  char buf[1024];
  (void)filename;    /*could use filename to determine file type*/

  strcpy(buf, "HTTP/1.0 200 OK\r\n");
  send(client, buf, strlen(buf), 0);
  strcpy(buf, SERVER_STRING);
  send(client, buf, strlen(buf), 0);
  strcpy(buf, "Content-Type: text/html\r\n");
  send(client, buf, strlen(buf), 0);
  strcpy(buf, "\r\n");
  send(client, buf, strlen(buf), 0);
printf("headers\n");
}

/*
 * Give a client a 404 not found status message.
 */
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

int get_line(int sock, char *buf, int size)
{
  int i = 0;
  char c = '\0';
  int n;

  while ((i < size - 1) && (c != '\n'))
  {
    //if recv return 0, This mean only one thing:the remote side has closed the connection on you.
    n = recv(sock, &c, 1, 0);
    printf("%02X\n",c);
    if (n > 0) {
      if (c == '\r') {
        n = recv(sock, &c, 1, MSG_PEEK);
        if ((n > 0) && (c != '\n')) {
          recv(sock, &c, 1, 0);
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

void serve_file(int client, const char *filename)
{
  FILE *resource = NULL;
  int numchars = 1;
  char buf[1024];

  buf[0] = 'A'; buf[1] = '\0';
  while ((numchars > 0) && strcmp("\n", buf))    /* read & discard headers */
    numchars = get_line(client, buf, sizeof(buf));

  resource = fopen(filename, "r");
  if (resource == NULL)
    not_found(client);
  else{
printf("serve_file:after fopen success\n");
    headers(client, filename);
    cat(client, resource);
  }
  fclose(resource);
}

int main()
{
  int server_sock = -1;
  u_short port = 56784;
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
      exit(1);
    }
    accept_request(client_sock);
  }

  close(server_sock);
  return 0;
}
