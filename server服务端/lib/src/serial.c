#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <termios.h>

/*********************************************************************************  
* Description：  串口初始化
* Input:         devpath：串口设备文件路径
* Output:        baudrate：波特率
* Return:       无
* Others:       无。
**********************************************************************************/
int serial_init(const char *devpath, int baudrate)
{
	int fd;
	struct termios oldtio, newtio;

	assert(devpath != NULL);

	fd = open(devpath, O_RDWR | O_NOCTTY); 
	if (fd == -1) {
		perror("serial->open");
		return -1;
	}

	tcgetattr(fd, &oldtio);		/* save current port settings */

	memset(&newtio, 0, sizeof(newtio));
	newtio.c_cflag = baudrate | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	newtio.c_lflag = 0;		/* set input mode (non-canonical, no echo,...) */

	newtio.c_cc[VTIME] = 40;	/* set timeout value, n * 0.1 S */
	newtio.c_cc[VMIN] = 0;

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	return fd;
}

/*********************************************************************************  
* Description：  串口接收
* Input:         fd：串口设备描述符
*				 count:接收数据长度
* Output:        buf：接收buf，用户提供，保证有效性。
* Return:       >=0 接收了的实际数据长度，<0表示失败。
* Others:       无。
**********************************************************************************/
ssize_t serial_recv(int fd, void *buf, size_t count)
{
	ssize_t ret;

	assert(buf != NULL);

	ret = read(fd, buf, count);
	if (ret == -1)
		perror("serial->send");

	return ret;
}

/*********************************************************************************  
* Description：  串口发送
* Input:         fd：串口设备描述符
*				 buf：发送buf，用户提供，保证有效性。
*				 count:发送数据长度
* Output:        无。
* Return:       >=0 发送了的实际数据长度，<0表示失败。
* Others:       无。
**********************************************************************************/
ssize_t serial_send(int fd, const void *buf, size_t count)
{
	ssize_t ret;

	assert(buf != NULL);

	ret = write(fd, buf, count);
	if (ret == -1)
		perror("serial->send");

	return ret;
}

/*********************************************************************************  
* Description：  大数据的串口接收
* Input:         fd：串口设备描述符
*				 count:接收数据长度
* Output:        buf：接收buf，用户提供，保证有效性。
* Return:       >0 接收了的实际数据长度，<=0表示失败。
* Others:       该接口要么接收完整用户传入长度数据，要么返回失败。
**********************************************************************************/
ssize_t serial_recv_exact_nbytes(int fd, void *buf, size_t count)
{
	ssize_t ret;
	ssize_t total = 0;

	assert(buf != NULL);

	while (total != count) {
		ret = read(fd, buf + total, count - total);
		if (ret == -1) {
			perror("serial->recv");
			total = -1;
			break;
		} else if (ret == 0) {
			fprintf(stdout, "serial->recv: timeout or end-of-file\n");
			total = 0;
			break;
		} else
			total += ret;
	}

	return total;
}

/*********************************************************************************  
* Description：  大数据的串口发送
* Input:         fd：串口设备描述符
*				 buf：发送buf，用户提供，保证有效性。
*				 count:发送数据长度
* Output:        无。
* Return:       >=0 发送了的实际数据长度，<0表示失败。
* Others:       该接口要么发送完整用户传入长度数据，要么返回失败。。
**********************************************************************************/
ssize_t serial_send_exact_nbytes(int fd, const void *buf, size_t count)
{
	ssize_t ret;
	ssize_t total = 0;

	assert(buf != NULL);

	while (total != count) {
		ret = write(fd, buf + total, count - total);
		if (ret == -1) {
			perror("serial->send");
			total = -1;
			break;
		} else
			total += ret;
	}

	return total;
}

/*********************************************************************************  
* Description：  串口退出
* Input:         fd：设备描述符
* Output:        无
* Return:       无
* Others:       无。
**********************************************************************************/
int serial_exit(int fd)
{
	if (close(fd)) {
		perror("serial->exit");
		return -1;
	}

	return 0;
}
