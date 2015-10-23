#include "streams.h"

#define NUM_MENU_SECTIONS 1
#define MENU_CELL_HEIGHT 55
#define MENU_STACK_DEPTH 2 // Deepest: games -> streams

// For text-scrolling
#define SCROLL_MENU_ITEM_WAIT_TIMER 1000
#define SCROLL_MENU_ITEM_TIMER 400
#define MENU_CHARS_VISIBLE 16
#define SUBTITLE_CHARS_VISIBLE 22

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

// For text-scrolling
static void initiate_menu_scroll_timer(StreamsMenu* menu_ptr);
static void scroll_menu_callback(void* data);

void streams_menu_init(char *query) {
  // New item in menu stack
  menu_stack_pointer++;

  // Set menu properties
  menu_stack[menu_stack_pointer].window = window_create();
  menu_stack[menu_stack_pointer].count = 0;
  menu_stack[menu_stack_pointer].query = query;
  menu_stack[menu_stack_pointer].titles = malloc(sizeof(char *));
  menu_stack[menu_stack_pointer].subtitles1 = malloc(sizeof(char *));
  menu_stack[menu_stack_pointer].subtitles2 = malloc(sizeof(char *));
  menu_stack[menu_stack_pointer].viewer_icon = gbitmap_create_with_resource(RESOURCE_ID_viewer);

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
  StreamsMenu *menu = (StreamsMenu *)callback_context;
  if (strcmp(menu->query, "Games") == 0) {
    streams_menu_init(menu->titles[cell_index->row]);
  }
}

static void selection_changed(MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context) {
  StreamsMenu *menu = (StreamsMenu *)callback_context;
  // TODO: Make loading of menu items smoother
  if (menu->count % 5 == 0 && menu->count - new_index.row == 1  && old_index.row != 0) {
    send_message(menu->query, menu->count);
  }

  menu->moving_forwards_in_menu = new_index.row >= old_index.row;
  if (!menu->menu_reloading_to_scroll) {
    initiate_menu_scroll_timer(menu);
  } else {
    menu->menu_reloading_to_scroll = false;
  }
}

static void draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
  StreamsMenu *menu = (StreamsMenu *)callback_context;
  MenuIndex menuIndex = menu_layer_get_selected_index(menu->layer);
  // Draw title
  graphics_context_set_compositing_mode(ctx, GCompOpAssignInverted);

  // Explicitly black for B&W compatibility
  #ifdef PBL_PLATFORM_APLITE
    graphics_context_set_text_color(ctx, GColorBlack);
  #endif

  // Draw Title
  if (menuIndex.row == cell_index->row) {
    graphics_draw_text(ctx,
            (menu->scrolling_still_required) ? menu->titles[cell_index->row] + menu->menu_scroll_offset : menu->titles[cell_index->row],
            fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(5, 0, 140, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  }
  else {
    graphics_draw_text(ctx, menu->titles[cell_index->row],
            fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(5, 0, 140, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  }


  // Draw first subtitle
   if (menuIndex.row == cell_index->row) {
     graphics_draw_text(ctx,
             (menu->subtitle_scrolling_required) ? menu->subtitles1[cell_index->row] + menu->menu_scroll_offset : menu->subtitles1[cell_index->row],
             fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(5, 15, 140, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
   }
  else {
    graphics_draw_text(ctx, menu->subtitles1[cell_index->row],
            fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(5, 15, 140, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  }

  // Draw viewer icon according to platform
#ifdef PBL_PLATFORM_BASALT
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_draw_bitmap_in_rect(ctx, menu->viewer_icon, GRect(5, 35, 20, 20));
#elif PBL_PLATFORM_APLITE
  graphics_context_set_compositing_mode(ctx, GCompOpAssignInverted);
  graphics_draw_bitmap_in_rect(ctx, menu->viewer_icon, GRect(5, 37, 14, 14));
#endif

  // Draw second subtitle
  graphics_draw_text(ctx, menu->subtitles2[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(25, 32, 140, 15), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

  // Need to determine whether to keep scrolling
  int title_length = strlen(menu->titles[menuIndex.row]);
  int subtitle1_length = strlen(menu->subtitles1[menuIndex.row]);

  if (title_length - MENU_CHARS_VISIBLE - menu->menu_scroll_offset > 0) {
    menu->scrolling_still_required = true;
  }

  if (subtitle1_length - SUBTITLE_CHARS_VISIBLE - menu->menu_scroll_offset > 0) {
    menu->subtitle_scrolling_required = true;
  }
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

  menu_layer_set_callbacks(menu_stack[menu_stack_pointer].layer, &menu_stack[menu_stack_pointer], (MenuLayerCallbacks) {
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

  // Reset menu scroll
  menu_stack[menu_stack_pointer].menu_scroll_offset = 0;

  // One less item in the menu stack
  menu_stack_pointer--;

  window_destroy(window);
}

static void initiate_menu_scroll_timer(StreamsMenu* menu_ptr) {
  // If there is already a timer then reschedule it, otherwise create one
  bool need_to_create_timer = true;
  menu_ptr->scrolling_still_required = false;
  menu_ptr->subtitle_scrolling_required = false;
  menu_ptr->menu_scroll_offset = 0;
  menu_ptr->menu_reloading_to_scroll = false;
  if (menu_ptr->menu_scroll_timer) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Rescheduling timer");
    need_to_create_timer = !app_timer_reschedule(menu_ptr->menu_scroll_timer, SCROLL_MENU_ITEM_WAIT_TIMER);
  }
  if (need_to_create_timer) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Creating timer");
    menu_ptr->menu_scroll_timer = app_timer_register(SCROLL_MENU_ITEM_WAIT_TIMER, scroll_menu_callback, menu_ptr);
  }
}

static void scroll_menu_callback(void* data) {
  StreamsMenu* menu = (StreamsMenu*) data;
  if (!menu->layer) {
    return;
  }

  MenuIndex menuIndex = menu_layer_get_selected_index(menu->layer);
  // int temp = strlen(menu->titles[menuIndex.row]);
  // APP_LOG(APP_LOG_LEVEL_INFO, "Subtitle length: %d Offset: %d", temp, menu->menu_scroll_offset);
  // APP_LOG(APP_LOG_LEVEL_INFO, "Math : %d", temp - MENU_CHARS_VISIBLE - menu->menu_scroll_offset);

  menu->menu_scroll_timer = NULL;
  menu->menu_scroll_offset++;
  if (!menu->scrolling_still_required && !menu->subtitle_scrolling_required) {
    return;
  }
  if(menuIndex.row != 0) {
    menu->menu_reloading_to_scroll = true;
  }
  menu->scrolling_still_required = false;
  menu->subtitle_scrolling_required = false;
  menu_layer_reload_data(menu->layer);
  menu->menu_scroll_timer = app_timer_register(SCROLL_MENU_ITEM_TIMER, scroll_menu_callback, menu);
}
