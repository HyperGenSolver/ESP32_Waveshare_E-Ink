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
    const size_t read_buffer_size = 1024;
    uint8_t read_buffer[read_buffer_size];

    uint8_t hex_command[] = {0xA5, 0x00, 0x09, 0x00, 0xCC, 0x33, 0xC3, 0x3C, 0xAC};
    size_t hex_command_len = sizeof(hex_command) / sizeof(hex_command[0]);
    uart_write_bytes(uart_num, (const char *)hex_command, hex_command_len);
    int len = uart_read_bytes(uart_num, read_buffer, (read_buffer_size - 1), 1000 / portTICK_PERIOD_MS);
    if (len > 0) {
        read_buffer[len] = '\0';
        if (strcmp((char *) read_buffer, "OK") == 0) {
            return true;
        }
        else{
            ESP_LOGE(TAG, "Handshake failed");
            return false;
        }

    }
    else{
        ESP_LOGE(TAG, "Handshake timeout");
        return false;
    }
    
}
// Set the storage area of the e-ink display: 0 = internal Nand, 1 = SD card
bool set_waveshare_e_ink_storage_area(uart_port_t uart_num, uint8_t storage_area){
    uint8_t hex_command[] = {0xA5, 0x00, 0x0A, 0x07, 0x00, 0xCC, 0x33, 0xC3, 0x3C, 0xA8};
    size_t hex_command_len = sizeof(hex_command) / sizeof(hex_command[0]);
    switch (storage_area) // Set the font size
    {
    case 0: // 32 dots
        hex_command[4] = 0x00;
        break;
    case 1: // 48 dots
        hex_command[4] = 0x01;
        break;
    default:
        ESP_LOGE(TAG, "Invalid font size: %d", storage_area);
        return false; // Invalid font size
        break;
    }
    // Compute the parity byte
    hex_command[9] = _compute_parity(hex_command, hex_command_len);
    uart_write_bytes(uart_num, (const char *)hex_command, hex_command_len);

    const size_t read_buffer_size = 1024;
    uint8_t read_buffer [read_buffer_size];
    int len = uart_read_bytes(uart_num, read_buffer, (read_buffer_size - 1), 100 / portTICK_PERIOD_MS);
    if (len > 0) {
        read_buffer[len] = '\0';
        if (strcmp((char *) read_buffer, "OK") == 0) 
            return true;
        else {
            ESP_LOGE(TAG, "Set storage area failed: %s", read_buffer);
            return false;
            }
    }
    else
    {
        ESP_LOGE(TAG, "Set storage area timeout");
        return false;
    }
}
// Set the color pallet of the e-ink display: 0 = black, 1 = dark grey, 2 = light grey, 3 = white
bool set_waveshare_e_ink_pallet(uart_port_t uart_num, uint8_t foreground_color, uint8_t background_color)
{
    uint8_t hex_command[] = {0xA5, 0x00, 0x0B, 0x10, 0x00, 0x03, 0xCC, 0x33, 0xC3, 0x3C, 0xBD};
    size_t hex_command_len = sizeof(hex_command) / sizeof(hex_command[0]);
    hex_command[4] = foreground_color; // Set the foreground color
    hex_command[5] = background_color; // Set the background color
    // Compute the parity byte
    hex_command[10] = _compute_parity(hex_command, hex_command_len);
    uart_write_bytes(uart_num, (const char *)hex_command, hex_command_len);

    const size_t read_buffer_size = 1024;
    uint8_t read_buffer [read_buffer_size];
    int len = uart_read_bytes(uart_num, read_buffer, (read_buffer_size - 1), 10 / portTICK_PERIOD_MS);
    if (len > 0) {
        read_buffer[len] = '\0';
        if (strcmp((char *) read_buffer, "OK") == 0) 
            return true;
        else {
            ESP_LOGE(TAG, "Set pallet failed: %s", read_buffer);
            return false;
            }
    }
    else
    {
        ESP_LOGE(TAG, "Set pallet timeout");
        return false;
    }
}
// --- Draw functions ---
bool get_waveshare_e_ink_pallet(uart_port_t uart_num, uint8_t* foreground_color, uint8_t* background_color)
{
    const uint16_t read_buffer_size = 1024;
    uint8_t *read_buffer = (uint8_t *) malloc(read_buffer_size);
    
    uint8_t hex_command[] = {0xA5, 0x00, 0x09, 0x11, 0xCC, 0x33, 0xC3, 0x3C, 0xBD};
    size_t hex_command_len = sizeof(hex_command) / sizeof(hex_command[0]);
    uart_write_bytes(uart_num, (const char *)hex_command, hex_command_len);
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
// Stet the font size of the e-ink display: 0 = 32dots, 1 = 48dots, 2 = 64dots
bool set_waveshare_e_ink_font_size(uart_port_t uart_num, uint8_t font_size){
    uint8_t hex_command[] = {0xA5, 0x00, 0x0A, 0x1E, 0x01, 0xCC, 0x33, 0xC3, 0x3C, 0xA3};
    size_t hex_command_len = sizeof(hex_command) / sizeof(hex_command[0]);
    switch (font_size) // Set the font size
    {
    case 0: // 32 dots
        hex_command[4] = 0x01;
        break;
    case 1: // 48 dots
        hex_command[4] = 0x02;
        break;
    case 2: // 64 dots
        hex_command[4] = 0x03;
        break;
    default:
        ESP_LOGE(TAG, "Invalid font size: %d", font_size);
        return false; // Invalid font size
        break;
    }
    // Compute the parity byte
    hex_command[9] = _compute_parity(hex_command, hex_command_len);
    uart_write_bytes(uart_num, (const char *)hex_command, hex_command_len);

    const size_t read_buffer_size = 1024;
    uint8_t read_buffer [read_buffer_size];
    int len = uart_read_bytes(uart_num, read_buffer, (read_buffer_size - 1), 100 / portTICK_PERIOD_MS);
    if (len > 0) {
        read_buffer[len] = '\0';
        if (strcmp((char *) read_buffer, "OK") == 0) 
            return true;
        else {
            ESP_LOGE(TAG, "Set font size failed: %s", read_buffer);
            return false;
            }
    }
    else
    {
        ESP_LOGE(TAG, "Set font size timeout");
        return false;
    }
}
// Set the orientation of the e-ink display: 0 = 0 degrees, 1 = 90 degrees, 2 = 180 degrees, 3 = 270 degrees
bool set_waveshare_e_ink_orientation(uart_port_t uart_num, uint8_t orientation){
    uint8_t hex_command[] = {0xA5, 0x00, 0x0A, 0x0D, 0x01, 0xCC, 0x33, 0xC3, 0x3C, 0xA3};
    size_t hex_command_len = sizeof(hex_command) / sizeof(hex_command[0]);
    hex_command[4] = orientation; // Set the orientation
    // Compute the parity byte
    hex_command[9] = _compute_parity(hex_command, hex_command_len);
    uart_write_bytes(uart_num, (const char *)hex_command, hex_command_len);

    const size_t read_buffer_size = 1024;
    uint8_t read_buffer [read_buffer_size];
    int len = uart_read_bytes(uart_num, read_buffer, (read_buffer_size - 1), 100 / portTICK_PERIOD_MS);
    if (len > 0) {
        read_buffer[len] = '\0';
        if (strcmp((char *) read_buffer, "OK") == 0) 
            return true;

    }

    return false;

}
bool import_waveshare_e_ink_font(uart_port_t uart_num){
    uint8_t hex_command[] = {0xA5, 0x00, 0x09, 0x0E, 0xCC, 0x33, 0xC3, 0x3C, 0xA2};
    size_t hex_command_len = sizeof(hex_command) / sizeof(hex_command[0]);
    uart_write_bytes(uart_num, (const char *)hex_command, hex_command_len);

    const size_t read_buffer_size = 1024;
    uint8_t read_buffer[read_buffer_size];
    int len = uart_read_bytes(uart_num, read_buffer, (read_buffer_size - 1), 8000 / portTICK_PERIOD_MS);
    if (len > 0) {
        read_buffer[len] = '\0';
        if (strcmp((char *) read_buffer, "OK") == 0) 
            return true;
        else {
            ESP_LOGE(TAG, "Import font failed: %s", read_buffer);
            return false;
        }
    }
    else{
        ESP_LOGE(TAG, "Import font timeout");
        return false;
    }
}
// Update the e-ink display with the internal video buffer
bool refresh_waveshare_e_ink(uart_port_t uart_num)
{
    uint8_t hex_command[] = {0xA5, 0x00, 0x09, 0x0A, 0xCC, 0x33, 0xC3, 0x3C, 0xA6};
    size_t hex_command_len = sizeof(hex_command) / sizeof(hex_command[0]);
    uart_write_bytes(uart_num, (const char *)hex_command, hex_command_len);

    const size_t read_buffer_size = 1024;
    uint8_t read_buffer[read_buffer_size];
    int len = uart_read_bytes(uart_num, read_buffer, (read_buffer_size - 1), 100 / portTICK_PERIOD_MS);
    if (len > 0) {
        read_buffer[len] = '\0';
        if (strcmp((char *) read_buffer, "OK") == 0) 
            return true;

    }
    return false;
}
// Clear the e-ink display with the background color
bool clear_waveshare_e_ink(uart_port_t uart_num)
{
    uint8_t hex_command[] = {0xA5, 0x00, 0x09, 0x2E, 0xCC, 0x33, 0xC3, 0x3C, 0x82};
    size_t hex_command_len = sizeof(hex_command) / sizeof(hex_command[0]);
    uart_write_bytes(uart_num, (const char *)hex_command, hex_command_len);

    const size_t read_buffer_size = 1024;
    uint8_t read_buffer[read_buffer_size];
    int len = uart_read_bytes(uart_num, read_buffer, (read_buffer_size - 1), 100 / portTICK_PERIOD_MS);
    if (len > 0) {
        read_buffer[len] = '\0';
        if (strcmp((char *) read_buffer, "OK") == 0) 
            return true;

    }
    return false;
}
// Not working yet, need to check the command
bool draw_point_waveshare_e_ink(uart_port_t uart_num, uint16_t x, uint16_t y){
    //0x20 = draw point
    //0x00 = high_byte x
    //0x0A = low_byte x
    //0x00 = high_byte y
    //0x0A = low_byte y
    uint8_t hex_command[] = {0xA5, 0x00, 0x0D, 0x20, 0x00, 0x0A, 0x00, 0x0A, 0xCC, 0x33, 0xC3, 0x3C, 0x88};
    size_t hex_command_len = sizeof(hex_command) / sizeof(hex_command[0]);
    hex_command[4] = (x >> 8) & 0xFF; // high byte of x
    hex_command[5] = x & 0xFF; // low byte of x
    hex_command[6] = (y >> 8) & 0xFF; // high byte of y
    hex_command[7] = y & 0xFF;        // low byte of y
    // Compute the parity byte
    hex_command[12] = _compute_parity(hex_command, hex_command_len); 
    uart_write_bytes(uart_num, (const char *)hex_command, hex_command_len);

    const size_t read_buffer_size = 1024;
    uint8_t read_buffer[read_buffer_size];
    int len = uart_read_bytes(uart_num, read_buffer, (read_buffer_size - 1), 100 / portTICK_PERIOD_MS);
    if (len > 0) {
        read_buffer[len] = '\0';
        if (strcmp((char *) read_buffer, "OK") == 0) 
            return true;

    }
    return false;
}
bool draw_line_waveshare_e_ink(uart_port_t uart_num, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{ // ------------------------Header, Length   ,cmd , x0        , y0        , x1        , y1        , frame end             , parity byte
    uint8_t hex_command[] = {0xA5, 0x00, 0x11, 0x22, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0xFF, 0x00, 0xFF, 0xCC, 0x33, 0xC3, 0x3C, 0x96};
    size_t hex_command_len = sizeof(hex_command) / sizeof(hex_command[0]);
    hex_command[4] = (x0 >> 8) & 0xFF; // high byte of x0
    hex_command[5] = x0 & 0xFF; // low byte of x0
    hex_command[6] = (y0 >> 8) & 0xFF; // high byte of y0
    hex_command[7] = y0 & 0xFF;        // low byte of y0
    hex_command[8] = (x1 >> 8) & 0xFF; // high byte of x1
    hex_command[9] = x1 & 0xFF; // low byte of x1
    hex_command[10] = (y1 >> 8) & 0xFF; // high byte of y1
    hex_command[11] = y1 & 0xFF;        // low byte of y1
    // Compute the parity byte
    hex_command[16] = _compute_parity(hex_command, hex_command_len);
    uart_write_bytes(uart_num, (const char *)hex_command, hex_command_len);
    
    const size_t read_buffer_size = 1024;
    uint8_t read_buffer[read_buffer_size];
    int len = uart_read_bytes(uart_num, read_buffer, (read_buffer_size - 1), 10 / portTICK_PERIOD_MS);
    if (len > 0) {
        read_buffer[len] = '\0';
        if (strcmp((char *) read_buffer, "OK") == 0) 
            return true;

    }
    return false;
}
bool draw_circe_waveshare_e_ink(uart_port_t uart_num, uint16_t x, uint16_t y, uint16_t radius, bool filled)
{
    uint8_t hex_command[] = { 0xA5, 0x00, 0x0F, 0x26, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x80, 0xCC, 0x33, 0xC3, 0x3C, 0x0C};
    size_t hex_command_len = sizeof(hex_command) / sizeof(hex_command[0]);
    if(filled)
        hex_command[3] = 0x27; // filled circle

    hex_command[4] = (x >> 8) & 0xFF; // high byte of x
    hex_command[5] = x & 0xFF; // low byte of x
    hex_command[6] = (y >> 8) & 0xFF; // high byte of y
    hex_command[7] = y & 0xFF;        // low byte of y
    hex_command[8] = (radius >> 8) & 0xFF; // high byte of radius
    hex_command[9] = radius & 0xFF; // low byte of radius
    hex_command[14] = _compute_parity(hex_command, hex_command_len); // Compute the parity byte
    uart_write_bytes(uart_num, (const char *)hex_command, hex_command_len);

    const size_t read_buffer_size = 1024;
    uint8_t read_buffer[read_buffer_size];
    int len = uart_read_bytes(uart_num, read_buffer, (read_buffer_size - 1), 100 / portTICK_PERIOD_MS);
    if (len > 0) {
        read_buffer[len] = '\0';
        if (strcmp((char *) read_buffer, "OK") == 0) 
            return true;

    }
    return false;
}
bool plot_text_waveshare_e_ink(uart_port_t uart_num, uint16_t x, uint16_t y, char* text)
{
    uint16_t text_len = strlen(text); // Get the length of the text e.g World = 5, does not include the null terminator
    uint16_t hex_command_len = 14 + text_len; // Header (1) + Length(2) + cmd(1) + x(2) + y(2) + text(text_len+1) + frame end (4) + parity byte (1)
    uint8_t *hex_command = (uint8_t *) malloc(hex_command_len); // Allocate memory for the command
    hex_command[0] = 0xA5; // Header
    hex_command[1] = (hex_command_len >> 8) & 0xFF; // High byte of hex_command_len
    hex_command[2] = hex_command_len & 0xFF;       // Low byte of hex_command_len
    hex_command[3] = 0x30; // Command
    hex_command[4] = (x >> 8) & 0xFF; // High byte of x
    hex_command[5] = x & 0xFF;       // Low byte of x
    hex_command[6] = (y >> 8) & 0xFF; // High byte of x
    hex_command[7] = y & 0xFF;       // Low byte of x
    memcpy(&hex_command[8], text, text_len);
    hex_command[8 + text_len] = 0x00; // Null terminator
    hex_command[9 + text_len] = 0xCC; // Frame end
    hex_command[10 + text_len] = 0x33; // Frame end
    hex_command[11 + text_len] = 0xC3; // Frame end
    hex_command[12 + text_len] = 0x3C; // Frame end
    hex_command[13 + text_len] = _compute_parity(hex_command, hex_command_len); // Compute the parity byte
    //uint8_t hex_command[] = {0xA5, 0x00, 0x17, 0x30, 0x00, 0x0A, 0x00, 0x0A, 0xC4, 0xE3, 0xBA, 0xC3, 0x57, 0x6F, 0x72, 0x6C, 0x64, 0x00, 0xCC, 0x33, 0xC3, 0x3C, 0x9E};
    uart_write_bytes(uart_num, (const char *)hex_command, hex_command_len);
    free(hex_command); // Free the allocated memory
    // --- Read the response from the e-ink display
    const size_t read_buffer_size = 1024;
    uint8_t read_buffer[read_buffer_size];
    int len = uart_read_bytes(uart_num, read_buffer, (read_buffer_size - 1), 100 / portTICK_PERIOD_MS);
    if (len > 0) {
        read_buffer[len] = '\0';
        if (strcmp((char *) read_buffer, "OK") == 0) 
            return true;

    }
    return false;
}

// --- Helper functions ---
uint8_t _compute_parity(uint8_t *hex_command, size_t len)
{
    uint8_t parity = 0;
    for (size_t i = 0; i < (len-1); i++) { //exclude last byte

        parity ^= hex_command[i];
    }
    return parity;
}
