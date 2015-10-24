#pragma once

#include "streams.h"
#include "event.h"

#define NUM_MENU_SECTIONS 1
#define NUM_MENU_ITEMS 3

typedef struct {
  MenuLayer *layer;
  char *titles[NUM_MENU_ITEMS];
  GBitmap *icons[4];
} HomeMenu;

void init(void);
