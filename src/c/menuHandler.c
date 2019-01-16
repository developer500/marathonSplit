#include "constants.h"
#include "menuHandler.h"

#define NUM_MENU_SECTIONS 2
#define NUM_FIRST_MENU_ITEMS 1

static MenuLayer *s_menu_layer = 0;

static bool s_showActualTines;

static int s_numTimes;
static time_t s_plannedTotalTime;
static time_t s_actualTotalTime;

static time_t s_plannedTime[MAX_BANDS];
static time_t s_actualTime[MAX_BANDS];


void setShowActualTimes() {
	s_showActualTines = true;
}
void setShowPlannedTimes() {
	s_showActualTines = false;
}

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      return NUM_FIRST_MENU_ITEMS;
    case 1:
      return s_numTimes;
    default:
      return 0;
  }
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT+6;
}

static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  return MENU_CELL_BASIC_HEADER_HEIGHT+6;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Determine which section we're working with
  switch (section_index) {
    case 0:
      // Draw title text in the section header
      menu_cell_basic_header_draw(ctx, cell_layer, "Total Time");
      break;
    case 1:
      menu_cell_basic_header_draw(ctx, cell_layer, s_showActualTines ? "Split Times" : "Planned Lap Times");
      break;
  }
}

typedef struct timeFormatStruct {
  time_t maxTime;
  char * format;
  bool chopPrefix;
  bool showTenths;
} timeFormat;

#define MAX_TIME_FORMATS 6

timeFormat timeFormats[MAX_TIME_FORMATS] = {
    { SECONDS_PER_MINUTE / 6, "%S", true, true},
    { 1 * SECONDS_PER_MINUTE, "%S", false, true},
    { 10 * SECONDS_PER_MINUTE, "%M:%S", true, false},
    { 1 * SECONDS_PER_HOUR, "%M:%S", false, false},
    { 10 * SECONDS_PER_HOUR, "%H:%M:%S", true, false},
    { 24 * SECONDS_PER_HOUR, "%H:%M:%S", false, false},
  };

int getTimeFromFormat(struct tm * inTimeP, int inTenths, int inFormatIndex, char * outPlusTime) {
  
  char strTimeBuffer[11];
  
  int returnLength = strftime(strTimeBuffer, sizeof(strTimeBuffer), timeFormats[inFormatIndex].format, inTimeP);
  char * pstrTimeBuffer = strTimeBuffer;
  if (timeFormats[inFormatIndex].chopPrefix) {
    pstrTimeBuffer++;
    returnLength--;
  }
  
  if (timeFormats[inFormatIndex].showTenths) {
    if (inTenths!=0) {
      pstrTimeBuffer += returnLength;
      snprintf(pstrTimeBuffer, 3, ".%d", inTenths);
      pstrTimeBuffer -= returnLength;
      returnLength+=2;
    }
  }

  strcpy(outPlusTime, pstrTimeBuffer);
  
  return returnLength;
}

int getTimeFromTimeT(time_tds inTime, bool inAddPlus, char * outPlusTime) {
  
  int returnLength = 0;
  
  if (inAddPlus) {
    outPlusTime[0] = inTime < 0 ? '-' : '+';
    outPlusTime++;
    returnLength++;
  }
  
  time_t absInTime = abs(inTime/10);
  struct tm * inTimeP = gmtime(&absInTime);
  int tenths = abs(inTime % 10);
  
  int formatIndex;
  for (formatIndex = 0 ; formatIndex < (MAX_TIME_FORMATS - 1) ; formatIndex++) {
    if (absInTime < timeFormats[formatIndex].maxTime) {
      returnLength += getTimeFromFormat(inTimeP, tenths, formatIndex, outPlusTime);
      return returnLength;
    }
  }
  returnLength += getTimeFromFormat(inTimeP, tenths, formatIndex, outPlusTime);
  return returnLength;
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  // Determine which section we're going to draw in
  
  char mainBuffer[50];
  
  switch (cell_index->section) {
    case 0:
      // Use the row to specify which item we'll draw
      switch (cell_index->row) {
        case 0: {
        
          // This is a basic menu item with a title and subtitle
          char * mainBufferP = mainBuffer + getTimeFromTimeT(s_showActualTines ? s_actualTotalTime : s_plannedTotalTime, false, mainBuffer);

          if (s_showActualTines) {
              strcpy(mainBufferP, "   ");
              mainBufferP+=3;

              getTimeFromTimeT((s_actualTotalTime - s_plannedTotalTime), true, mainBufferP);
          }

          menu_cell_basic_draw(ctx, cell_layer, mainBuffer, NULL, NULL);
          break;
        }
      }
      break;
    case 1:
     {
        char * mainBufferP = mainBuffer + snprintf(mainBuffer, sizeof(mainBuffer), "%d)  ", cell_index->row + 1);

        mainBufferP += getTimeFromTimeT(s_showActualTines ? s_actualTime[cell_index->row] : s_plannedTime[cell_index->row], false, mainBufferP);

        strcpy(mainBufferP, "   ");
        mainBufferP+=3;

        if (s_showActualTines) {
            getTimeFromTimeT(s_actualTime[cell_index->row] - s_plannedTime[cell_index->row]
              , true, mainBufferP);
        } else {
        	strcpy(mainBufferP, "   ");
        	//snprintf(mainBufferP, 4, "%d", (int)(s_plannedTime[cell_index->row]/10));
        }


        menu_cell_basic_draw(ctx, cell_layer, mainBuffer, NULL, NULL);
     }
  }
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  
}

MenuLayer * loadMenu(Window *window) {

  // Now we prepare to initialize the menu layer
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // Create the menu layer
  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .get_cell_height = menu_get_cell_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(s_menu_layer, window);

  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
  
  return s_menu_layer;
}

void lapTimes_window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
}

void setTimes(time_tds plannedTime[], time_tds actualTime[], int inNumTimes) {

  s_numTimes = inNumTimes;
  
  s_plannedTotalTime = 0;
  s_actualTotalTime = 0;

  for (int timeIndex = 0; timeIndex < inNumTimes; timeIndex++) {
    s_plannedTime[timeIndex] = *plannedTime;
    s_actualTime[timeIndex] = *actualTime;
    s_plannedTotalTime += *plannedTime;
    s_actualTotalTime += *actualTime;
    plannedTime++;
    actualTime++;
  }
}
