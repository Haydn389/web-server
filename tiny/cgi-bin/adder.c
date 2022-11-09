/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

int main(void)
{
  char *buf, *p;
  char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
  int n1 = 0, n2 = 0;

  /*두 개의 인자를 분리한다*/
  if ((buf = getenv("QUERY_STRING")) != NULL)
  {
    p = strchr(buf, '&');
    *p = '\0';
    strcpy(arg1, buf);
    strcpy(arg2, p + 1);

    p=strchr(arg1,'=');
    strcpy(arg1,p+1);
    p=strchr(arg2,'=');
    strcpy(arg2,p+1);

    n1 = atoi(arg1);
    n2 = atoi(arg2);
  }
  /*response body 만들기*/
  sprintf(content, "QUARY_STRING=%s", buf);
  sprintf(content, "Welcome to add.com: ");
  sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
  // sprintf(content, "%sInput 1 : <input>\r\n<p>", content);
  // sprintf(content, "%sinput 2 : <input>\r\n<p>", content);
  sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>", content, n1, n2, n1 + n2);
  sprintf(content, "%sThanks for visiting!\r\n", content);

  /*HTTP response 생성*/
  printf("Connection: close\r\n");
  printf("Content-length: %d\r\n", (int)strlen(content));
  printf("Content-type: text/html\r\n\r\n");
  //여기까지가 헤더

  //여기까지 바디출력
  printf("%s", content);
  fflush(stdout);
  exit(0);
}
/* $end adder */
