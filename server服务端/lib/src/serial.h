#ifndef   __SERIAL_H__
#define  __SERIAL_H__

int serial_init(const char *devpath, int baudrate);
ssize_t serial_recv(int fd, void *buf, size_t count);
ssize_t serial_send(int fd, const void *buf, size_t count);
ssize_t serial_recv_exact_nbytes(int fd, void *buf, size_t count);
ssize_t serial_send_exact_nbytes(int fd, const void *buf, size_t count);

int serial_exit(int fd);

#endif
