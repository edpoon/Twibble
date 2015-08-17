#include "streams.h"

#define NUM_MENU_SECTIONS 1
#define NUM_MENU_ITEMS 4

// Main Window
static Window *s_main_window;
static MenuLayer *s_main_menu_layer;
static GBitmap *cell_icons[NUM_MENU_ITEMS];
static char *titles[NUM_MENU_ITEMS];
static char *subtitles[NUM_MENU_ITEMS];

static void menu_layer_selection_select_call_back(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
        streams_window_init(cell_index->row);
}

static void draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
        // Transparent background
        graphics_context_set_compositing_mode(ctx, GCompOpSet);
        menu_cell_basic_draw(ctx, cell_layer, titles[cell_index->row], subtitles[cell_index->row], cell_icons[cell_index->row]);
}

static uint16_t get_num_sections_callback(MenuLayer *menu_layer, void *ctx) {
        return NUM_MENU_SECTIONS;
}

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *ctx) {
        return NUM_MENU_ITEMS;
}

static void main_window_load(Window *window) {
        cell_icons[0] = gbitmap_create_with_resource(RESOURCE_ID_video_camera);
        cell_icons[1] = gbitmap_create_with_resource(RESOURCE_ID_gamepad);
        cell_icons[2] = gbitmap_create_with_resource(RESOURCE_ID_heart);
        cell_icons[3] = gbitmap_create_with_resource(RESOURCE_ID_wrench);

        titles[0] = "Channels";
        titles[1] = "Games";
        titles[2] = "Following";
        titles[3] = "Settings";

        subtitles[0] = "Top streams";
        subtitles[1] = "Top games";
        subtitles[2] = "Your favorites";
        subtitles[3] = "Your account";

        Layer *window_layer = window_get_root_layer(window);
        GRect bounds = layer_get_bounds(window_layer);
        s_main_menu_layer = menu_layer_create(bounds);

        menu_layer_set_callbacks(s_main_menu_layer, NULL, (MenuLayerCallbacks) {
                                         .get_num_sections = get_num_sections_callback,
                                         .get_num_rows = get_num_rows_callback,
                                         .draw_row = draw_row,
                                         .select_click = menu_layer_selection_select_call_back,
                                 });

        menu_layer_set_click_config_onto_window(s_main_menu_layer, window);

        layer_add_child(window_layer, menu_layer_get_layer(s_main_menu_layer));
}

static void main_window_unload(Window *window) {
        menu_layer_destroy(s_main_menu_layer);
        for (int i = 0; i < NUM_MENU_ITEMS; i++) {
                gbitmap_destroy(cell_icons[i]);
        }
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
