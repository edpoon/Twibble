#pragma once

#include "event.h"

typedef struct {
        MenuLayer *layer;
        uint16_t query;
        uint16_t count;
        char **titles;
        char **subtitles1;
  char **subtitles2;
} StreamsMenu;

void streams_window_init(uint8_t index);
