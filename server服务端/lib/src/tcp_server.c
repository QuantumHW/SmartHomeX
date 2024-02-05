/*************************************************************************
    > File Name: server.c
# File Name: server.c
# Author :枫叶
# QQ : 719993600
# Email:719993600@qq.com
# Blog：http://www.cnblogs.com/shenlanqianlan/
# Created Time: Mon 09 Jan 2017 10:34:29 AM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#define MAX_BACKLOG	1024

int tcp_server_init(int port, int backlog)
{
	int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(0 > s){
		perror("socket");
		return -1;
	}
	int reuseaddr = 1;
	if(0 > setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr))){
		perror("can`t reuse addr on rw socket!");
		return -1;
	}
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(struct sockaddr_in));
	servaddr.sin_family 		= AF_INET;
	servaddr.sin_port 		= htons(port);
	servaddr.sin_addr.s_addr 	= htonl(INADDR_ANY);
	
	socklen_t len = sizeof(servaddr);
	/*****绑定服务器三元组信息************/
	if(0 > bind(s, (struct sockaddr*)&servaddr, len)){
		perror("bind");
		goto ERR_STEP;
	}
	/*******设定监听套接字*************/
	if(0 > listen(s, backlog)){
		perror("listen");
		goto ERR_STEP;
	}
	return s;
	/****出错释放资源处理*******/
ERR_STEP:
	close(s);
	return -1;
}

int  tcp_server_wait_connect(int listen_fd)
{
	struct sockaddr_in clientaddr;
	socklen_t cli_len = sizeof(clientaddr);
	
	int connfd = accept(listen_fd, (struct sockaddr *)&clientaddr, &cli_len);
	if (connfd  == -1){
		perror("accept:");
		return -1;
	}
	printf("client: %s:%u\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	
	return connfd;
}

int tcp_server_recv(int connfd, void *buf, int count)
{
	ssize_t ret;

	assert(buf != NULL);

	ret = read(connfd, buf, count);
	if (ret == -1) {
		perror("server->read");
		return -1;
	} else if (ret == 0) {
		fprintf(stderr, "server->read: end-of-file\n");
		return 0;
	} else
		return ret;
}

int tcp_server_send(int connfd, const void *buf, size_t count)
{
	ssize_t ret;

	assert(buf != NULL);

	ret = write(connfd, buf, count);
	if (ret == -1) {
		perror("server->read");
		return -1;
	} else
		return ret;
}

int tcp_server_recv_exact_nbytes(int connfd, void *buf, int count)
{
	ssize_t ret;
	ssize_t total = 0;
	assert(buf != NULL);

	while (total != count) {
		ret = read(connfd, buf + total, count - total);
		if (ret == -1) {
			perror("server->read");
			sleep(2);
			break;
		} else if (ret == 0) {
			fprintf(stderr, "server->read: end-of-file\n");
			return total;
		} else
			total += ret;
	}

	return total;
}

int tcp_server_send_exact_nbytes(int connfd, const void *buf, int count)
{
	ssize_t ret;
	ssize_t total = 0;

	assert(buf != NULL);

	while (total != count) {
		ret = write(connfd, buf + total, count - total);
		if (ret == -1) {
			perror("server->write");
			return total;
		} else
			total += ret;
	}

	return total;
}

int tcp_server_disconnect(int connfd)
{
	if (close(connfd)) {
		perror("server->close");
		return -1;
	}

	return 0;
}

