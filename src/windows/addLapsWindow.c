/**
 * Example implementation of the dialog choice UI pattern.
 */

#include "addLapsWindow.h"
#include "../c/constants.h"
#include "../c/main.h"
#include "../c/timeCalculator.h"

static ActionBarLayer *s_action_bar_layer;

static GBitmap *s_add_bitmap;

void add_lap_click_handler(ClickRecognizerRef recognizer, void* context) {

	time_tds currentTime = getTimeDs();
	addLap(currentTime);
	window_stack_pop(true);
}

void add_laps_window_load(Window* window) {

	GColor backColor = s_blackBackground ? GColorBlack : GColorWhite;
	//GColor foreColor = s_blackBackground ? GColorWhite : GColorBlack;

	window_set_background_color(window, backColor);

	setRevertToDisplay(getTimeDs() + (time_tds)100);

	addTextLayers(ADD_LAPS_WINDOW_INDEX);

	s_add_bitmap = gbitmap_create_with_resource(RESOURCE_ID_REPLACE);


	s_action_bar_layer = action_bar_layer_create();
	action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_UP, s_add_bitmap);

	action_bar_layer_add_to_window(s_action_bar_layer, window);
}

void add_laps_click_provider(void* context) {
	window_single_click_subscribe(BUTTON_ID_UP, add_lap_click_handler);
}
