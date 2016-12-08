#include <pebble.h>
#include "constants.h"

// 1 Jan 2016 00:00
time_t epochTime = (time_t)0x5685c180;

time_tds getTimeDs() {
  
  time_t currentTime = time(NULL);
  
  uint16_t millis = time_ms(NULL, NULL);
  
  time_tds retTime = getTimeDsFromTime(currentTime, millis);
      
  return retTime;
}

time_t getTimeFromDs(time_tds inTimeDs) {
  
  return (time_t) ((inTimeDs / 10) + epochTime);
}

time_t getTimeDsFromTime(time_t inTime, uint16_t inMillis) {
  
  time_t timeSinceEpoch = inTime - epochTime;
  
  return (time_tds)(timeSinceEpoch*10) + (time_tds)(inMillis/100);
  
}