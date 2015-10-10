#include "event.h"

// Key values for AppMessage dictionary
enum {
  QUERY_KEY,
  OFFSET_KEY,
  TITLE_KEY,
  SUBTITLE1_KEY,
  SUBTITLE2_KEY,
  USERNAME_KEY,
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

// Error message generated upon failure to retrieve proper info from API
void display_error(char *error_message) {
  APP_LOG(APP_LOG_LEVEL_INFO, "display error");
  Window *window = window_stack_get_top_window();
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  GBitmap *error_image = gbitmap_create_with_resource(RESOURCE_ID_error);
  GRect error_image_bounds = gbitmap_get_bounds(error_image);
  BitmapLayer *error_layer = bitmap_layer_create(GRect(0, 0, bounds.size.w, error_image_bounds.size.h));
#ifdef PBL_PLATFORM_BASALT
  bitmap_layer_set_compositing_mode(error_layer, GCompOpSet);
#elif PBL_PLATFORM_APLITE
  bitmap_layer_set_compositing_mode(error_layer, GCompOpAssignInverted);
#endif
  bitmap_layer_set_bitmap(error_layer, error_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(error_layer));

  TextLayer *error_message_layer = text_layer_create(GRect(0, error_image_bounds.size.h, bounds.size.w, bounds.size.h - error_image_bounds.size.h));
  text_layer_set_text(error_message_layer, error_message);
  text_layer_set_text_alignment(error_message_layer, GTextAlignmentCenter);
  text_layer_set_font(error_message_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(error_message_layer));

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
