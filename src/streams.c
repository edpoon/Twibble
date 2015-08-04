#include "streams.h"
#include "event.h"

static Window *streams_window;
static StreamsMenu stream_menu;

void streams_window_create(uint8_t index) {
        stream_menu.item_count = 0;
        stream_menu.query = index;
        static StreamsMenu *stream_menu_ptr = &stream_menu;

        app_message_set_context(stream_menu_ptr);

        // Send a request to PebbleKitJS for streams/games
        send_message(stream_menu.query);

        // Create streams window element and assign to pointer
        streams_window = window_create();

        // Attach the query number to the streams window
        //  window_set_user_data(streams_window, query_ptr);

        // Set handlers to manage the elements inside the window
        window_set_window_handlers(streams_window, (WindowHandlers) {
                                           .load = streams_window_load,
                                           .unload = streams_window_unload,
                                   });

        // Show the window on the watch with animated = true
        window_stack_push(streams_window, true);
}

void streams_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
        menu_cell_basic_draw(ctx, cell_layer, stream_menu.streamers[cell_index->row], stream_menu.games[cell_index->row], NULL);
}

uint16_t streams_menu_get_num_sections_callback(MenuLayer *menu_layer, void *ctx) {
        return 1;
}

uint16_t streams_menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
        // return num_streams;
        return stream_menu.item_count;
}

void streams_window_load(Window *window) {
        // Get the window's root layer
        Layer *window_layer = window_get_root_layer(window);

        // Get the boundaries
        GRect bounds = layer_get_bounds(window_layer);

        // streams_menu_layer = menu_layer_create(bounds);
        stream_menu.layer = menu_layer_create(bounds);

        menu_layer_set_callbacks(stream_menu.layer, NULL, (MenuLayerCallbacks) {
                                         .get_num_sections = streams_menu_get_num_sections_callback,
                                         .get_num_rows = streams_menu_get_num_rows_callback,
                                         .draw_row = streams_draw_row,
                                 });

        menu_layer_set_click_config_onto_window(stream_menu.layer, window);

        // Add the menuLayer to the window's root layer
        layer_add_child(window_layer, menu_layer_get_layer(stream_menu.layer));
}

void streams_window_unload(Window *window) {
        menu_layer_destroy(stream_menu.layer);
        window_destroy(streams_window);
}
