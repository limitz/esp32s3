#include "wifi.h"
#include <string.h>

static const char* TAG = "wifi";

static void wifi_handler(void* arg, esp_event_base_t base, int32_t id, void* data)
{
	if (WIFI_EVENT == base && WIFI_EVENT_STA_START == id)
	{
		esp_wifi_connect();
		ESP_LOGI(TAG, "Starting WIFI");
	}
	if (WIFI_EVENT == base && WIFI_EVENT_STA_DISCONNECTED == id)
	{
		esp_wifi_connect();
		ESP_LOGW(TAG, "Reconnecting WIFI");
	}
	if (IP_EVENT == base && IP_EVENT_STA_GOT_IP == id)
	{
		ip_event_got_ip_t* evt = (ip_event_got_ip_t*) data;
		ESP_LOGI(TAG, "Obtained IP: " IPSTR, IP2STR(&evt->ip_info.ip));
	}
}

int wifi_connect(const wifi_credentials_t* cred)
{
	assert(cred->ssid);
	assert(cred->password);

	assert(0 == esp_netif_init());
	assert(0 == esp_event_loop_create_default());
	esp_netif_create_default_wifi_sta();

	wifi_init_config_t initConfig = WIFI_INIT_CONFIG_DEFAULT();
	assert(0 == esp_wifi_init(&initConfig));

	wifi_config_t wifiConfig =
	{
		.sta = 
		{
			//.ssid = cred->ssid,
			//.password = cred->password,
			.threshold.authmode = WIFI_AUTH_WPA2_PSK,

			.pmf_cfg = 
			{
				.capable = true,
				.required = false,
			},
		},
	};
	assert(strlen(cred->ssid) < sizeof(wifiConfig.sta.ssid));
	strncpy((char*)wifiConfig.sta.ssid, cred->ssid, sizeof(wifiConfig.sta.ssid)-1);
	
	assert(strlen(cred->password) < sizeof(wifiConfig.sta.password));
	strncpy((char*)wifiConfig.sta.password, cred->password, sizeof(wifiConfig.sta.password)-1);
	
	assert(0 == esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,    &wifi_handler, NULL));
	assert(0 == esp_event_handler_register(IP_EVENT,   IP_EVENT_STA_GOT_IP, &wifi_handler, NULL));
	assert(0 == esp_wifi_set_mode(WIFI_MODE_STA));
	assert(0 == esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));
	assert(0 == esp_wifi_start());
	return 0;
}
