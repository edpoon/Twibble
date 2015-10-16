#include "home.h"

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
  HomeMenu *menu = callback_context;
  // Transparent background
  graphics_context_set_compositing_mode(ctx, GCompOpAssignInverted);
  menu_cell_basic_draw(ctx, cell_layer, menu->titles[cell_index->row], NULL, menu->icons[cell_index->row]);
}

static void draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
  HomeMenu *menu = callback_context;
  graphics_context_set_text_color(ctx, GColorBlack);

  graphics_draw_bitmap_in_rect(ctx, menu->icons[3], GRect(30, 0, 30, 30));
  graphics_draw_text(ctx, "Twibble", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(50, 5, 70, 70), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

static uint16_t get_num_sections_callback(MenuLayer *menu_layer, void *ctx) {
  return NUM_MENU_SECTIONS;
}

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *ctx) {
  return NUM_MENU_ITEMS;
}

static int16_t get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
  return 30;
}

static void menu_layer_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  HomeMenu *menu = callback_context;
  streams_window_init(menu->titles[cell_index->row]);
}

static void main_window_load(Window *window) {
  HomeMenu *menu = window_get_user_data(window);

  // Create the icons for the main menu items
  menu->icons[0] = gbitmap_create_with_resource(RESOURCE_ID_video_camera);
  menu->icons[1] = gbitmap_create_with_resource(RESOURCE_ID_gamepad);
  menu->icons[2] = gbitmap_create_with_resource(RESOURCE_ID_heart);
  menu->icons[3] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_ICON);

  // Setup the main menu titles
  menu->titles[0] = "Channels";
  menu->titles[1] = "Games";
  menu->titles[2] = "Following";

  // Setup the main menu
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  menu->layer = menu_layer_create(bounds);

  menu_layer_set_callbacks(menu->layer, menu, (MenuLayerCallbacks) {
      .get_num_sections = get_num_sections_callback,
      .get_num_rows = get_num_rows_callback,
      .draw_row = draw_row_callback,
      .select_click = menu_layer_select_callback,
#ifdef PBL_PLATFORM_BASALT
      .get_header_height = get_header_height_callback,
      .draw_header = draw_header_callback
#endif
      });

  menu_layer_set_click_config_onto_window(menu->layer, window);
  layer_add_child(window_layer, menu_layer_get_layer(menu->layer));
}

static void main_window_unload(Window *window) {
  HomeMenu *menu = window_get_user_data(window);
  uint8_t i;
  // Free memory
  for (i = 0; i < 4; i++) {
    gbitmap_destroy(menu->icons[i]);
  }
  menu_layer_destroy(menu->layer);
  free(menu);
  window_destroy(window);
}

void init(void) {
  // Register the AppMessage handlers
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_failed(out_failed_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  // Create main window element and assign to pointer
  Window *window = window_create();

  // Set handlers to manage the elements inside the window
  window_set_window_handlers(window, (WindowHandlers) {
      .load = main_window_load,
      .unload = main_window_unload,
      });

  HomeMenu *menu = malloc(sizeof(HomeMenu));
  memset(menu, 0, sizeof(HomeMenu));

  window_set_user_data(window, menu);

  // Show the window on the watch, with animated = true
  window_stack_push(window, true);
}

