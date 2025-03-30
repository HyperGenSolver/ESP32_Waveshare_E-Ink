/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_log.h"
#include "dht.h"

#include "WIFI_receiver.h"
#include "Waveshare_e-ink_driver.h"

const char *TAG = "Waveshare E-Ink UART";
extern QueueHandle_t temperature_data_queue;

void app_main(void)
{
    // Waveshare E-Ink wiki: https://www.waveshare.com/wiki/4.3inch_e-Paper_UART_Module
    // ESP32 Wiki: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uart.html
    
    // --- Init esp now ---
    /*
    init_esp_now();
      // Setup temperature data queue
    temperature_data_queue = xQueueCreate(1, sizeof(temperature_data_struct));
    if (temperature_data_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create temperature queue"); while(1){}
    }
    static temperature_data_struct latest_temp_data = {-1, 0, 0};
    while(1){
        xQueueReceive(temperature_data_queue, &latest_temp_data, ( TickType_t ) 0); // get the latest temperature data
        ESP_LOGI(TAG, "ID: %d, Time: %d, Temperature: %.2f", latest_temp_data.id, latest_temp_data.time, latest_temp_data.temperature_outside);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    */
    
    /*
    // --- Init DHT sensor ---
    int16_t hum_buffer = 0;
    int16_t temp_buffer = 0;
    dht_read_data(DHT_TYPE_DHT11, 21, &hum_buffer, &temp_buffer);
    ESP_LOGI(TAG, "Humidity: %d.%d %% Temperature: %d.%d C", hum_buffer / 10, hum_buffer % 10, temp_buffer / 10, temp_buffer % 10);
    */
    // --- Init waveshare e-ink ---
    init_waveshare_e_ink(UART_NUM_2, 16, 17);
    if(!waveshare_e_ink_handshare(UART_NUM_2)){
        ESP_LOGE(TAG, "Handshake failed");
        while(1){vTaskDelay(100 / portTICK_PERIOD_MS);} // Wait forever if handshake failed
    }
    clear_waveshare_e_ink(UART_NUM_2);
    set_waveshare_e_ink_orientation(UART_NUM_2, 2); // Set orientation to 180 degrees

    //import_waveshare_e_ink_font(UART_NUM_2);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    draw_line_waveshare_e_ink(UART_NUM_2, 100, 10, 255, 255);
    plot_text_waveshare_e_ink(UART_NUM_2, 10, 10, "Hello World", 0);
    refresh_waveshare_e_ink(UART_NUM_2);
    
}
