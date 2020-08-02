#include "cache.h"

static inline unsigned int current_el(void)
{
    unsigned int el;
    asm volatile("mrs %0, CurrentEL" : "=r" (el) : : "cc");
    return el >> 2;
}

static inline void set_sctlr(unsigned int val)
{
    unsigned int el;

    el = current_el();
    if (el == 1)
        asm volatile("msr sctlr_el1, %0" : : "r" (val) : "cc");
    else if (el == 2)
        asm volatile("msr sctlr_el2, %0" : : "r" (val) : "cc");
    else
        asm volatile("msr sctlr_el3, %0" : : "r" (val) : "cc");

    asm volatile("isb");
}

static inline unsigned int get_sctlr(void)
{
    unsigned int el, val;
    el = current_el();

    if (el == 1)
        asm volatile("mrs %0, sctlr_el1" : "=r" (val) : : "cc");
    else if (el == 2)
        asm volatile("mrs %0, sctlr_el2" : "=r" (val) : : "cc");
    else
        asm volatile("mrs %0, sctlr_el3" : "=r" (val) : : "cc");
    return val;
}

void disable_dcache(void)
{
    unsigned int sctlr;
    sctlr = get_sctlr();

    /* if cache isn't enabled no need to disable */
    if (!(sctlr & CR_C) && !(sctlr & CR_M))
        return;

    set_sctlr(sctlr & ~(CR_C|CR_M));
    __flush_cache_all();
    __invalidate_tlb_all();
}

void flush_dcache_range(unsigned long start, unsigned long stop) {

}

void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
    
}