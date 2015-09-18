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
  // Complete assignment of tuples used in the app messages
  Tuple *title_tuple = dict_find(received, TITLE_KEY);
  Tuple *subtitle1_tuple = dict_find(received, SUBTITLE1_KEY);
  Tuple *subtitle2_tuple = dict_find(received, SUBTITLE2_KEY);
  Tuple *username_tuple = dict_find(received, USERNAME_KEY);
  Tuple *logout_tuple = dict_find(received, 9001);

  // If we are receiving a TITLE_KEY, we know it must have been called
  // from the streams menu, so we know the context is a StreamsMenu,
  // so we can perform this cast.
  if (title_tuple) {
    StreamsMenu *menu = (StreamsMenu *)app_message_get_context();
    uint8_t count = menu->count;
    menu->count++;

    // Allocate memory for another pointer
    menu->titles = realloc(menu->titles, menu->count * sizeof(char *));
    menu->subtitles1 = realloc(menu->subtitles1, menu->count * sizeof(char *));
    menu->subtitles2 = realloc(menu->subtitles2, menu->count * sizeof(char *));

    // Allocate memory for another string
    menu->titles[count] = malloc(strlen(title_tuple->value->cstring) + 1);
    menu->subtitles1[count] = malloc(strlen(subtitle1_tuple->value->cstring) + 1);
    menu->subtitles2[count] = malloc(strlen(subtitle2_tuple->value->cstring) + 1);

    // Add stream to storage
    strcpy(menu->titles[count], title_tuple->value->cstring);

    // Add game to storage
    if (subtitle1_tuple) {
      strcpy(menu->subtitles1[count], subtitle1_tuple->value->cstring);
    }

    if (subtitle2_tuple) {
      strcpy(menu->subtitles2[count], subtitle2_tuple->value->cstring);
    }

    menu_layer_reload_data(menu->layer);
  }

  if (username_tuple || logout_tuple) {
    AccountMenu *menu = (AccountMenu *)app_message_get_context();
    char *text = username_tuple ? username_tuple->value->cstring : logout_tuple->value->cstring;
    text_layer_set_text(menu->text, text);
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
