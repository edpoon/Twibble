#pragma once

#include <pebble.h>
#include "home.h"

void send_message(const char *query, uint8_t offset);
void in_received_handler(DictionaryIterator *received, void *context);
void in_dropped_handler(AppMessageResult reason, void *context);
void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context);
