#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <esp_mac.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <nvs_flash.h>


#define ESP_CHANNEL 1
static uint8_t peer_mac [ESP_NOW_ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

typedef struct struct_message {
    uint8_t value;
    /* 
    Add data
    ...
    uint8_t data_uint8;
    uint16_t data_uint16;
    float data_float;
    ...
    */
} struct_message;
struct_message my_struct;

extern "C" void app_main();

static const char * TAG = "ESP_NOW";

static esp_err_t init_wifi(void)
{
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    
    esp_netif_init();
    esp_event_loop_create_default();
    nvs_flash_init();
    esp_wifi_init(&wifi_init_config);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_storage(WIFI_STORAGE_FLASH);
    esp_wifi_start();

    return ESP_OK;
}

void recv_cb(const esp_now_recv_info_t * esp_now_info, const uint8_t *data, int data_len)
{
    ESP_LOGI(TAG, "Data received from " MACSTR "\t Value: %u", MAC2STR(esp_now_info->src_addr), my_struct.value);
}

void send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    if(status == ESP_NOW_SEND_SUCCESS)
    {
        ESP_LOGI(TAG, "ESP_NOW_SEND_SUCCESS");
    }
    else
    {
        ESP_LOGW(TAG, "ESP_NOW_SEND_FAIL");
    }
}

static esp_err_t init_esp_now(void)
{
    esp_now_init();
    esp_now_register_recv_cb(recv_cb);
    esp_now_register_send_cb(send_cb);

    return ESP_OK;
}

static esp_err_t register_peer(uint8_t *peer_addr)
{
    esp_now_peer_info_t esp_now_peer_info;
    memset(&esp_now_peer_info, 0, sizeof(esp_now_peer_info_t));
    esp_now_peer_info.channel = ESP_CHANNEL;
    esp_now_peer_info.ifidx = WIFI_IF_STA;
    esp_now_peer_info.encrypt = false;
    memcpy(esp_now_peer_info.peer_addr, peer_mac, ESP_NOW_ETH_ALEN);
    esp_now_add_peer(&esp_now_peer_info);

    return ESP_OK;
}


void app_main()
{
    if(init_wifi() == ESP_OK)
    {
        ESP_LOGI(TAG, "WiFi Init Completed");
    }
    else{
        ESP_LOGE(TAG, "WiFi Init Error");
    }

    if(init_esp_now() == ESP_OK)
    {
        ESP_LOGI(TAG, "EspNow Init Completed");
    }
    else{
        ESP_LOGE(TAG, "EspNow Init Error");
    } 

    if(register_peer(peer_mac) == ESP_OK)
    {
        ESP_LOGI(TAG, "Register Peer Completed");
    }
    else{
        ESP_LOGE(TAG, "Register Peer Error");
    } 

    my_struct.value = 0x01;

    for(;;)
    {
        esp_now_send(peer_mac, (uint8_t *) &my_struct, sizeof(my_struct));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

}