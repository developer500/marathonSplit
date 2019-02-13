//#define testOnly
#include "main.h"
#include "../windows/dialogChoiceWindow.h"
#include "../windows/replaceDataWindow.h"
#include "../windows/confirmedWindow.h"
#include "../windows/addLapsWindow.h"
#include "persistHandler.h"
#include "menuHandler.h"

#ifdef testOnly
#include "testMain.c"
#endif

#ifndef testOnly
#include <pebble.h>
#include "timeCalculator.h"

static void singleBuzz() {
	vibes_short_pulse();
}

static time_tds s_revertToDisplay = 0;
static time_tds s_projectedFinishTime = 0;

static int s_drawWidth;
static int s_drawHeight;

#define MAX_EVENTS 4

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

static struct vec s_pointerOffset[QUADRANTS];

#define POINTER_LEN 10
#define POINTER_WID 12

#define COUNT_DOWN_LAYER_INDEX 0
#define CURRENT_BAND_LAYER_INDEX 1
#define PROJECTED_LAYER_INDEX 2
#define NEXT_LAP_PACE_INDEX 3

#define POINTER_LAYER 14
#define ACTION_BAR_LAYER (POINTER_LAYER + 1)
#define LAP_TIMES_LAYER_INDEX (POINTER_LAYER + 2)
#define NUM_LAYERS (POINTER_LAYER + 3)

static void main_click_config_provider(void* context);
static void empty_click_config_provider(void* context) {}
static void lap_times_click_config_provider(void* context);
static void yes_no_click_config_provider(void* context);

static void main_window_load(Window* window);
static void window_unload(Window * inWindow);
static void projected_window_load(Window* window);
static void lapTimes_window_load(Window* window);

typedef struct {
	Window* window;
	void (*click_provider)(void* context);
	void (*load_provider)(Window* inWindow);
	void (*unload_provider)(Window* inWindow);
} WindowStore;

static WindowStore s_windowStores[NUM_WINDOWS] = {
		{NULL, &main_click_config_provider, &main_window_load, &window_unload},
		{NULL, &empty_click_config_provider, &projected_window_load, &window_unload},
		{NULL, &lap_times_click_config_provider, &lapTimes_window_load, &lapTimes_window_unload},
		{NULL, &yes_no_click_config_provider, &dialog_choice_window_load, &dialog_choice_window_unload},
		{NULL, &replace_data_click_provider, &replace_data_window_load, &replace_data_window_unload},
		{NULL, &empty_click_config_provider, &confirmed_window_load, &window_unload},
		{NULL, &add_laps_click_provider, &add_laps_window_load, &window_unload},
};

#define TYPE_TEXT 0
#define TYPE_POINTER 1
#define TYPE_ACTION_BAR 2

typedef struct {
	Layer* layer;
	char *res;
	int  textHeight;
	int drawHeightTimes;
	int drawHeightDividedBy;
	int windowIndex;
	bool fullWidth;
	char * initialText;
	int layeType;
} LayerStore;

static LayerStore s_layerStores[NUM_LAYERS] = {
		{ NULL, FONT_KEY_ROBOTO_BOLD_SUBSET_49, 49, 1, 4, MAIN_WINDOW_INDEX, false, "   ", TYPE_TEXT},
		{ NULL, FONT_KEY_ROBOTO_BOLD_SUBSET_49, 49, 3, 4, MAIN_WINDOW_INDEX, false, "   ", TYPE_TEXT},
		{ NULL, FONT_KEY_ROBOTO_BOLD_SUBSET_49, 49, 1, 4, PROJECTED_WINDOW_INDEX, true, "00:00", TYPE_TEXT},
		{ NULL, FONT_KEY_ROBOTO_BOLD_SUBSET_49, 49, 3, 4, PROJECTED_WINDOW_INDEX, true, "00:00", TYPE_TEXT},
		{ NULL, FONT_KEY_GOTHIC_18, 21, 3, 32, MAIN_WINDOW_INDEX, false, "Sec this lap", TYPE_TEXT},
		{ NULL, FONT_KEY_GOTHIC_18, 21, 9, 16, MAIN_WINDOW_INDEX, false, "Laps remaining", TYPE_TEXT},
		{ NULL, FONT_KEY_GOTHIC_18, 21, 5, 64, PROJECTED_WINDOW_INDEX, false, "Projected finish", TYPE_TEXT},
		{ NULL, FONT_KEY_GOTHIC_18, 21, 37, 64, PROJECTED_WINDOW_INDEX, false, "Next Lap Pace", TYPE_TEXT},
		{ NULL, FONT_KEY_GOTHIC_18, 21, 4, 32, REPLACE_DATA_WINDOW_INDEX, false, "Sec. Replace", TYPE_TEXT},
		{ NULL, FONT_KEY_GOTHIC_18, 21, 15, 32, REPLACE_DATA_WINDOW_INDEX, false, "Whole Replace", TYPE_TEXT},
		{ NULL, FONT_KEY_GOTHIC_18, 21, 27, 32, REPLACE_DATA_WINDOW_INDEX, false, "Back", TYPE_TEXT},
		{ NULL, FONT_KEY_GOTHIC_18, 21, 12, 32, CONFIRMED_WINDOW_INDEX, false, "Whole Replace       ", TYPE_TEXT},
		{ NULL, FONT_KEY_GOTHIC_18, 21, 20, 32, CONFIRMED_WINDOW_INDEX, false, "Confirmed", TYPE_TEXT},
		{ NULL, FONT_KEY_GOTHIC_18, 21, 3, 32, ADD_LAPS_WINDOW_INDEX, false, "Add a lap", TYPE_TEXT},
		{ NULL, FONT_KEY_GOTHIC_18, 0, 0, 0, MAIN_WINDOW_INDEX, false, "", TYPE_POINTER},
		{ NULL, FONT_KEY_GOTHIC_18, 0, 0, 0, MAIN_WINDOW_INDEX, false, "", TYPE_ACTION_BAR},
		{ NULL, FONT_KEY_GOTHIC_18, 0, 0, 0, MAIN_WINDOW_INDEX, false, "", TYPE_ACTION_BAR},
};

static GBitmap *s_empty_bitmap, *s_play_bitmap, *s_pause_bitmap, *s_next_bitmap, *s_cross_bitmap, *s_info_bitmap , *s_settings_bitmap;

#define INITIAL_STATE 0
#define RUNNING_STATE 1
#define PAUSED_STATE 2
#define FINISHED_STATE 3

#define BUTTONS_DEFINED 3

static void (*upCP)(time_tds inCurrentTime);
static void (*selectCP)(time_tds inCurrentTime);
static void (*downCP)(time_tds inCurrentTime);

static void handleBuzz(time_t inDisplayTime) {

	struct event {
		time_t timeTill;
		void (*function)();
	} events[MAX_EVENTS] = {
			{ 0, singleBuzz },
			{ 10, singleBuzz },
			{ 20, singleBuzz },
			{ 40, singleBuzz },
	};

	if (getIsPaused()) {
		return;
	}

	for (int eventIndex = 0; eventIndex < MAX_EVENTS; eventIndex++) {
		if (events[eventIndex].timeTill == inDisplayTime) {
			events[eventIndex].function();
		}
	}
}

static void showBandAndCountdownTH(time_tds inCurrentTime) {

	if (window_stack_contains_window(s_windowStores[PROJECTED_WINDOW_INDEX].window) ||
			window_stack_contains_window(s_windowStores[CONFIRMED_WINDOW_INDEX].window) ||
			window_stack_contains_window(s_windowStores[ADD_LAPS_WINDOW_INDEX].window))
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
	text_layer_set_text((TextLayer *)s_layerStores[COUNT_DOWN_LAYER_INDEX].layer, s_buffer1);

	snprintf(s_buffer2, sizeof(s_buffer2), "%d", (int)getCountDownBand(inCurrentTime));
	text_layer_set_text((TextLayer *)s_layerStores[CURRENT_BAND_LAYER_INDEX].layer, s_buffer2);

	handleBuzz(displayTime);
}

static void showProjectedFinishTimeTH(time_tds inCurrentTime) {

	time_t projectedFinishTime = getTimeFromDs(s_projectedFinishTime);

	struct tm *projFinishTimeP = gmtime(&projectedFinishTime);
	static char s_time_tdsext[] = "00:00:00";
	strftime(s_time_tdsext, sizeof(s_time_tdsext), "%M:%S", projFinishTimeP);
	text_layer_set_text((TextLayer *)s_layerStores[PROJECTED_LAYER_INDEX].layer, s_time_tdsext);

	time_t nextPlannedLapTime = (time_t)(getCurrentPlannedLapTime(inCurrentTime)/10);
	struct tm *nextPlannedLapTimeP = gmtime(&nextPlannedLapTime);
	static char s_next_planned[] = "00:00:00";
	strftime(s_next_planned, sizeof(s_next_planned), "%M:%S", nextPlannedLapTimeP);

	char * next_plannedP = s_next_planned;

	if (s_next_planned[0]=='0') {
		next_plannedP++;
	}

	text_layer_set_text((TextLayer *)s_layerStores[NEXT_LAP_PACE_INDEX].layer, next_plannedP);
}

void main_window_stack_push(int inWindowIndex) {
	window_stack_push(s_windowStores[inWindowIndex].window, true);
	if (inWindowIndex != LAP_TIMES_WINDOW_INDEX) {
		window_set_click_config_provider(s_windowStores[inWindowIndex].window, s_windowStores[inWindowIndex].click_provider);
	}
}

static void changeDisplayProjectedFinish(time_tds inCurrentTime) {

	s_projectedFinishTime = getProjectedFinishTime(inCurrentTime);
	main_window_stack_push(PROJECTED_WINDOW_INDEX);
	s_revertToDisplay = inCurrentTime + (time_tds)80;
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

static void set_action_icons(int inSystemState);

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

	if (!getIsFinished(inCurrentTime)) {
		changeDisplayProjectedFinish(inCurrentTime);
	} else {
		main_window_stack_push(REPLACE_DATA_WINDOW_INDEX);
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

static void addLapsCP(time_tds inCurrentTime) {
	main_window_stack_push(ADD_LAPS_WINDOW_INDEX);
}

static void set_action_icons(int inSystemState) {

	struct {
		GBitmap** bitmap;
		void (*click_provider)(time_tds inCurrentTime);
	} sub_click_providers[(FINISHED_STATE+1)*BUTTONS_DEFINED] = {
			//               up                              center                           down
			/*initial */ {&s_settings_bitmap, &plannedCP},  {&s_empty_bitmap, &emptyCP},     {&s_play_bitmap, &initialStartCP},
			/*running */ {&s_pause_bitmap, &setPausedCP},    {&s_empty_bitmap, &addLapsCP},     {&s_next_bitmap, &runningStartCP},
			/*paused  */ {&s_empty_bitmap, &emptyCP},        {&s_cross_bitmap, &clearDataCP}, {&s_play_bitmap, &resetPausedCP},
			/*finished*/ {&s_empty_bitmap, &emptyCP},        {&s_cross_bitmap, &clearDataCP}, {&s_info_bitmap, &runningStartCP},
	};

	int click_providerIndex = BUTTONS_DEFINED*inSystemState;
	action_bar_layer_set_icon((ActionBarLayer *)s_layerStores[ACTION_BAR_LAYER].layer, BUTTON_ID_UP, *sub_click_providers[click_providerIndex++].bitmap);
	action_bar_layer_set_icon((ActionBarLayer *)s_layerStores[ACTION_BAR_LAYER].layer, BUTTON_ID_SELECT, *sub_click_providers[click_providerIndex++].bitmap);
	action_bar_layer_set_icon((ActionBarLayer *)s_layerStores[ACTION_BAR_LAYER].layer, BUTTON_ID_DOWN, *sub_click_providers[click_providerIndex++].bitmap);

	click_providerIndex = BUTTONS_DEFINED*inSystemState;

	upCP = sub_click_providers[click_providerIndex++].click_provider;
	selectCP = sub_click_providers[click_providerIndex++].click_provider;
	downCP = sub_click_providers[click_providerIndex++].click_provider;
}

static void initData() {

	char runName[50];

	time_tds timeBands[MAX_BANDS];
	int numberBands = getTimeFromPersist(0, timeBands, runName);

	setCountDownBands(timeBands, numberBands);

	initDisplayTime();
	set_action_icons(INITIAL_STATE);
}

static void yes_no_up_click_handler(ClickRecognizerRef recognizer, void* context) {
	window_stack_pop(true);
	if (window_stack_get_top_window() == s_windowStores[LAP_TIMES_WINDOW_INDEX].window) {
		window_stack_pop(true);
	}
	initData();
}

static void yes_no_down_click_handler(ClickRecognizerRef recognizer, void* context) {
	window_stack_pop(true);
}

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

static void empty_click_handler(ClickRecognizerRef recognizer, void* context) {
}

static void yes_no_click_config_provider(void* context) {
	window_single_click_subscribe(BUTTON_ID_UP, yes_no_up_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, yes_no_down_click_handler);
}

static void lap_times_click_config_provider(void* context) {
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void main_click_config_provider(void* context) {
	window_single_click_subscribe(BUTTON_ID_UP, main_up_click_handler);
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, main_down_click_handler);

	window_long_click_subscribe(BUTTON_ID_DOWN, 1000, empty_click_handler, empty_click_handler);
}

static void drawPointer(GContext* ctx, int inQuadrant, int inPercentTimes10) {

	int xStart = s_pointerOffset[inQuadrant].xMult * inPercentTimes10 / 1000 + s_pointerOffset[inQuadrant].xOffset;
	int yStart = s_pointerOffset[inQuadrant].yMult * inPercentTimes10 / 1000 + s_pointerOffset[inQuadrant].yOffset;

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

static void tick_handler(struct tm* tick_time, TimeUnits units_changed) {

	time_tds currentTimeds = getTimeDs();

	showBandAndCountdownTH(currentTimeds);
}

void setRevertToDisplay(time_tds inNewTime) {
	s_revertToDisplay = inNewTime;
}

void setLayerStoreText(char * inText, int inLayerIndex) {
	strcpy(s_layerStores[inLayerIndex].initialText, inText);
}

void addTextLayers(int inWindowIndex) {

	GColor backColor = s_blackBackground ? GColorWhite : GColorBlack;
	GColor foreColor = s_blackBackground ? GColorBlack : GColorWhite;

	const int drawHeightWithoutPointer = s_drawHeight - (POINTER_WID * 2 + 2);

	LayerStore * layerPointer = s_layerStores;

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

		Layer* windowLayer = window_get_root_layer(s_windowStores[layerPointer->windowIndex].window);
		layer_add_child(windowLayer, text_layer_get_layer((TextLayer *)layerPointer->layer));

		text_layer_set_text(((TextLayer *)layerPointer->layer), layerPointer->initialText);
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

	s_layerStores[POINTER_LAYER].layer = layer_create(GRect(0, 0, s_drawWidth, s_drawHeight));
	layer_set_update_proc(s_layerStores[POINTER_LAYER].layer, pointer_layer_update_callback);
	layer_add_child(window_layer, s_layerStores[POINTER_LAYER].layer);

	addTextLayers(MAIN_WINDOW_INDEX);

	s_play_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PLAY);
	s_pause_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PAUSE);
	s_next_bitmap = gbitmap_create_with_resource(RESOURCE_ID_NEXT);
	s_cross_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CROSS);
	s_info_bitmap = gbitmap_create_with_resource(RESOURCE_ID_INFO);
	s_settings_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SETTINGS);

	s_layerStores[ACTION_BAR_LAYER].layer = (Layer *)action_bar_layer_create();
	action_bar_layer_add_to_window((ActionBarLayer *)s_layerStores[ACTION_BAR_LAYER].layer, window);

	for (int quad = 0; quad < QUADRANTS; quad++) {

		s_pointerOffset[quad].xMult = s_lineOffset[quad].xMult * (s_drawWidth - 1 - (POINTER_WID + 1));
		s_pointerOffset[quad].xOffset = s_lineOffset[quad].xOffset * (s_drawWidth - 1 - (POINTER_WID + 1)) + (POINTER_WID + 1) / 2;

		s_pointerOffset[quad].yMult = s_lineOffset[quad].yMult * (s_drawHeight - 1 - (POINTER_WID + 1));
		s_pointerOffset[quad].yOffset = s_lineOffset[quad].yOffset * (s_drawHeight - 1 - (POINTER_WID + 1)) + (POINTER_WID + 1) / 2;

		s_lineOffset[quad].xMult *= (s_drawWidth - 1);
		s_lineOffset[quad].xOffset *= (s_drawWidth - 1);

		s_lineOffset[quad].yMult *= (s_drawHeight - 1);
		s_lineOffset[quad].yOffset *= (s_drawHeight - 1);
	}
}

static void projected_window_load(Window* window) {

	GColor backColor =  s_blackBackground ? GColorBlack : GColorWhite;
	//GColor foreColor = s_blackBackground ? GColorWhite : GColorBlack;

	window_set_background_color(window, backColor);

	addTextLayers(PROJECTED_WINDOW_INDEX);

	time_tds currentTime = getTimeDs();
	showProjectedFinishTimeTH(currentTime);
}

static void lapTimes_window_load(Window* window) {

	GColor backColor =  s_blackBackground ? GColorBlack : GColorWhite;
	GColor foreColor = s_blackBackground ? GColorWhite : GColorBlack;

	s_layerStores[LAP_TIMES_LAYER_INDEX].layer = (Layer *)loadMenu(window);
	menu_layer_set_normal_colors((MenuLayer *)s_layerStores[LAP_TIMES_LAYER_INDEX].layer, backColor, foreColor);
}

int getWindowIndexFromWindow(Window * inWindow) {

	for (int windowIndex = 0 ; windowIndex < NUM_WINDOWS  ; windowIndex++) {
		if (s_windowStores[windowIndex].window == inWindow) {
			return windowIndex;
		}
	}

	return -1;
}

static void window_unload(Window * inWindow) {

	int windowIndex = getWindowIndexFromWindow(inWindow);

	LayerStore * layerPointer = s_layerStores;
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

static void init() {

	for (int windowIndex = 0 ; windowIndex < NUM_WINDOWS ; windowIndex++) {
		s_windowStores[windowIndex].window = window_create();

		window_set_window_handlers(s_windowStores[windowIndex].window, (WindowHandlers) {
			.load = s_windowStores[windowIndex].load_provider,
					.unload = s_windowStores[windowIndex].unload_provider
		});
	}

	// Show the Window on the watch, with animated=true
	main_window_stack_push(MAIN_WINDOW_INDEX);

	initData();

	// Register with TickTimerService
	tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit() {
	// Destroy Window
	window_destroy(s_windowStores[PROJECTED_WINDOW_INDEX].window);
	window_destroy(s_windowStores[MAIN_WINDOW_INDEX].window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}

#endif
