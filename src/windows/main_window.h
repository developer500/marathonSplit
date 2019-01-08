#include <pebble.h>

#define DIALOG_CHOICE_WINDOW_MESSAGE "Clear Data?"

static GBitmap *s_empty_bitmap, *s_play_bitmap, *s_pause_bitmap, *s_next_bitmap, *s_cross_bitmap, *s_info_bitmap , *s_settings_bitmap;

static void main_window_load(Window *window);
void main_window_unload(Window *window);
