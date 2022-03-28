#include <freertos/FreeRTOS.h>
#include <nvs_flash.h>
#include <wifi.h>
#include <servo.h>
#include <dmx.h>

#ifndef CONFIG_WIPKAT_SERVO_1
#define CONFIG_WIPKAT_SERVO_1 0
#endif

#ifndef CONFIG_WIPKAT_SERVO_2
#define CONFIG_WIPKAT_SERVO_2 0
#endif

#ifndef CONFIG_WIPKAT_DMX_U1_EN
#define CONFIG_WIPKAT_DMX_U1_EN 0
#endif

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

	#if CONFIG_WIPKAT_DMX_U1_EN
	dmx_init();
	#endif

	//vTaskDelay(10000 / portTICK_PERIOD_MS); 
	uint16_t pattern = 0;
	for(;;) 
	{
		#if CONFIG_WIPKAT_DMX_U1_EN
		DMX.U.frame->channel[1] = 215;
		DMX.U.frame->channel[2] = 0;
		DMX.U.frame->channel[3] = 0;
		DMX.U.frame->channel[4] = 0;
		DMX.U.frame->channel[5] = 63;
		DMX.U.frame->channel[6] = 127&((pattern++)/4);
		DMX.U.frame->channel[7] = 0;
		DMX.U.frame->channel[8] = 63&((pattern++)/8);
		DMX.U.frame->channel[9] = 0;
		DMX.U.frame->channel[10] = 0;
		DMX.U.frame->channel[11] = 0;
		DMX.U.frame->channel[12] = 0;
		dmx_update();	
		#endif
		
		#ifdef SERVO2
		servo_update(SERVO2, +0.5);
		vTaskDelay(3000 / portTICK_PERIOD_MS); 
		servo_update(SERVO2, +1.0);	
		vTaskDelay(1500 / portTICK_PERIOD_MS); 
		servo_update(SERVO2,  0.0);
		vTaskDelay(1000 / portTICK_PERIOD_MS); 
		servo_update(SERVO2, -1.0);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		servo_update(SERVO2,  0.2);
		#endif
		
		#ifdef SERVO1
		servo_update(SERVO1, +1);
		vTaskDelay(500 / portTICK_PERIOD_MS);
		servo_update(SERVO1,  0);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		servo_update(SERVO1, -1);
		vTaskDelay(500 / portTICK_PERIOD_MS);
		servo_update(SERVO1,  0);
		vTaskDelay(150 / portTICK_PERIOD_MS);
		#endif

		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}
