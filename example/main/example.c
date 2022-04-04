#include <freertos/FreeRTOS.h>
#include <nvs_flash.h>
#include <wifi.h>
#include <servo.h>
#include <dmx.h>
#include <driver/uart.h>

#ifndef CONFIG_WIPKAT_SERVO_1
#define CONFIG_WIPKAT_SERVO_1 0
#endif

#ifndef CONFIG_WIPKAT_SERVO_2
#define CONFIG_WIPKAT_SERVO_2 0
#endif

#ifndef CONFIG_WIPKAT_DMX_U1_EN
#define CONFIG_WIPKAT_DMX_U1_EN 0
#endif

#define TAG "main"

void config_uart(uint8_t port, uint8_t pin_tx, uint8_t pin_rx)
{
	int rc; 
	uart_config_t config = {
		.baud_rate = 115200,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_APB,
	};

	rc = uart_driver_install(port, 2048, 0, 0, NULL, ESP_INTR_FLAG_IRAM);
	assert(0 == rc);
	
	rc = uart_param_config(port, &config);
	assert(0 == rc);

	rc = uart_set_pin(port, pin_tx, pin_rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	assert(0 == rc);

	rc = gpio_set_pull_mode(pin_rx, GPIO_PULLUP_ONLY);
	assert(0 == rc);
}

uint8_t read_uart(uint8_t port)
{
	uint8_t buffer[1024];
	int rc = uart_read_bytes(port, buffer, 1, 100 / portTICK_PERIOD_MS);
	if (!rc) buffer[0] = 0;
	return buffer[0];
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

	config_uart(1, 14, 27);

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
	uint8_t brightness= 0xFF;
	uint8_t colorW = 0xFF;
	uint8_t colorR = 0;
	uint8_t colorG = 0;
	uint8_t colorB = 0;
	for(;;) 
	{
		uint8_t msg = read_uart(1);
		switch (msg)
		{
		case 'w': colorW = 0xFF; colorG = colorB = colorR = 0; break;
		case 'r': colorR = 0xFF; colorG = colorB = colorW = 0; break;
		case 'g': colorG = 0xFF; colorR = colorB = colorW = 0; break;
		case 'b': colorB = 0xFF; colorR = colorG = colorW = 0; break;
		case '[': brightness--; break;
		case ']': brightness++; break;
		default: break;
		}
		#if CONFIG_WIPKAT_DMX_U1_EN
		DMX.U.frame->channel[1] = brightness;
		DMX.U.frame->channel[2] = 0; // strobe open closed speed
		DMX.U.frame->channel[3] = 0; // duration
		DMX.U.frame->channel[4] = colorR;
		DMX.U.frame->channel[5] = colorG;
		DMX.U.frame->channel[6] = colorB;
		DMX.U.frame->channel[7] = colorW;
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
	}
}
