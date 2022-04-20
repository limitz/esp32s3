#include <freertos/FreeRTOS.h>
#include <nvs_flash.h>
#include <wifi.h>
#include <servo.h>
#include <esp_http_server.h>

#ifndef CONFIG_WIPKAT_SERVO_1
#define CONFIG_WIPKAT_SERVO_1 0
#endif

#ifndef CONFIG_WIPKAT_SERVO_2
#define CONFIG_WIPKAT_SERVO_2 0
#endif

#define TAG "main"
#define GAMEPAD_SET 1
#define GAMEPAD_LOST -5;

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

static struct
{
	int set;
	float throttle;
	float brake;
	float steer;
	float view;
	float gear;
	float neutral;
} s_gamepad;

static esp_err_t ws_handler(httpd_req_t *req)
{
	if (req->method == HTTP_GET) return ESP_OK;
	
	int rc;
	uint8_t buffer[256] = {0};
	httpd_ws_frame_t ws_packet = {
		.payload = buffer,
		.type = HTTPD_WS_TYPE_BINARY,
	};
	rc = httpd_ws_recv_frame(req, &ws_packet, 256);
	assert(0 == rc);
	
	if (ws_packet.type == HTTPD_WS_TYPE_BINARY)
	{
		float* f = (float*)ws_packet.payload;
		s_gamepad.steer = f[0];
		s_gamepad.view = f[3];
		s_gamepad.brake = f[2];
		s_gamepad.throttle = f[5];
		s_gamepad.gear = f[7];
		s_gamepad.neutral = f[6];
		s_gamepad.set = GAMEPAD_SET;
		//ESP_LOGI(__func__, "S%0.02f T%0.02f B%0.02f V%0.02f", 
		//		s_gamepad.steer,
		//		s_gamepad.throttle, 
		//		s_gamepad.brake, 
		//		s_gamepad.view);
	
	}
	return ESP_OK;
}

static esp_err_t root_handler(httpd_req_t *req)
{
	httpd_resp_send(req, (char*) index_html_start, index_html_end - index_html_start);
	return ESP_OK;
}

static const httpd_uri_t uri_ws = {
	.uri         = "/ws",
	.method      = HTTP_GET,
	.handler     = ws_handler,
	.is_websocket= true
};

static const httpd_uri_t uri_root = {
	.uri         = "/",
	.method      = HTTP_GET,
	.handler     = root_handler,
};

static void start_httpd(void* arg, esp_event_base_t base, int32_t id, void* data)
{
	httpd_handle_t* server = (httpd_handle_t*) arg;
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	if (!*server)
	{
		ESP_LOGI(__func__, "Starting http server");
		int rc = httpd_start(server, &config);
		assert(0 == rc);

		httpd_register_uri_handler(*server, &uri_ws);
		httpd_register_uri_handler(*server, &uri_root);
	}
}

static void stop_httpd(void* arg, esp_event_base_t base, int32_t id, void* data)
{
	httpd_handle_t* server = (httpd_handle_t*) arg;
	if (*server)
	{
		ESP_LOGI(__func__, "Stopping http server");
		int rc = httpd_stop(*server);
		assert(0 == rc);
		*server = NULL;
	}
}

void app_main(void)
{
	esp_err_t rc = nvs_flash_init();
	if (ESP_ERR_NVS_NO_FREE_PAGES == rc || ESP_ERR_NVS_NEW_VERSION_FOUND == rc)
	{
		assert(0 == nvs_flash_erase());
		rc = nvs_flash_init();
	}
	assert(0 == rc);


	#if CONFIG_WIPKAT_WIFI
	wifi_credentials_t cred = 
	{
		.ssid = CONFIG_WIPKAT_WIFI_SSID,
		.password = CONFIG_WIPKAT_WIFI_PASSWORD,
	};
	wifi_connect(&cred);
	
	httpd_handle_t server = NULL;
	esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &start_httpd, &server);
	esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &stop_httpd, &server);
	#endif

	#if CONFIG_WIPKAT_SERVO_1
	#define SERVO1 CONFIG_WIPKAT_SERVO_1_CHANNEL
	servo_config_t servo1 =
	{
		.channel = SERVO1,
		.pin = CONFIG_WIPKAT_SERVO_1_PIN,
		.minValue = CONFIG_WIPKAT_SERVO_1_MIN,
		.maxValue = CONFIG_WIPKAT_SERVO_1_MAX,
	};
	servo_config(&servo1);
	servo_update(SERVO1,  0);	
	#endif

	#if CONFIG_WIPKAT_SERVO_2
	#define SERVO2 CONFIG_WIPKAT_SERVO_2_CHANNEL
	servo_config_t servo2 =
	{
		.channel = SERVO2,
		.pin = CONFIG_WIPKAT_SERVO_2_PIN,
		.minValue = CONFIG_WIPKAT_SERVO_2_MIN,
		.maxValue = CONFIG_WIPKAT_SERVO_2_MAX,
	};
	servo_config(&servo2);
	servo_update(SERVO2,  0);	
	#endif

	float brake = 0;
	float throttle = 0;
	float gear = 0;
	for(;;) 
	{
		brake = (s_gamepad.brake + 1) / 2;
		
		if (brake == 1 && s_gamepad.gear == -1)
		{
			gear = 1;
			brake = 0;
			throttle = 0;
		}
		if (brake == 1 && s_gamepad.gear == 1)
		{
			gear = -1;
			throttle = 0;
		}
		if (s_gamepad.neutral)
		{
			gear = 0;
		}

		#ifdef SERVO2
		if (!gear) servo_update(SERVO2, 0);
		else if (gear > 0)
		{

			if (brake > 0.10)
			{
				throttle *= 0.8;
				ESP_LOGI("SERVO2", "BRAKE %f", brake);
				servo_update(SERVO2, -brake);
			}
			else
			{
				throttle *= 0.9;
				throttle += (s_gamepad.throttle+1) / 100.0f;
				if (throttle < 0) throttle = 0;
				if (throttle > 1) throttle = 1;
				ESP_LOGI("SERVO2", "FWD %f", throttle);
				servo_update(SERVO2, throttle + 0.1f);
			}
		}
		else if (gear < 0)
		{
			if (brake > 0.10)
			{
				throttle = 0;
				ESP_LOGI("SERVO2", "BRAKE %f", brake);
				servo_update(SERVO2, 0);
			}
			else
			{
				throttle *= 0.9;
				throttle += (s_gamepad.throttle+1) / 200.0f;
				if (throttle < 0) throttle = 0;
				if (throttle > 0.5) throttle = 0.5;
				ESP_LOGI("SERVO2", "BACK %f", throttle);
				servo_update(SERVO2, -(throttle + 0.1f));
			}
		}
		#endif
		
		#ifdef SERVO1
		servo_update(SERVO1, s_gamepad.steer);
		#endif

		vTaskDelay(20 / portTICK_PERIOD_MS);
	}
}
