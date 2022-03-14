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
	int (*connect);
	int (*disconnect);
	int (*read)(tcpclient_t* client, uint8_t* buffer, size_t len);
	int (*write)(tcpclient_t* client, uint8_t* buffer, size_t len);

} tcpclient_t;
