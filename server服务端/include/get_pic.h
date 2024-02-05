#ifndef __GET_PIC_H__
#define __GET_PIC_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

#include <pthread.h>

#include "cam.h"

#define CAMERA_USB "/dev/video0"

#define JPG_MAX_SIZE	(1024 * 1024 - sizeof (unsigned int))
#define SIZE 16

#define W 640
#define H 480


struct jpg_buf_t {
	char jpg_buf[JPG_MAX_SIZE];
	unsigned int jpg_size;
};


extern struct jpg_buf_t *jpg;
extern pthread_mutex_t cam_mutex;


int camera_on();


#endif
