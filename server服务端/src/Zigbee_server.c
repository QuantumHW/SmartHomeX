#include "Zigbee_server.h"

int zgbfd;

void *thread_Zigbee(void *arg)
{
	char *devpath[] = {"/dev/ttyUSB0", "/dev/ttyUSB1", "/dev/ttyUSB2"};
	int i = 0;
AA:	
    /*
    *   调用Zigbee.c 中的函数，初始化串口设备，得到串口的文件描述符
    */
    zgbfd=zigbee_init("/dev/ttyUSB0",B115200);
	if (zgbfd == -1){
		sleep(2);
		printf("init zigbee error\n");
		i++;
		goto AA;
	}else
		fprintf(stdout, "init zigbee success\n");

	while (1) {
        /*
        *   调用 Zigbee.c 中的函数，得到串口的环境信息数据
        */
		int ret=zigbee_get_dat(zgbfd);
		if (ret < 0){
			perror("env_recv:");
			goto AA;
		}
		sleep(1);
	}
}

void *thread_ZgbServer(void *arg)
{
	int *tempfd;
	int connfd;
	pthread_t cmd_tid, env_tid;
	
    /*
    *   调用 tcp_server.c 中的函数，初始化 tcp 服务器，得到套接字
    */
	//int sockfd=tcp_server_init(8888,16);
	int sockfd=tcp_server_init(9527,5);
	if(0>sockfd){
		printf("init_server failed!\n");
		pthread_exit(NULL);
	}
	while(1) {
        
        /*
        *   调用 tcp_server.c 的函数，阻塞等待客户端的连接，得到连接成功的客户端 id号
        */
		if((connfd=tcp_server_wait_connect(sockfd))!=-1){
			tempfd = malloc(sizeof(int));
			if (!tempfd) {
				fprintf(stderr, "server->malloc: malloc failure\n");
				close(connfd);
				close(sockfd);
			}
			*tempfd = connfd;
			int ret = pthread_create(&cmd_tid, NULL, thread_CmdRecv, tempfd);
			if (ret) {
				perror("server->thread");
				close(connfd);
			} else
				printf("server->thread: create CmdRecv thread success\n");
			pthread_detach(cmd_tid);
			
			ret = pthread_create(&env_tid, NULL, thread_EnvSend, tempfd);
			if (ret) {
				perror("server->thread");
				close(connfd);
			} else
				printf("server->thread: create EnvSend thread success\n");
			pthread_detach(env_tid);
			
		}
	}
}
void *thread_CmdRecv(void *arg)
{
	int ret;
	int connfd = *(int *)arg;
	char request[32] = {0};
	while(1){
		memset(request, 0, sizeof(request));

        /*
        *   调用 tcp_server.c 中的函数，读取客户端发送的控制命令，读取 32个字节
        */
		ret=read(connfd,request,sizeof(request));
		if (ret <= 0){
			close(connfd);
			pthread_exit(NULL);
		}

        /*
        *   调用zigbee.c 中的函数，将读取到的客户端命令与串口设备文件描述符，传递过去
        */
		zigbee_exe_cmd(zgbfd,request);
	}
	return (void *)0;
}

void *thread_EnvSend(void *arg)
{
	char response[32] = {0};
	int connfd = *(int *)arg;
	while(1){
		memset(response, 0, sizeof(response));

        /*
        *   将全局结构体中存储的 环境信息，组装到 response 中
        */
		sprintf(response,"t:%d,h:%d,l:%d",data.tem,data.hum,data.light);
		//printf("resopnse: %s\n", response);

        /*
        *   调用 tcp_server.c 的函数，将 response 中存储的环境信息发送给客户端
        */
		//int ret=tcp_server_send(connfd,response,sizeof(response));
		int ret=write(connfd,response,sizeof(response));
		if (ret != sizeof(response)) {
			fprintf(stderr, "server->write: send response failed\n");
			sleep(2);
			continue;
		}
		usleep(1500000);
	}
	return (void *)0;
}
