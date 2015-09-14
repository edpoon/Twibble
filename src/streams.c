#include "streams.h"
#include "event.h"

#define NUM_MENU_SECTIONS 1
#define MENU_CELL_HEIGHT 55
#define MENU_STACK_DEPTH 2 // Deepest: games -> streams

static StreamsMenu menu_stack[MENU_STACK_DEPTH];
static int8_t menu_stack_pointer = -1; // Points to the current position in the menu stack

static void streams_window_load(Window *window);
static void streams_window_unload(Window *window);

// MenuLayer callbacks
static void selection_changed(MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context);
static void draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);
static uint16_t get_num_sections_callback(MenuLayer *menu_layer, void *ctx);
static uint16_t get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t get_cell_height(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *data);

void streams_window_init(char *query) {
  // New item in menu stack
  menu_stack_pointer++;

  // Set menu properties
  menu_stack[menu_stack_pointer].window = window_create();
  menu_stack[menu_stack_pointer].count = 0;
  menu_stack[menu_stack_pointer].query = query;
  menu_stack[menu_stack_pointer].titles = NULL;
  menu_stack[menu_stack_pointer].subtitles1 = NULL;
  menu_stack[menu_stack_pointer].subtitles2 = NULL;
  menu_stack[menu_stack_pointer].viewer_icon = gbitmap_create_with_resource(RESOURCE_ID_viewer);

  //  app_message_set_context(&menu);
  app_message_set_context(&menu_stack[menu_stack_pointer]);

  // Send a request to PebbleKitJS for items
  send_message(menu_stack[menu_stack_pointer].query, menu_stack[menu_stack_pointer].count);

  // Set handlers to manage the elements inside the window
  window_set_window_handlers(menu_stack[menu_stack_pointer].window, (WindowHandlers) {
      .load = streams_window_load,
        .unload = streams_window_unload,
        });

  // Show the window on the watch with animated = true
  window_stack_push(menu_stack[menu_stack_pointer].window, true);
}

static void menu_layer_select_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  if (strcmp(menu_stack[menu_stack_pointer].query, "Games") == 0) {
    streams_window_init(menu_stack[menu_stack_pointer].titles[cell_index->row]);
  }
}

static void selection_changed(MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context) {
  if (menu_stack[menu_stack_pointer].count % 5 == 0 && menu_stack[menu_stack_pointer].count - new_index.row == 6  && old_index.row != 0) {
    send_message(menu_stack[menu_stack_pointer].query, menu_stack[menu_stack_pointer].count);
  }
}

static void draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "%d", menu_stack_pointer);
  // Draw title
  graphics_draw_text(ctx, menu_stack[menu_stack_pointer].titles[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(5, 0, 140, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  // Draw first subtitle
  graphics_draw_text(ctx, menu_stack[menu_stack_pointer].subtitles1[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(5, 15, 140, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

  // Draw viewer icon according to platform
#ifdef PBL_PLATFORM_BASALT
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_draw_bitmap_in_rect(ctx, menu_stack[menu_stack_pointer].viewer_icon, GRect(5, 35, 20, 20));
#elif PBL_PLATFORM_APLITE
  graphics_context_set_compositing_mode(ctx, GCompOpAssignInverted);
  graphics_draw_bitmap_in_rect(ctx, menu_stack[menu_stack_pointer].viewer_icon, GRect(5, 37, 14, 14));
#endif

  // Draw second subtitle
  graphics_draw_text(ctx, menu_stack[menu_stack_pointer].subtitles2[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(25, 32, 140, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
}

static uint16_t get_num_sections_callback(MenuLayer *menu_layer, void *ctx) {
  return NUM_MENU_SECTIONS;
}

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *ctx) {
  return menu_stack[menu_stack_pointer].count;
}

static int16_t get_cell_height(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *ctx) {
  return MENU_CELL_HEIGHT;
}

static void streams_window_load(Window *window) {
  // Get the window's root layer
  Layer *window_layer = window_get_root_layer(window);

  // Get the boundaries
  GRect bounds = layer_get_bounds(window_layer);

  menu_stack[menu_stack_pointer].layer = menu_layer_create(bounds);

  menu_layer_set_callbacks(menu_stack[menu_stack_pointer].layer, NULL, (MenuLayerCallbacks) {
      .get_num_sections = get_num_sections_callback,
        .get_num_rows = get_num_rows_callback,
        .get_cell_height = get_cell_height,
        .draw_row = draw_row,
        .selection_changed = selection_changed,
        .select_click = menu_layer_select_click
        });

  menu_layer_set_click_config_onto_window(menu_stack[menu_stack_pointer].layer, window);

  // Add the menuLayer to the window's root layer
  layer_add_child(window_layer, menu_layer_get_layer(menu_stack[menu_stack_pointer].layer));
}

static void streams_window_unload(Window *window) {
  uint8_t i;
  // Clear memory
  menu_layer_destroy(menu_stack[menu_stack_pointer].layer);
  for (i = 0; i < menu_stack[menu_stack_pointer].count; i++) {
    free(menu_stack[menu_stack_pointer].titles[i]);
    free(menu_stack[menu_stack_pointer].subtitles1[i]);
    free(menu_stack[menu_stack_pointer].subtitles2[i]);
  }
  free(menu_stack[menu_stack_pointer].titles);
  free(menu_stack[menu_stack_pointer].subtitles1);
  free(menu_stack[menu_stack_pointer].subtitles2);
  gbitmap_destroy(menu_stack[menu_stack_pointer].viewer_icon);

  // Set to NULL to avoid double free
  menu_stack[menu_stack_pointer].titles = NULL;
  menu_stack[menu_stack_pointer].subtitles1 = NULL;
  menu_stack[menu_stack_pointer].subtitles2 = NULL;

  window_destroy(window);
  // One less item in the menu stack
  menu_stack_pointer--;
}
