#include "mainInit.h"
#include "modules/constants.h"
#include "windows/projected_window.h"
#include "windows/dialog_choice_window.h"
#include "windows/lapTimes_window.h"
#include "windows/main_window.h"

#include "menuHandler.h"


#define INITIAL_STATE 0
#define RUNNING_STATE 1
#define PAUSED_STATE 2
#define FINISHED_STATE 3

#define BUTTONS_DEFINED 3




typedef struct {
  GBitmap** bitmap;
  void (*click_provider)(time_tds inCurrentTime);
} Sub_Click_Provider;

static Sub_Click_Provider sub_click_providers[(FINISHED_STATE+1)*BUTTONS_DEFINED] = {
  //               up                              center                           down
  /*initial */ {&s_settings_bitmap, &plannedCP},  {&s_empty_bitmap, &emptyCP},     {&s_play_bitmap, &initialStartCP},
  /*running */ {&s_pause_bitmap, &setPausedCP},    {&s_empty_bitmap, &emptyCP},     {&s_next_bitmap, &runningStartCP},
  /*paused  */ {&s_empty_bitmap, &emptyCP},        {&s_cross_bitmap, &clearDataCP}, {&s_play_bitmap, &resetPausedCP},
  /*finished*/ {&s_empty_bitmap, &emptyCP},        {&s_cross_bitmap, &clearDataCP}, {&s_info_bitmap, &runningStartCP},
};

static void addTextLayers(int inWindowIndex) {

  GColor backColor = s_blackBackground ? GColorWhite : GColorBlack;
  GColor foreColor = s_blackBackground ? GColorBlack : GColorWhite;

  const int drawHeightWithoutPointer = s_drawHeight - (POINTER_WID * 2 + 2);

  LayerStore * layerPointer = layerStores;

  for (int layerIndex = 0 ; layerIndex < NUM_LAYERS; layerIndex++, layerPointer++) {

    if (layerPointer->layeType != TYPE_TEXT) {
      continue;
    }

    if (inWindowIndex != layerPointer->windowIndex) {
      continue;
    }

    int textYCoord = layerPointer->drawHeightTimes * drawHeightWithoutPointer / layerPointer->drawHeightDividedBy
      + POINTER_WID - layerPointer->textHeight / 2;

    int textXCoord = layerPointer->fullWidth ? 0 : POINTER_WID;
    int textXWidthOffset = layerPointer->fullWidth ? - ACTION_BAR_WIDTH : POINTER_WID * 2 + 2;

    layerPointer->layer = (Layer *)text_layer_create(GRect(textXCoord, textYCoord,
                                                          s_drawWidth - textXWidthOffset,
                                                          layerPointer->textHeight));

    text_layer_set_background_color((TextLayer *)layerPointer->layer, foreColor);
    text_layer_set_text_color((TextLayer *)layerPointer->layer, backColor);

    text_layer_set_font((TextLayer *)layerPointer->layer,
                        fonts_get_system_font(layerPointer->res));

    text_layer_set_text_alignment((TextLayer*)layerPointer->layer, GTextAlignmentLeft);

    Layer* windowLayer = window_get_root_layer(windowStores[layerPointer->windowIndex].window);
    layer_add_child(windowLayer, text_layer_get_layer((TextLayer *)layerPointer->layer));

    text_layer_set_text(((TextLayer *)layerPointer->layer), layerPointer->initialText);
  }
}

static void changeDisplayPlannedOrActualTimes(time_tds inCurrentTime) {

  time_tds plannedTime[MAX_BANDS];
  time_tds actualTime[MAX_BANDS];
  int numberBands = getCounDownBands();

  for (int bandIndex = 0 ; bandIndex < numberBands; bandIndex++) {
    plannedTime[bandIndex] = getTimeForBand(bandIndex);
    actualTime[bandIndex] = getActualTime(bandIndex);
  }

  setTimes(plannedTime, actualTime, numberBands);

  main_window_stack_push(LAP_TIMES_WINDOW_INDEX);
}

static void changeDisplayPlannedTimes(time_tds inCurrentTime) {

	setShowPlannedTimes();
	changeDisplayPlannedOrActualTimes(inCurrentTime);
}

static void changeDisplayActualTimes(time_tds inCurrentTime) {

	setShowActualTimes();
	changeDisplayPlannedOrActualTimes(inCurrentTime);
}

static void emptyCP(time_tds inCurrentTime) {}

static void setPausedCP(time_tds inCurrentTime) {
  togglePaused(inCurrentTime);
  set_action_icons(PAUSED_STATE);
}

static void resetPausedCP(time_tds inCurrentTime) {
  togglePaused(inCurrentTime);
  set_action_icons(RUNNING_STATE);
}

static void plannedCP(time_tds inCurrentTime) {
	changeDisplayPlannedTimes(inCurrentTime);
}

static void runningStartCP(time_tds inCurrentTime) {

  setStart(inCurrentTime);

  bool finished = getIsFinished(inCurrentTime);
  if (!finished) {
    changeDisplayProjectedFinish(inCurrentTime);
  } else {
	changeDisplayActualTimes(inCurrentTime);
    set_action_icons(FINISHED_STATE);
  }
}

static void clearDataCP(time_tds inCurrentTime) {
  main_window_stack_push(YES_NO_WINDOW_INDEX);
}

static void initialStartCP(time_tds inCurrentTime) {
  setStart(inCurrentTime);
  set_action_icons(RUNNING_STATE);
}

static void data_received_window_load(Window* window) {

  GColor backColor =  s_blackBackground ? GColorBlack : GColorWhite;
  //GColor foreColor = s_blackBackground ? GColorWhite : GColorBlack;

  window_set_background_color(window, backColor);

  addTextLayers(DATA_RECEIVED_WINDOW_INDEX);
}

static void invalid_state_window_load(Window* window) {

  GColor backColor =  s_blackBackground ? GColorBlack : GColorWhite;
  //GColor foreColor = s_blackBackground ? GColorWhite : GColorBlack;

  window_set_background_color(window, backColor);

  addTextLayers(INVALID_STATE_WINDOW_INDEX);
}

int getWindowIndexFromWindow(Window * inWindow) {

  for (int windowIndex = 0 ; windowIndex <= INVALID_STATE_WINDOW_INDEX  ; windowIndex++) {
    if (windowStores[windowIndex].window == inWindow) {
      return windowIndex;
    }
  }

  return -1;
}


static void set_action_icons(int inSystemState) {

  int click_providerIndex = BUTTONS_DEFINED*inSystemState;
  action_bar_layer_set_icon((ActionBarLayer *)layerStores[ACTION_BAR_LAYER].layer, BUTTON_ID_UP, *sub_click_providers[click_providerIndex++].bitmap);
  action_bar_layer_set_icon((ActionBarLayer *)layerStores[ACTION_BAR_LAYER].layer, BUTTON_ID_SELECT, *sub_click_providers[click_providerIndex++].bitmap);
  action_bar_layer_set_icon((ActionBarLayer *)layerStores[ACTION_BAR_LAYER].layer, BUTTON_ID_DOWN, *sub_click_providers[click_providerIndex++].bitmap);

  click_providerIndex = BUTTONS_DEFINED*inSystemState;

  upCP = sub_click_providers[click_providerIndex++].click_provider;
  selectCP = sub_click_providers[click_providerIndex++].click_provider;
  downCP = sub_click_providers[click_providerIndex++].click_provider;
}

static void window_unload(Window * inWindow) {

  int windowIndex = getWindowIndexFromWindow(inWindow);

  LayerStore * layerPointer = layerStores;
  for (int layerIndex = 0 ; layerIndex < NUM_LAYERS; layerIndex++, layerPointer++) {
    if (windowIndex != layerPointer->windowIndex) {
      continue;
  }

    switch(layerPointer->layeType) {
      case TYPE_TEXT:
        text_layer_destroy((TextLayer *)layerPointer->layer);
        break;

      case TYPE_POINTER:
        layer_destroy(layerPointer->layer);
        break;

      case TYPE_ACTION_BAR:
        action_bar_layer_destroy((ActionBarLayer *)layerPointer->layer);
        break;

      default:
        break;
    }
  }
}

void mainInit() {

	s_play_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PLAY);
	  s_pause_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PAUSE);
	  s_next_bitmap = gbitmap_create_with_resource(RESOURCE_ID_NEXT);
	  s_cross_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CROSS);
	  s_info_bitmap = gbitmap_create_with_resource(RESOURCE_ID_INFO);
	  s_settings_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SETTINGS);

	  for (int windowIndex = 0 ; windowIndex <= INVALID_STATE_WINDOW_INDEX ; windowIndex++) {
	      windowStores[windowIndex].window = window_create();
	    }

	    // Set handlers to manage the elements inside the Window
	    window_set_window_handlers(windowStores[MAIN_WINDOW_INDEX].window, (WindowHandlers) {
	      .load =
	        main_window_load, .unload = window_unload
	    });

	    window_set_window_handlers(windowStores[PROJECTED_WINDOW_INDEX].window, (WindowHandlers) {
	      .load =
	        projected_window_load, .unload = window_unload
	    });


	    window_set_window_handlers(windowStores[LAP_TIMES_WINDOW_INDEX].window, (WindowHandlers) {
	      .load =
	        lapTimes_window_load, .unload = lapTimes_window_unload
	    });

	    window_set_window_handlers(windowStores[YES_NO_WINDOW_INDEX].window, (WindowHandlers) {
	      .load =
	        dialog_choice_window_load, .unload = dialog_choice_window_unload
	    });

	    window_set_window_handlers(windowStores[DATA_RECEIVED_WINDOW_INDEX].window, (WindowHandlers) {
	      .load =
	        data_received_window_load, .unload = window_unload
	    });

	    window_set_window_handlers(windowStores[INVALID_STATE_WINDOW_INDEX].window, (WindowHandlers) {
	      .load =
	        invalid_state_window_load, .unload = window_unload
	    });

}
