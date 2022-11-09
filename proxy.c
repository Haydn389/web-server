#include <stdio.h>
#include "csapp.h"

void doit(int fd);
void parse_uri(char *uri, char *host, char *filename,char *port);

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

int main(int argc, char **argv)
{
  printf("%s", user_agent_hdr);
  int listenfd, connfd;
  char host[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  // open_listenfd 함수를 호출해서 듣기 소켓을 오픈한 후에,
  listenfd = Open_listenfd(argv[1]);
  // 무한 서버 루프를 실행하고,
  while (1) {
    clientlen = sizeof(clientaddr);
    // 반복적으로 연결 요청을 접수하고,
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);  // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, host, MAXLINE, port, MAXLINE, 0);
    printf("Accepted connection from (%s, %s)\n", host, port);
    // 트랜잭션을 수행하고,
    doit(connfd);   // line:netp:tiny:doit
    // 자신 쪽의 연결 끝을 닫는다.
    Close(connfd);  // line:netp:tiny:close
  }
  return 0;
}

void doit(int connfd)
{
  int clientfd;
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char host[MAXLINE], filename[MAXLINE];
  char response[MAX_OBJECT_SIZE];    
  rio_t rio,tiny_rio;//서버와 주고받을 tiny_rio버퍼생성
  char port[MAXLINE]="80"; //proxy <--> tiny 포트번호
  // char *host, *port;

  /* Read request line and headers*/
  Rio_readinitb(&rio, connfd);
  Rio_readlineb(&rio, buf, MAXLINE);
  printf("Request headers:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);// [GET / HTTP/1.1]
  printf(">>method:%s, uri:%s, version:%s\n", method, uri, version);

  /*error 처리*/
  if(strcasecmp(method,"GET") && strcasecmp(method,"HEAD"))
  { 
    sprintf(buf,"Proxy does not implement this method\r\n");
    Rio_writen(connfd,buf,strlen(buf));
    return;
  }
  /*host, filename, port 파싱*/
  parse_uri(uri, host, filename, port);

  // char post_str[100];
  // sprintf(post_str,"%d",port); //port 자료형을 int -> str 변환
  /*requests headers 생성*/
  strcpy(uri,"");
  strcat(uri,filename);
  
  sprintf(buf, "%s %s %s\r\n", method, uri, "HTTP/1.0");
  sprintf(buf, "%sHost: %s:%s\r\n", buf, host,port);
  sprintf(buf, "%s%s", buf, user_agent_hdr);
  sprintf(buf, "%sConnection: close\r\n", buf);
  sprintf(buf, "%sProxy-Connetion : close\r\n\r\n", buf);
  printf("requests headers:\n");
  printf("%s", buf);

  /*clientfd open*/

  clientfd = Open_clientfd(host, port);
  Rio_readinitb(&tiny_rio, clientfd);//tiny_rio 와 clientfd 연결

  /*proxy -> tiny_server로 전송(echo client이용)*/
  Rio_writen(clientfd, buf, strlen(buf));

  /*proxy -> client로 전송(echo server이용)*/
  Rio_readnb(&tiny_rio, response, MAX_OBJECT_SIZE);
  Rio_writen(connfd, response, MAX_OBJECT_SIZE);//클라이언트로 전송

  Close(clientfd);
}

void parse_uri(char *uri, char *host, char *filename,char *port)
{
  printf(">>>>>>parse 전 uri :%s\n",uri);
  char *ptr;
  /* 문자열"//" 존재확인 후 ptr에 uri 시작주소 저장 */
  ptr=strstr(uri,"//");// 
  if (ptr==NULL)
    ptr=uri;
  else
    ptr+=2;
  
  /*host 업데이트 후 filename 파싱*/
  sscanf(ptr,"%s",host);
  ptr=strstr(host,"/");
  sscanf(ptr,"%s",filename);

  /*filename 앞쪽은 host로 업데이트*/
  *ptr='\0';
  sscanf(host,"%s",host);

  /*만약 현재 host문자열에 port번호도 함께 존재할 경우 해당 포트번호 파싱*/
  if (ptr=strstr(host,":"))
  {
    *ptr='\0';
    sscanf(ptr+1,"%s",port);
    sscanf(host,"%s",host);
  }
  /*값 확인*/
  printf(">>>>parse 후 host :%s\n",host);
  printf(">>>>parse 후 port  :%s\n",port);
  printf(">>>>parse 후 filename  :%s\n",filename);
  return;
}