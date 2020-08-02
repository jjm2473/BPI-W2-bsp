
#include "common.h"
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>

#define X_SOH 0x01
#define X_STX 0x02
#define X_ACK 0x06
#define X_NAK 0x15
#define X_CAN 0x18
#define X_EOT 0x04

struct chunk {
        uint8_t start;
        uint8_t block;
        uint8_t block_neg;
        uint8_t payload[128];
        uint16_t crc;
} __attribute__((packed));

struct chunk1k {
        uint8_t start;
        uint8_t block;
        uint8_t block_neg;
        uint8_t payload[1024];
        uint16_t crc;
} __attribute__((packed));

int send_raw_wait_ack(int tty_fd, const void* data, size_t size);
int send_chunk128(int seq, const char* data, int size, int tty_fd);
int send_chunk(int seq, const char* data, int size, int tty_fd);
int waitC(int tty_fd);

int ymodem(const char *file_name, int tty_fd) {
    char buf[1024];
    int err = -2;
    int rs;
    char eot = X_EOT;
    FILE *fd = NULL;
    time_t start_at = 0;
    if (waitC(tty_fd)) {
        return -2;
    }

    fd = fopen(file_name, "rb");
    if (fd == NULL) {
        printf("open %s failed: %s\n", file_name, strerror(errno)) ;
        return -1;
    }

    fseek(fd, 0, SEEK_END);
    long size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    char *p = buf;
    const char* name = strrchr(file_name, '/');
    if (name == NULL) {
        name = file_name;
    }
    strcpy(p, name);
    p += strlen(name);
    *(p++) = '\0';
    p += sprintf(p, "%ld", size);
    *p = '\0';

    printf("send header\n");
    err = send_chunk(0, buf, p - buf, tty_fd);

    if (err) {
        printf("send header failed: %d, %s\n", err, err<0?strerror(errno):"");
        goto cleanup;
    }

    err = waitC(tty_fd);

    if (err) {
        goto cleanup;
    }

    puts("send content ");
    fflush(stdout);
    time(&start_at);
    int seq = 0;
    while((rs = fread(buf, 1, 1024, fd)) >= 0) {
        if (rs > 0) {
            ++seq;
            err = send_chunk(seq, buf, rs, tty_fd);
            if (err) {
                printf("send_chunk failed: %d, %d, %s\n", err, errno, err<0?strerror(errno):"");
                goto cleanup;
            }
            fwrite("*", 1, 1, stdout);
            fflush(stdout);
        } else if (!feof(fd)) {
            err = -1;
            printf("read file failed: %s\n", strerror(errno));
            goto cleanup;
        } else {
            break;
        }
    }
    putc('\n', stdout);
    fclose(fd);
    fd = NULL;

    printf("%ld kb/s\n", size / (time(NULL) - start_at + 1) / 1024);

    printf("send EOT\n");
    // EOT
    err = send_raw_wait_ack(tty_fd, &eot, 1);
    if (err) {
        printf("EOT failed: %d, %s\n", err, err<0?strerror(errno):"");
        goto cleanup;
    }

    err = waitC(tty_fd);
    if (err) {
        goto cleanup;
    }

    memset(buf, 0, 128);
    err = send_chunk128(0, buf, 128, tty_fd);
    if (err) {
        printf("send eot chunk failed: %d, %s\n", err, err<0?strerror(errno):"");
        goto cleanup;
    }

    err = 0;

cleanup:
    if (fd) {
        fclose(fd);
    }
    return err;
}

int waitC(int tty_fd) {
    int rs;
    char res;
    time_t first_resp_at = 0;
    time(&first_resp_at);
    while(1) {
        rs = read(tty_fd, &res, 1);
        if (rs < 0) {
            printf("Read() error:%s\n", strerror(errno));
            return -2;
        } else {
            if (rs == 0 || res != 'C') {
                if (time(NULL) - first_resp_at > 1) {
                    printf("Expected 'C', but not found after 1 second!\n");
                    return -5;
                }
            } else {
                break;
            }
        }
    }
    return 0;
}

#define CRC_POLY 0x1021

static uint16_t crc_update(uint16_t crc_in, int incr)
{
    uint16_t xor = crc_in >> 15;
    uint16_t out = crc_in << 1;

    if (incr)
        out |= 1;

    if (xor)
        out ^= CRC_POLY;

    return out;
}

uint16_t crc16(const uint8_t* data, int size) {
    uint16_t crc, i;

    for (crc = 0; size > 0; --size, ++data)
        for (i = 0x80; i; i >>= 1)
            crc = crc_update(crc, *data & i);

    for (i = 0; i < 16; ++i)
        crc = crc_update(crc, 0);

    return crc;
}

static uint16_t swap16(uint16_t in)
{
    return (in >> 8) | ((in & 0xff) << 8);
}

int send_raw_wait_ack(int tty_fd, const void* data, size_t size) {
    char res;
    time_t first_resp_at = 0;
    int try = 2;
    int writen;
    const char *p;
    size_t remain;

    while (try--) {
        time(&first_resp_at);
        p = data;
        remain = size;
        while (remain) {
            writen = write(tty_fd, p, remain);
            if (writen < 0) {
                if (errno != EWOULDBLOCK) {
                    return -3;
                }
            } else if (writen > 0) {
                remain -= writen;
                p += writen;
            }
        }
        res = X_NAK;
        while (read(tty_fd, &res, 1) == 0 || res == 'C') {
            if (time(NULL) - first_resp_at > 10) {
                res = 0;
                break;
            }
        }
        if (res == X_ACK) {
            return 0;
        }
        if (res == X_CAN) {
            return X_CAN;
        }
        // NAK
    }
    return X_NAK;
}

int send_chunk128(int seq, const char* data, int size, int tty_fd) {
    struct chunk ck;
    ck.start = X_SOH;
    ck.block = seq & 0xff;
    ck.block_neg = ~ck.block;
    memset(ck.payload, 0, 128);
    memcpy(ck.payload, data, size);
    ck.crc = swap16(crc16(ck.payload, 128));
    return send_raw_wait_ack(tty_fd, &ck, 133);
}

int send_chunk1024(int seq, const char* data, int size, int tty_fd) {
    struct chunk1k ck;
    ck.start = X_STX;
    ck.block = seq & 0xff;
    ck.block_neg = ~ck.block;
    memset(ck.payload, 0, 1024);
    memcpy(ck.payload, data, size);
    ck.crc = swap16(crc16(ck.payload, 1024));
    return send_raw_wait_ack(tty_fd, &ck, 1029);
}

int send_chunk(int seq, const char* data, int size, int tty_fd) {
    if (size > 1024) {
        return 1;
    }
    if (size > 128) {
        return send_chunk1024(seq, data, size, tty_fd);
    } else {
        return send_chunk128(seq, data, size, tty_fd);
    }
}

int setNonblocking(int fd)
{
    int flags;

    /* If they have O_NONBLOCK, use the Posix way to do it */
#if defined(O_NONBLOCK)
    /* Fixme: O_NONBLOCK is defined but broken on SunOS 4.1.x and AIX 3.2.5. */
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    /* Otherwise, use the old way of doing it */
    flags = 1;
    return ioctl(fd, FIOBIO, &flags);
#endif
}   