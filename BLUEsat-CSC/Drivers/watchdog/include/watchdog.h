#ifndef WATCHDOG_H_
#define WATCHDOG_H_

#define WDEN        0x00000001
#define WDRESET     0x00000002
#define WDTOF       0x00000004
#define WDINT       0x00000008

#define WDT_FEED_VALUE 0x00078000

void watchdog_init(void);

void watchdog_feed(void);

#endif /* WATCHDOG_H_ */
