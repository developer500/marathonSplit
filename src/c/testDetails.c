#include <pebble.h>
#include "testDetails.h"
#include "timeCalculator.h"
#include "menuHandler.h"

#define TENTHS_PER_MINUTE SECONDS_PER_MINUTE*10

bool basicStartStopTest(char* inOutStatusMessage) {

	time_tds testBands[2] = {40, 30};
	setCountDownBands(testBands, 2);

	setRollover(true);

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
	if (getDisplayTime(startTime) != 4) {return false;}

	startTime += 10;
	if (getDisplayTime(startTime) != 3) {return false;}

	startTime += 10;
	if (getDisplayTime(startTime) != 2) {return false;}

	setRollover(false);

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

#define setStartFn 1
#define noSet 0
#define numberChecks 15

bool testReset(char* inOutStatusMessage) {

	time_tds testBands[3] = {700, 500, 300};

	time_tds expected[numberChecks][3] = {
			{200, setStartFn, 70},  // This is equivalent to going back to start
			{100, noSet, 60},
			{400, noSet, 20},
			{150, setStartFn, 50}, // This makes elapsed 650 - get to next band
			{100, noSet, 40},
			{100, noSet, 30},
			{100, noSet, 20},
			{100, noSet, 10},
			{100, noSet, 0},
			{10, noSet, -1},
			{0, setStartFn, 30},
			{100, noSet, 20},
			{100, noSet, 10},
			{100, noSet, 0}, // The user has not pressed reset - they are at the end of the set.
			{100, noSet, 10}, // 10 seconds since it should have been end of set
	};


	setCountDownBands(testBands, 3);

	initDisplayTime();

	time_t startTimet = (time_t)0x56c30f40;
	time_tds startTime = getTimeDsFromTime(startTimet, 0);

	setStart(startTime);

	for (int checkIndex = 0 ; checkIndex < numberChecks; checkIndex++) {

		APP_LOG(APP_LOG_LEVEL_DEBUG, "Check %d", checkIndex);

		startTime += expected[checkIndex][0];

		if (expected[checkIndex][1]==setStartFn) {
			setStart(startTime);
		}

		int displayT = getDisplayTime(startTime);

		if (displayT != expected[checkIndex][2]) {
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Check %d Expected %d but got %d", checkIndex, expected[checkIndex][2], displayT);
			return false;
		}
	}

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

	if (getDisplayTime(startTime) != 2) {return false;}

	// the above shows there are 2 seconds to go - but have to be within 1 sec to finish

	// add 1.5 seconds - his makes it finish - so that was 145 seconds after the start
	// this is the 1st press of the set start
	startTime += 15;

	setStart(startTime);

	if (getIsFinished(startTime) != true) {return false;}

	if (getTimeDiffFromPlanned() != -5) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Expected -5 but was %d", getTimeDiffFromPlanned());
		return false;
	}

	return true;
}

bool testFinishRound2(char* inOutStatusMessage) {

	// 50 and 100 seconds
	time_tds testBands[2] = {500, 1000};
	setCountDownBands(testBands, 2);

	initDisplayTime();

	time_t startTimet = (time_t)0x56c30f40;
	time_tds startTime = getTimeDsFromTime(startTimet, 0);
	setStart(startTime);

	//// After 46 seconds, user has 4 seconds to go on the 1st band
	startTime += 460;
	if (getDisplayTime(startTime) != 4) {return false;}

	// user presses start - display time becomes 0 - jumps forward
	setStart(startTime);
	if (getDisplayTime(startTime) != 100) {return false;}

	//// After 70 more seconds, there will be 30 seconds left on the 2nd band
	startTime += 700;
	if (getDisplayTime(startTime) != 30) {return false;}

	// user presses start - causes the set to end.
	setStart(startTime);

    // before pressing start the time was 30 - it is still 30...
	if (getDisplayTime(startTime) != 30) {return false;}

	// but now since the 1st band has been clicked the user must have jumoed
	// forwards, so the clock is finished.
	if (getIsFinished(startTime) != true) {return false;}

	return true;
}

bool testProjectedFinish(char* inOutStatusMessage) {

	time_tds testBands[3] = {100, 150, 200};
	setCountDownBands(testBands, 3);

	initDisplayTime();

	time_t startTimet = (time_t)0x56c30f40;
	time_tds startTime = getTimeDsFromTime(startTimet, 0);
	setStart(startTime);

	startTime += 90;
	if (getDisplayTime(startTime) != 1) {return false;}

	// user presses start causing the clock to go forwards

	setStart(startTime);
	if (getDisplayTime(startTime) != 15) {return false;}


	// The projected finish to go is
	// 9 + 15 + 20 = 44

	if (getProjectedFinishTime(startTime) != 440) {return false;}

	return true;
}

bool testProjectedFinish2(char* inOutStatusMessage) {

	time_tds testBands[3] = {1000, 1500, 2000};
	setCountDownBands(testBands, 3);

	initDisplayTime();

	time_t startTimet = (time_t)0x56c30f40;
	time_tds startTime = getTimeDsFromTime(startTimet, 0);
	setStart(startTime);

	startTime += 1200;
	if (getDisplayTime(startTime) != 130) {return false;}

	// user presses start causing the clock to go backwards
	setStart(startTime);

	getDisplayTime(startTime);

	if (getDisplayTime(startTime) != 150) {return false;}

	// The projected finish to go is
	// 12 (so far) + 15 + 20 = 47

	if (getProjectedFinishTime(startTime) != 4700) {return false;}

	return true;
}

bool testProjectedFinish3(char* inOutStatusMessage) {

	time_tds testBands[2] = {150, 100};
	setCountDownBands(testBands, 2);

	initDisplayTime();

	time_t startTimet = (time_t)0x56c30f40;
	time_tds startTime = getTimeDsFromTime(startTimet, 0);
	setStart(startTime);

	startTime += 135;
	if (getDisplayTime(startTime) != 1) {return false;}

	setStart(startTime);

	// now the projected time should be
	// 13.5 + 10 = 235

	if (getProjectedFinishTime(startTime) != 235) {return false;}

	return true;
}

bool testDisplayMessage(char* inOutStatusMessage) {

	time_tds thisplannedTimes[6] = {35999, 6582, 6582, 6582, 6582, 6582};
	time_tds thisactualTimes[6] = {35991, 110, 90, 100, 100, 10};

	setTimes(thisplannedTimes, thisactualTimes, 6);

	return true;
}

bool testRecordingSplitTimes(char* inOutStatusMessage) {

	time_t press1 = 901;
	time_t press2 = 1050;
	time_t press3 = 3020;
	time_t press4 = 260;

	time_tds testBands[6] = {1000, 1000, 1000, 1000, 1000, 2500};

	setCountDownBands(testBands, 6);
	initDisplayTime();

	time_t startTimet = (time_t)0x56c30f40;
	time_tds startTime = getTimeDsFromTime(startTimet, 0);
	setStart(startTime);

	// 10 seconds early
	startTime += press1;
	setStart(startTime);

	// 5 late
	startTime += press2;
	setStart(startTime);

	// 3 late and miss out 2 in the middle - note the subtle round down.
	startTime += press3;
	setStart(startTime);

	// finished.
	startTime += press4;
	setStart(startTime);

	time_tds expectedResults[6] = {press1, press2, press3/3, 1+press3/3, 1+press3/3, press4};

	for (int bandIndex = 0; bandIndex < 6; bandIndex++) {
		if (getActualTime(bandIndex) != expectedResults[bandIndex]) {
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Check %d Expected %d but got %d", bandIndex, expectedResults[bandIndex], getActualTime(bandIndex));

			return false;
		}
	}

	return true;
}

#define testBandNumber 6
#define bandChecksNumber 13

bool testRecordingSplitTimes2(char* inOutStatusMessage) {

	time_tds testBands[testBandNumber] = {1000, 1000, 1000, 1000, 1000, 250};

	time_tds expected[bandChecksNumber][testBandNumber] = {
			{998, 998, 998, 998, 998, 250},   //5240
			{998, 998, 998, 998, 999, 250},   //5241
			{998, 998, 998, 999, 999, 250},   //5242
			{998, 998, 999, 999, 999, 250},   //5243
			{998, 999, 999, 999, 999, 250},   //5244
			{999, 999, 999, 999, 999, 250},   //5245
			{999, 999, 999, 999, 1000, 250},  //5246
			{999, 999, 999, 1000, 1000, 250}, //5247
			{999, 999, 1000, 1000, 1000, 250}, //5248
			{999, 1000, 1000, 1000, 1000, 250}, //5249
			{1000, 1000, 1000, 1000, 1000, 250}, //5250
			{1000, 1000, 1000, 1000, 1000, 251}, //5251
			{1000, 1000, 1000, 1000, 1001, 251}, //5252
	};

	setCountDownBands(testBands, 6);
	int startTimeToCheck = 5240;
	for (int timesToCheck = startTimeToCheck ; timesToCheck < (startTimeToCheck + 12); timesToCheck++) {
		initDisplayTime();

		time_t startTimet = (time_t)0x56c30f40;
		time_tds startTime = getTimeDsFromTime(startTimet, 0);
		setStart(startTime);

		startTime += timesToCheck;
		setStart(startTime);

		for (int bandIndex = 0; bandIndex < testBandNumber; bandIndex++) {
			//APP_LOG(APP_LOG_LEVEL_DEBUG, "Actual Time %d", getActualTime(bandIndex));
			if (getActualTime(bandIndex) != expected[timesToCheck - startTimeToCheck][bandIndex]) {
				return false;
			}
		}
	}

	return true;
}

bool testRoundingDown(char* inOutStatusMessage) {

	time_tds testBands[2] = {500, 550};

	setCountDownBands(testBands, 2);
	initDisplayTime();

	time_t startTimet = (time_t)0x56c30f40;
	time_tds startTime = getTimeDsFromTime(startTimet, 0);
	setStart(startTime);

    // 
	if (getDisplayTime(startTime) != 50) {return false;}

	// 10 seconds early
	startTime += 491;
	setStart(startTime);

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Get Display Time 1 %d", (int)getDisplayTime(startTime));

    // They are now onto the next lap which is 55 seconds
	if (getDisplayTime(startTime) != 55) {return false;}

	return true;
}

bool testOverTime(char* inOutStatusMessage) {

    time_t press1 = 350;
	time_t press2 = 600;
	time_t press3 = 1400;

	time_tds testBands[2] = {400, 800};

	setCountDownBands(testBands, 2);
	initDisplayTime();

	time_t startTimet = (time_t)0x56c30f40;
	time_tds startTime = getTimeDsFromTime(startTimet, 0);
	setStart(startTime);

	startTime += press1;
	setStart(startTime);

	startTime += press2;
	addLap(startTime);

	startTime += press3;
	setStart(startTime);

	time_tds expectedResults[3] = {press1, press2, press3};

	for (int bandIndex = 0; bandIndex < 3; bandIndex++) {
		if (getActualTime(bandIndex) != expectedResults[bandIndex]) {
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Overtime Check %d Expected %d but got %d", bandIndex, expectedResults[bandIndex], getActualTime(bandIndex));

			return false;
		}
	}

	return true;
}

#define MAX_TIME_FORMATS 19

bool testGetTimeStrings(char* inOutStatusMessage) {

	typedef struct  {
		time_tds testTime;
		char * expectedString;
		bool addPlus;
		int size;
	} testTimeFormat;

	testTimeFormat testTimeFormats[MAX_TIME_FORMATS] = {
			{ 0, "0", false, 1},
			{ 1, "0.1", false, 3},
			{ 9, "0.9", false, 3},
			{ 10, "1", false, 1},
			{ 11, "1.1", false, 3},
			{ 90, "9", false, 1},
			{ 100, "10", false, 2},
			{ (SECONDS_PER_MINUTE-1)*10, "59", false, 2},
			{ (SECONDS_PER_MINUTE-1)*10 + 1, "59.1", false, 4},
			{ TENTHS_PER_MINUTE, "1:00", false, 4},
			{ TENTHS_PER_MINUTE + 1, "1:00", false, 4},       // Over 59 seconds - don't show the tenths.
			{ (TENTHS_PER_MINUTE - 1) * 10 , "9:59", false, 4},
			{ TENTHS_PER_MINUTE*10, "10:00", false, 5},
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
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "Time String is %s", strTimeBuffer);
		if (strcmp(strTimeBuffer, testTimeFormats[testIndex].expectedString) !=0) {
			return false;
		}
		if (strLength != testTimeFormats[testIndex].size) {
			return false;
		}
	}

	return true;
}

bool testAddLap1(char* inOutStatusMessage) {

	time_tds testBands[4] = {200, 300, 500, 700};

	setCountDownBands(testBands, 4);

	initDisplayTime();

	time_t startTimet = (time_t)0x56c30f40;
	time_tds startTime = getTimeDsFromTime(startTimet, 0);
	setStart(startTime);

	startTime += 120;  // 120 in - add a lap
	addLap(startTime);

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Bands %d", (int)getCounDownBands());

	if (getCounDownBands() != 5) {return false;}

	time_tds expectedPlanned[5] = {120, 80, 300, 500, 700};

	for (int bandIndex = 0; bandIndex < 5; bandIndex++) {
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "Planned time %d", (int)getTimeForBand(bandIndex));
		if (getTimeForBand(bandIndex) != expectedPlanned[bandIndex]) {
			return false;
		}
	}


	return true;
}

bool testAddLap2(char* inOutStatusMessage) {

	time_tds testBands[6] = {200, 300, 500, 700};

	setCountDownBands(testBands, 4);

	initDisplayTime();

	time_t startTimet = (time_t)0x56c30f40;
	time_tds startTime = getTimeDsFromTime(startTimet, 0);
	setStart(startTime);

	startTime += 290;  // 0 in - add a lap
	addLap(startTime);

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Bands %d", (int)getCounDownBands());

	if (getCounDownBands() != 5) {return false;}

	time_tds expectedPlanned[5] = {200, 90, 210, 500, 700};

	for (int bandIndex = 0; bandIndex < 5; bandIndex++) {
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "Planned time 2 %d", (int)getTimeForBand(bandIndex));
		if (getTimeForBand(bandIndex) != expectedPlanned[bandIndex]) {
			return false;
		}
	}


	return true;
}

#define NUMBER_TESTS 17

bool testExisting(char* inOutStatusMessage) {

	struct {
		char* message;
		bool (*method)(char* inOutStatusMessage);
	} tests[NUMBER_TESTS] = {
			{"Basic Start and Stop Test", &basicStartStopTest},
			{"Test Paused", &testPaused},
			{"Test Reset", &testReset},
			{"Test Finished", &testFinish},
			{"Test Finished Round2", &testFinishRound2},
			{"Test projected finish", &testProjectedFinish},
			{"Test projected finish2", &testProjectedFinish2},
			{"Test projected finish3", &testProjectedFinish3},
			{"Test get time strings", &testGetTimeStrings},
			{"Test display message", &testDisplayMessage},
			{"Test recording split times", &testRecordingSplitTimes},
			{"Test recording split times2", &testRecordingSplitTimes2},
			{"Test rounding down", &testRoundingDown},
			{"Test get Time strings", &testGetTimeStrings},
			{"Test add Lap 1", &testAddLap1},
			{"Test add Lap 2", &testAddLap2},
			{"Test Over time", &testOverTime},
	};

	bool retval;

	for (int testIndex = 0 ; testIndex < NUMBER_TESTS; testIndex++) {

		retval = tests[testIndex].method(" ");

		if (!retval) {
			strcpy(inOutStatusMessage, tests[testIndex].message);
			return retval;
		}
	}

	return true;
}

bool testTimeCalculator(char* inOutStatusMessage) {

	if (!testExisting(inOutStatusMessage)) { return false; }


	//strcpy(inOutStatusMessage, "failed");

	if (!testRoundingDown(inOutStatusMessage)) { return false; }

	return true;
}
