#pragma once

#include <stdlib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_netif.h>

typedef struct
{
	const char* ssid;
	const char* password;
} wifi_credentials_t;

int wifi_connect(const wifi_credentials_t* credentials);
