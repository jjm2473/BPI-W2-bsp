#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include "common.h"

static int sendfile(const char* cmd, const char* filename, int tty_fd, int waitsec);

int main(int argc, const char **argv)
{
    int tty_fd = -1 ;
    int rv = -1, rs = 0;
    struct termios options;
    const char *dev;
    const char *hwsetting;
    const char *dvrboot;
    const char *loadaddr = "01500000";
    int esc = 0;
    char loadaddrb[9];
    char buf[1024];
    time_t first_resp_at = 0;
    int err = -2;
    int i;
	if (argc < 4) {
		printf("Usage:\t onchip /dev/tty.usbserial-0001 hwsetting.bin dvrboot.exe.bin [loadaddr] [ESC]\n\n"
        "  loadaddr: load address (hex) for dvrboot.exe.bin, default 01500000\n"
        "  ESC: send ESC when dvrboot starting\n");
		return -1;
	}

    dev = argv[1];
    hwsetting = argv[2];
    dvrboot = argv[3];
    if (argc > 4) {
        loadaddr = argv[4];
        if (loadaddr[0] == 'x') {
            ++loadaddr;
        } else if (loadaddr[1] == 'x') {
            loadaddr += 2;
        }
        rs = strlen(loadaddr);
        if (rs > 8) {
            printf("loadaddr format error!\n");
            return -1;
        }
        memset(loadaddrb, '0', 8);
        loadaddrb[8] = '\0';
        strcpy(loadaddrb + (8 - rs), loadaddr);
        loadaddr = loadaddrb;
    }

    if (argc > 5) {
        esc = 1;
    }

    tty_fd = open(dev, O_RDWR|O_NOCTTY|O_NDELAY) ; //打开串口设备
    if(tty_fd < 0)
    {
        printf("open tty %s failed: %s\n", dev, strerror(errno)) ;
        return -1;
    }
    printf("open devices sucessful!\n");
    
    memset(&options, 0, sizeof(options)) ;
    rv = tcgetattr(tty_fd, &options); //获取原有的串口属性的配置
    if(rv != 0)
    {
        printf("tcgetattr() failed: %s\n",strerror(errno));
        err = -2;
        goto cleanup;
    }
    options.c_cflag |= (CLOCAL|CREAD ); // CREAD 开启串行数据接收，CLOCAL并打开本地连接模式
    options.c_cflag &= ~CSIZE;// 先使用CSIZE做位屏蔽
    options.c_cflag |= CS8; //设置8位数据位
    options.c_cflag &= ~PARENB; //无校验位

    /* 设置115200波特率  */
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);

    options.c_cflag &= ~CSTOPB;/* 设置一位停止位; */

    options.c_cc[VTIME] = 0;/* 非规范模式读取时的超时时间；*/
    options.c_cc[VMIN]  = 0; /* 非规范模式读取时的最小字符数*/
    tcflush(tty_fd ,TCIFLUSH);/* tcflush清空终端未完成的输入/输出请求及数据；TCIFLUSH表示清空正收到的数据，且不读取出来 */

    if((tcsetattr(tty_fd, TCSANOW, &options))!=0)
    {
        printf("tcsetattr failed:%s\n", strerror(errno));
        err = -2;
        goto cleanup;
    }

    // send 'ctrl+q' s, wait for chip response
    while(1)
    {
        rv = write(tty_fd, "\x11\x11\x11\x11\x11", 5);
        if(rv < 0)
        {
            printf("Write() error:%s\n", strerror(errno));
            err = -2;
            goto cleanup;
        }
        fwrite("^Q", 2 , 1, stdout);
        fflush(stdout);
        usleep(25000);
        rs = read(tty_fd, buf, 1024);
        if (rs < 0) {
            printf("Read() error:%s\n", strerror(errno));
            err = -2;
            goto cleanup;
        }
        if (rs > 0) {
            if (first_resp_at == 0) {
                time(&first_resp_at);
            }
            fwrite(buf, rs ,1, stdout);
            fflush(stdout);
        }
        usleep(25000);
        if (rs >= 7 && memcmp(buf+rs-7, "\nd/g/r>", 7) == 0) {
            break;
        } else if (first_resp_at != 0) {
            if (time(NULL) - first_resp_at > 1) {
                printf("Unexpected response!\n");
                err = 3;
                goto cleanup;
            }
        }
    }

    err = sendfile("h", hwsetting, tty_fd, 1);
    if (err) {
        goto cleanup;
    }

    puts("set load address\n");
    puts("s\n");
    write(tty_fd, "s", 1);
    usleep(25000);
    puts("98007058\n");
    write(tty_fd, "98007058\n", 9);
    usleep(25000);
    puts(loadaddr);
    puts("\n");
    write(tty_fd, loadaddr, 8);
    write(tty_fd, "\n", 1);

    puts("address sent, wait device\n");
    for (i=100; i>0; --i) {
        rs = read(tty_fd, buf, 1024);
        if (rs > 0) {
            fwrite(buf, rs ,1, stdout);
            fflush(stdout);
            if (rs >= 7 && memcmp(buf+rs-7, "\nd/g/r>", 7) == 0) {
                break;
            }
        }
        usleep(10000);
    }
    if (i==0) {
        err = 5;
        goto cleanup;
    }
    puts("device ready\n");

    err = sendfile("d", dvrboot, tty_fd, 2);
    if (err) {
        goto cleanup;
    }

    puts("go\n");
    puts("g\n");
    write(tty_fd, "g", 1);
    while (1) {
        rs = read(tty_fd, buf, 1024);
        if (rs < 0) {
            printf("Read() error:%s\n", strerror(errno));
            return 1;
        } else if (rs > 0) {
            fwrite(buf, rs , 1, stdout);
            fflush(stdout);
        }
        if (esc) {
            // ESC
            write(tty_fd, "\x1b", 1);
        }
        usleep(25000);
    }

    err = 0;

cleanup:
    close(tty_fd);
    return err;
}

static int sendfile(const char* cmd, const char* filename, int tty_fd, int waitsec) {
    char buf[1024];
    int rs = 0;
    time_t first_resp_at = 0;
    int i;
    puts(cmd);
    puts("\n");
    write(tty_fd, cmd, strlen(cmd));
    time(&first_resp_at);
    while (1) {
        usleep(25000);
        rs = read(tty_fd, buf, 1024);
        if (rs < 0) {
            printf("Read() error:%s\n", strerror(errno));
            return -2;
        } else if (rs > 0) {
            if (buf[rs-1] != 'C') {
                if (time(NULL) - first_resp_at > 1) {
                    printf("Expected 'C', but '%c'!\n", buf[0]);
                    return 5;
                }
            }
            break;
        }
    }

    puts("start send file\n");
    i = ymodem(filename, tty_fd);
    if (i) {
        return i;
    }

    puts("file sent, wait device\n");
    for (i = 100*waitsec; i>0; --i) {
        rs = read(tty_fd, buf, 1024);
        if (rs < 0) {
            printf("Read() error:%s\n", strerror(errno));
            return -2;
        } else if (rs > 0) {
            fwrite(buf, rs ,1, stdout);
            fflush(stdout);
            if (rs >= 7 && memcmp(buf+rs-7, "\nd/g/r>", 7) == 0) {
                break;
            }
        }
        usleep(10000);
    }
    if (i == 0) {
        puts("timeout!\n");
        return 5;
    }
    puts("device ready\n");
    return 0;
}