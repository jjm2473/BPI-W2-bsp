#ifndef FS_H
#define FS_H
typedef unsigned char		uchar;
#include <part.h>
#include "fat.h"

disk_partition_t *get_first_fat_part(block_dev_desc_t *dev, disk_partition_t *output);

#endif