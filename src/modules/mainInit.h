#include <pebble.h>
#include "timeCalculator.h"

static void addTextLayers(int inWindowIndex);

static void emptyCP(time_tds inCurrentTime);
static void initialStartCP(time_tds inCurrentTime);
static void setPausedCP(time_tds inCurrentTime);
static void resetPausedCP(time_tds inCurrentTime);
static void clearDataCP(time_tds inCurrentTime);
static void runningStartCP(time_tds inCurrentTime);
static void plannedCP(time_tds inCurrentTime);

static void set_action_icons(int inSystemState);

void mainInit();
