#ifndef WAVESHARE_E_INK_DRIVER_H
#define WAVESHARE_E_INK_DRIVER_H

#include <stdint.h> // Include standard types if needed
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_system.h"
#include <string.h>

void init_waveshare_e_ink(uart_port_t uart_num, uint8_t tx_pin, uint8_t rx_pin);
bool waveshare_e_ink_handshare(uart_port_t uart_num);
bool get_waveshare_e_ink_pallet(uart_port_t uart_num, uint8_t* foreground_color, uint8_t* background_color);
bool refresh_waveshare_e_ink(uart_port_t uart_num);
bool draw_circe_waveshare_e_ink(uart_port_t uart_num);
#endif 