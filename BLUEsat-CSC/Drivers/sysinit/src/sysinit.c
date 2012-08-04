/**
 *  \file sysinit.c
 *
 *  \brief System initialisation implementation
 */

#include "irq.h"
#include "sysinit.h"

static void sys_interrupt_init(void)
{
    init_VIC();
}

void sys_init(void)
{
    sys_interrupt_init();
}
