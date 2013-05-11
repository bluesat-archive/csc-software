#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "rtc.h"
#include "lpc24xx.h"

rtc_time_t time;

void
rtc_init(void)
{
    RTC_AMR = 0;
    RTC_CIIR = 0;
    RTC_CCR = RTC_CCR_EXT_CLOCK_SET;
    RTC_PREINT = 0;
    RTC_PREFRAC = 0;
}

void
rtc_start(void)
{
    RTC_CCR |= RTC_CCR_START;
}

void
rtc_stop(void)
{
    RTC_CCR &= ~RTC_CCR_START;
}

void
rtc_reset(void)
{
    RTC_CCR |= RTC_CCR_RESET;
}

void
rtc_get_current_time(rtc_time_t *buf)
{
    buf->rtcSec = RTC_SEC;
    buf->rtcMin = RTC_MIN;
    buf->rtcHour = RTC_HOUR;
    buf->rtcMday = RTC_DOM;
    buf->rtcWday = RTC_DOW;
    buf->rtcYday = RTC_DOY;
    buf->rtcMon = RTC_MONTH;
    buf->rtcYear = RTC_YEAR;
}

void
rtc_set_current_time(rtc_time_t curTime)
{
    RTC_SEC = curTime.rtcSec;
    RTC_MIN = curTime.rtcMin;
    RTC_HOUR = curTime.rtcHour;
    RTC_DOM = curTime.rtcMday;
    RTC_DOW = curTime.rtcWday;
    RTC_DOY = curTime.rtcYday;
    RTC_MONTH = curTime.rtcMon;
    RTC_YEAR = curTime.rtcYear;
}
