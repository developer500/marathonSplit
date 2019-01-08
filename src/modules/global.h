
static int s_drawWidth;
static int s_drawHeight;

static void (*upCP)(time_tds inCurrentTime);
static void (*selectCP)(time_tds inCurrentTime);
static void (*downCP)(time_tds inCurrentTime);

#define POINTER_WID 12

#define POINTER_LAYER 10
#define ACTION_BAR_LAYER (POINTER_LAYER + 1)
#define LAP_TIMES_LAYER_INDEX (POINTER_LAYER + 2)
#define NUM_LAYERS (POINTER_LAYER + 3)

#define MAIN_WINDOW_INDEX 0
#define PROJECTED_WINDOW_INDEX 1
#define LAP_TIMES_WINDOW_INDEX 2
#define YES_NO_WINDOW_INDEX 3
#define DATA_RECEIVED_WINDOW_INDEX 4
#define INVALID_STATE_WINDOW_INDEX 5
#define NUM_WINDOWS (INVALID_STATE_WINDOW_INDEX + 1)

#define TYPE_TEXT 0
#define TYPE_POINTER 1
#define TYPE_ACTION_BAR 2

#define COUNT_DOWN_LAYER_INDEX 0
#define CURRENT_BAND_LAYER_INDEX 1
#define PROJECTED_LAYER_INDEX 2
#define NEXT_LAP_PACE_INDEX 3

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

static LayerStore layerStores[NUM_LAYERS] = {
    { NULL, FONT_KEY_ROBOTO_BOLD_SUBSET_49, 49, 1, 4, MAIN_WINDOW_INDEX, false, "   ", TYPE_TEXT},
    { NULL, FONT_KEY_ROBOTO_BOLD_SUBSET_49, 49, 3, 4, MAIN_WINDOW_INDEX, false, "   ", TYPE_TEXT},
    { NULL, FONT_KEY_ROBOTO_BOLD_SUBSET_49, 49, 1, 4, PROJECTED_WINDOW_INDEX, true, "00:00", TYPE_TEXT},
    { NULL, FONT_KEY_ROBOTO_BOLD_SUBSET_49, 49, 3, 4, PROJECTED_WINDOW_INDEX, true, "00:00", TYPE_TEXT},
    { NULL, FONT_KEY_GOTHIC_18, 21, 3, 32, MAIN_WINDOW_INDEX, false, "Sec this lap", TYPE_TEXT},
    { NULL, FONT_KEY_GOTHIC_18, 21, 9, 16, MAIN_WINDOW_INDEX, false, "Laps remaining", TYPE_TEXT},
    { NULL, FONT_KEY_GOTHIC_18, 21, 5, 64, PROJECTED_WINDOW_INDEX, false, "Projected finish", TYPE_TEXT},
    { NULL, FONT_KEY_GOTHIC_18, 21, 37, 64, PROJECTED_WINDOW_INDEX, false, "Next Lap Pace", TYPE_TEXT},
    { NULL, FONT_KEY_GOTHIC_18, 21, 3, 32, DATA_RECEIVED_WINDOW_INDEX, false, "Data Received", TYPE_TEXT},
    { NULL, FONT_KEY_GOTHIC_18, 21, 3, 32, INVALID_STATE_WINDOW_INDEX, false, "Must be reset", TYPE_TEXT},
    { NULL, FONT_KEY_GOTHIC_18, 0, 0, 0, MAIN_WINDOW_INDEX, false, "", TYPE_POINTER},
    { NULL, FONT_KEY_GOTHIC_18, 0, 0, 0, MAIN_WINDOW_INDEX, false, "", TYPE_ACTION_BAR},
    { NULL, FONT_KEY_GOTHIC_18, 0, 0, 0, MAIN_WINDOW_INDEX, false, "", TYPE_ACTION_BAR},
  };

static time_tds s_revertToDisplay = 0;
static bool s_blackBackground = true;
