
#include "main_window.h"
#include "modules/constants.h"
#include "modules/global.h"
#include "modules/mainInit.h"

struct vec {
  int xMult;
  int xOffset;
  int yMult;
  int yOffset;
  int xPointDir;
  int yPointDir;
};

#define QUADRANTS 4
static struct vec s_lineOffset[QUADRANTS] = {
  {  1, 0 ,  0, 0,  0,  1},
  {  0, 1 ,  1, 0, -1,  0},
  { -1, 1 ,  0, 1,  0, -1},
  {  0, 0 , -1, 1,  1,  0}
};

static struct vec pointerOffset[QUADRANTS];

#define POINTER_LEN 10

#define MAX_EVENTS 4
struct event {
  time_t timeTill;
  void (*function)();
};

static void singleBuzz() {
  vibes_short_pulse();
}

static struct event s_events[MAX_EVENTS] = {
  { 0, singleBuzz },
  { 10, singleBuzz },
  { 20, singleBuzz },
  { 40, singleBuzz },
};

static void handleBuzz(time_t inDisplayTime) {

  if (getIsPaused()) {
    return;
  }

  for (int eventIndex = 0; eventIndex < MAX_EVENTS; eventIndex++) {
    if (s_events[eventIndex].timeTill == inDisplayTime) {
      s_events[eventIndex].function();
    }
  }
}

static void showBandAndCountdownTH(time_tds inCurrentTime) {

  if ((window_stack_contains_window(windowStores[PROJECTED_WINDOW_INDEX].window)) ||
      (window_stack_contains_window(windowStores[DATA_RECEIVED_WINDOW_INDEX].window)) ||
      (window_stack_contains_window(windowStores[INVALID_STATE_WINDOW_INDEX].window)))
  {
    if ((s_revertToDisplay==0) || (inCurrentTime >= s_revertToDisplay)) {
      window_stack_pop(true);
      return;
    }
  }

  time_t displayTime = getDisplayTime(inCurrentTime);

  static char s_buffer1[9];
  static char s_buffer2[9];

  snprintf(s_buffer1, sizeof(s_buffer1), "%d", (int)displayTime);
  text_layer_set_text((TextLayer *)layerStores[COUNT_DOWN_LAYER_INDEX].layer, s_buffer1);

  snprintf(s_buffer2, sizeof(s_buffer2), "%d", (int)getCountDownBand(inCurrentTime));
  text_layer_set_text((TextLayer *)layerStores[CURRENT_BAND_LAYER_INDEX].layer, s_buffer2);

  handleBuzz(displayTime);
}

static void drawPointer(GContext* ctx, int inQuadrant, int inPercentTimes10) {

  int xStart = pointerOffset[inQuadrant].xMult * inPercentTimes10 / 1000 + pointerOffset[inQuadrant].xOffset;
  int yStart = pointerOffset[inQuadrant].yMult * inPercentTimes10 / 1000 + pointerOffset[inQuadrant].yOffset;

  graphics_fill_circle(ctx, GPoint(xStart, yStart), POINTER_WID / 2);
}

static void drawSingleLine(GContext* ctx, int inQuadrant, int inPercentTimes10) {

  int xStart = s_lineOffset[inQuadrant].xMult * inPercentTimes10 / 1000 + s_lineOffset[inQuadrant].xOffset;
  int yStart = s_lineOffset[inQuadrant].yMult * inPercentTimes10 / 1000 + s_lineOffset[inQuadrant].yOffset;

  int xEnd = xStart + s_lineOffset[inQuadrant].xPointDir * POINTER_LEN;
  int yEnd = yStart + s_lineOffset[inQuadrant].yPointDir * POINTER_LEN;

  graphics_draw_line(ctx, GPoint(xStart, yStart), GPoint(xEnd, yEnd));
}

void pointer_layer_update_callback(Layer* layer, GContext* ctx) {

  //GColor backColor = s_blackBackground ? GColorBlack : GColorWhite;
  GColor foreColor = s_blackBackground ? GColorWhite : GColorBlack;

  graphics_context_set_stroke_color(ctx, foreColor);
  graphics_context_set_fill_color(ctx, foreColor);

  time_tds currentTime = getTimeDs();

  int percentCompleteTimes10 = getPercentCompleteTimes10(currentTime);
  int quadrant = percentCompleteTimes10 / 250;
  // make sure it is not > 3
  quadrant &= 3;

  int percentCompleteInThisQuadrantTimes10 = 4 * (percentCompleteTimes10 - quadrant * 250);

  drawPointer(ctx, quadrant, percentCompleteInThisQuadrantTimes10);

  for (int quad = 0; quad < QUADRANTS; quad++) {
    drawSingleLine(ctx, quad, 0);
    drawSingleLine(ctx, quad, 1000);
  }
}

static void main_window_load(Window* window) {

  GColor backColor =  s_blackBackground ? GColorBlack : GColorWhite;
  //GColor foreColor = s_blackBackground ? GColorWhite : GColorBlack;

  window_set_background_color(window, backColor);

  // Get information about the Window
  Layer* window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_drawWidth = bounds.size.w - ACTION_BAR_WIDTH;
  s_drawHeight = bounds.size.h;

  layerStores[POINTER_LAYER].layer = layer_create(GRect(0, 0, s_drawWidth, s_drawHeight));
  layer_set_update_proc(layerStores[POINTER_LAYER].layer, pointer_layer_update_callback);
  layer_add_child(window_layer, layerStores[POINTER_LAYER].layer);

  addTextLayers(MAIN_WINDOW_INDEX);

  currTickHand = &showBandAndCountdownTH;

  layerStores[ACTION_BAR_LAYER].layer = (Layer *)action_bar_layer_create();
  action_bar_layer_set_icon((ActionBarLayer *)layerStores[ACTION_BAR_LAYER].layer, BUTTON_ID_DOWN, s_play_bitmap);
  action_bar_layer_add_to_window((ActionBarLayer *)layerStores[ACTION_BAR_LAYER].layer, window);

  for (int quad = 0; quad < QUADRANTS; quad++) {

    pointerOffset[quad].xMult = s_lineOffset[quad].xMult * (s_drawWidth - 1 - (POINTER_WID + 1));
    pointerOffset[quad].xOffset = s_lineOffset[quad].xOffset * (s_drawWidth - 1 - (POINTER_WID + 1)) + (POINTER_WID + 1) / 2;

    pointerOffset[quad].yMult = s_lineOffset[quad].yMult * (s_drawHeight - 1 - (POINTER_WID + 1));
    pointerOffset[quad].yOffset = s_lineOffset[quad].yOffset * (s_drawHeight - 1 - (POINTER_WID + 1)) + (POINTER_WID + 1) / 2;

    s_lineOffset[quad].xMult *= (s_drawWidth - 1);
    s_lineOffset[quad].xOffset *= (s_drawWidth - 1);

    s_lineOffset[quad].yMult *= (s_drawHeight - 1);
    s_lineOffset[quad].yOffset *= (s_drawHeight - 1);
  }
}

