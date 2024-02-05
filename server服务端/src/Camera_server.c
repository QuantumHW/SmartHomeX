#include "Camera_server.h"

void *thread_cam()
{
	pthread_mutex_init(&cam_mutex, NULL);
	
	int ret = -1;
	while (ret < 0 ){
        /*
        *   调用 get_pic.c 的函数，实现摄像头的初始化、采集等操作
        */
		ret=camera_on();//camera_init();
		if(ret<0){
			return (void*)-1;
		}
		
		sleep(3);
	}
	return (void *)0;
}

void *thread_CamServer()
{
	int *tempfd;
	int connfd;
	pthread_t PicSend_tid;//服务器读取客户端线程号
	
    /*
    *   调用tcp_server.c的函数初始化tcp 服务器，得到套接字
    */
	int sockfd=tcp_server_init(8080,5);

	if(0>sockfd){
		printf("init_server failed!\n");
		pthread_exit(NULL);
	}
	while(1) {

        /*
        *   调用 tcp_server.c 的函数，阻塞等待客户端的连接，得到连接成功的客户端 id号
        */
		if ( (connfd=tcp_server_wait_connect(sockfd)) != -1) {
			tempfd = malloc(sizeof(int));
			if (!tempfd) {
				fprintf(stderr, "server->malloc: malloc failure\n");
				close(connfd);
				close(sockfd);
			}
			*tempfd = connfd;
			int ret = pthread_create(&PicSend_tid, NULL, thread_PicSend, tempfd);
			if (ret) {
				errno = ret;
				perror("server->thread");
				close(connfd);
				close(sockfd);
			} else
				printf("server->thread: create PicSend thread success\n");
				
			ret = pthread_detach(PicSend_tid);
			if (ret) {
				errno = ret;
				perror("deta`ch android server thread\n");
			} else
				printf("detach PicSend thread success\n");
			}
	}
}
void *thread_PicSend(void *arg)
{
	int ret;
	int connfd = *(int *)arg;
	char response[10] = {0};
	while(1){
		pthread_mutex_lock(&cam_mutex);
            
            /*
            *   将全局结构体中存储的 jpeg 图片大小，组装到 response 中
            */
			sprintf(response,"%d",jpg->jpg_size);
	
            /*
            *   调用 tcp_server.c 的函数，将 response 中存储的图片大小发送给客户端
            */
			ret=write(connfd,response,sizeof(response));
			
		if (ret != sizeof(response)) {
			fprintf(stderr, "server->write: send response failed\n");
			pthread_mutex_unlock(&cam_mutex);
			pthread_exit("errnor");
		}

            /*
            *   调用 tcp_server.c 的函数，将全局结构中的图片内容按照图片的真实大小发送给客户端
            */
            ret = write(connfd,jpg->jpg_buf,jpg->jpg_size);
			
		if (ret != jpg->jpg_size) {
			fprintf(stderr, "server->write: send response failed\n");
			pthread_mutex_unlock(&cam_mutex);
			pthread_exit("errnor");
		}
		pthread_mutex_unlock(&cam_mutex);
		usleep(100000);
	}
	return (void *)0;
}
