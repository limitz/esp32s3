idf_component_register(
	SRCS "wifi.c"
	INCLUDE_DIRS .
	REQUIRES "lwip" "esp_rom" "esp_wifi"
)
