#include "event.h"

#define MAX_NUM_MENU_ITEMS 50
#define MAX_TITLE_LENGTH 25
#define MAX_SUBTITLE_LENGTH 50

// Key values for AppMessage dictionary
enum {
        QUERY_KEY,
        OFFSET_KEY,
        TITLE_KEY,
        SUBTITLE_KEY
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
        StreamsMenu *menu = (StreamsMenu *)app_message_get_context();
        uint8_t count = menu->count;
        menu->count++;

        // Allocate memory for another pointer
        menu->titles = realloc(menu->titles, menu->count * sizeof(char *));
        menu->subtitles = realloc(menu->subtitles, menu->count * sizeof(char *));

        // Allocate memory for another string
        menu->titles[count] = malloc(MAX_TITLE_LENGTH + 1);
        menu->subtitles[count] = malloc(MAX_SUBTITLE_LENGTH + 1);

        Tuple *tuple;

        // Add stream to storage
        tuple = dict_find(received, TITLE_KEY);
        strcpy(menu->titles[count], tuple->value->cstring);

        // Add game to storage
        tuple = dict_find(received, SUBTITLE_KEY);
        strcpy(menu->subtitles[count], tuple->value->cstring);

        menu_layer_reload_data(menu->layer);
}

// Called when an incoming message from PebbleKitJS is dropped
void in_dropped_handler(AppMessageResult reason, void *context) {
}

// Called when PebbleKitJS does not acknowledge receipt of a message
void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context){
}
