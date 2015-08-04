#pragma once

#include <streams.h>

void send_message(uint8_t query);
void in_received_handler(DictionaryIterator *received, void *context);
void in_dropped_handler(AppMessageResult reason, void *context);
void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context);
