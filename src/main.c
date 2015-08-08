#include "streams.h"

#define NUM_MENU_SECTIONS 1
#define NUM_MENU_ITEMS 4

// Main Window
static Window *s_main_window;
static SimpleMenuLayer *s_main_menu_layer;
static SimpleMenuItem s_main_menu_items[NUM_MENU_ITEMS];
static SimpleMenuSection s_main_menu_sections[NUM_MENU_SECTIONS];

static void menu_layer_selection_select_call_back(int index, void *context) {
        streams_window_init(index);
}

static void main_window_load(Window *window) {
        // Setup the main menu items
        s_main_menu_items[0] = (SimpleMenuItem) {
                .title = "Following",
                .subtitle = "Your streams",
                .callback = menu_layer_selection_select_call_back
        };

        s_main_menu_items[1] = (SimpleMenuItem) {
                .title = "Top",
                .subtitle = "Popular streams",
                .callback = menu_layer_selection_select_call_back
        };

        s_main_menu_items[2] = (SimpleMenuItem) {
                .title = "Front Page",
                .subtitle = "Featured streams",
                .callback = menu_layer_selection_select_call_back
        };

        s_main_menu_items[3] = (SimpleMenuItem) {
                .title = "About",
                .subtitle = "About twebble"
        };

        // Setup the main menu section
        s_main_menu_sections[0] = (SimpleMenuSection) {
                .num_items = NUM_MENU_ITEMS,
                .items = s_main_menu_items
        };

        // Get the window's root layer
        Layer *window_layer = window_get_root_layer(window);

        // Get the boundaries
        GRect bounds = layer_get_bounds(window_layer);

        s_main_menu_layer = simple_menu_layer_create(bounds, window, s_main_menu_sections, NUM_MENU_SECTIONS, NULL);

        // Add the main menu layer to the main window's root layer
        layer_add_child(window_layer, simple_menu_layer_get_layer(s_main_menu_layer));
}

static void main_window_unload(Window *window) {
        // Destroy the Simple Menu Layer
        simple_menu_layer_destroy(s_main_menu_layer);
}

static void init(void) {
        // Register the AppMessage handlers
        app_message_register_inbox_received(in_received_handler);
        app_message_register_inbox_dropped(in_dropped_handler);
        app_message_register_outbox_failed(out_failed_handler);
        app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

        // Create main window element and assign to pointer
        s_main_window = window_create();

        // Set handlers to manage the elements inside the window
        window_set_window_handlers(s_main_window, (WindowHandlers) {
                                           .load = main_window_load,
                                           .unload = main_window_unload,
                                   });

        // Show the window on the watch, with animated = true
        window_stack_push(s_main_window, true);
}

static void deinit(void) {
        window_destroy(s_main_window);
}

int main(void) {
        init();
        app_event_loop();
        deinit();
}
