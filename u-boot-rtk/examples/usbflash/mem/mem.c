
#include "configs/rtd1295_qa_emmc_64.h"
#include <malloc.h>

void mem_init() {
    mem_malloc_init (CONFIG_HEAP_ADDR, CONFIG_SYS_MALLOC_LEN);
}