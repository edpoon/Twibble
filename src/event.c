#include "event.h"

#define MAX_NUM_MENU_ITEMS 50
#define MAX_TITLE_LENGTH 150
#define MAX_SUBTITLE_LENGTH 50
#define MAX_USERNAME_LENGTH 25

// Key values for AppMessage dictionary
enum {
  QUERY_KEY,
  OFFSET_KEY,
  TITLE_KEY,
  SUBTITLE1_KEY,
  USERNAME_KEY,
  SUBTITLE2_KEY
};

// Write message to buffer and send
void send_message(uint8_t query, uint8_t offset) {
  Tuplet queryTuplet = TupletInteger(QUERY_KEY, query);
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
  //Complete assignment of tuples used in the app messages
  Tuple *title_tuple = dict_find(received, TITLE_KEY);
  Tuple *subtitle1_tuple = dict_find(received, SUBTITLE1_KEY);
  Tuple *subtitle2_tuple = dict_find(received, 9001);
  Tuple *username_tuple = dict_find(received, USERNAME_KEY);

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
    menu->titles[count] = malloc(MAX_TITLE_LENGTH + 1);
    menu->subtitles1[count] = malloc(MAX_SUBTITLE_LENGTH + 1);
    menu->subtitles2[count] = malloc(MAX_SUBTITLE_LENGTH + 1);

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

  if (username_tuple) {
    AccountMenu *menu = (AccountMenu *)app_message_get_context();

    //Use of snprintf here is to get the exact space needed to hold the text.
    //text_layer_set_text doesn't seem to support formatting, hence the buffer
    size_t needed = snprintf(NULL, 0, "Currently logged in as: \n %s \n \n Log out?", username_tuple->value->cstring);
    char  *buffer = malloc(needed+1);
    snprintf(buffer, needed+1, "Currently logged in as: \n %s \n \n Log out?", username_tuple->value->cstring);
    text_layer_set_text(menu->text, buffer);
    free(buffer);
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
