#include "WIFI_receiver.h"

// Debug Clock ESP32 MAC address: 24:6f:28:24:75:1c
// True Clock ESP32 MAC address: e0:e2:e6:0d:72:14
//temperature_data_struct incomingData;

static const char *TAG = "ESP-NOW Receiver";
QueueHandle_t temperature_data_queue;

// Callback when data is received
void on_data_receive(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    static temperature_data_struct incDat;
    memcpy(&incDat, data, sizeof(incDat));

    if (xQueueOverwrite(temperature_data_queue, &incDat) != pdPASS) {
        ESP_LOGE("Queue", "Failed to send temperature data to queue");
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