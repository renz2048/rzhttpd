#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>

#define PORT "5678"
#define BACKLOG 10

#define SERVER_STRING "Server: rzhttpd/0.1.0\r\n"

void accept_request(int);
void cat(int , FILE *);
void *get_in_addr(struct sockaddr *sa);
int get_line(int, char *, int);
void headers(int , const char *);
int init_net();
void not_found(int);
void serve_file(int , const char *);
void sigchld_handler(int s);
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
  //puts(buf);
  while(!isspace(buf[j]) && (i < sizeof(method) - 1))
  {
    method[i] = buf[i];
    i++; j++;
  }
  method[i] = '\0';

  //puts(method);
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
  //puts(url);

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

void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
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

int init_net()
{
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int yes=1;
  int rv;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  for(p = servinfo;p != NULL;p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }

    break;
  }

  freeaddrinfo(servinfo);

  if( (listen(sockfd, BACKLOG)) == -1 ) {
    perror("listen");
    exit(1);
  }

  return sockfd;
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
    //printf("hhhhhhhhhhh");
    cat(client, resource);
  }
  fclose(resource);
}

void sigchld_handler(int s)
{
  while(waitpid(-1, NULL, WNOHANG) > 0);
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

int main(void)
{
  int server_sock = -1;
  int client_sock = -1;
  struct sockaddr_storage client_addr;
  socklen_t client_len = sizeof(client_addr);
  struct sigaction sa;
  char s[INET6_ADDRSTRLEN];

  server_sock = init_net();

  //signore handler
  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  while(1)
  {
    client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
    if( client_sock == -1 ) {
      perror("accept");
      continue;
    }

    inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), s, sizeof s);
    printf("server: got connectin from %s\n", s);

    if (!fork()) {//child process
      close(server_sock);
      accept_request(client_sock);
      exit(0);
    }
    close(client_sock);//parent process
  }

  close(server_sock);
  return 0;
}
