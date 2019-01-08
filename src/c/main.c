//#define testOnly
#include "modules/mainInit.h"
#include "modules/constants.h"


#ifdef testOnly
#include "testMain.c"
#endif

#ifndef testOnly

#define MAX_SECTION_BANDS 4

static void tick_handler(struct tm* tick_time, TimeUnits units_changed) {

  time_tds currentTimeds = getTimeDs();

  (*currTickHand)(currentTimeds);
}

static void initData() {
  initDisplayTime();
  set_action_icons(INITIAL_STATE);
}

static void init() {
 
  // Show the Window on the watch, with animated=true
  main_window_stack_push(MAIN_WINDOW_INDEX);

  initData();

  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit() {
  // Destroy Window
  window_destroy(windowStores[PROJECTED_WINDOW_INDEX].window);
  window_destroy(windowStores[MAIN_WINDOW_INDEX].window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

#endif
