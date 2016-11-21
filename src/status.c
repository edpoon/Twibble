#include "status.h"

typedef struct {
  GBitmap *image;
  BitmapLayer *bitmap_layer;
  TextLayer *text_layer;
} StatusScreen;

void window_load(Window *window) {
  char *status_message = window_get_user_data(window);
  StatusScreen *status_screen = malloc(sizeof(StatusScreen));

  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Draw status image
  status_screen->image = gbitmap_create_with_resource(RESOURCE_ID_error);
  GRect image_bounds = gbitmap_get_bounds(status_screen->image);

  status_screen->bitmap_layer = bitmap_layer_create(GRect(0, 0, window_bounds.size.w, image_bounds.size.h));

#ifdef PBL_PLATFORM_APLITE
  bitmap_layer_set_compositing_mode(status_screen->bitmap_layer, GCompOpAssignInverted);
#else
  bitmap_layer_set_compositing_mode(status_screen->bitmap_layer, GCompOpSet);
#endif
  bitmap_layer_set_bitmap(status_screen->bitmap_layer, status_screen->image);

  // Draw status message
  GRect status_layer_bounds = GRect(0, image_bounds.size.h, window_bounds.size.w, window_bounds.size.h - image_bounds.size.h);
  status_screen->text_layer = text_layer_create(status_layer_bounds);
  text_layer_set_text(status_screen->text_layer, status_message);
  text_layer_set_text_alignment(status_screen->text_layer, GTextAlignmentCenter);
  text_layer_set_font(status_screen->text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));

  // Add layers
  layer_add_child(window_layer, bitmap_layer_get_layer(status_screen->bitmap_layer));
  layer_add_child(window_layer, text_layer_get_layer(status_screen->text_layer));
  window_set_user_data(window, status_screen);
}

static void window_unload(Window *window) {
  StatusScreen *status_screen = window_get_user_data(window);
  gbitmap_destroy(status_screen->image);
  bitmap_layer_destroy(status_screen->bitmap_layer);
  text_layer_destroy(status_screen->text_layer);
  free(status_screen);
  window_destroy(window);
}

// Display current status of app on phone
void display_status(char *status_message) {
  Window *window = window_create();

  window_set_window_handlers(window, (WindowHandlers) {
                               .load = window_load,
                               .unload = window_unload,
                             });

  window_set_user_data(window, status_message);

  bool animated = false;
  window_stack_pop(animated);
  window_stack_push(window, animated);
}
