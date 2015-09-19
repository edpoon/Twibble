#include "event.h"

// Key values for AppMessage dictionary
enum {
  QUERY_KEY,
  OFFSET_KEY,
  TITLE_KEY,
  SUBTITLE1_KEY,
  SUBTITLE2_KEY,
  USERNAME_KEY,
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
  AccountMenu *settings = (AccountMenu *)app_message_get_context();

  Tuple *tuple = dict_read_first(received);
  while (tuple) {
    switch (tuple->key) {
    case TITLE_KEY:
      menu->count++;
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
      menu_layer_reload_data(menu->layer);
      break;
    case USERNAME_KEY...9001:
      text_layer_set_text(settings->text, tuple->value->cstring);
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
