#include "timeCalculator.h"
#include "global.h"

static void main_window_stack_push(int inWindowIndex);



typedef struct {
  Window* window;
  void (*click_provider)(void* context);
} WindowStore;

static WindowStore windowStores[NUM_WINDOWS];


static void (*currTickHand)(time_tds inCurrentTime);

time_tds getTimeDs();
time_t getTimeFromDs(time_tds inTimeDs);
time_t getTimeDsFromTime(time_t inTime, uint16_t inMillis); 
