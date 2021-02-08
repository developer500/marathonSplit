#include <pebble.h>
#include "timeCalculator.h"

// Rollover happens when doing laps and there is 1 time band
// no rollover is like a race multiple bands
static bool s_noRollover = true;

static time_tds s_startTime;
static time_tds s_realStartTime;
static time_tds s_pauseTime;

static time_tds s_actualTimes[MAX_BANDS];

static int s_countDownBands = 0;
static time_tds s_countDownBand[MAX_BANDS];

static time_tds s_totalBandTime;

// required to forward declare.
bool clearPausedFn(time_tds inCurrentTime);

static void (*s_startFunctionP)(time_tds inCurrentTime);

static bool (*s_pauseFunctionP)(time_tds inCurrentTime);

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

void setRollover(bool inHasRollover) {
  s_noRollover = !inHasRollover;
}

bool getIsStarted() {
  return s_startTime != 0;
}

bool getIsPaused() {
  return s_pauseTime != 0;
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

int getIndexOfLastConfirmedBand() {

  int firstZeroActualBand = getIndexOfFirstZeroActualTime();

  return firstZeroActualBand == -1 ? firstZeroActualBand : firstZeroActualBand -1;
}

time_tds getElapsedTime(time_tds inCurrentTime) {
  
   if (!getIsStarted()) {
     return (time_tds)0;
   }
  
  time_tds absoluteCurrentTime = getIsPaused() ? s_pauseTime : inCurrentTime;  // eg 4:13:02  - current time or paused time.
    
  return absoluteCurrentTime - s_startTime;                // eg 0:00:12  - 12 seconds since start was pressed.
}

time_tds getEstimatedBandTime(int inBandIndex) {
  return s_actualTimes[inBandIndex] == 0 ? s_countDownBand[inBandIndex] : s_actualTimes[inBandIndex];
}

int getCurrrentBandAndSmallElapsed(time_tds inCurrentTime, time_tds* inOutSmallElapsd, bool inRoundForwards) {
  
  time_tds smallelapsedTime = getElapsedTime(inCurrentTime); 

  int bandIndex = 0;

  time_tds estimatedBandTime = getEstimatedBandTime(bandIndex);
  while (smallelapsedTime >= estimatedBandTime) {
    bandIndex++;
    if (bandIndex==s_countDownBands) {
      if (s_noRollover) {
        bandIndex--;
        break;
      }
    }
    bandIndex %= s_countDownBands;
    smallelapsedTime -= estimatedBandTime;
    estimatedBandTime = getEstimatedBandTime(bandIndex);
  }
  
  if (inRoundForwards) {
    if (smallelapsedTime == getEstimatedBandTime(bandIndex)) {
      bandIndex++;
      if (bandIndex >= s_countDownBands) {
        if (s_noRollover) {
          bandIndex--;
        } else {
          bandIndex %= s_countDownBands;
        }
      }
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

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Band >%d< Remaining >%d<", bandIndex,remainingTime ); 
	  

  return inFunction(remainingTime, bandIndex);
}

// If a runner is within this # of sec of the next band then jump forwards
time_tds getMinimumJumpForwardTime(time_tds inCurrentBandTime) {

	// either 15 % or 25 sec which ever is bigger
	const time_tds ydSeconds = 250;

	const int lowPercent = 15;
	const int highPercent = 50;

	time_tds lowTime = inCurrentBandTime * lowPercent/ 100;
	time_tds highTime = inCurrentBandTime * highPercent / 100;

	time_tds retTime = lowTime;

	if (lowTime < ydSeconds) {
		retTime = ydSeconds;

		if (ydSeconds > highTime) {
			retTime = highTime;
		}
	}

	return retTime;
}

bool shouldJumpForwards(time_tds inSmallElapsed, int inBandIndex) {

  time_tds currentBandTime = s_countDownBand[inBandIndex];

  time_tds remainingTime = currentBandTime - inSmallElapsed;

  if (remainingTime <= getMinimumJumpForwardTime(currentBandTime)) {
    return true;
  }

  if ((inBandIndex == (getIndexOfLastConfirmedBand() + 1)) && (inSmallElapsed > 200) ) {
    // within 20 seconds
    return true;
  } 
  
  return false;
}

void setPaused(time_tds inCurrentTime) {
  s_pauseTime = inCurrentTime;
}

void clearPaused() {
  s_pauseTime = 0;
}

void populateActualTimes(time_tds inCurrentTime, time_tds inSmallElapsed, int inCurrentBand) {
    
  int firstZeroBand = getIndexOfFirstZeroActualTime();
  if (firstZeroBand==-1) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Tried to populate actual times with no 1st zero band");
    return;
  }

  bool jumpForwards = shouldJumpForwards(inSmallElapsed, inCurrentBand);

  if (!jumpForwards) {
    if ((firstZeroBand == 0) && (inCurrentBand==0)) {
      // The user pressed start then pressed start again quickly
      s_startTime = inCurrentTime;
      return;
    }

    if (inCurrentBand == firstZeroBand) {
      // Pressed start twice after crossing a band..
      s_actualTimes[inCurrentBand-1] += inSmallElapsed;
      return;
    }
  }

  time_tds timeToDistribute = 0;
  int bandIndex = 0;
  for (bandIndex = firstZeroBand; bandIndex < inCurrentBand ; bandIndex++) {
	  timeToDistribute += s_countDownBand[bandIndex];
  }
  time_tds plannedTimeOverPeriod = timeToDistribute + (jumpForwards ? s_countDownBand[inCurrentBand] : 0);
  timeToDistribute += inSmallElapsed;

  // Jumping forwards means the current band is actually the next band.
  inCurrentBand += (jumpForwards ? 1 : 0);
  int lastBand = inCurrentBand -1;

  time_tds timeDistributed = 0;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "======== Planned time over period >%d< %d", plannedTimeOverPeriod, timeToDistribute);

  for (bandIndex = firstZeroBand; bandIndex <= lastBand; bandIndex++) {
	  s_actualTimes[bandIndex] = timeToDistribute * s_countDownBand[bandIndex] / plannedTimeOverPeriod;
	  timeDistributed += s_actualTimes[bandIndex];
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Actual >%d< distributed >%d<", bandIndex, s_actualTimes[bandIndex]);
  }

  time_tds roundingError = timeToDistribute - timeDistributed;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Rounding %d", roundingError);

  for (bandIndex = lastBand; bandIndex > (lastBand - abs(roundingError)) ; bandIndex--) {
      time_tds roundingThisBand = roundingError > 0 ? 1 : -1;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Added %d to band %d", roundingThisBand, bandIndex);
  	  s_actualTimes[bandIndex] += roundingThisBand;
    }
}

void runningStartFn(time_tds inCurrentTime) {

  time_tds smallElapsed = 0;
  
  int currentBand = getCurrrentBandAndSmallElapsed(inCurrentTime, &smallElapsed, false);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Current >%d< small Elapsed >%d< last confirmed >%d<", inCurrentTime, smallElapsed, currentBand);

  populateActualTimes(inCurrentTime, smallElapsed, currentBand);
 
  // Did the user press reset which caused the clock to jump forwards and finish the set?
  if (getElapsedTime(inCurrentTime) >= s_totalBandTime) {
    setPaused(inCurrentTime);
    return;
  }
  
  clearPaused();
}

void initialsetStartFn(time_tds inCurrentTime) {
  
  s_startTime = inCurrentTime;
  s_realStartTime = inCurrentTime;
  clearPaused();
  s_startFunctionP = &runningStartFn;
}

bool initialPausedFn(time_tds inCurrentTime) {

  setPaused(inCurrentTime);
  s_pauseFunctionP = &clearPausedFn;
  return true;
}

bool clearPausedFn(time_tds inCurrentTime) {

  time_tds timeElapsedWhenPaused = inCurrentTime - s_pauseTime;
  s_startTime += timeElapsedWhenPaused;
  s_realStartTime += timeElapsedWhenPaused;
  clearPaused();
  s_pauseFunctionP = &initialPausedFn;
  
  return false;
}

time_tds getActualTime(int inBandIndex) {
  return s_actualTimes[inBandIndex];
}

void initDisplayTime() {
  
  s_startTime = 0;
  s_realStartTime = 0;
  s_pauseTime = 0;

  s_totalBandTime = 0;

  for (int bandIndex = 0; bandIndex < s_countDownBands; bandIndex++) {
    s_totalBandTime += s_countDownBand[bandIndex];
    s_actualTimes[bandIndex] = (time_tds)0;
  }
  
  s_startFunctionP = &initialsetStartFn;
  
  s_pauseFunctionP = &initialPausedFn;
}

bool togglePaused(time_tds inCurrentTime) {
  return (*s_pauseFunctionP)(inCurrentTime);
}

time_tds getCurrentPlannedLapTime(time_tds inCurrentTime) {
  
  time_tds smallElapsed = 0;
  int bandIndex = getCurrrentBandAndSmallElapsed(inCurrentTime, &smallElapsed, true);
  
  return s_countDownBand[bandIndex];
}

time_tds getTimeDiffFromPlanned() {

  time_tds plannedTime = 0;
  time_tds actualTime = 0;

  for (int bandIndex = 0; bandIndex < s_countDownBands ; bandIndex++) {
    plannedTime += s_countDownBand[bandIndex];
    actualTime += s_actualTimes[bandIndex];
  }

  return (actualTime - plannedTime);
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

  APP_LOG(APP_LOG_LEVEL_DEBUG, "GetIndex %d sCountDown %d", getIndexOfFirstZeroActualTime(), s_countDownBands);

  if (getIndexOfFirstZeroActualTime() == s_countDownBands) {
    return true;
  }
  
  return false;
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

void addLap(time_tds inCurrentTime) {

  time_tds smallElapsed = 0;
  int bandIndex = getCurrrentBandAndSmallElapsed(inCurrentTime, &smallElapsed, false);

  time_tds newLapTime = s_countDownBand[bandIndex] - smallElapsed;
  s_countDownBands++;

  for (int bandIt = s_countDownBands-1; bandIt > (bandIndex+1); bandIt--) {
	  s_countDownBand[bandIt] = s_countDownBand[bandIt-1];
  }

  s_countDownBand[bandIndex] = smallElapsed;
  s_countDownBand[bandIndex + 1] = newLapTime;

  setStart(inCurrentTime);
}

time_tds getDisplayTimeFn(time_tds inRemainingTime, int inBandIndex) {
  
  // is the runner late getting to the split?
  if (inBandIndex > 0) {
    if ((inBandIndex-1) > getIndexOfLastConfirmedBand()) {
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

  return abs(inRemainingTime);
}

int getDisplayTime(time_tds inCurrentTime) {
  
  time_tds retTime = getRemaingAndBandTime(inCurrentTime, getDisplayTimeFn);  
    
  //  return retTime>=0 ? retTime/10 : (retTime-9)/10;
  return (retTime/10) % 1000;
}

time_tds getPercentCompleteTimes10Fn(time_tds inRemainingTime, int inBandIndex) {

  time_tds currentBandTime = s_countDownBand[inBandIndex];

  return 1000 * (currentBandTime - inRemainingTime) / currentBandTime;
}

int getPercentCompleteTimes10(time_tds inCurrentTime) {

  return getRemaingAndBandTime(inCurrentTime, getPercentCompleteTimes10Fn);
}

void setStart(time_tds inCurrentTime) {
  
  (*s_startFunctionP)(inCurrentTime);

}