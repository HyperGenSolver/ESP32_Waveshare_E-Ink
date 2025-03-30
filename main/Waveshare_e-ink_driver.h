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
bool set_waveshare_e_ink_orientation(uart_port_t uart_num, uint8_t orientation);
bool import_waveshare_e_ink_font(uart_port_t uart_num);
bool refresh_waveshare_e_ink(uart_port_t uart_num);
bool clear_waveshare_e_ink(uart_port_t uart_num);

bool draw_point_waveshare_e_ink(uart_port_t uart_num, uint16_t x, uint16_t y);
bool draw_line_waveshare_e_ink(uart_port_t uart_num, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
bool draw_circe_waveshare_e_ink(uart_port_t uart_num, uint16_t x, uint16_t y, uint16_t radius, bool filled);
bool plot_text_waveshare_e_ink(uart_port_t uart_num, uint16_t x, uint16_t y, const char* text, uint8_t font_size);
uint8_t _compute_parity(uint8_t *hex_command, size_t len);
#endif 