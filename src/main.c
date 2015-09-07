#include "streams.h"
#include "settings.h"

#define NUM_MENU_SECTIONS 1
#define NUM_MENU_ITEMS 4

// Main Window
static Window *s_main_window;
static SimpleMenuLayer *s_main_menu_layer;
static SimpleMenuItem s_main_menu_items[NUM_MENU_ITEMS];
static SimpleMenuSection s_main_menu_sections[NUM_MENU_SECTIONS];
static GBitmap *cell_icons[NUM_MENU_ITEMS];

static void menu_layer_select_callback(int index, void *callback_context) {
  switch (index) {
    case 0 ... 2:
      APP_LOG(APP_LOG_LEVEL_INFO, "Streams window init");
      streams_window_init(index);
      break;
    case 3:
      account_window_init(index);
      break;
    }
}

static void main_window_load(Window *window) {
  // Create the icons for the main menu items
  cell_icons[0] = gbitmap_create_with_resource(RESOURCE_ID_video_camera);
  cell_icons[1] = gbitmap_create_with_resource(RESOURCE_ID_gamepad);
  cell_icons[2] = gbitmap_create_with_resource(RESOURCE_ID_heart);
  cell_icons[3] = gbitmap_create_with_resource(RESOURCE_ID_wrench);

  // Setup the main menu items
  s_main_menu_items[0] = (SimpleMenuItem) {
    .title = "Channels",
    .icon = cell_icons[0],
    .callback = menu_layer_select_callback
  };

  s_main_menu_items[1] = (SimpleMenuItem) {
    .title = "Games",
    .icon = cell_icons[1],
    .callback = menu_layer_select_callback
  };

  s_main_menu_items[2] = (SimpleMenuItem) {
    .title = "Following",
    .icon = cell_icons[2],
    .callback = menu_layer_select_callback
  };

  s_main_menu_items[3] = (SimpleMenuItem) {
    .title = "Settings",
    .icon = cell_icons[3],
    .callback = menu_layer_select_callback
  };

  // Setup the main menu section
  s_main_menu_sections[0] = (SimpleMenuSection) {
    .num_items = NUM_MENU_ITEMS,
    .items = s_main_menu_items
  };

  // Get root layer from main window
  Layer *window_layer = window_get_root_layer(window);

  // Get the boundaries
  GRect bounds = layer_get_bounds(window_layer);

  s_main_menu_layer = simple_menu_layer_create(bounds, window, s_main_menu_sections, NUM_MENU_SECTIONS, NULL);
  // s_main_menu_layer = menu_layer_create(bounds);

  layer_add_child(window_layer, simple_menu_layer_get_layer(s_main_menu_layer));
}

static void main_window_unload(Window *window) {
  for (int i = 0; i < NUM_MENU_ITEMS; i++) {
    gbitmap_destroy(cell_icons[i]);
  }
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

