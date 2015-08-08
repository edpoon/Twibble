#pragma once

#include "event.h"

#define MAX_NUM_MENU_ITEMS 50
#define MAX_STREAMER_NAME_LENGTH 25
#define MAX_GAME_NAME_LENGTH 50

typedef struct {
        MenuLayer *layer;
        uint16_t query;
        uint16_t count;
        char **streamers;
        char **games;
} StreamsMenu;

void streams_window_init(uint8_t index);
