#include "lapTimes_window.h"
#include "modules/menuHandler.h"

static void lapTimes_window_load(Window* window) {

  GColor backColor =  s_blackBackground ? GColorBlack : GColorWhite;
  GColor foreColor = s_blackBackground ? GColorWhite : GColorBlack;

  layerStores[LAP_TIMES_LAYER_INDEX].layer = (Layer *)loadMenu(window);
  menu_layer_set_normal_colors((MenuLayer *)layerStores[LAP_TIMES_LAYER_INDEX].layer, backColor, foreColor);
}
