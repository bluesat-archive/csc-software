#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "watchdog.h"
#include "lpc24xx.h"
#include "debug.h"
#include "irq.h"
#include "vic.h"

void
watchdog_init(void)
{
    WDCLKSEL = 0x2;

    WDTC = WDT_FEED_VALUE;
    WDMOD = WDEN | WDRESET; /* once WDEN is set, the WDT will start after feeding */

    WDFEED = 0xAA;        /* Feeding sequence */
    WDFEED = 0x55;
}

void
watchdog_feed(void)
{
    WDFEED = 0xAA; /* Feeding sequence. */
    WDFEED = 0x55;
}
