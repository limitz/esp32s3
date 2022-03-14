#include "servo.h"
#include "esp_log.h"

const char* TAG = "servo";

static servo_config_t s_configs[SERVO_MAX_CHANNELS] = {0};

inline double s_duty_for_us(double us)
{
	return (1<<14) * us / 20000;
}

int servo_config(const servo_config_t* config)
{
	int rc;
	assert(config->channel >= 0);
	assert(config->channel < SERVO_MAX_CHANNELS);
	assert(config->minValue < config->maxValue);
	
	double duty = s_duty_for_us((config->minValue + config->maxValue) / 2);
	ESP_LOGI(TAG, "claiming pin %d. Channel %d, Timer %d, Duty: %f", 
			config->pin, config->channel, LEDC_TIMER_0, duty);
	
	ledc_timer_config_t timerConfig =
	{
		.duty_resolution = LEDC_TIMER_14_BIT,
		.freq_hz = 50,
		.speed_mode = LEDC_LOW_SPEED_MODE,
		.timer_num = LEDC_TIMER_0,
	};
	rc = ledc_timer_config(&timerConfig);
	assert(0 == rc);

	ledc_channel_config_t channelConfig = 
	{
		.channel = config->channel,
		.duty = duty,
		.gpio_num = config->pin,
		.intr_type = LEDC_INTR_DISABLE,
		.speed_mode = LEDC_LOW_SPEED_MODE,
		.timer_sel = LEDC_TIMER_0,
	};
	ledc_channel_config(&channelConfig);
	assert(0 == rc);

	s_configs[config->channel] = *config;
	return 0;
}

int servo_stop(servo_channel_t channel)
{
	assert(channel >= 0);
	assert(channel < SERVO_MAX_CHANNELS);
	
	int rc = ledc_stop(LEDC_LOW_SPEED_MODE, channel, 0);
	assert(0 == rc);
	return 0;
}

int servo_update(servo_channel_t channel, double value)
{
	int rc;

	assert(channel >= 0);
	assert(channel < SERVO_MAX_CHANNELS);
	assert(value >= -1);
	assert(value <= +1);
	
	servo_config_t* config = &s_configs[channel];

	double us = ((value + 1) / 2) * (config->maxValue - config->minValue) + config->minValue;
	double duty = s_duty_for_us(us);

	rc = ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, duty);
	assert(0 == rc);

	rc = ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
	assert(0 == rc);
	return 0;
}
