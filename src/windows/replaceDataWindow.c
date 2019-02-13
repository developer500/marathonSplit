/**
 * Example implementation of the dialog choice UI pattern.
 */

#include "replaceDataWindow.h"
#include "../c/constants.h"
#include "../c/main.h"
#include "../c/timeCalculator.h"
#include "../c/persistHandler.h"
#include "../c/menuHandler.h"

static ActionBarLayer *s_action_bar_layer;

static GBitmap *s_replace_bitmap, *s_whole_bitmap, *s_cross_bitmap;

//(*upCP)(currentTime);

void updateTimes(time_tds (*funcPtr)(time_tds inActualTime, time_tds inPlannedTime)) {

	time_tds newTimes[MAX_BANDS];
	int numberBands = getCounDownBands();

	time_tds timeSaved = 0;

	for (int bandIndex = 0 ; bandIndex < numberBands; bandIndex++) {
		int plannedTime = getTimeForBand(bandIndex);
		int actualTime = getActualTime(bandIndex);
		newTimes[bandIndex] = funcPtr(actualTime, plannedTime);
		timeSaved += (plannedTime - newTimes[bandIndex]);
	}

	setTimeToPersist(newTimes, numberBands, "default", 0);

	char mainBuffer[50];
	char* mainBufferp = mainBuffer;

	if (timeSaved < 0) {
		strcpy(mainBufferp, "Lost     ");
	} else {
		strcpy(mainBufferp, "Improved ");
	}

	mainBufferp+=9;

	getTimeFromTimeT(timeSaved, false, mainBufferp);

	setLayerStoreText(mainBuffer, 11);

	window_stack_pop(true);

	setRevertToDisplay(getTimeDs() + (time_tds)20);

	main_window_stack_push(CONFIRMED_WINDOW_INDEX);
}

time_tds replace(time_tds inActualTime, time_tds inPlannedtime) {
	return (inActualTime != 0) && (inActualTime < inPlannedtime) ? inActualTime : inPlannedtime;
}

time_tds whole(time_tds inActualTime, time_tds inPlannedtime) {
	return inActualTime;
}

void replace_click_handler(ClickRecognizerRef recognizer, void* context) {

	updateTimes(&replace);
}

void whole_click_handler(ClickRecognizerRef recognizer, void* context) {
	updateTimes(&whole);
}

void back_click_handler(ClickRecognizerRef recognizer, void* context) {
	window_stack_pop(true);
}

void replace_data_window_load(Window* window) {

	GColor backColor = s_blackBackground ? GColorBlack : GColorWhite;
	//GColor foreColor = s_blackBackground ? GColorWhite : GColorBlack;

	window_set_background_color(window, backColor);

	addTextLayers(REPLACE_DATA_WINDOW_INDEX);

	s_replace_bitmap = gbitmap_create_with_resource(RESOURCE_ID_REPLACE);
	s_whole_bitmap = gbitmap_create_with_resource(RESOURCE_ID_WHOLE);
	s_cross_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CROSS);

	s_action_bar_layer = action_bar_layer_create();
	action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_UP, s_replace_bitmap);
	action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_SELECT, s_whole_bitmap);
	action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_DOWN, s_cross_bitmap);
	action_bar_layer_add_to_window(s_action_bar_layer, window);
}

void replace_data_click_provider(void* context) {
	window_single_click_subscribe(BUTTON_ID_UP, replace_click_handler);
	window_single_click_subscribe(BUTTON_ID_SELECT, whole_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, back_click_handler);
}

void replace_data_window_unload(Window *window) {

  action_bar_layer_destroy(s_action_bar_layer);

  gbitmap_destroy(s_replace_bitmap);
  gbitmap_destroy(s_whole_bitmap);
  gbitmap_destroy(s_cross_bitmap);
}
