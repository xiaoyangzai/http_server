#ifndef __ROBUST_IO_H__
#define __ROBUST_IO_H__
#define RIO_BUFSIZE 50
typedef struct _rio_t{
	//需要缓冲的文件描述符
	int fd;
	//缓冲区中剩余的未读取字节数
	size_t unread_cnt;	
	//缓冲区中下次读取位置
	char *nextread_ptr;
	//缓冲区
	char buf[RIO_BUFSIZE];
}rio_t;

//初始化RIO缓冲区结构
void rio_init(rio_t *rp,int fd);

//从文件描述符对应的文件中读取数据
//填充缓冲区，更新相关参数
//成功返回实际读取字节数,网络关闭或者文件末尾返回0,出错返回-1；
ssize_t rio_read(rio_t *rp,char *userbuf,size_t n);

//从缓冲区中读取数据
//成功返回实际读取字节数,网络关闭或者文件末尾返回0,出错返回-1；
ssize_t rio_readn(rio_t *rp,void *userbuf,size_t n);

//从缓冲区中读取一行,包括换行字符'\n'
//成功返回实际读取字节数,网络关闭或者文件末尾返回0,出错返回-1；
ssize_t rio_readline(rio_t *rp,void *userbuf,size_t n);

//完备读函数
//readn函数只会在遇到文件末尾或网络关闭时，返回值小于要求读取的字节数n
ssize_t readn(int fd,void *buf,size_t n); 

//完备写函数
//writen函数总会写入要求的字节数，否则出错
ssize_t writen(int fd,void *buf,size_t n); 

#endif
