#pragma once

<<<<<<< HEAD
#include "event.h"
=======
#include <pebble.h>
>>>>>>> 357746961e5d87d965d50653632eb031f5420b7b

#define MAX_NUM_MENU_ITEMS 50
#define MAX_STREAMER_NAME_LENGTH 25
#define MAX_GAME_NAME_LENGTH 50

typedef struct {
        MenuLayer *layer;
        uint16_t query;
<<<<<<< HEAD
        uint16_t count;
        char **streamers;
        char **games;
} StreamsMenu;

void streams_window_init(uint8_t index);
=======
        uint16_t item_count;
        char streamers[MAX_NUM_MENU_ITEMS][MAX_STREAMER_NAME_LENGTH + 1];
        char games[MAX_NUM_MENU_ITEMS][MAX_GAME_NAME_LENGTH + 1];
} StreamsMenu;


void streams_window_create(uint8_t index);
uint16_t streams_menu_get_num_sections_callback(MenuLayer *menu_layer, void *ctx);
uint16_t streams_menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
void streams_window_load(Window *window);
void streams_window_unload(Window *window);
>>>>>>> 357746961e5d87d965d50653632eb031f5420b7b
