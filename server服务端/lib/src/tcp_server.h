#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

int tcp_server_init(int port, int backlog);
int  tcp_server_wait_connect(int listen_fd);
int tcp_server_recv(int connfd, void *buf, int count);
int tcp_server_send(int connfd, void *buf, int count);
int tcp_server_recv_exact_nbytes(int connfd, void *buf, int count);
int tcp_server_send_exact_nbytes(int connfd, const void *buf, int count);
int tcp_server_disconnect(int connfd);




#endif
