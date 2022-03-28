#include "dmx.h"
#include "esp_system.h"
#include "esp_log.h"

#if DMX_U1_EN

dmx_driver_t DMX = 
{
	.universe = 
	{
		{ 
			.pin = DMX_U1_PIN, 
			.uart = DMX_U1_UART, 
		},
	}
};
			


int dmx_universe_update(dmx_universe_t* self)
{
	if (++self->_fbidx >= DMX_NUM_BUFFERS) self->_fbidx = 0;

	const char p = 0;
	
	// send break
	uart_wait_tx_done(self->uart,100000);
	uart_set_baudrate(self->uart, 57600);
	uart_write_bytes(self->uart, &p, 1);

	uart_wait_tx_done(self->uart, 100000);
	uart_set_baudrate(self->uart, 250000);
	uart_write_bytes(self->uart, (const char*) self->frame->channel, DMX_U1_NUM_CHANNELS+1);
	//uart_wait_tx_done(self->uart,1000);

	
	ESP_LOG_BUFFER_HEX(__func__, self->frame->channel, DMX_U1_NUM_CHANNELS);
	self->frame = &self->_fb[self->_fbidx];
	return ESP_OK;
}

int dmx_update()
{
	for (int i=0; i<DMX_NUM_UNIVERSES; i++)
	{
		dmx_universe_update(&DMX.universe[i]);
	}
	return ESP_OK;
}

int dmx_init()
{
	uart_config_t cfg = {
		.baud_rate = DMX_UART_BAUD_RATE,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_2,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
	};

	for (int i=0; i<DMX_NUM_UNIVERSES; i++)
	{
		DMX.universe[i].frame = &DMX.universe[i]._fb[DMX.universe[i]._fbidx];
		ESP_ERROR_CHECK(uart_param_config(DMX.universe[i].uart, &cfg));
		
		ESP_ERROR_CHECK(uart_set_pin(DMX.universe[i].uart, 
				DMX.universe[i].pin, 
				UART_PIN_NO_CHANGE, 
				UART_PIN_NO_CHANGE,
				UART_PIN_NO_CHANGE));

		ESP_ERROR_CHECK(uart_driver_install(DMX.universe[i].uart,
					550,
					0, //MS12A_RX_BUFFER_SIZE, 
					10, &DMX.queue, 0));
	}
	return ESP_OK;
}

int dmx_deinit()
{
	for (int i=0; i<DMX_NUM_UNIVERSES; i++)
	{
		uart_driver_delete(DMX.universe[i].uart);
	}
	return ESP_OK;
}
#endif
