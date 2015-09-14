#include "streams.h"

#define NUM_MENU_SECTIONS 1
#define NUM_MENU_ITEMS 4

typedef struct {
  Window *window;
  MenuLayer *layer;
  char *titles[NUM_MENU_ITEMS];
  GBitmap *icons[NUM_MENU_ITEMS];
} MainMenu;

static MainMenu menu;

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
  // Transparent background
  graphics_context_set_compositing_mode(ctx, GCompOpAssignInverted);
  menu_cell_basic_draw(ctx, cell_layer, menu.titles[cell_index->row], NULL, menu.icons[cell_index->row]);
}

static uint16_t get_num_sections_callback(MenuLayer *menu_layer, void *ctx) {
  return NUM_MENU_SECTIONS;
}

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *ctx) {
  return NUM_MENU_ITEMS;
}

static void menu_layer_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  switch (cell_index->row) {
    case 0 ... 2:
      streams_window_init(menu.titles[cell_index->row]);
      break;
    case 3:
      account_window_init();
      break;
  }
}

static void main_window_load(Window *window) {
  // Create the icons for the main menu items
  menu.icons[0] = gbitmap_create_with_resource(RESOURCE_ID_video_camera);
  menu.icons[1] = gbitmap_create_with_resource(RESOURCE_ID_gamepad);
  menu.icons[2] = gbitmap_create_with_resource(RESOURCE_ID_heart);
  menu.icons[3] = gbitmap_create_with_resource(RESOURCE_ID_wrench);

  // Setup the main menu titles
  menu.titles[0] = "Channels";
  menu.titles[1] = "Games";
  menu.titles[2] = "Following";
  menu.titles[3] = "Settings";

  // Setup the main menu
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  menu.layer = menu_layer_create(bounds);

  menu_layer_set_callbacks(menu.layer, NULL, (MenuLayerCallbacks) {
    .get_num_sections = get_num_sections_callback,
    .get_num_rows = get_num_rows_callback,
    .draw_row = draw_row_callback,
    .select_click = menu_layer_select_callback
  });

  menu_layer_set_click_config_onto_window(menu.layer, window);
  layer_add_child(window_layer, menu_layer_get_layer(menu.layer));
}

static void main_window_unload(Window *window) {
  uint8_t i;
  // Free memory
  for (i = 0; i < NUM_MENU_ITEMS; i++) {
    gbitmap_destroy(menu.icons[i]);
  }
  menu_layer_destroy(menu.layer);
}

static void init(void) {
  // Register the AppMessage handlers
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_failed(out_failed_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  // Create main window element and assign to pointer
  menu.window = window_create();

  // Set handlers to manage the elements inside the window
  window_set_window_handlers(menu.window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });

  app_message_set_context(&menu);

  // Show the window on the watch, with animated = true
  window_stack_push(menu.window, true);
}

static void deinit(void) {
  window_destroy(menu.window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
