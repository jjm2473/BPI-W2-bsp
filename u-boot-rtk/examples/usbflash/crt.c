#include <linux/string.h>

extern void __bss_start__;
extern void __bss_end__;

void init_bss() {
    memset(&__bss_start__, 0, ((unsigned long)&__bss_end__) - ((unsigned long)&__bss_start__));
}
