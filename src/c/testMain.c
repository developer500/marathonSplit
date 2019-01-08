#ifdef testOnly
#include <pebble.h>
#include "testDetails.h"
#include "modules/menuHandler.h"
      
static Window *window;
static TextLayer *text_layer;
      
static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
    
  //---gets the bound of the layer---
  GRect bounds = layer_get_bounds(window_layer);
      
  // default values---
  text_layer = text_layer_create((GRect) {
    .origin = { 0, 72 },
    .size = { bounds.size.w, 20 } });
  
  text_layer_set_text(text_layer, testTimeCalculator("asd") ? "Passed" : "Failed");
  
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_load_menu(Window *window) {
  testTimeCalculator("asd");
  loadMenu(window);
}
      
static void window_unload(Window *window) {
  //---destroys the TextLayer---
  text_layer_destroy(text_layer);
}
      
static void init(void) {
  window = window_create();
    
  window_set_window_handlers(window, (WindowHandlers) {
     .load = window_load,
//     .load = window_load_menu,
//     .load = dialog_choice_window_load,
    .unload = window_unload,
  });
    
  //---push the window onto the navigation stack using sliding
  // animation---
  const bool animated = true;
  window_stack_push(window, animated);
}
      
static void deinit(void) {
  window_destroy(window);
}
      
int main(void) {
  
  init();
    
  app_event_loop();
    
  deinit();
}
#endif
