#pragma once

#include "event.h"

typedef struct {
  Window *window;
  MenuLayer *layer;
  uint16_t count;
  char *query;
  char **titles;
  char **subtitles1;
  char **subtitles2;
  // Icon that will be used for the number of viewers
  GBitmap *viewer_icon;
} StreamsMenu;

void streams_window_init(char *query);
