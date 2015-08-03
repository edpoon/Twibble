#include <pebble.h>
#include "event.h"

// Key values for AppMessage dictionary
#define QUERY_KEY 0
#define STREAMER_KEY 1
#define GAME_KEY 2

MenuLayer *streams_menu_layer;
uint16_t num_streams;
char streamers[100][25];
char games[100][50];

// Write message to buffer and send
void send_message(uint8_t query) {
        DictionaryIterator *iter;

        app_message_outbox_begin(&iter);
        dict_write_uint8(iter, QUERY_KEY, query);

        dict_write_end(iter);
        app_message_outbox_send();
}

// Called when a message is received from PebbleKitJS
void in_received_handler(DictionaryIterator *received, void *context) {
        Tuple *tuple;

        // Add stream to storage
        tuple = dict_find(received, STREAMER_KEY);
        strcpy(streamers[num_streams], tuple->value->cstring);

        // Add game to storage
        tuple = dict_find(received, GAME_KEY);
        strcpy(games[num_streams], tuple->value->cstring);

        // Update MenuLayer
        ++num_streams;
        menu_layer_reload_data(streams_menu_layer);
}

// Called when an incoming message from PebbleKitJS is dropped
void in_dropped_handler(AppMessageResult reason, void *context) {
}

// Called when PebbleKitJS does not acknowledge receipt of a message
void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context){
}
