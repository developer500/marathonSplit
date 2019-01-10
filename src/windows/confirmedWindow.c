/**
 * Example implementation of the dialog choice UI pattern.
 */

#include "confirmedWindow.h"
#include "../c/constants.h"
#include "../c/main.h"

void confirmed_window_load(Window* window) {

	GColor backColor = s_blackBackground ? GColorBlack : GColorWhite;
	//GColor foreColor = s_blackBackground ? GColorWhite : GColorBlack;

	window_set_background_color(window, backColor);

	addTextLayers(CONFIRMED_WINDOW_INDEX);
}
