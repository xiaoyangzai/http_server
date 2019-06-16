#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include "robust_io.h"
#include "http_server.h"

void do_client(int fd)
{
	//读取HTTP请求报文	
	char buf[BUFFSIZE];
	int n = 0;
	//读取请求行
	rio_t rio;
	rio_init(&rio,fd);
	n = rio_readline(&rio,buf,sizeof(buf));
	if(n < 0)
		sys_error("read failed");
	char method[128];
	char url[128];
	char version[128];
	sscanf(buf,"%s %s %s\r\n",method,url,version);
	printf("method: %s\turl: %s\tversion: %s\n",method,url,version);
	//读取HTTP请求数据包的剩余数据
	printf("HTTP header: \n%s",buf);
	while(1)
	{
		n = rio_readline(&rio,buf,sizeof(buf));
		if(n < 0)
			sys_error("read failed");
		if(n == 0)
		{
			printf("client offline\n");
			return;
		}
		printf("%s",buf);
		if(strcmp(buf,"\r\n") == 0)
			break;
	}
	
	//判断请求方法是否合理
	if(strcmp(method,"GET") == 0)
		do_get_response(fd,url);
	else
		do_error(fd,500,"Only Support GET Method");
	return ;
}
void do_error(int fd,int code,const char *info)
{
	//构建HTTP响应报文
	char buf[BUFFSIZE];
	char error_html[] = "<html><title>Server Error</title><body>Only Support GET Method!</body></html>";
	sprintf(buf,"%s %d %s\r\n","HTTP/1.1",code,info);
	sprintf(buf,"%sContent-Length: %ld\r\n",buf,strlen(error_html));
	sprintf(buf,"%sContent-Type: %s\r\n\r\n",buf,"text/html");

	//发送HTTP响应报文请求头
	if(writen(fd,buf,strlen(buf)) < 0)
		sys_error("write failed");
	//发送HTTP响应报文的主体
	if(writen(fd,error_html,strlen(error_html)) < 0)
		sys_error("write failed");
	return;
}

void do_get_response(int fd,const char *url)
{
	struct stat st;	
	char filename[256];	
	strcat(filename,ROOT_DIR);
	if(strcmp(url,"/") == 0)
		strcat(filename,"index.html");
	else
		strcat(filename,url);
	char buf[BUFFSIZE];
	if(stat(filename,&st) < 0)
	{
		
		//构建HTTP响应报文
		//构建响应行
		sprintf(buf,"%s","HTTP/1.1 404 NotFound\r\n");

		//获取404.html文件的属性
		strcpy(filename,"./www/404.html");
		stat(filename,&st);
	}
	else
	{
		//构建正常响应报文
		sprintf(buf,"%s","HTTP/1.1 200 OK\r\n");
	}

	printf("filename: %s\n",filename);
	//构建首部字段
	sprintf(buf,"%sContent-Length: %ld\r\n",buf,st.st_size);
	sprintf(buf,"%sContent-Type: %s\r\n\r\n",buf,"text/html");
	//发送响应报文头
	if(writen(fd,buf,strlen(buf)) < 0)
		sys_error("writen failed");
	//将资源的内容作为响应主体发送给客户端
	int source_fd = open(filename,O_RDONLY);
	if(source_fd < 0)
		sys_error("open failed");
	int n;
	while(1)
	{
		n = read(source_fd,buf,sizeof(buf));
		if(n < 0)
			sys_error("read failed");
		else if(n == 0)
			break;
		writen(fd,buf,n);
	}
	return;
}
