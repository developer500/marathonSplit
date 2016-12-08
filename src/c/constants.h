#include <pebble.h>

#define MAX_BANDS 50

typedef int time_tds;

time_tds getTimeDs();
time_t getTimeFromDs(time_tds inTimeDs);
time_t getTimeDsFromTime(time_t inTime, uint16_t inMillis); 