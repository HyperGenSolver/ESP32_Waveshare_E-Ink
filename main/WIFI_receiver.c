#include "WIFI_receiver.h"

// Debug Clock ESP32 MAC address: 24:6f:28:24:75:1c
// True Clock ESP32 MAC address: e0:e2:e6:0d:72:14
//temperature_data_struct incomingData;

static const char *TAG = "ESP-NOW Receiver";

extern uint8_t dht_sensor_pint; // GPIO pin for DHT sensor
extern CircularBuffer unified_temp_cbuffer; // Circular buffer for temperature data
extern uint16_t current_time[3]; // Current time in hours, minutes and seconds
static int last_stored_time = 0; // Variable to store the last time data was received

// Callback when data is received from wifi
void on_data_receive(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    temp_extern_data_struct incDat;
    memcpy(&incDat, data, sizeof(incDat));

    if(incDat.id > 0 && incDat.time > 0) { // Check if the data is valid   
        //Update latest time
        decode_time_from_int(incDat.time, &current_time[0], &current_time[1], &current_time[2]); // Decode the time from the integer
        uint16_t sample_time = 43200 / unified_temp_cbuffer.size; // Calculate the sample time based on the size of the circular buffer
        if (incDat.time - last_stored_time >= sample_time)
        {
            int16_t hum_buffer = 0;
            int16_t temp_buffer = 0;
            dht_read_data(DHT_TYPE_DHT11, dht_sensor_pint, &hum_buffer, &temp_buffer); // Read DHT sensor data
            temp_unified_data_struct unified_temp_data = {0};                          // Initialize the structure to zero
            unified_temp_data.id = incDat.id;
            unified_temp_data.time = incDat.time;
            unified_temp_data.temperature_outside = incDat.temperature_outside;
            unified_temp_data.temperature_inside = (float)temp_buffer / 10.0f; // Convert to float
            unified_temp_data.humidity_inside = (float)hum_buffer / 10.0f;     // Convert to float
            add_to_circular_buffer(&unified_temp_cbuffer, &unified_temp_data); // Add to circular buffer
            last_stored_time = unified_temp_data.time;                         // Update the last stored time
        }
    }

}
void get_mac_address()
{
    uint8_t mac[6];
    esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);
    ESP_LOGI("MAC address", "MAC address: %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
void init_esp_now() {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize WiFi with default config
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Initialize ESP-NOW
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(on_data_receive));

    
}
bool decode_time_from_int(int time, int *hour, int *minute, int *second) {
    *hour = time / 3600;
    *minute = (time % 3600) / 60;
    *second = time % 60;
    return true;
}
// Initialize the circular buffer
void init_circular_buffer(CircularBuffer *cb, int size) {
    cb->buffer = malloc(size * sizeof(temp_unified_data_struct));
    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
    cb->size = size;
}
bool add_to_circular_buffer(CircularBuffer *cb, temp_unified_data_struct *item) {
    if (cb == NULL || cb->buffer == NULL) {
        ESP_LOGE("CircularBuffer", "Buffer not initialized");
        return false;
    }  
    cb->buffer[cb->head] = *item;
    cb->head = (cb->head + 1) % cb->size;

    if (cb->count == cb->size) {
        // Buffer is full, so the tail needs to advance to discard the oldest element
        cb->tail = (cb->tail + 1) % cb->size;
    } else {
        // Buffer is not full, so increment the count
        cb->count++;
    }
    return true;
}
// Index can be from 0 (head) to count-1 (tail)
bool read_from_circular_buffer(CircularBuffer *cb, int logical_index, temp_unified_data_struct *out_data) {
    // Ensure the index is within the valid range
    if (logical_index < 0 || logical_index >= cb->count) {
        ESP_LOGE("CircularBuffer", "Index out of range: %d", logical_index);
        return false;
    }
    else if(cb == NULL || cb->buffer == NULL) {
        ESP_LOGE("CircularBuffer", "Buffer not initialized");
        return false;
    }
    // Calculate the actual index in the buffer array
    // The head points to the next available slot, so the latest element is at head - 1 (with wrap-around)
    int actual_index = (cb->head - 1 - logical_index + cb->size) % cb->size;

    *out_data = cb->buffer[actual_index];
    return true;
}