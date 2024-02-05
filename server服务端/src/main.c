#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include <unistd.h>
#include "Camera_server.h"
#include "Zigbee_server.h"


pthread_t cam_tid;
pthread_t zgb_tid;
pthread_t srv_tid;

int main(int argc, char *argv[])
{
	int ret = -1;

	/* camera thread */
	while(ret < 0){
		ret = pthread_create(&cam_tid, NULL, thread_cam, NULL);
		if (ret) {
			errno = ret;
			perror("create camera thread");
			exit(EXIT_FAILURE);
		} else
			printf("create camera thread success\n");
		ret = pthread_detach(cam_tid);
		if (ret) {
			errno = ret;
			perror("detach camera thread");
			exit(EXIT_FAILURE);
		} else
			printf("detach camera thread success\n");
	}
	ret = -1;
#if 1
	/*thread_zgb*/
	while(ret < 0){
		ret = pthread_create(&zgb_tid, NULL, thread_Zigbee, NULL);
		if (ret) {
			errno = ret;
			perror("create zigbee thread");
			exit(EXIT_FAILURE);
		} else
			printf("create zigbee thread success\n");
		ret = pthread_detach(zgb_tid);
		if (ret) {
			errno = ret;
			perror("detach zigbee thread");
			exit(EXIT_FAILURE);
		} else
			printf("detach zigbee thread success\n");
	}
#endif
	ret = -1;
	/* Camera server*/
	while (ret < 0){
		ret = pthread_create(&srv_tid, NULL, thread_CamServer, NULL);
		if (ret) {
			errno = ret;
			perror("create android server thread\n");
			exit(EXIT_FAILURE);
		} else
			printf("create CamServer thread success\n");

		ret = pthread_detach(srv_tid);
		if (ret) {
			errno = ret;
			perror("detach CamServer thread\n");
			exit(EXIT_FAILURE);
		} else
			printf("detach CamServer thread success\n");
	}
	ret = -1;
	/*Zigbee server*/
	while(ret < 0){
		ret = pthread_create(&zgb_tid, NULL, thread_ZgbServer, NULL);
		if (ret) {
			errno = ret;
			perror("create zigbee thread");
			exit(EXIT_FAILURE);
		} else
			printf("create ZgbServer thread success\n");
		ret = pthread_detach(zgb_tid);
		if (ret) {
			errno = ret;
			perror("detach ZgbServer thread");
			exit(EXIT_FAILURE);
		} else
			printf("detach ZgbServer thread success\n");
	}

	/* main thread, process environment data form m0 board or process short message */
	while (1) {
		//pthread_mutex_destroy(&cam_mutex);
	}
	exit(EXIT_SUCCESS);
}
