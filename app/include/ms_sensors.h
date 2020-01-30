#ifndef _MS_SENSORS_H_
#define _MS_SENSORS_H_

#include "mgos_gpio.h"
#include "ms_sensor_bme280.h"
#include "ms_sensor_water_level.h"


#define LED_GPIO GPIO_NUM_12  //GPIO number for LED
#define AP_GPIO GPIO_NUM_2 //GPIO number for reading in user toggle AP mode flag


void init_sensors(void);
void read_sensors(void);
void perform_readings(int button);

#endif