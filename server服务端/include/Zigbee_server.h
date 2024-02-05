#ifndef __ZIGBEE_SERVER_H__
#define __ZIGBEE_SERVER_H__

#include "zigbee.h"
#include "tcp_server.h"

void *thread_Zigbee(void *arg);
void *thread_ZgbServer(void *arg);
void *thread_CmdRecv(void *arg);
void *thread_EnvSend(void *arg);


#endif
