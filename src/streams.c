#include "streams.h"
#include "event.h"

#define NUM_MENU_SECTIONS 1
#define MENU_CELL_HEIGHT 65

static Window *streams_window;
static StreamsMenu menu;

static void streams_window_load(Window *window);
static void streams_window_unload(Window *window);

// MenuLayer callbacks
static void selection_changed(MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context);
static void draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);
static uint16_t get_num_sections_callback(MenuLayer *menu_layer, void *ctx);
static uint16_t get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t get_cell_height(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *data);

void streams_window_init(uint8_t index) {
        // Setup menu struct
        menu.count = 0;
        menu.query = index;
        menu.titles = NULL;
        menu.subtitles = NULL;

        static StreamsMenu *menu_ptr = &menu;

        app_message_set_context(menu_ptr);

        // Send a request to PebbleKitJS for items
        send_message(menu.query, menu.count);

        // Create streams window element and assign to pointer
        streams_window = window_create();

        // Set handlers to manage the elements inside the window
        window_set_window_handlers(streams_window, (WindowHandlers) {
                                           .load = streams_window_load,
                                           .unload = streams_window_unload,
                                   });

        // Show the window on the watch with animated = true
        window_stack_push(streams_window, true);
}

static void selection_changed(MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context) {
        // Capping streams number of menu items at 250
        if (menu.count % 5 == 0 && menu.count - new_index.row == 6  && old_index.row != 0 && menu.count < 250) {
                APP_LOG(APP_LOG_LEVEL_INFO, "Row: %d, Count: %d", new_index.row, menu.count);
                send_message(menu.query, menu.count);
        }
}

static void draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
        // Draw title
        graphics_draw_text(ctx, menu.titles[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GRect(0, 0, 145, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
        // Draw subtitle
        graphics_draw_text(ctx, menu.subtitles[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(0, 15, 145, 45), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
}

static uint16_t get_num_sections_callback(MenuLayer *menu_layer, void *ctx) {
        return NUM_MENU_SECTIONS;
}

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *ctx) {
        return menu.count;
}

static int16_t get_cell_height(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *ctx) {
        return MENU_CELL_HEIGHT;
}

static void streams_window_load(Window *window) {
        // Get the window's root layer
        Layer *window_layer = window_get_root_layer(window);

        // Get the boundaries
        GRect bounds = layer_get_bounds(window_layer);

        // streams_menu_layer = menu_layer_create(bounds);
        menu.layer = menu_layer_create(bounds);

        menu_layer_set_callbacks(menu.layer, NULL, (MenuLayerCallbacks) {
                                         .get_num_sections = get_num_sections_callback,
                                         .get_num_rows = get_num_rows_callback,
                                         .get_cell_height = get_cell_height,
                                         .draw_row = draw_row,
                                         .selection_changed = selection_changed
                                 });

        menu_layer_set_click_config_onto_window(menu.layer, window);

        // Add the menuLayer to the window's root layer
        layer_add_child(window_layer, menu_layer_get_layer(menu.layer));
}

static void streams_window_unload(Window *window) {
        // Clear memory
        menu_layer_destroy(menu.layer);
        for (int i = 0; i < menu.count; i++) {
                free(menu.titles[i]);
                free(menu.subtitles[i]);
        }
        free(menu.titles);
        free(menu.subtitles);

        // Set to NULL to avoid double free
        menu.titles = NULL;
        menu.subtitles = NULL;

        window_destroy(streams_window);
}
