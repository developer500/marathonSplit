#pragma once
#include <pebble.h>

#define MAX_BANDS 90

#define MAIN_WINDOW_INDEX 0
#define PROJECTED_WINDOW_INDEX 1
#define LAP_TIMES_WINDOW_INDEX 2
#define YES_NO_WINDOW_INDEX 3
#define REPLACE_DATA_WINDOW_INDEX 4
#define CONFIRMED_WINDOW_INDEX 5
#define ADD_LAPS_WINDOW_INDEX 6
#define NUM_WINDOWS (ADD_LAPS_WINDOW_INDEX + 1)

static bool s_blackBackground = true;

typedef int time_tds;

time_tds getTimeDs();
time_t getTimeFromDs(time_tds inTimeDs);
time_tds getTimeDsFromTime(time_t inTime, uint16_t inMillis);
