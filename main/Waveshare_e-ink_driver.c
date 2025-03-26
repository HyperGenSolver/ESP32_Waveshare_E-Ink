#include "Waveshare_e-ink_driver.h"

extern const char *TAG;

void init_waveshare_e_ink(uart_port_t uart_num, uint8_t tx_pin, uint8_t rx_pin)
{

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    // Setup UART buffered IO with event queue
    const int uart_buffer_size = (1024 * 2);
    QueueHandle_t uart_queue;
    ESP_ERROR_CHECK(uart_driver_install(uart_num, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0));
}
// --- Returns true if the handshake was successful, false otherwise
bool waveshare_e_ink_handshare(uart_port_t uart_num) 
{
    const uint16_t read_buffer_size = 1024;
    uint8_t *read_buffer = (uint8_t *) malloc(read_buffer_size);
    
    uint8_t hex_command[] = {0xA5, 0x00, 0x09, 0x00, 0xCC, 0x33, 0xC3, 0x3C, 0xAC};
    uart_write_bytes(uart_num, (const char *)hex_command, sizeof(hex_command));
    int len = uart_read_bytes(uart_num, read_buffer, (read_buffer_size - 1), 100 / portTICK_PERIOD_MS);
    if (len > 0) {
        read_buffer[len] = '\0';
        if (strcmp((char *) read_buffer, "OK") == 0) {
            free(read_buffer);
            return true;
        }

    }

    free(read_buffer);
    return false;
    
}
bool get_waveshare_e_ink_pallet(uart_port_t uart_num, uint8_t* foreground_color, uint8_t* background_color)
{
    const uint16_t read_buffer_size = 1024;
    uint8_t *read_buffer = (uint8_t *) malloc(read_buffer_size);
    
    uint8_t hex_command[] = {0xA5, 0x00, 0x09, 0x11, 0xCC, 0x33, 0xC3, 0x3C, 0xBD};
    uart_write_bytes(uart_num, (const char *)hex_command, sizeof(hex_command));
    int len = uart_read_bytes(uart_num, read_buffer, (read_buffer_size - 1), 100 / portTICK_PERIOD_MS);
    if (len > 0) {
        read_buffer[len] = '\0';

        *foreground_color = read_buffer[0] - '0'; // Convert the first character to an integer
        *background_color = read_buffer[1] - '0'; // Convert the second character to an integer
        free(read_buffer);
        return true;

    }

    free(read_buffer);
    return false;
}
// Update the e-ink display with the internal video buffer
bool refresh_waveshare_e_ink(uart_port_t uart_num)
{
    uint8_t hex_command[] = {0xA5, 0x00, 0x09, 0x0A, 0xCC, 0x33, 0xC3, 0x3C, 0xA6};
    if(uart_write_bytes(uart_num, (const char *)hex_command, sizeof(hex_command)) >= 0)
        return true;
    return false;
}
bool draw_circe_waveshare_e_ink(uart_port_t uart_num)
{
    uint8_t hex_command[] = { 0xA5, 0x00, 0x0F, 0x26, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x80, 0xCC, 0x33, 0xC3, 0x3C, 0x0C};
    if( uart_write_bytes(uart_num, (const char *)hex_command, sizeof(hex_command)) >= 0)
        return true;
    return false;
}