#ifndef WIFI_RECEIVER_H
#define WIFI_RECEIVER_H
#include <stdio.h>
#include <string.h>
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_async_memcpy.h"
#include "dht.h"

extern const uint16_t temp_buffer_item_size;
typedef struct temp_extern_data_struct { 
    int id;
    int time;
    float temperature_outside;
} temp_extern_data_struct;

typedef struct temp_unified_data_struct { 
    int id;
    int time;
    float temperature_outside;
    float temperature_inside;
    float humidity_inside;
} temp_unified_data_struct;
typedef struct {
    temp_unified_data_struct *buffer; // Pointer to dynamically allocated buffer
    int head; // Index of the newest item
    int tail; // Index of the oldest item
    int count; // Number of items in the buffer
    int size; // Size of the buffer
} CircularBuffer;

void on_data_receive(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len);
void get_mac_address();
void init_esp_now();
bool decode_time_from_int(int time, int *hour, int *minute, int *second);

void init_circular_buffer(CircularBuffer *cb, int size);
bool add_to_circular_buffer(CircularBuffer *cb, temp_unified_data_struct *item);
bool read_from_circular_buffer(CircularBuffer *cb, int index, temp_unified_data_struct *item);
#endif 