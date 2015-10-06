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

// Called when a message is received from PebbleKitJS
void in_received_handler(DictionaryIterator *received, void *context) {
  StreamsMenu *menu = (StreamsMenu *)context;
  HomeMenu *main_menu = (HomeMenu *)context;

  Tuple *tuple = dict_find(received, TITLE_KEY);
  Tuple *error_tuple = dict_find(received, ERROR_KEY);
  if (tuple) {
    menu->count++;
  }
  // Error message is generated upon failure to retrieve proper info from API
  else if (error_tuple) {
    APP_LOG(APP_LOG_LEVEL_ERROR, error_tuple->value->cstring);
    Window *window = window_stack_get_top_window();
    Layer *window_layer = window_get_root_layer(window);
    TextLayer *error_layer = text_layer_create(GRect(2, 32, 140, 120));
    text_layer_set_text(error_layer, error_tuple->value->cstring);
    text_layer_set_text_alignment(error_layer, GTextAlignmentCenter);
    text_layer_set_font(error_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    layer_add_child(window_layer, text_layer_get_layer(error_layer));
  }

  tuple = dict_read_first(received);
  while (tuple) {
    switch (tuple->key) {
    case TITLE_KEY:
      // Allocate memory for another pointer
      menu->titles = realloc(menu->titles, menu->count * sizeof(char *));
      // Allocate memory for another string
      menu->titles[menu->count - 1] = malloc(strlen(tuple->value->cstring) + 1);
      // Add title to storage
      strcpy(menu->titles[menu->count - 1], tuple->value->cstring);
      menu_layer_reload_data(menu->layer);
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
      menu_layer_reload_data(menu->layer);
      break;
    case USERNAME_KEY:
      strcpy(main_menu->titles[3], tuple->value->cstring);
      menu_layer_reload_data(main_menu->layer);
      break;
    }
    tuple = dict_read_next(received);
  }
}

// Called when an incoming message from PebbleKitJS is dropped
void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Pebble did not receive message");
}

// Called when PebbleKitJS does not acknowledge receipt of a message
void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context){
  APP_LOG(APP_LOG_LEVEL_INFO, "Phone did not receive message");
}
