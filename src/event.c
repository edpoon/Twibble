#include "event.h"

// Key values for AppMessage dictionary
enum {
  QUERY_KEY,
  OFFSET_KEY,
  TITLE_KEY,
  SUBTITLE1_KEY,
  SUBTITLE2_KEY,
  ERROR_KEY
};

// Write message to buffer and send
void send_message(const char *query, uint8_t offset) {
  Tuplet queryTuplet = TupletCString(QUERY_KEY, query);
  Tuplet offsetTuplet = TupletInteger(OFFSET_KEY, offset);
  DictionaryIterator *iter;

  app_message_outbox_begin(&iter);
  dict_write_tuplet(iter, &queryTuplet);
  dict_write_tuplet(iter, &offsetTuplet);
  dict_write_end(iter);

  app_message_outbox_send();
}

typedef struct {
  GBitmap *image;
  BitmapLayer *image_layer;
  TextLayer *message_layer;
} ErrorScreen;

void error_window_load(Window *window) {
  char *error_message = window_get_user_data(window);
  ErrorScreen *error = malloc(sizeof(ErrorScreen));
  memset(error, 0, sizeof(ErrorScreen));

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Draw error image
  error->image = gbitmap_create_with_resource(RESOURCE_ID_error);
  GRect error_image_bounds = gbitmap_get_bounds(error->image);
  error->image_layer = bitmap_layer_create(GRect(0, 0, bounds.size.w, error_image_bounds.size.h));
#ifdef PBL_PLATFORM_BASALT
  bitmap_layer_set_compositing_mode(error->image_layer, GCompOpSet);
#elif PBL_PLATFORM_APLITE
  bitmap_layer_set_compositing_mode(error->image_layer, GCompOpAssignInverted);
#endif
  bitmap_layer_set_bitmap(error->image_layer, error->image);
  layer_add_child(window_layer, bitmap_layer_get_layer(error->image_layer));

  // Draw error message
  error->message_layer = text_layer_create(GRect(0, error_image_bounds.size.h, bounds.size.w, bounds.size.h - error_image_bounds.size.h));
  text_layer_set_text(error->message_layer, error_message);
  text_layer_set_text_alignment(error->message_layer, GTextAlignmentCenter);
  text_layer_set_font(error->message_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));

  layer_add_child(window_layer, text_layer_get_layer(error->message_layer));
  window_set_user_data(window, error);
}

void error_window_unload(Window *window) {
  ErrorScreen *error = window_get_user_data(window);
  gbitmap_destroy(error->image);
  bitmap_layer_destroy(error->image_layer);
  text_layer_destroy(error->message_layer);
  free(error);
  window_destroy(window);
}

// Error message generated upon failure to retrieve proper info from API
static void display_error(char *error_message) {
  Window *window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
      .load = error_window_load,
        .unload = error_window_unload,
    });
  window_set_user_data(window, error_message);

  bool animated = false;
  window_stack_pop(animated);
  window_stack_push(window, animated);
}

// Called when a message is received from PebbleKitJS
void in_received_handler(DictionaryIterator *received, void *context) {
  StreamsMenu *menu = (StreamsMenu *)context;

  // Check if there was an error in retrieving information
  Tuple *error_tuple = dict_find(received, ERROR_KEY);
  if (error_tuple) {
    display_error(error_tuple->value->cstring);
    return;
  }

  Tuple *tuple = dict_read_first(received);
  menu->count++;
  while (tuple) {
    switch (tuple->key) {
    case TITLE_KEY:
      // Allocate memory for another pointer
      menu->titles = realloc(menu->titles, menu->count * sizeof(char *));
      // Allocate memory for another string
      menu->titles[menu->count - 1] = malloc(strlen(tuple->value->cstring) + 1);
      // Add title to storage
      strcpy(menu->titles[menu->count - 1], tuple->value->cstring);
      break;
    case SUBTITLE1_KEY:
      menu->subtitles1 = realloc(menu->subtitles1, menu->count * sizeof(char *));
      menu->subtitles1[menu->count - 1] = malloc(strlen(tuple->value->cstring) + 1);
      strcpy(menu->subtitles1[menu->count - 1], tuple->value->cstring);
      break;
    case SUBTITLE2_KEY:
      menu->subtitles2 = realloc(menu->subtitles2, menu->count * sizeof(char *));
      menu->subtitles2[menu->count - 1] = malloc(strlen(tuple->value->cstring) + 1);
      strcpy(menu->subtitles2[menu->count - 1], tuple->value->cstring);
      break;
    }
    tuple = dict_read_next(received);
  }
  menu_layer_reload_data(menu->layer);
}

// Called when an incoming message from PebbleKitJS is dropped
void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Pebble did not receive message");
}

// Called when PebbleKitJS does not acknowledge receipt of a message
void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context){
  APP_LOG(APP_LOG_LEVEL_INFO, "Phone did not receive message");
}
