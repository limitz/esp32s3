#include <freertos/FreeRTOS.h>
#include <nvs_flash.h>
#include <wifi.h>
#include <servo.h>

#ifndef CONFIG_WIPKAT_SERVO_1
#define CONFIG_WIPKAT_SERVO_1 0
#endif

#ifndef CONFIG_WIPKAT_SERVO_2
#define CONFIG_WIPKAT_SERVO_2 0
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

	wifi_init();

	servo_channel_t s1 = -1;
	servo_channel_t s2 = -1;

	#if CONFIG_WIPKAT_SERVO_1
	servo_config_t servo1 =
	{
		.channel = CONFIG_WIPKAT_SERVO_1_CHANNEL,
		.pin = CONFIG_WIPKAT_SERVO_1_PIN,
		.minValue = CONFIG_WIPKAT_SERVO_1_MIN,
		.maxValue = CONFIG_WIPKAT_SERVO_1_MAX,
	};
	servo_config(&servo1);
	s1 = CONFIG_WIPKAT_SERVO_1_CHANNEL;
	#endif

	#if CONFIG_WIPKAT_SERVO_2
	servo_config_t servo2 =
	{
		.channel = CONFIG_WIPKAT_SERVO_2_CHANNEL,
		.pin = CONFIG_WIPKAT_SERVO_2_PIN,
		.minValue = CONFIG_WIPKAT_SERVO_2_MIN,
		.maxValue = CONFIG_WIPKAT_SERVO_2_MAX,
	};
	servo_config(&servo2);
	s2 = CONFIG_WIPKAT_SERVO_2_CHANNEL;
	#endif

	for(;;) 
	{ 
		servo_update(s1, -1);
		vTaskDelay(1000 / portTICK_PERIOD_MS); 
		
		servo_update(s1,  0);	
		vTaskDelay(1000 / portTICK_PERIOD_MS); 

		servo_update(s1, +1);
		vTaskDelay(1000 / portTICK_PERIOD_MS); 
	
		servo_update(s1,  0);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
