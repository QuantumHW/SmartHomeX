#include "zigbee.h"

struct env data;

int zigbee_init( char *devpath, int baudrate)
{
    /*
    *   初始化串口设备,返回串口的设备文件描述符
    */
	return serial_init(devpath,baudrate);
}

int zigbee_get_dat(int fd)
{
	//ssize_t ret;
	int ret;
	unsigned char buf[36];
	
	memset(buf, 0, sizeof(buf));
    
    /*
    *   读取串口的环境信息到 buf 字符数组中，读取 36个字节
    */
	//ret=read(fd,buf,sizeof(buf));
	ret=serial_recv_exact_nbytes(fd,buf,sizeof(buf));
	if (ret == -1){
		return -1;
	}else if (ret == sizeof(buf)) {
#if 0
		int i;
		for (i = 0; i < 36; i++){
			printf("%.2x ", buf[i]);
		}
		printf("\n");
#endif
		if (buf[0] == 0xBB) {
			memset(&data, 0, sizeof(struct env));
			data.tem =  (int)buf[5] + buf[4] ;          //将得到的温度数值存储到 全局结构体中
			data.hum =  (int)buf[7] + buf[6] ;          //将得到的湿度信息存储到 全局结构体中
			data.light =  (int)((buf[23] << 24) + (buf[22] << 16) + (buf[21] << 8) + buf[20]);
		}
	}
	memset(buf, 0, sizeof(buf));
	return ret;
}

int zigbee_exe_cmd(int fd, char *p)
{
	unsigned char buf[ZGB_DATA_SIZE] = {0xdd, 0x06, 0x24, 0x00, 0x00};
int ret;
	if (strstr(p, "LIGHT_ON") != NULL){             //如果匹配到客户端发送的是 LIGHT_ON 则将 buf 数组的地 4位设置为 0x00，表示开灯
		buf[4] = 0x00; ret=write(fd,buf,36);
	} else if (strstr(p, "LIGHT_OFF") != NULL){
		buf[4] = 0x01; ret=write(fd,buf,36);
	} else if (strstr(p, "BUZZ_ON") != NULL){
		buf[4] = 0x02; ret=write(fd,buf,36);
	} else if (strstr(p, "BUZZ_OFF") != NULL){
		buf[4] = 0x03; ret=write(fd,buf,36);
	} else if (strstr(p, "FAN_ON") != NULL){
		buf[4] = 0x04; ret=write(fd,buf,36);
	} else if (strstr(p, "FAN_OFF") != NULL){
		buf[4] = 0x08; ret=write(fd,buf,36);
	} 
	else if (strstr(p, "REC_ON") != NULL){
ret=1;
		printf("REC_ON\n");
	} 
	else if (strstr(p, "REC_OFF") != NULL){
ret=1;
		printf("REC_OFF\n");
	} 
    /*
    *   将 buf 中的控制命令发送到串口中，进行控制 m0设备，发送 36 字节
    */
	//int ret=write(fd,buf,36);
	return ret;
}

int zigbee_exit(int fd)
{
	return serial_exit(fd);
}
