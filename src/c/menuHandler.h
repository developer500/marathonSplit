#include <pebble.h>

MenuLayer * loadMenu(Window *window);
void lapTimes_window_unload(Window *window);
void setTimes(time_tds plannedTime[], time_tds actualTime[], int inNumTimes);
int  getTimeFromTimeT(time_tds inTime, bool inAddPlus, char * outPlusTime);