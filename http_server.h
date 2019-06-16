#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#define sys_error(S) do{ \
	fprintf(stdout,"[%s:%d]: %s %s\n",__FILE__,__LINE__,S,strerror(errno)); \
	exit(-1); \
}while(0)

#define BUFFSIZE 2048
#define ROOT_DIR "./www/"

void do_client(int fd);
void do_error(int fd,int code,const char *info);
void do_get_response(int fd,const char *url);
#endif
