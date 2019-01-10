#include "constants.h"




static void main_window_stack_push(int inWindowIndex) {
  window_stack_push(windowStores[inWindowIndex].window, true);
  if (inWindowIndex != LAP_TIMES_WINDOW_INDEX) {
    window_set_click_config_provider(windowStores[inWindowIndex].window, windowStores[inWindowIndex].click_provider);
  }
}

static void empty_click_config_provider(void* context) {}

static void empty_click_handler(ClickRecognizerRef recognizer, void* context) {}

static void main_up_click_handler(ClickRecognizerRef recognizer, void* context) {
  time_tds currentTime = getTimeDs();
  (*upCP)(currentTime);
}

static void select_click_handler(ClickRecognizerRef recognizer, void* context) {
  time_tds currentTime = getTimeDs();
  (*selectCP)(currentTime);
}

static void main_down_click_handler(ClickRecognizerRef recognizer, void* context) {
  time_tds currentTime = getTimeDs();
  (*downCP)(currentTime);
}

static void main_click_config_provider(void* context) {
  window_single_click_subscribe(BUTTON_ID_UP, main_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, main_down_click_handler);

  window_long_click_subscribe(BUTTON_ID_DOWN, 1000, empty_click_handler, empty_click_handler);
}

static void lap_times_click_config_provider(void* context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void yes_no_up_click_handler(ClickRecognizerRef recognizer, void* context) {
  window_stack_pop(true);
  if (window_stack_get_top_window() == windowStores[LAP_TIMES_WINDOW_INDEX].window) {
    window_stack_pop(true);
  }
}

static void yes_no_down_click_handler(ClickRecognizerRef recognizer, void* context) {
  window_stack_pop(true);
}

static void yes_no_click_config_provider(void* context) {
  window_single_click_subscribe(BUTTON_ID_UP, yes_no_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, yes_no_down_click_handler);
}

static WindowStore windowStores[NUM_WINDOWS] = {
  {NULL, &main_click_config_provider},
  {NULL, &empty_click_config_provider},
  {NULL, &lap_times_click_config_provider},
  {NULL, &yes_no_click_config_provider},
  {NULL, &empty_click_config_provider},
  {NULL, &empty_click_config_provider},
};

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
