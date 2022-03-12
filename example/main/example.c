#include <freertos/FreeRTOS.h>
#include <nvs_flash.h>
#include <wifi.h>

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
	for(;;) { vTaskDelay(1000 / portTICK_PERIOD_MS); }
}
