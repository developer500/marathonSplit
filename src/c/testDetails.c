#include <pebble.h>
#include "testDetails.h"
#include "timeCalculator.h"
#include "menuHandler.h"

bool basicStartStopTest(char* inOutStatusMessage) {

  time_tds testBands[2] = {40, 30};
  setCountDownBands(testBands, 2);

  initDisplayTime();

  // assume the race starts on 1Feb16 at 12pm
  time_t startTimet = (time_t)0x56c30f40;
  time_tds startTime = getTimeDsFromTime(startTimet, 0);
  setStart(startTime);
  
  if (getDisplayTime(startTime) != 4) {return false;} // this is when it is started
  
  startTime += 10;
  if (getDisplayTime(startTime) != 3) {return false;}
  
  startTime += 10;
  if (getDisplayTime(startTime) != 2) {return false;}
  
  startTime += 10;
  if (getDisplayTime(startTime) != 1) {return false;}
  
  startTime += 10;
  if (getDisplayTime(startTime) != 0) {return false;}
    
  startTime += 10;
  
  if (getDisplayTime(startTime) != -1) {return false;}

  startTime += 10;
  if (getDisplayTime(startTime) != 1) {return false;}

  startTime += 10;
  if (getDisplayTime(startTime) != 0) {return false;}
  
  startTime += 10;
  if (getDisplayTime(startTime) != 3) {return false;}
  
  startTime += 10;
  if (getDisplayTime(startTime) != 2) {return false;}

  return true;
}

bool testPaused(char* inOutStatusMessage) {
  
  time_tds testBands[2] = {70, 40};
  setCountDownBands(testBands, 2);

  initDisplayTime();

  time_t startTimet = (time_t)0x56c30f40;
  time_tds startTime = getTimeDsFromTime(startTimet, 0);
  setStart(startTime);

  startTime += 50;

  for (int bandIndex = 0; bandIndex < 2; bandIndex++) {

    // The 1st time 5 seconds have elapsed
    if (getDisplayTime(startTime) != 2) {return false;}
    
    if (getIsPaused()==true) {return false;}
    
    togglePaused(startTime);
    
    if (getIsPaused()==false) {return false;}
    
    // its still same
    if (getDisplayTime(startTime) != 2) {return false;}

    startTime += 20;

    // it should still be same
    if (getDisplayTime(startTime) != 2) {return false;}

    togglePaused(startTime);
    
    if (getIsPaused()==true) {return false;}
    
    // still same
    if (getDisplayTime(startTime) != 2) {return false;}

    startTime += 10;

    // now one less
    if (getDisplayTime(startTime) != 1) {return false;}

    startTime += 30;
  }

  return true;
}

bool testReset(char* inOutStatusMessage) {
  
  time_tds testBands[3] = {70, 50, 30};
  setCountDownBands(testBands, 3);

  initDisplayTime();

  time_t startTimet = (time_t)0x56c30f40;
  time_tds startTime = getTimeDsFromTime(startTimet, 0);
  setStart(startTime);
  
  //// 2 seconds later do a rest - this is equivalent to pressing reset back to start - make sure
  // the 0th actual time is not filled in.
  startTime += 20;
  setStart(startTime);
  if (getDisplayTime(startTime) != 7) {return false;}
  
  startTime += 10;
  if (getDisplayTime(startTime) != 6) {return false;}
  
  startTime += 40;  // 4 seconds later makes it next band
  if (getDisplayTime(startTime) != 2) {return false;}
  
  setStart(startTime);
  if (getDisplayTime(startTime) != 0) {return false;}
  
  startTime += 10;
  if (getDisplayTime(startTime) != 4) {return false;}
    
  startTime += 10;
  if (getDisplayTime(startTime) != 3) {return false;}
  
  startTime += 10;
  if (getDisplayTime(startTime) != 2) {return false;}
    
  startTime += 10;
  if (getDisplayTime(startTime) != 1) {return false;}
    
  startTime += 10;
  if (getDisplayTime(startTime) != 0) {return false;}
  
  startTime += 10;
  if (getDisplayTime(startTime) != -1) {return false;}
  
  setStart(startTime);
  if (getDisplayTime(startTime) != 0) {return false;}
  
  startTime += 10; 
  if (getDisplayTime(startTime) != 2) {return false;}
  
  startTime += 10; 
  if (getDisplayTime(startTime) != 1) {return false;}
    
  startTime += 10; 
  if (getDisplayTime(startTime) != 0) {return false;} 
      
  startTime += 10; 
  if (getDisplayTime(startTime) != 6) {return false;}

  return true;
}

bool testFinish(char* inOutStatusMessage) {
  
  time_tds testBands[2] = {50, 100};
  setCountDownBands(testBands, 2);

  initDisplayTime();

  time_t startTimet = (time_t)0x56c30f40;
  time_tds startTime = getTimeDsFromTime(startTimet, 0);
  setStart(startTime);
  
  //// 13 seconds later, the user is finished...
  startTime += 130;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Display time 1 %d", getDisplayTime(startTime));

  if (getDisplayTime(startTime) != 2) {return false;}
  
  setStart(startTime);
    
  if (getIsFinished(startTime) != true) {return false;}
  
  if (getTimeDiffFromPlanned() != -20) {return false;}

  return true;
}

bool testFinishRound2(char* inOutStatusMessage) {
  
  time_tds testBands[2] = {50, 100};
  setCountDownBands(testBands, 2);

  initDisplayTime();

  time_t startTimet = (time_t)0x56c30f40;
  time_tds startTime = getTimeDsFromTime(startTimet, 0);
  setStart(startTime);
  
  //// After 4 user presses start - means they now have 1 extra second
  startTime += 40;
  if (getDisplayTime(startTime) != 1) {return false;}
  
  
  setStart(startTime);
  if (getDisplayTime(startTime) != 0) {return false;}

  
  //// After 7 user presses start again - means they now have 3 extra seconds
  // making the total 4
  startTime += 70;
  if (getDisplayTime(startTime) != 3) {return false;}
    
  setStart(startTime);
  
  if (getDisplayTime(startTime) != 0) {return false;}
  
  if (getIsFinished(startTime) == false) {return false;}
  
  if (getTimeDiffFromPlanned() != -40) {return false;}

  return true;
}

bool testProjectedFinish(char* inOutStatusMessage) {
  
  time_tds testBands[3] = {100, 150, 200};
  setCountDownBands(testBands, 3);

  initDisplayTime();

  time_t startTimet = (time_t)0x56c30f40;
  time_tds startTime = getTimeDsFromTime(startTimet, 0);
  setStart(startTime);
  
  startTime += 80;
  if (getDisplayTime(startTime) != 2) {return false;}
  
  // user presses start causing the clock to go forwards
  
  setStart(startTime);
  if (getDisplayTime(startTime) != 0) {return false;}
  
  // The projected finish to go is
  // 8 + 15 + 20 = 43
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Projected Finish %d", (int)getProjectedFinishTime(startTime));
  if (getProjectedFinishTime(startTime) != 430) {return false;}
  
  return true;
}

bool testProjectedFinish2(char* inOutStatusMessage) {
  
  time_tds testBands[3] = {100, 150, 200};
  setCountDownBands(testBands, 3);

  initDisplayTime();

  time_t startTimet = (time_t)0x56c30f40;
  time_tds startTime = getTimeDsFromTime(startTimet, 0);
  setStart(startTime);
  
  startTime += 120;
  if (getDisplayTime(startTime) != 13) {return false;}
    
  // user presses start causing the clock to go backwards
  setStart(startTime);
  
  getDisplayTime(startTime);
  
  if (getDisplayTime(startTime) != 0) {return false;}
  
  // The projected finish to go is
  // 12 (so far) + 15 + 20 = 47
  
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Projected Finish %d", (int)getProjectedFinishTime(startTime));
  if (getProjectedFinishTime(startTime) != 470) {return false;}
  
  return true;
}

bool testProjectedFinish3(char* inOutStatusMessage) {
  
  time_tds testBands[2] = {150, 100};
  setCountDownBands(testBands, 2);

  initDisplayTime();

  time_t startTimet = (time_t)0x56c30f40;
  time_tds startTime = getTimeDsFromTime(startTimet, 0);
  setStart(startTime);
  
  startTime += 80;
  if (getDisplayTime(startTime) != 7) {return false;}
  
  setStart(startTime);
    
  // now the projected time should be
  // 8 + 10 = 18
  
  if (getProjectedFinishTime(startTime) != 180) {return false;}
  
  return true;
}

bool testCurrentReplannedLapTime(char* inOutStatusMessage) {
  
  time_tds testBands[6] = {1000, 1000, 1000, 1000, 1000, 1000};
  
  setLastBandPercentOfMax(100);
  
  setCountDownBands(testBands, 6);
  
  {
    initDisplayTime();

    time_t startTimet = (time_t)0x56c30f40;
    time_tds startTime = getTimeDsFromTime(startTimet, 0);
    setStart(startTime);
  
    startTime += 900;  // runner is 10 sec ahead of schedule
    //there are 5 laps to go
    // meaning the next time should be 2 sec longer...
  
    setStart(startTime);
  
    if (getCurrentRePlannedLapTime(startTime) != 1020) { return false; }
  }
  
  {
    initDisplayTime();

    time_t startTimet = (time_t)0x56c30f40;
    time_tds startTime = getTimeDsFromTime(startTimet, 0);
    setStart(startTime);
  
    startTime += 1100;  // runner is 10 sec behind of schedule
    //there are 5 laps to go
    // meaning the next time should be 2 sec longer...
  
    setStart(startTime);
  
    if (getCurrentRePlannedLapTime(startTime) != 980) { return false; }
  }
  return true;
}

bool testCurrentReplannedLapTime2(char* inOutStatusMessage) {
  
  time_tds testBands[6] = {1000, 1000, 1000, 1000, 1000, 250};
  
  setLastBandPercentOfMax(25);
  
  setCountDownBands(testBands, 6);
  
  initDisplayTime();

  time_t startTimet = (time_t)0x56c30f40;
  time_tds startTime = getTimeDsFromTime(startTimet, 0);
  setStart(startTime);
  
  startTime += 830;  // runner is 17 sec ahead of schedule
  //there are 5 laps to go - but the last lap
  //is only .25 of a regular lap - so time per lap is 1700/425 = 4 sec
  //a lap
  //without this he time would be 1700/500 = 3 sec a lap
  
  setStart(startTime);
  
  if (getCurrentRePlannedLapTime(startTime) != 1040) { return false; }
  
  return true;
}

bool testCurrentReplannedLapTime3(char* inOutStatusMessage) {
  
  time_tds testBands[6] = {1000, 1000, 1000, 1000, 1000, 250};
  
  setLastBandPercentOfMax(25);
  
  setCountDownBands(testBands, 6);
  
  initDisplayTime();

  time_t startTimet = (time_t)0x56c30f40;
  time_tds startTime = getTimeDsFromTime(startTimet, 0);
  setStart(startTime);
  
  startTime += 5010;  // runner has almost finised
  // but is 1 second behind schedule
  setStart(startTime);
    
  if (getCurrentRePlannedLapTime(startTime) != 240) { return false; }
  
  return true;
}

bool testDisplayMessage(char* inOutStatusMessage) {
  
  time_tds thisplannedTimes[6] = {35999, 6582, 6582, 6582, 6582, 6582};
  time_tds thisactualTimes[6] = {35991, 110, 90, 100, 100, 10};
  
  setTimes(thisplannedTimes, thisactualTimes, 6);
  
  return true;
}

bool testRecordingSplitTimes(char* inOutStatusMessage) {
  
  time_tds testBands[6] = {1000, 1000, 1000, 1000, 1000, 250};
  
  setLastBandPercentOfMax(25);
  setCountDownBands(testBands, 6);
  initDisplayTime();

  time_t startTimet = (time_t)0x56c30f40;
  time_tds startTime = getTimeDsFromTime(startTimet, 0);
  setStart(startTime);
  
  // 10 seconds early
  startTime += 900;
  setStart(startTime);
  
  // 5 late
  startTime += 1050;
  setStart(startTime);
    
  // 3 late and miss out 2 in the middle - note the subtle round down.
  startTime += 3020;
  setStart(startTime);
  
  // finished.
  startTime += 260;
  setStart(startTime);
  
  time_tds expectedResults[6] = {900, 1050, 1007, 1007, 1006, 260};
  
  for (int bandIndex = 0; bandIndex < 6; bandIndex++) {
    if (getActualTime(bandIndex) != expectedResults[bandIndex]) {
      return false;
    }
  }
  
  return true;
}

bool testRecordingSplitTimes2(char* inOutStatusMessage) {
  
  time_tds testBands[6] = {1000, 1000, 1000, 1000, 1000, 250};
  
  setLastBandPercentOfMax(25);
  setCountDownBands(testBands, 6);
  for (int timesToCheck = 5240 ; timesToCheck <= 5250; timesToCheck++) {
    initDisplayTime();

    time_t startTimet = (time_t)0x56c30f40;
    time_tds startTime = getTimeDsFromTime(startTimet, 0);
    setStart(startTime);
  
    startTime += timesToCheck;
    setStart(startTime);
  
    time_tds expectedResults[6] = {1000, 1000, 1000, 1000, 1000, 240};

    for (int bandIndex = 0; bandIndex < 5; bandIndex++) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Actual Time %d", getActualTime(bandIndex));
      if (getActualTime(bandIndex) != expectedResults[bandIndex]) {
        return false;
      }
    }
    
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Actual Time 5 %d", getActualTime(5));
    if (getActualTime(5) != (timesToCheck-5000)) {
      return false;
    }
  }
  
  return true;
}

#define MAX_TIME_FORMATS 18

bool testGetTimeStrings(char* inOutStatusMessage) {
  
  typedef struct  {
    time_tds testTime;
    char * expectedString;
    bool addPlus;
    int size;
  } testTimeFormat;

  testTimeFormat testTimeFormats[MAX_TIME_FORMATS] = {
      { 0, ":00", false, 3},
      { 1, ":00.1", false, 5},
      { 9, ":00.9", false, 5},        
      { 10, ":01", false, 3},
      { 90, ":09", false, 3},        
      { 100, ":10", false, 3},
      { (SECONDS_PER_MINUTE-1)*10, ":59", false, 3},
      { (SECONDS_PER_MINUTE-1)*10 + 1, ":59.1", false, 5},
      { SECONDS_PER_MINUTE*10, "1:00", false, 4},
      { SECONDS_PER_MINUTE*10 + 1, "1:00", false, 4},       // Over 59 seconds - don't show the tenths.
      { (SECONDS_PER_MINUTE*10 - 1) * 10 , "9:59", false, 4},
      { SECONDS_PER_MINUTE*100, "10:00", false, 5},
      { (SECONDS_PER_HOUR-1) * 10, "59:59", false, 5},
      { SECONDS_PER_HOUR * 10, "1:00:00", false, 7},
      { SECONDS_PER_HOUR*13 * 10, "13:00:00", false, 8},
      { SECONDS_PER_HOUR*24 * 10, "00:00:00", false, 8},
      { (SECONDS_PER_MINUTE*10-1) * 10, "+9:59", true, 5},
      { (-1*(SECONDS_PER_MINUTE*10-1)) * 10, "-9:59", true, 5},
    };
  
  char strTimeBuffer[9];
  int strLength = 0;
  for (int testIndex = 0; testIndex < MAX_TIME_FORMATS ; testIndex++) {
   strLength = getTimeFromTimeT(testTimeFormats[testIndex].testTime, testTimeFormats[testIndex].addPlus, strTimeBuffer);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Time String is %s", strTimeBuffer);
   if (strcmp(strTimeBuffer, testTimeFormats[testIndex].expectedString) !=0) {
      return false;
    }
    if (strLength != testTimeFormats[testIndex].size) {
      return false;
    } 
  }
  
  return true;
}

bool testExisting(char* inOutStatusMessage) {
  
  /*
  if (!basicStartStopTest(inOutStatusMessage)) { return false; }

  if (!testPaused(inOutStatusMessage)) { return false; }
   
  if (!testReset(inOutStatusMessage)) { return false; }
  
  if (!testFinish(inOutStatusMessage)) { return false; }
  
  if (!testFinishRound2(inOutStatusMessage)) {return false; }
    
  if (!testProjectedFinish(inOutStatusMessage)) {return false;}

  if (!testProjectedFinish2(inOutStatusMessage)) {return false;}

  if (!testProjectedFinish3(inOutStatusMessage)) {return false;}

  if (!testCurrentReplannedLapTime(inOutStatusMessage)) { return false; }

  if (!testCurrentReplannedLapTime2(inOutStatusMessage)) { return false; }

  if (!testCurrentReplannedLapTime3(inOutStatusMessage)) { return false; }
    
  if (!testGetTimeStrings(inOutStatusMessage)) { return false; }
  
  if (!testDisplayMessage(inOutStatusMessage)) { return false; }
  
  if (!testRecordingSplitTimes(inOutStatusMessage)) { return false; }
  */

  return true;
}

bool testTimeCalculator(char* inOutStatusMessage) {
 
  //if (!testExisting(inOutStatusMessage)) { return false; }
  
  if (!testDisplayMessage(inOutStatusMessage)) { return false; }
  
  return true;
}