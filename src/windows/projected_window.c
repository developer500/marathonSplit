#include "projected_window.h"
#include "modules/constants.h"
#include "modules/global.h"
#include "modules/mainInit.h"

static time_tds s_projectedFinishTime = 0;

static void changeDisplayProjectedFinish(time_tds inCurrentTime) {

  s_projectedFinishTime = getProjectedFinishTime(inCurrentTime);
  main_window_stack_push(PROJECTED_WINDOW_INDEX);
  s_revertToDisplay = inCurrentTime + (time_tds)80;
}

static void showProjectedFinishTimeTH(time_tds inCurrentTime) {

  time_t projectedFinishTime = getTimeFromDs(s_projectedFinishTime);

  struct tm *projFinishTimeP = gmtime(&projectedFinishTime);
  static char s_time_tdsext[] = "00:00:00";
  strftime(s_time_tdsext, sizeof(s_time_tdsext), "%M:%S", projFinishTimeP);
  text_layer_set_text((TextLayer *)layerStores[PROJECTED_LAYER_INDEX].layer, s_time_tdsext);

  time_t nextPlannedLapTime = (time_t)(getCurrentPlannedLapTime(inCurrentTime)/10);
  struct tm *nextPlannedLapTimeP = gmtime(&nextPlannedLapTime);
  static char s_next_planned[] = "00:00:00";
  strftime(s_next_planned, sizeof(s_next_planned), "%M:%S", nextPlannedLapTimeP);

  char * next_plannedP = s_next_planned;

  if (s_next_planned[0]=='0') {
    next_plannedP++;
  }

  text_layer_set_text((TextLayer *)layerStores[NEXT_LAP_PACE_INDEX].layer, next_plannedP);
}

static void projected_window_load(Window* window) {

  GColor backColor =  s_blackBackground ? GColorBlack : GColorWhite;
  //GColor foreColor = s_blackBackground ? GColorWhite : GColorBlack;

  window_set_background_color(window, backColor);

  addTextLayers(PROJECTED_WINDOW_INDEX);

  time_tds currentTime = getTimeDs();
  showProjectedFinishTimeTH(currentTime);
}
