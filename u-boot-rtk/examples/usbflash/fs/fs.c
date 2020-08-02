
#include <linux/stddef.h>
#include "fs.h"

#define MAX_SEARCH_PARTITIONS 16

disk_partition_t *get_first_fat_part(block_dev_desc_t *dev, disk_partition_t *info) {
    int p;
    if (dev->part_type == PART_TYPE_UNKNOWN) {
        info->start = 0;
		info->size = dev->lba;
		info->blksz = dev->blksz;
		info->bootable = 0;
        if (!fat_set_blk_dev(dev, info)) {
            printf("whole disk is fat part\n");
            return info;
        }
    }

    for (p = 1; p <= MAX_SEARCH_PARTITIONS; p++) {
        if (get_partition_info(dev, p, info))
            continue;

        if (!fat_set_blk_dev(dev, info)) {
            printf("NO.%d is fat part\n", p);
            return info;
        }
    }

    return NULL;
}
