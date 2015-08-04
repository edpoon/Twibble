#include "event.h"

// Key values for AppMessage dictionary
enum {
        QUERY_KEY,
        STREAMER_KEY,
        GAME_KEY,
};

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
        StreamsMenu *stream_menu = (StreamsMenu *)app_message_get_context();

        Tuple *tuple;

        // Add stream to storage
        tuple = dict_find(received, STREAMER_KEY);
        strcpy(stream_menu->streamers[stream_menu->item_count], tuple->value->cstring);

        // Add game to storage
        tuple = dict_find(received, GAME_KEY);
        strcpy(stream_menu->games[stream_menu->item_count], tuple->value->cstring);

        // Update MenuLayer
        stream_menu->item_count++;

        // uint16_t *item_count = (uint16_t *)app_message_get_context();
        // APP_LOG(APP_LOG_LEVEL_INFO, "%d", *item_count);
        menu_layer_reload_data(stream_menu->layer);
}

// Called when an incoming message from PebbleKitJS is dropped
void in_dropped_handler(AppMessageResult reason, void *context) {
}

// Called when PebbleKitJS does not acknowledge receipt of a message
void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context){
}
