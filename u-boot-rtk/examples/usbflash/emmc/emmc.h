#ifndef EMMC_H
#define EMMC_H

int emmc_init();

/**
 * return block count readed, 0 for error
 */
int emmc_read(unsigned int blk_addr, unsigned int data_size, unsigned char *buffer);

/**
 * return block count writed, 0 for error
 */
int emmc_write(unsigned int blk_addr, unsigned int data_size, unsigned char *buffer);

#endif