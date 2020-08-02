
#include "uart/uart.h"
#include "printf/printf.h"
#include "mem/mem.h"
#include "timer/timer.h"
#include "usb/usb.h"
#include "emmc/emmc.h"
#include "fs/fs.h"
#include "util.h"
#include <u-boot/crc.h>
#include <malloc.h>
#include <asm/arch/cpu.h>

typedef struct {
    int hwsetting_size;
    int uboot_size;
    int fsbl_size;
    int fsbl_os_size;
    int bl31_size;
    int rsa_fw_size;
    int rsa_tee_size;
    int rescue_size;
} BootcodeHeader;

#define DMA_ALIGN 512

static int bss_test;
// bss
//static unsigned char buf[100*1024*1024+DMA_ALIGN];

disk_partition_t *get_first_usb_fat_part(disk_partition_t *info) {
    block_dev_desc_t * dev = usb_stor_get_dev(0);
    if (dev == NULL) {
        return NULL;
    }
    return get_first_fat_part(dev, info);
}

int usbflash( int argc, char * const argv[] )
{
    unsigned char buf[101*1024*1024+DMA_ALIGN];
    unsigned char *dma = (unsigned char *)(((unsigned long)buf + (DMA_ALIGN-1)) & (~(DMA_ALIGN-1)));
    disk_partition_t info;
    init_uart();
    set_focus_uart(0); //default : uart0

    prints("prints hello world!\n");
    printf("printf hello world!\n");

    printf("cpu %d rev %d\n", get_cpu_id(), get_rtd129x_cpu_revision()>>16);
    if (bss_test != 0) {
        printf("bss not initinalized!\n");
        goto main_loop;
    }

    printf("buf.addr %#x dma.addr %#x\n", buf, dma);

    mem_init();

    void* al = malloc(1);
    printf("test malloc 0x%x\n", (int)al);
    if (!al) {
        printf("malloc failed!\n");
        goto main_loop;
    }

    free(al);

    al = memalign(4096, 4096);
    printf("test memalign 0x%x\n", (int)al);
    if (!al) {
        printf("memalign failed!\n");
        goto main_loop;
    }
    free(al);

    printf("timer_init\n");
    timer_init();
    printf("mdelay 2000\n");
    mdelay(2000);
    printf("get_timer\n");
    unsigned long t = get_timer(0);
    printf("time: %#x\n", t);

    // usb
    while(1) {
        if (usb_init()) {
            printf("usb_init failed!\n");
            goto main_loop;
        }
        usb_stor_reset();
        if (!usb_stor_scan(1)) {
            break;
        }
        usb_stop();
        printf("no usb storage device found, rescan after 5 seconds!\n");
        mdelay(5000);
    }

    if (!get_first_usb_fat_part(&info)) {
        printf("no fat partition in first usb storage device!\n");
        goto main_loop;
    }
    printf("'ABCD' checksum %#x\n", crc32(0, "ABCD", 4));

    // emmc
    if (emmc_init()) {
        prints("emmc_init failed!\n");
        goto main_loop;
    }

    t = get_timer(0);
    loff_t backup_size;
    if (fat_read_file("fwbackup.img", dma, 0, 101*1024*1024, &backup_size)) {
        goto main_loop;
    }

    printf("readed %lu bytes in %ld ms\n", backup_size, get_timer(0) - t);

    if (backup_size <= 0) {
        prints("ERROR: fwbackup.img is empty, failed!\n");
        goto main_loop;
    }

    if (backup_size > 100*1024*1024) {
        prints("ERROR: fwbackup.img bigger than 100MB, failed!\n");
        goto main_loop;
    }

    uint32_t checksum = crc32(0, dma, backup_size);
    printf("fwbackup.img checksum %#x\n", checksum);

    prints("write to emmc...\n");
    if (!emmc_write(0, backup_size, dma)) {
        prints("emmc_write failed!\n");
        goto main_loop;
    }

    memset(dma, 0, backup_size);
    prints("read back from emmc...\n");
    if (!emmc_read(0, backup_size, dma)) {
        prints("emmc_read failed!\n");
        goto main_loop;
    }

    uint32_t checksum2 = crc32(0, dma, backup_size);
    if (checksum ^ checksum2) {
        printf("crc32 check failed! fwbackup.img:%#x emmc:%#x\n", checksum, checksum2);
        goto main_loop;
    } else {
        prints("crc32 check passed!\n");
    }

    BootcodeHeader *header = (BootcodeHeader*)(dma+(0x100<<9));
    int pos = 0x20060;
    int uboot_blk = (pos + header->hwsetting_size + 32 + 511)/512;
    int uboot_size = header->uboot_size;

    printf("uboot blk.no=0x%08x size=0x%08x\n", uboot_blk, uboot_size);

    if (!memcpy((unsigned char *)0x20000, dma+(uboot_blk<<9), uboot_size)) {
        prints("load uboot failed!\n");
        goto main_loop;
    }
    // invoke uboot
    //((void(*)())0x20000)();

main_loop:
    usb_stop();
    rtk_usb_power_off();

    while(1);
    return 0;
}
