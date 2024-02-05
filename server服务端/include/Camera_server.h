#ifndef __CAMERA_SERVER_H__
#define __CAMERA_SERVER_H__

#include "get_pic.h"
#include "tcp_server.h"

void *thread_cam();
void *thread_CamServer();
void *thread_PicSend(void *arg);


#endif
