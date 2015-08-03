#pragma once
#include <pebble.h>

extern MenuLayer *streams_menu_layer;
extern uint16_t num_streams;
extern char streamers[100][25];
extern char games[100][50];

void send_message(uint8_t query);
void in_received_handler(DictionaryIterator *received, void *context);
void in_dropped_handler(AppMessageResult reason, void *context);
void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context);
