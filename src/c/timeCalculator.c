#include <pebble.h>
#include "timeCalculator.h"

static time_tds s_startTime;
static time_tds s_realStartTime;
static time_tds s_pauseTime;


static bool s_finishedOneSet;
static time_tds s_waitAfterFinish = (time_tds)20;  // wait 20 seconds after the projected finish to assume the set has finished

static int s_lastConfirmedBand;
\
static int s_lastBandPercentOfMax = 100;
static time_tds s_actualTimes[MAX_BANDS];

//static int s_countDownBands = 16;
//static time_tds s_countDownBand[MAX_BANDS] = {
//   1340, 1340, 1340, 1340,
//   1340, 1340, 1340, 1340,
//   1340, 1340, 1340, 1340,
//   1340, 1340, 1340, 1340,
//};


// static int s_countDownBands = 32;
// static time_tds s_countDownBand[MAX_BANDS] = {
//   1340, 1340, 1340, 1340,
//   1340, 1340, 1340, 1340,
//   1340, 1340, 1340, 1340,
//   1340, 1340, 1340, 1340,
//   1340, 1340, 1340, 1340,
//   1340, 1340, 1340, 1340,
//   1340, 1340, 1340, 1340,
//   1340, 1340, 1340, 1340,
// };

// static int s_countDownBands = 7;
// static time_tds s_countDownBand[MAX_BANDS] = {
//  1200, 1200, 1200, 1200,
//  1157, 1157, 1157
// };


// static int s_countDownBands = 27;
// static time_tds s_countDownBand[MAX_BANDS] = {
//  4800, 4800, 4630, 4630,
//  4630, 4630, 4630, 4630,
//  4630, 4630, 4630, 4630,
//  4630, 4630, 4630, 4630,
//  4630, 4630, 4630, 4630,
//  4630, 4630, 4630, 4630,
//  4630, 4630, 930
// };


// static int s_countDownBands = 3;
// static time_tds s_countDownBand[MAX_BANDS] = {150, 150, 150};

//static int s_countDownBands = 10;
//static time_tds s_countDownBand[MAX_BANDS] = {10,10,10,10,10,10,10,10,10,10};

// static int s_countDownBands = 48;
// static time_tds s_countDownBand[MAX_BANDS] = {
//  1335, 1335, 1335, 1335, 1335,
//  1335, 1335, 1335, 1335, 1335,
//  1335, 1335, 1335, 1335, 1335,
//  1335, 1335, 1335, 1335, 1335,
//  1335, 1335, 
//  801,
//  1335,
  
//  1335, 1335, 1335, 1335, 1335,
//  1335, 1335, 1335, 1335, 1335,
//  1335, 1335, 1335, 1335, 1335,
//  1335, 1335, 1335, 1335, 1335,
//  1335, 1335, 
//  801,
//  1335
// };

// static int s_countDownBands = 33;
// static time_tds s_countDownBand[MAX_BANDS] = {
//  538, 538, 538, 538, 538,    // 2.5 laps = 1000m @  4:29 for 1000 = 2690sec = 2690/5 = 538 for 200m
//  668,                        // 200 m recovery
//  538, 538, 538, 538, 538,    // 2.5 laps = 1000m @  4:29 for 1000 = 2690sec = 2690/5 = 538 for 200m
//  668,                        // 200 m recovery
//  538, 538, 538, 538, 538,    // 2.5 laps = 1000m @  4:29 for 1000 = 2690sec = 2690/5 = 538 for 200m
//  668,                        // 200 m recovery
//  538, 538, 538, 538, 538,    // 2.5 laps = 1000m @  4:29 for 1000 = 2690sec = 2690/5 = 538 for 200m

//  490, 490, 490, 490,         // 2 laps = 800m @  4:07 for 1000 = 1960sec = 1960/4 = 490 for 200m
//  1336,                       // 400 m recovery

//  490, 490, 490, 490,         // 2 laps = 800m @  4:07 for 1000 = 1960sec = 1960/4 = 490 for 200m
//  1336,                       // 400 m recovery
// };

// static int s_countDownBands = 19;
// static time_tds s_countDownBand[MAX_BANDS] = {
//  575, 575, 575, 575,     // 2 laps
//  900,                    // 200 m recovery
//  575, 575, 575, 575,     // 2 laps
//  900,                    // 200 m recovery
//  575, 575, 575, 575,     // 2 laps
//  900,                    // 200 m recovery
//  575, 575, 575, 575,     // 2 laps
// };

static int s_countDownBands = 28;
static time_tds s_countDownBand[MAX_BANDS] = {
  115, 115, 110, 110,
  110, 110, 110, 110,
  110, 110, 110, 110,
  110, 110, 110, 110,
  110, 110, 110, 110,
  110, 110, 110, 110,
  110, 110, 110, 110
};

//                                                    463, 463, 463, 463, 463, 463, 463,
//                                                    458, 458, 458, 458, 458, 458, 458, 458,
//                                                    451, 451, 451, 451, 451, 451, 451, 451, 451,
//                                                    96};

//static int s_countDownBands = 14;
//static time_tds s_countDownBand[MAX_BANDS] = {460, 460, 440, 440, 440,
//                                                    440, 440, 440, 440, 440,
//                                                    440, 440, 440, 48};
//static int s_countDownBands = 8;
//static time_tds s_countDownBand[MAX_BANDS] = {134,134,134,134,
//                                                    134,134,134,134};

/*
static int s_countDownBands = 28;
static time_tds s_countDownBand[MAX_BANDS] = {115,115,110,110,
                                                   134,134,134,134,
                                                   134,134,134,134,
                                                   134,134,134,134,
                                                   134,134,134,134,
                                                   134,134,134,134,
                                                   134,134,134,134,
                                                   134};
static int s_countDownBands = 26;
static time_tds s_countDownBand[MAX_BANDS] = {116, 116, 111, 111,
                                                   134, 134, 134, 134,
                                                   134, 134, 134, 134,
                                                   134, 134, 134, 134,
                                                   134, 134, 134, 134,
                                                   134, 134, 134, 134,
                                                   134, 134};
*/

static time_tds s_totalBandTime;

// required to forward declare.
bool clearPausedFn(time_tds inCurrentTime);

static void (*startFunctionP)(time_tds inCurrentTime);

static bool (*pauseFunctionP)(time_tds inCurrentTime);

int getCounDownBands() {
  return s_countDownBands;
}

time_tds getTimeForBand(int inCountDownBand) {
  return s_countDownBand[inCountDownBand];
}

void setCountDownBands(time_tds countDownBandPtr[], int inNumBands) {

  s_countDownBands = inNumBands;

  for (int bandIndex = 0; bandIndex < inNumBands; bandIndex++) {
    s_countDownBand[bandIndex] = *countDownBandPtr;
    countDownBandPtr++;
  }
}

void setLastBandPercentOfMax(int inPercentOfMax) {
  s_lastBandPercentOfMax = inPercentOfMax;
}

bool getIsStarted() {
  return s_startTime != 0;
}

bool getIsPaused() {
  return s_pauseTime != 0;
}

time_tds getElapsedTime(time_tds inCurrentTime) {
  
   if (!getIsStarted()) {
     return (time_tds)0;
   }
  
  time_tds absoluteCurrentTime = getIsPaused() ? s_pauseTime : inCurrentTime;  // eg 4:13:02  - current time or paused time.
    
  return absoluteCurrentTime - s_startTime;                // eg 0:00:12  - 12 seconds since start was pressed.
}

int getCurrrentBandAndSmallElapsed(time_tds inCurrentTime, time_tds* inOutSmallElapsd, bool roundForwards) {
  
  time_tds smallelapsedTime = getElapsedTime(inCurrentTime);  
  
  time_tds roundedTotalBandTime = s_totalBandTime * (smallelapsedTime / s_totalBandTime);

  smallelapsedTime -= smallelapsedTime > roundedTotalBandTime ? roundedTotalBandTime : 0;

  int bandIndex = 0;

  while (smallelapsedTime > s_countDownBand[bandIndex]) {
    smallelapsedTime -= s_countDownBand[bandIndex];
    bandIndex++;
    bandIndex %= s_countDownBands;
  }
  
  if (roundForwards) {
    if (smallelapsedTime == s_countDownBand[bandIndex]) {
      bandIndex++;
      bandIndex %= s_countDownBands;
    }
  }
  
  *inOutSmallElapsd = smallelapsedTime;

  return bandIndex;
}

time_tds getRemaingAndBandTime(time_tds inCurrentTime, time_tds (*inFunction)(time_tds inRemainingTime, int inBandIndex)) {
  
  if (!getIsStarted()) {
    return inFunction(s_countDownBand[0], 0);
  }

  time_tds smallElapsed = 0;
  int bandIndex = getCurrrentBandAndSmallElapsed(inCurrentTime, &smallElapsed, false);

  time_tds currentBandTime = s_countDownBand[bandIndex];

  // if the bands are {4,3} then at t=0,7,14 elapsed will be 0 - when this happens make remaining 0
  time_tds remainingTime = currentBandTime - smallElapsed;  

  return inFunction(remainingTime, bandIndex);
}

time_tds getTimeDeltaFn(time_tds inRemainingTime, int inBandIndex) {

  time_tds deltaTime;

  time_tds currentBandTime = s_countDownBand[inBandIndex];

  if (inRemainingTime <= (currentBandTime / 2)) {
    // jump forwards
    deltaTime = -1 * inRemainingTime;
  } else {
    deltaTime = currentBandTime - inRemainingTime;
  }

  return deltaTime;
}

void setPaused(time_tds inCurrentTime) {
  s_pauseTime = inCurrentTime;
}

void clearPaused() {
  s_pauseTime = 0;
}

int getIndexOfFirstZeroActualTime() {
  
  for (int bandIndex = 0; bandIndex < MAX_BANDS ; bandIndex++) {
    if (s_actualTimes[bandIndex]==(time_tds)0) {
      return bandIndex;
    }
  }
  // could not be found.
  return -1;
}

time_tds getPreviousTotalActualTime(int inFirstZeroBandIndex) {
  
  time_tds retTime = (time_tds)0;
  for (int bandIndex = 0; bandIndex < inFirstZeroBandIndex ; bandIndex++) {
    retTime += s_actualTimes[bandIndex];
  }
  
  return retTime;
}

void populateActualTimes(time_tds inCurrentTime, int inHighestBandToFillIn) {
    
  int firstZeroBand = getIndexOfFirstZeroActualTime();
  if (firstZeroBand==-1) {
    return;
  }
      
  int bandsToFilltimes100 = (inHighestBandToFillIn - firstZeroBand + 1)*100;
  if (bandsToFilltimes100<=0) {
    // this happens if the user presses start button twice in short time.
    return;
  }
  
  bandsToFilltimes100 -= (inHighestBandToFillIn==(s_countDownBands-1)) ? (100 - s_lastBandPercentOfMax) : 0;
    
  time_tds prevTotalTime = getPreviousTotalActualTime(firstZeroBand);
    
  time_tds timeToDistribute = inCurrentTime - s_realStartTime - prevTotalTime;
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Time to distribute %d", timeToDistribute);
    
  time_tds timeEachBand = timeToDistribute*100 / bandsToFilltimes100;
  time_tds remaindertimes100 = timeToDistribute*100 % bandsToFilltimes100;

  while (timeToDistribute>0) {
    time_tds timeWithRoundUp = (remaindertimes100>0) ? (timeEachBand+1) : timeEachBand;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Time With Round Up %d", timeWithRoundUp);
    time_tds timeForBand = timeWithRoundUp > timeToDistribute ? timeToDistribute : timeWithRoundUp;
    s_actualTimes[firstZeroBand] = timeForBand;
    timeToDistribute -= timeForBand;  
    remaindertimes100-=100;
    firstZeroBand++;
 }
}

void runningStartFn(time_tds inCurrentTime) {

  s_startTime += getRemaingAndBandTime(inCurrentTime, getTimeDeltaFn);
    
  time_tds smallElapsed = 0;
  s_lastConfirmedBand = getCurrrentBandAndSmallElapsed(inCurrentTime, &smallElapsed, false);
    
  populateActualTimes(inCurrentTime, s_lastConfirmedBand);
  
  // Did the user press reset which caused the clock to jump forwards and finish the set?
  if (getElapsedTime(inCurrentTime) >= s_totalBandTime) {
    s_finishedOneSet = true;
    setPaused(inCurrentTime);
    return;
  }        
  
  clearPaused();
}

void initialsetStartFn(time_tds inCurrentTime) {
    
  s_startTime = inCurrentTime;
  s_realStartTime = inCurrentTime;
  clearPaused();
  startFunctionP = &runningStartFn;
}

bool initialPausedFn(time_tds inCurrentTime) {

  setPaused(inCurrentTime);
  pauseFunctionP = &clearPausedFn;
  return true;
}

bool clearPausedFn(time_tds inCurrentTime) {

  time_tds timeElapsedWhenPaused = inCurrentTime - s_pauseTime;
  s_startTime += timeElapsedWhenPaused;
  s_realStartTime += timeElapsedWhenPaused;
  clearPaused();
  pauseFunctionP = &initialPausedFn;
  
  return false;
}

time_tds getActualTime(int inBandIndex) {
  return s_actualTimes[inBandIndex];
}

void initDisplayTime() {
  
  s_startTime = 0;
  s_realStartTime = 0;
  s_pauseTime = 0;

  s_finishedOneSet = false;
  s_totalBandTime = 0;
  
  s_lastConfirmedBand = -1;

  for (int bandIndex = 0; bandIndex < s_countDownBands; bandIndex++) {
    s_totalBandTime += s_countDownBand[bandIndex];
    s_actualTimes[bandIndex] = (time_tds)0;
  }
  
  startFunctionP = &initialsetStartFn;
  
  pauseFunctionP = &initialPausedFn;
}

bool togglePaused(time_tds inCurrentTime) {
  return (*pauseFunctionP)(inCurrentTime);
}

time_tds getCurrentPlannedLapTime(time_tds inCurrentTime) {
  
  time_tds smallElapsed = 0;
  int bandIndex = getCurrrentBandAndSmallElapsed(inCurrentTime, &smallElapsed, true);
  
  return s_countDownBand[bandIndex];
}

time_tds getTimeDiffFromPlanned() {
  return (s_startTime - s_realStartTime);
}

time_tds getCurrentRePlannedLapTime(time_tds inCurrentTime) {
  
  time_tds smallElapsed = 0;
  int bandIndex = getCurrrentBandAndSmallElapsed(inCurrentTime, &smallElapsed, true);
  
  // If the diff time from planned is +ve then runner is late
  // and seconds have to be subtracted to get back on time.
  time_tds timeDifferencetimes100 = -100 * getTimeDiffFromPlanned();
      
  int bandsRemaining = s_countDownBands - bandIndex;
  
  int bandsRemainingtimes100 = (bandsRemaining - 1)*100;
  if (bandsRemainingtimes100<0) {bandsRemainingtimes100=0;}
  
  bandsRemainingtimes100 += (bandsRemaining == 1) ? 100 : s_lastBandPercentOfMax;
  
  time_tds timeMakeUpThisBand = timeDifferencetimes100 / bandsRemainingtimes100;
    
  if ((bandsRemaining > 2) && (timeMakeUpThisBand < (time_tds)-20)) {
    // runner is not going to make it - instead return the original target
    return s_countDownBand[bandIndex];
  }
  
  time_tds newReplannedLapTime = s_countDownBand[bandIndex] + timeMakeUpThisBand;

  if (newReplannedLapTime < 0) {
    return s_countDownBand[bandIndex];
  }
  
  return newReplannedLapTime;
}

int getCountDownBand(time_tds inCurrentTime) {
  
  time_tds smallElapsed = 0;
  int bandIndex = getCurrrentBandAndSmallElapsed(inCurrentTime, &smallElapsed, false);
  
  return s_countDownBands - bandIndex;
}

time_tds getRealElapsedTime(time_tds inCurrentTime) {
  
  if (!getIsStarted()) {
     return (time_tds)0;
   }
  
  time_tds absoluteCurrentTime = getIsPaused() ? s_pauseTime : inCurrentTime;  // eg 4:13:02  - current time or paused time.
  return absoluteCurrentTime - s_realStartTime;                // eg 0:00:12  - 12 seconds since start was pressed.
}

bool getIsFinished(time_tds inCurrentTime) {
  
  if (s_finishedOneSet) {
    return true;
  }
  
  if (!getIsStarted()) {
    return false;
  }
  
  if (getElapsedTime(inCurrentTime) >= (s_totalBandTime + s_waitAfterFinish)) {
    // This causes it to be finished
    setStart(inCurrentTime);
    return true;
  }    
  
  return false;
}

time_tds getTotalTime() {
  return s_totalBandTime + getTimeDiffFromPlanned();
}

time_tds getProjectedFinishTime(time_tds inCurrentTime) {
  
  time_tds smallElapsed = 0;
  int bandIndex = getCurrrentBandAndSmallElapsed(inCurrentTime, &smallElapsed, true);
  
  time_tds projectedFinishTime = getRealElapsedTime(inCurrentTime);
  for (; bandIndex < s_countDownBands; bandIndex++) {
    projectedFinishTime += s_countDownBand[bandIndex];
  }
  
  return projectedFinishTime;
}

time_tds getDisplayTimeFn(time_tds inRemainingTime, int inBandIndex) {
  
  // is the runner late getting to the split?
  if (inBandIndex > 0) {
    if ((inBandIndex-1) > s_lastConfirmedBand) {
      time_tds currentBandTime = s_countDownBand[inBandIndex];
    
      time_tds smallElapsed = currentBandTime - inRemainingTime;
      
      // is this 8% of the previous band?
      time_tds previousBandTimeFraction = s_countDownBand[inBandIndex-1] * 8 / 100;
      
      // or 10 whichever is higher
      if (previousBandTimeFraction<10) {
        previousBandTimeFraction=10;
      }
       
      if (smallElapsed <= previousBandTimeFraction) {
        // The runner is late - 
        return -1 * smallElapsed;
      }
    }
  }

  return inRemainingTime;
}

int getDisplayTime(time_tds inCurrentTime) {
  
  time_tds retTime = getRemaingAndBandTime(inCurrentTime, getDisplayTimeFn);  
    
  //  return retTime>=0 ? retTime/10 : (retTime-9)/10;
  return retTime/10;
}

time_tds getPercentCompleteTimes10Fn(time_tds inRemainingTime, int inBandIndex) {

  time_tds currentBandTime = s_countDownBand[inBandIndex];

  return 1000 * (currentBandTime - inRemainingTime) / currentBandTime;
}

int getPercentCompleteTimes10(time_tds inCurrentTime) {

  return getRemaingAndBandTime(inCurrentTime, getPercentCompleteTimes10Fn);
}

void setStart(time_tds inCurrentTime) {
  
  (*startFunctionP)(inCurrentTime);
}
