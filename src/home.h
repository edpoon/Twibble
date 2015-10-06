#pragma once

#include "streams.h"
#include "event.h"

#define NUM_MENU_SECTIONS 1
#define NUM_MENU_ITEMS 4

typedef struct {
  Window *window;
  MenuLayer *layer;
  char *titles[NUM_MENU_ITEMS];
  GBitmap *icons[NUM_MENU_ITEMS];
} HomeMenu;

void init(void);
