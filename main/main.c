/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
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

const char *TAG = "Waveshare E-Ink UART Main";
CircularBuffer unified_temp_cbuffer; // Circular buffer for temperature data (inside and outside) and time
uint8_t current_time [3] = {0, 0, 0}; // Current time in hours, minutes and seconds

const uint8_t dht_sensor_pint = 21; // GPIO pin for DHT sensor
const uint16_t temp_buffer_item_size = 360; // Number of items in the temperature data queue

const uint16_t time_wheel_center_pos[2] = { 300, 400 }; // Center position of the time wheel (x, y) in pixels
const uint16_t time_wheel_radii[2] = {180, 280}; // Radius of the time wheel min and max radius in pixels
const uint16_t temp_wheel_ranges[2] = {0, 30}; // Temperature range in Celsius (min and max)

short outer_ring_degree_text_offset = -20; // Offset from the outer ring in pixels
short inner_ring_degree_text_offset = 0;
uint16_t time_wheel_radius_offset = 20; // Offset from the time wheel radius in pixels of the time lines
uint16_t current_time_text_pos[2] = { 260, 410 }; // Position of the current time text in pixels (x, y)

void draw_time_wheel(void);

void app_main(void)
{
    // Waveshare E-Ink wiki: https://www.waveshare.com/wiki/4.3inch_e-Paper_UART_Module
    // ESP32 Wiki: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uart.html
    
    // --- Init esp now ---
    //init_esp_now();
    // --- Intit the circular buffer for the temperature---
    init_circular_buffer(&unified_temp_cbuffer, temp_buffer_item_size);

    // --- Init waveshare e-ink ---
    
    init_waveshare_e_ink(UART_NUM_2, 16, 17);
    waveshare_e_ink_handshare(UART_NUM_2);
    /*
    set_waveshare_e_ink_storage_area(UART_NUM_2, 0); 
    clear_waveshare_e_ink(UART_NUM_2);
    set_waveshare_e_ink_orientation(UART_NUM_2, 3); 

    //import_waveshare_e_ink_font(UART_NUM_2);
    set_waveshare_e_ink_font_size(UART_NUM_2, 1); 
    //plot_text_waveshare_e_ink(UART_NUM_2, 10, 10, "test");

    draw_time_wheel(); // Draw the time wheel

    refresh_waveshare_e_ink(UART_NUM_2);
    */
}
void draw_time_wheel()
{
    // Fake data in the circular buffer for testing
    for (int i = 0; i < 50; i++) {
        temp_unified_data_struct item;
        item.id = i+1;
        item.time = i * 120; // Simulate time in seconds
        item.temperature_inside = 25;
        item.temperature_outside = i *0.1; // Simulate temperature in Celsius
        add_to_circular_buffer(&unified_temp_cbuffer, &item); // Add to circular buffer
    }
    // --- Draw the time wheel
    set_waveshare_e_ink_pallet(UART_NUM_2, 2, 3); // Set the color pallet to black and white
    draw_circe_waveshare_e_ink(UART_NUM_2, time_wheel_center_pos[0], time_wheel_center_pos[1], time_wheel_radii[0], false); // Draw the time wheel
    draw_circe_waveshare_e_ink(UART_NUM_2, time_wheel_center_pos[0], time_wheel_center_pos[1], time_wheel_radii[1], false); // Draw the time wheel
    // -- Draw temp indication
    set_waveshare_e_ink_font_size(UART_NUM_2, 0); 
    char temp_range_str[10]; // Allocate a buffer to hold the string (large enough for the number and null terminator)
    sprintf(temp_range_str, "%uC", temp_wheel_ranges[0]); // Convert the number to a string adn add the degree symbol and 'C'
    plot_text_waveshare_e_ink(UART_NUM_2, time_wheel_center_pos[0] - 5, time_wheel_center_pos[1] - time_wheel_radii[0] + outer_ring_degree_text_offset, temp_range_str);
    sprintf(temp_range_str, "%uC", temp_wheel_ranges[1]); // Convert the number to a string adn add the degree symbol and 'C'
    plot_text_waveshare_e_ink(UART_NUM_2, time_wheel_center_pos[0] - 5, time_wheel_center_pos[1] - time_wheel_radii[1] + inner_ring_degree_text_offset, temp_range_str);
    // -- Draw time indication
    set_waveshare_e_ink_pallet(UART_NUM_2, 2, 3);
    for(int i = 0; i < 12; i++) // Draw the time indication in hours
    {
        float polar_line[4]; // angle 0, radius 0, angle 1, radius 1
        polar_line[0] = i*0.5235; // Calculate the polar angle of the item
        polar_line[1] = time_wheel_radii[0] + time_wheel_radius_offset;
        polar_line[2] = polar_line[0];
        polar_line[3] = time_wheel_radii[1] - time_wheel_radius_offset;


        uint16_t cartesian_time_out[4]; // Cartesian coordinates of the item in pixels
        cartesian_time_out[0] = time_wheel_center_pos[0] + cos(1.571 - polar_line[0]) * polar_line[1]; // Calculate the x position of the item
        cartesian_time_out[1] = time_wheel_center_pos[1] - sin(1.571 - polar_line[0]) * polar_line[1]; // Calculate the y position of the item
        cartesian_time_out[2] = time_wheel_center_pos[0] + cos(1.571 - polar_line[2]) * polar_line[3]; // Calculate the x position of the item
        cartesian_time_out[3] = time_wheel_center_pos[1] - sin(1.571 - polar_line[2]) * polar_line[3]; // Calculate the y position of the item
        draw_line_waveshare_e_ink(UART_NUM_2, cartesian_time_out[0], cartesian_time_out[1], cartesian_time_out[2], cartesian_time_out[3]); // Draw the line
    }
    // -- draw data traces in the time wheel (inside and outside temperature)
    uint16_t prev_cartesian_sample[4] = {0};
    bool first_point = true; // Flag to handle the first point
    for(int i = 0; i < unified_temp_cbuffer.count; i++) //starting at head and going to tail
    {
        temp_unified_data_struct item;
        read_from_circular_buffer(&unified_temp_cbuffer, i, &item); // Read the item from the circular buffer
        float polar_temperature[3]; // Polar angle and radius of the item in radians
        polar_temperature[0] = item.time * 2.0 * 3.14156 / 43200.0; // Calculate the polar angle of the item
        polar_temperature[1] = time_wheel_radii[0] + (time_wheel_radii[1] - time_wheel_radii[0]) / (temp_wheel_ranges[1] - temp_wheel_ranges[0]) * item.temperature_outside; // Calculate the polar radius of the item
        polar_temperature[2] = time_wheel_radii[0] + (time_wheel_radii[1] - time_wheel_radii[0]) / (temp_wheel_ranges[1] - temp_wheel_ranges[0]) * item.temperature_inside;

        uint16_t cartesian_temperature_out[2]; // Cartesian coordinates of the item in pixels
        uint16_t cartesian_temperature_in[2]; // Cartesian coordinates of the item in pixels of inside temperature
        cartesian_temperature_out[0] = time_wheel_center_pos[0] + cos(1.571 - polar_temperature[0]) * polar_temperature[1]; // Calculate the x position of the item
        cartesian_temperature_out[1] = time_wheel_center_pos[1] - sin(1.571 - polar_temperature[0]) * polar_temperature[1]; // Calculate the y position of the item
        cartesian_temperature_in[0] = time_wheel_center_pos[0] + cos(1.571 - polar_temperature[0]) * polar_temperature[2]; // Calculate the x position of the item
        cartesian_temperature_in[1] = time_wheel_center_pos[1] - sin(1.571 - polar_temperature[0]) * polar_temperature[2]; // Calculate the y position of the item
        // Draw a line from the previous point to the current point
        if (!first_point) {
            set_waveshare_e_ink_pallet(UART_NUM_2, 0, 3); // Set the color pallet to black and white
            draw_line_waveshare_e_ink(UART_NUM_2, prev_cartesian_sample[0], prev_cartesian_sample[1], cartesian_temperature_out[0], cartesian_temperature_out[1]); // Draw the line
            set_waveshare_e_ink_pallet(UART_NUM_2, 1, 3); // Set the color pallet to black and white
            draw_line_waveshare_e_ink(UART_NUM_2, prev_cartesian_sample[2], prev_cartesian_sample[3], cartesian_temperature_in[0], cartesian_temperature_in[1]); // Draw the line
        } else {
            first_point = false; // Mark the first point as processed
        }

        // Update the previous point
        prev_cartesian_sample[0] = cartesian_temperature_out[0];
        prev_cartesian_sample[1] = cartesian_temperature_out[1];
        prev_cartesian_sample[2] = cartesian_temperature_in[0];
        prev_cartesian_sample[3] = cartesian_temperature_in[1];
    }
    // -- Draw text in the middle of the time wheel
    set_waveshare_e_ink_font_size(UART_NUM_2, 2);
    // - Time
    char time_str[10]; // Allocate a buffer to hold the string (large enough for the number and null terminator)
    sprintf(time_str, "%02d:%02d", current_time[0], current_time[1]); // Convert the number to a string and add the degree symbol and 'C'
    plot_text_waveshare_e_ink(UART_NUM_2, current_time_text_pos[0], current_time_text_pos[1], time_str); // Plot the text in the middle of the time wheel
}