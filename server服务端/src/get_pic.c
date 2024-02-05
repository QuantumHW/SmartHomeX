#include "get_pic.h"

struct jpg_buf_t *jpg;
pthread_mutex_t cam_mutex;

int camera_on()
{
	int i;
	int fd;
	int ret;
	unsigned int width;
	unsigned int height;
	unsigned int size;
	unsigned int index;
	unsigned int ismjpeg;
	char *yuv;

	width = W;
	height = H;
	/*
	 *	摄像头初始化操作，设置设备的文件名，照片的宽度高度，得到设备文件描述符
	 */
	 //
	fd=camera_init("/dev/video0",&width,&height,&size,&ismjpeg);
	
	if (fd == -1)
		return -1;
	
	/*
	 *	启动摄像头，开启视频采集
	 */
	ret=camera_start(fd);
	
	if (ret == -1)
		return -1;
	jpg = malloc(sizeof(struct jpg_buf_t));//给全局的结构体指针开辟空间
	if (!jpg) {
		perror("malloc");
		return -1;
	}
	if (ismjpeg == 0) {
		printf("------yuyv------\n");
	}
	// 采集几张图片丢弃 
	for (i = 0; i < 8; i++) {
		ret = camera_dqbuf(fd, (void **)&yuv, &size, &index);
		if (ret == -1)
			exit(EXIT_FAILURE);

		ret = camera_eqbuf(fd, index);
		if (ret == -1)
			exit(EXIT_FAILURE);
	}

	fprintf(stdout, "init camera success\n");
	
	while (1) {
		/*
		*	出队操作，得到采集的图片和图片的大小
		*/
		//ret=camera_dqbuf(int fd, void **buf, unsigned int *size, unsigned int *index);
		ret=camera_dqbuf(fd,(void**)&yuv,&size,&index);//?
		
		if (ret == -1)
			return -1;
		if (ismjpeg == 1) {			            //判断采集的图片的格式是否是 mjpeg
			pthread_mutex_lock(&cam_mutex);

			memset(jpg, 0, sizeof(jpg));        //将 jpg 全局结构体指针清零
            /*
            *   将出对得到的 图片内容存储到 全局的 缓冲区 
            	?camera_eqbuf(int fd, unsigned int index); 
            */
			memcpy(jpg->jpg_buf,yuv,size);
            /*
            *  将出对得到的图片大小，存储到全局结构体中 
            */
			jpg->jpg_size=size;
			
			pthread_mutex_unlock(&cam_mutex);

		}
#if 1	
		int fd1 = open("1.jpg", O_RDWR | O_CREAT, 0777);
		int count = 0;
		while(count <  jpg->jpg_size){
			int ret = write(fd1, jpg->jpg_buf + count, jpg->jpg_size - count);
			if (ret < jpg->jpg_size){
				printf("-----数据太少-----\n");
			}
			count += ret;
		}
		close(fd1);
#endif	
		ret = camera_eqbuf(fd, index);
		if (ret == -1)
			return -1;
	}
	/* 代码不应该运行到这里 */
	if (!ismjpeg) {
		//convert_rgb_to_jpg_exit();
		//free(jpg);
	}
	free(jpg);

	ret = camera_stop(fd);
	if (ret == -1)
		return -1;

	ret = camera_exit(fd);
	if (ret == -1)
		return -1;
}
