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

static char ** store_message_in_buffer(int index, char **buffer, char *message) {
  // Allocate memory for another pointer
  // If memory has not already been allocated, use malloc, since realloc fails on watch in that case
  if (!buffer) {
    buffer = malloc(index * sizeof(char *));
  } else {
    buffer = realloc(buffer, index * sizeof(char *));
  }
  // Allocate memory for another string
  buffer[index - 1] = malloc(strlen(message) + 1);
  // Add title to storage
  strcpy(buffer[index - 1], message);
  return buffer;
}

// Called when a message is received from PebbleKitJS
void in_received_handler(DictionaryIterator *received, void *context) {
  StreamsMenu *menu = (StreamsMenu *)context;

  // Check if there was an error in retrieving information
  Tuple *error_tuple = dict_find(received, ERROR_KEY);
  if (error_tuple) {
    display_status(error_tuple->value->cstring);
    return;
  }

  Tuple *tuple = dict_read_first(received);
  menu->count++;
  while (tuple) {
    switch (tuple->key) {
      case TITLE_KEY:
        menu->titles = store_message_in_buffer(menu->count, menu->titles, tuple->value->cstring);
        break;
      case SUBTITLE1_KEY:
        menu->first_subtitles = store_message_in_buffer(menu->count, menu->first_subtitles, tuple->value->cstring);
        break;
      case SUBTITLE2_KEY:
        menu->second_subtitles = store_message_in_buffer(menu->count, menu->second_subtitles, tuple->value->cstring);
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
