#include "event.h"

#define NUM_MENU_SECTIONS 1
#define NUM_MENU_ITEMS 3

typedef struct {
  MenuLayer *layer;
  char *titles[NUM_MENU_ITEMS];
  GBitmap *icons[NUM_MENU_ITEMS];
} MainMenuData;

// MenuLayer callbacks
//////////////////////////
static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
  MainMenuData *data = callback_context;
  // Draw icons according to platform
#ifdef PBL_PLATFORM_APLITE
  graphics_context_set_compositing_mode(ctx, GCompOpAssignInverted);
#else
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
#endif
  menu_cell_basic_draw(ctx, cell_layer, data->titles[cell_index->row], NULL, data->icons[cell_index->row]);
}

static uint16_t get_num_sections_callback(MenuLayer *menu_layer, void *ctx) {
  return NUM_MENU_SECTIONS;
}

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *ctx) {
  return NUM_MENU_ITEMS;
}

static void menu_layer_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  MainMenuData *data = callback_context;
  streams_window_init(data->titles[cell_index->row]);
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  GRect bounds = layer_get_bounds(menu_layer_get_layer(menu_layer));
  return (bounds.size.h / NUM_MENU_ITEMS);
}

// Main window
//////////////////////////
static void window_load(Window *window) {
  MainMenuData *data = window_get_user_data(window);

  // Create icons for the main menu items
  data->icons[0] = gbitmap_create_with_resource(RESOURCE_ID_video_camera);
  data->icons[1] = gbitmap_create_with_resource(RESOURCE_ID_gamepad);
  data->icons[2] = gbitmap_create_with_resource(RESOURCE_ID_heart);

  // Setup main menu titles
  data->titles[0] = "Channels";
  data->titles[1] = "Games";
  data->titles[2] = "Following";

  // Setup main menu
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  data->layer = menu_layer_create(bounds);

  menu_layer_set_callbacks(data->layer, data, (MenuLayerCallbacks) {
                             .get_num_sections = get_num_sections_callback,
                             .get_num_rows = get_num_rows_callback,
                             .draw_row = draw_row_callback,
                             .select_click = menu_layer_select_callback,
                             .get_cell_height = get_cell_height_callback
                           });

  menu_layer_set_click_config_onto_window(data->layer, window);
  layer_add_child(window_layer, menu_layer_get_layer(data->layer));
}

static void window_unload(Window *window) {
  MainMenuData *data = window_get_user_data(window);
  for (uint8_t i = 0; i < NUM_MENU_ITEMS; i++) {
    gbitmap_destroy(data->icons[i]);
  }
  menu_layer_destroy(data->layer);
  free(data);
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
                               .load = window_load,
                               .unload = window_unload,
                             });

  MainMenuData *data = malloc(sizeof(MainMenuData));
  memset(data, 0, sizeof(MainMenuData));

  window_set_user_data(window, data);

  bool animated = true;
  // Show the window on the watch
  window_stack_push(window, animated);
}

// App entry point
//////////////////////////
int main(void) {
  init();
  app_event_loop();
}
