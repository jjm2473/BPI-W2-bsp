
#include <linux/string.h>
#include "rtkemmc.h"

static unsigned char *dma_buf;

int emmc_init() {
    int ret_err = -1;
    set_emmc_pin_mux();
    set_emmc_freq();

    ret_err = rtk_eMMC_init();

    if (ret_err) {
        return ret_err;
    }

    dma_buf = (unsigned char *)REF_MALLOC_BASE;

    return ret_err;
}

/**
 * return block count readed, 0 for error
 */
int emmc_read(unsigned int blk_addr, unsigned int data_size, unsigned char *buffer) {
    unsigned int total_blk_cnt;
    unsigned int cur_blk_addr;
    UINT32 remain_blk_cnt;
    UINT32 cur_blk_cnt;
    UINT32 cur_offset;
    UINT32 cur_size;
    total_blk_cnt = data_size>>9;
	
    if(data_size & 0x1ff) {
        total_blk_cnt+=1;
    }
	remain_blk_cnt = total_blk_cnt;
    cur_blk_addr = blk_addr;
    cur_offset = 0;

    while (remain_blk_cnt) {
        if (remain_blk_cnt > 64)
            cur_blk_cnt = 64;
        else
            cur_blk_cnt = remain_blk_cnt;

        remain_blk_cnt -= cur_blk_cnt;
        cur_size = cur_blk_cnt << 9;
        if (!rtk_eMMC_read(cur_blk_addr, cur_size, dma_buf, 0)) {
            return 0;
        }
        memcpy(buffer + cur_offset, dma_buf, min(cur_size, data_size - cur_offset));
        cur_blk_addr += cur_blk_cnt;
        cur_offset += cur_size;
    }
    return total_blk_cnt;
}

/**
 * return block count writed, 0 for error
 */
int emmc_write(unsigned int blk_addr, unsigned int data_size, unsigned char *buffer) {
    unsigned int total_blk_cnt;
    unsigned int cur_blk_addr;
    UINT32 remain_blk_cnt;
    UINT32 cur_blk_cnt;
    UINT32 cur_offset;
    UINT32 cur_size;
    total_blk_cnt = data_size>>9;
	
    if(data_size & 0x1ff) {
        total_blk_cnt+=1;
    }
	remain_blk_cnt = total_blk_cnt;
    cur_blk_addr = blk_addr;
    cur_offset = 0;

    while (remain_blk_cnt) {
        if (remain_blk_cnt > 64)
            cur_blk_cnt = 64;
        else
            cur_blk_cnt = remain_blk_cnt;

        remain_blk_cnt -= cur_blk_cnt;
        cur_size = cur_blk_cnt << 9;

        memcpy(dma_buf, buffer + cur_offset, min(cur_size, data_size - cur_offset));
        if (!rtk_eMMC_write(cur_blk_addr, cur_size, dma_buf, 0)) {
            return 0;
        }

        cur_blk_addr += cur_blk_cnt;
        cur_offset += cur_size;
    }
    return total_blk_cnt;
}