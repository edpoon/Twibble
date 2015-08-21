#pragma once

#include "event.h"

typedef struct {
    TextLayer *text;
} AccountMenu;

void account_window_init(uint8_t index);
void down_click_config_handler(ClickRecognizerRef recognizer,void *context);
void up_click_config_handler(ClickRecognizerRef recognizer,void *context);
