#include <pebble.h>

#define MAX_BANDS 50

typedef int time_tds;

void initDisplayTime();
void setStart(time_tds inStartTime);
bool getIsPaused();
bool togglePaused(time_tds inCurrentTime);
bool getIsFinished(time_tds inCurrentTime);
time_tds getTimeDiffFromPlanned();
time_tds getProjectedFinishTime(time_tds inCurrentTime);
int getDisplayTime(time_tds inCurrentTime);
int getCountDownBand(time_tds inCurrentTime);
int getPercentCompleteTimes10(time_tds inCurrentTime);
void setCountDownBands(time_tds * countDownBandPtr, int inNumBands);
time_tds getTotalTime();
bool getIsStarted();
time_tds getCurrentPlannedLapTime(time_tds inCurrentTime);
int getCounDownBands();
time_tds getTimeForBand(int inCountDownBand);
time_tds getCurrentRePlannedLapTime(time_tds inCurrentTime);
void setLastBandPercentOfMax(int inPercentage);
time_tds getActualTime(int inBandIndex);
