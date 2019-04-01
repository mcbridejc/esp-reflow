#include <string.h>

#include "esp_log.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "mdns.h"

// Part of LWIP
#include "lwip/apps/sntp.h"

#include "freertos/event_groups.h"

/* Bit definition for event group */
const int CONNECTED_BIT = 0x00000001;

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

static const char logtag[] = "[wifi]";
static unsigned char ip_addr[4] = {0, 0, 0, 0};

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(logtag, "Got IP address %x", event->event_info.got_ip.ip_info.ip.addr);
        memcpy(ip_addr, &event->event_info.got_ip.ip_info.ip.addr, 4);
        ESP_LOGI(logtag, "Initializing SNTP");
        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setservername(0, "pool.ntp.org");
        sntp_init();
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
        ESP_LOGI(logtag, "STA_DISCONNECTED event");
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        ESP_LOGI(logtag, "Unhandled event %d", event->event_id);
        break;
    }
    mdns_handle_system_event(ctx, event);
    return ESP_OK;
}

void setup_mdns()
{
    uint8_t chipid[6];
    esp_efuse_mac_get_default(chipid);
    char hostname[32];
    snprintf(hostname, sizeof(hostname), "exp_%02x%02x", chipid[4], chipid[5]);

    //initialize mDNS service
    esp_err_t err = mdns_init();
    if (err) {
        printf("MDNS Init failed: %d\n", err);
        return;
    }

    ESP_LOGI("[main]", "Setting mDNS hostname to %s", hostname);
    //set hostname
    mdns_hostname_set(hostname);
    //set default instance
    mdns_instance_name_set("Explorer Node");
    mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
}

void WIFI_Initialize()
{
    /* Initialize non-volatile storage, as it is needed by wifi */
    ESP_ERROR_CHECK( nvs_flash_init() );

    tcpip_adapter_init();

    setup_mdns();

    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
        },
    };
    ESP_LOGI(logtag, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );

}

void WIFI_GetIp(unsigned char ip[4])
{
    memcpy(ip, ip_addr, 4);
}