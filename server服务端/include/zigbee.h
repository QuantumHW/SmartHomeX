#ifndef __ZIGBEE_H__
#define __ZIGBEE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <termios.h>

#include <error.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "serial.h"

#define REQ_DATA_SIZE   32
#define HDR_DATA_SIZE   128
#define ZGB_DATA_SIZE	36


struct env{
	int tem;
	int hum;
	int light;
};

extern struct env data;

int zigbee_init( char *devpath, int baudrate);
int zigbee_get_dat(int fd);
int zigbee_exe_cmd(int fd, char *p);
int zigbee_exit(int fd);

#endif
