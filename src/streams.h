#pragma once
#include <pebble.h>

void streams_window_create(uint8_t index);
void streams_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);
uint16_t streams_menu_get_num_sections_callback(MenuLayer *menu_layer, void *ctx);
uint16_t streams_menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
void streams_window_load(Window *window);
void streams_window_unload(Window *window);
