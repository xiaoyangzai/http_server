#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include "robust_io.h"

//初始化RIO缓冲区结构
void rio_init(rio_t *rp,int fd)
{
	rp->fd = fd;
	rp->nextread_ptr = rp->buf;
	rp->unread_cnt = 0;
}

//从文件描述符对应的文件中读取数据
//填充缓冲区，更新相关参数
//成功返回实际读取字节数,网络关闭或者文件末尾返回0,出错返回-1；
ssize_t rio_read(rio_t *rp,char *userbuf,size_t n)
{
	//判断缓冲区是否为空，为空则读取文件填充缓冲区	
	while(rp->unread_cnt <= 0)
	{
		rp->unread_cnt = read(rp->fd,rp->buf,sizeof(rp->buf)); 
		if(rp->unread_cnt < 0)
			return -1;
		if(rp->unread_cnt == 0)
			return 0;
		//填充完毕后，设置下次读取位置为缓冲区起始位置
		rp->nextread_ptr = rp->buf;
	}
	//获取要求读取字节数与可读字节数最小值
	size_t ncnt = n;
	if(rp->unread_cnt < n)
		ncnt = rp->unread_cnt; 
	//从缓冲区读取ncnt个字节拷贝到用户缓冲区userbuf中
	memcpy(userbuf,rp->nextread_ptr,ncnt);
	//更新下次读取位置以及剩余未读取字节数
	rp->nextread_ptr += ncnt;
	rp->unread_cnt -= ncnt;
	return ncnt;
}

//从缓冲区中读取数据
//成功返回实际读取字节数,网络关闭或者文件末尾返回0,出错返回-1；
ssize_t rio_readn(rio_t *rp,void *userbuf,size_t n)
{
	size_t nleft = n;	
	ssize_t nread = 0;
	char *buf = (char *)userbuf;
	while(nleft > 0)
	{
		if((nread = rio_read(rp,buf,nleft)) < 0)
			return -1;
		else if(nread == 0)
			break;
		nleft -= nread;
		buf += nread;
	}
	return n - nleft;
}

//从缓冲区中读取一行,包括换行字符'\n',在末尾追加'\0'
//最多读取n - 1个字节
//成功返回实际读取字节数,网络关闭或者文件末尾返回0,出错返回-1；
ssize_t rio_readline(rio_t *rp,void *userbuf,size_t n)
{
	size_t i = 0;
	char c;
	char *buf = (char *)userbuf;
	size_t nread = 0;
	for(i = 1;i < n;i++)
	{
		if((nread = rio_read(rp,&c,1)) < 0)	
			return -1;
		else if(nread == 1)
		{
			*buf++ = c;
			if(c == '\n')
				break;
		}
		else if(nread == 0)
		{
			if(i == 1)
				return 0;
			else
				break;
		}
	}
	*buf = '\0';
	return i;
}

//完备读函数
//readn函数只会在遇到文件末尾或网络关闭时，返回值小于要求读取的字节数n
ssize_t readn(int fd,void *buf,size_t n) 
{
	size_t nleft = n;
	size_t nread = 0;
	char *ptr = (char *)buf;
	while(nleft > 0)
	{
		if((nread = read(fd,ptr,nleft)) < 0)
		{
			if(errno == EINTR)
				nread= 0;
			else
				return -1;
		}
		else if(nread == 0)
			break;
		nleft -= nread;
		ptr += nread;
			
	}
	return n - nleft;
}

//完备写函数
//writen函数总会写入要求的字节数，否则出错
ssize_t writen(int fd,void *buf,size_t n)
{
	size_t nleft = n;
	size_t nwrite = 0;
	char *ptr = (char *)buf;
	while(nleft > 0)
	{
		if((nwrite = write(fd,ptr,nleft)) < 0)
		{
			if(errno == EINTR)
				nwrite = 0;
			else
				return -1;
		}
		else
		{
			nleft -= nwrite;
			ptr += nwrite;
		}
	}
	return n;
}


