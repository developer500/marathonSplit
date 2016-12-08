#pragma once

#include <pebble.h>

#define DIALOG_CHOICE_WINDOW_MESSAGE "Clear Data?"

void dialog_choice_window_load(Window *window);
void dialog_choice_window_unload(Window *window);