#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <arpa/inet.h>

#include "robust_io.h"
#include "http_server.h"

#define sys_error(S) do{ \
	fprintf(stdout,"[%s:%d]: %s %s\n",__FILE__,__LINE__,S,strerror(errno)); \
	exit(-1); \
}while(0)

void handle_exit_process(int sno);

int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("usage: ./server [PORT]\n");
		return -1;
	}

	//设定僵尸进程处理函数
	signal(SIGCHLD,handle_exit_process);

	//创建并监听网络套接字
	int serverfd = socket(AF_INET,SOCK_STREAM,0);
	if(serverfd < 0)
		sys_error("socket failed");
	short port = atoi(argv[1]); 

	struct sockaddr_in serverinfo;	
	serverinfo.sin_port = htons(port);
	serverinfo.sin_family = PF_INET;
	serverinfo.sin_addr.s_addr = inet_addr("0.0.0.0");

	//设定地址复用选项
	int opt = 1;
	if(setsockopt(serverfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt)) < 0)
		sys_error("setsockopt failed");

	if(bind(serverfd,(struct sockaddr*)&serverinfo,sizeof(serverinfo)) < 0)
		sys_error("bind failed");

	if(listen(serverfd,10) < 0)
		sys_error("listen failed");
	
	//等待客户端连接

	struct sockaddr_in clientinfo;	
	socklen_t len = sizeof(clientinfo);
	while(1)
	{
		printf("wait for client....\n");
		int clientfd = accept(serverfd,(struct sockaddr*)&clientinfo,&len);
		if(clientfd < 0)
			sys_error("accept failed");
		//创建子进程来处理客户端通信
		pid_t pid = fork();
		if(pid == 0)
		{
			close(serverfd);
			do_client(clientfd);
			close(clientfd);
			return 0;
		}
		close(clientfd);
	}
	return 0;
}

void handle_exit_process(int sno)
{
	wait(NULL);
}
