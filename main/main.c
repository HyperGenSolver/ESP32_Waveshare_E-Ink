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

#include "Waveshare_e-ink_driver.h"

const char *TAG = "Waveshare E-Ink UART";

void app_main(void)
{
    // Waveshare E-Ink wiki: https://www.waveshare.com/wiki/4.3inch_e-Paper_UART_Module
    // ESP32 Wiki: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uart.html

    init_waveshare_e_ink(UART_NUM_2, 16, 17);
    if(!waveshare_e_ink_handshare(UART_NUM_2))
        ESP_LOGE(TAG, "Handshake failed");
    uint8_t foreground_color;
    uint8_t background_color;
    if (get_waveshare_e_ink_pallet(UART_NUM_2, &foreground_color, &background_color))
        ESP_LOGI(TAG, "Foreground color: %d, Background color: %d", foreground_color, background_color);
    draw_circe_waveshare_e_ink(UART_NUM_2);    
    refresh_waveshare_e_ink(UART_NUM_2);
    /*
    uint8_t hex_command2[] = {0xA5, 0x00, 0x0B, 0x10, 0x00, 0x03, 0xCC, 0x33, 0xC3, 0x3C, 0xBD};
    uart_write_bytes(uart_num, (const char *)hex_command2, sizeof(hex_command2));
    vTaskDelay(100 / portTICK_PERIOD_MS);
    uint8_t hex_command3[] = {0xA5, 0x00, 0x11, 0x22, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0xFF, 0x00, 0xFF, 0xCC, 0x33, 0xC3, 0x3C, 0x96};
    uart_write_bytes(uart_num, (const char *)hex_command3, sizeof(hex_command3));
    vTaskDelay(100 / portTICK_PERIOD_MS);
    */
}
