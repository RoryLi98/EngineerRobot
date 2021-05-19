#pragma once
#include <iostream>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

typedef unsigned char uchar;

enum DATATYPE
{
    IMAGE,
    INFO
};

int openPort(const char *dev_name);
int configurePort(int fd);
bool sendXYZ(int fd, union data_send_float *data_send, char Flag, int cur_mode, char cnt);

class ControlReciever
{
public:
    ControlReciever();
    void set_fdcar(int fdcar);
    int engineer_reciever();
    int fd_car;
    int mode = 0;
    int status = 0;
};
