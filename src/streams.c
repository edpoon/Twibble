#include <pebble.h>
#include "streams.h"
#include "event.h"

#define NUM_STREAM_SECTIONS 1
#define MAX_STREAMER_NAME_LENGTH 25
#define MAX_GAME_NAME_LENGTH 50

static Window *streams_window;
MenuLayer *streams_menu_layer;
uint16_t num_streams;
char streamers[100][25];
char games[100][50];

void streams_window_create(uint8_t index) {
        num_streams = 0;

        static int query;
        query = index;
        int *query_ptr = &query;

        // Send a request to PebbleKitJS for streams/games
        send_message(*query_ptr);

        // Create streams window element and assign to pointer
        streams_window = window_create();

        // Attach the query number to the streams window
        window_set_user_data(streams_window, query_ptr);

        // Set handlers to manage the elements inside the window
        window_set_window_handlers(streams_window, (WindowHandlers) {
                                           .load = streams_window_load,
                                           .unload = streams_window_unload,
                                   });

        // Show the window on the watch with animated = true
        window_stack_push(streams_window, true);
}

void streams_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
        menu_cell_basic_draw(ctx, cell_layer, streamers[cell_index->row], games[cell_index->row], NULL);
}

uint16_t streams_menu_get_num_sections_callback(MenuLayer *menu_layer, void *ctx) {
        return NUM_STREAM_SECTIONS;
}

uint16_t streams_menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
        return num_streams;
}

void streams_window_load(Window *window) {
        // Get the window's root layer
        Layer *window_layer = window_get_root_layer(window);

        // Get the boundaries
        GRect bounds = layer_get_bounds(window_layer);

        streams_menu_layer = menu_layer_create(bounds);

        menu_layer_set_callbacks(streams_menu_layer, NULL, (MenuLayerCallbacks) {
                                         .get_num_sections = streams_menu_get_num_sections_callback,
                                         .get_num_rows = streams_menu_get_num_rows_callback,
                                         .draw_row = streams_draw_row,
                                 });

        menu_layer_set_click_config_onto_window(streams_menu_layer, window);

        // Add the menuLayer to the window's root layer
        layer_add_child(window_layer, menu_layer_get_layer(streams_menu_layer));
}

void streams_window_unload(Window *window) {
        menu_layer_destroy(streams_menu_layer);
        window_destroy(streams_window);
}
