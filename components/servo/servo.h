#include <driver/ledc.h>

#ifndef SERVO_MAX_CHANNELS
#define SERVO_MAX_CHANNELS 16
#endif

typedef ledc_channel_t servo_channel_t;

typedef struct
{
	servo_channel_t channel;
	int pin;
	int minValue;
	int maxValue;
} servo_config_t;

int servo_config(const servo_config_t* config);
int servo_update(servo_channel_t channel, double value);
int servo_stop(servo_channel_t channel);
