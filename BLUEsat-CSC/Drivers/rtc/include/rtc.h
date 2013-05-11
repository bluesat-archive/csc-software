#ifndef RTC_H_
#define RTC_H_

#define RTC_CCR_START         (0x1)
#define RTC_CCR_RESET         (0x2)
#define RTC_CCR_EXT_CLOCK_SET (1 << 4)

typedef struct {
    unsigned int rtcSec;     /* Second value - [0,59] */
    unsigned int rtcMin;     /* Minute value - [0,59] */
    unsigned int rtcHour;    /* Hour value - [0,23] */
    unsigned int rtcMday;    /* Day of the month value - [1,31] */
    unsigned int rtcMon;     /* Month value - [1,12] */
    unsigned int rtcYear;    /* Year value - [0,4095] */
    unsigned int rtcWday;    /* Day of week value - [0,6] */
    unsigned int rtcYday;    /* Day of year value - [1,365] */
} rtc_time_t;

void rtc_init(void);

void rtc_start(void);

void rtc_stop(void);

void rtc_reset(void);

void rtc_get_current_time(rtc_time_t *buf);

void rtc_set_current_time(rtc_time_t curTime);

#endif /* RTC_H_ */
