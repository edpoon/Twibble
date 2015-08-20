#include <settings.h>

static Window *account_window;
static ActionBarLayer *action_bar;
// static TextLayer *text;
static GBitmap *yes_icon;
static GBitmap *no_icon;
// static char *user_name;

AccountMenu account_menu;

void up_click_config_handler(ClickRecognizerRef recognizer, void *context) {
  // Tell phone to delete token
  send_message(3, 1);
  // Pop window with animated = true
  window_stack_pop(true);
}

void down_click_config_handler(ClickRecognizerRef recognizer, void *context) {
  // Pop window with animated = true
  window_stack_pop(true);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler)up_click_config_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler)down_click_config_handler);
}

static void account_window_load(Window *window) {
  // Get token from phone
  send_message(3, 0);
  yes_icon = gbitmap_create_with_resource(RESOURCE_ID_check);
  no_icon = gbitmap_create_with_resource(RESOURCE_ID_cross);

  action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(action_bar, account_window);
  action_bar_layer_set_click_config_provider(action_bar, click_config_provider);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, yes_icon);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, no_icon);

  account_menu.text = text_layer_create(GRect(0, 45, 110, 200));
  text_layer_set_text(account_menu.text, "Currently logged in as: \n Navies \n \n Log out?");
  text_layer_set_text_alignment(account_menu.text, GTextAlignmentCenter);

  Layer *window_layer = window_get_root_layer(window);
  layer_add_child(window_layer, text_layer_get_layer(account_menu.text));
}

static void account_window_unload(Window *window) {
  action_bar_layer_destroy(action_bar);
  gbitmap_destroy(yes_icon);
  gbitmap_destroy(no_icon);
  text_layer_destroy(account_menu.text);
  window_destroy(account_window);
}

void account_window_init(uint8_t index) {
  account_menu.user_name = NULL;
  account_window = window_create();
  window_set_window_handlers(account_window, (WindowHandlers) {
      .load = account_window_load,
        .unload = account_window_unload,
        });
  window_stack_push(account_window, true);
}
