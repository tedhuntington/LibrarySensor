/**
 * Everything to do with the water level sensor and sensing
  */
#ifndef _MS_SENSOR_WATER_LEVEL_H_
#define _MS_SENSOR_WATER_LEVEL_H_

#define ADC_GPIO GPIO_NUM_32 //GPIO number for reading in water sensor voltage (ADC currently uses pin32 ADC 1, Channel 4)

void read_water_level(void);


#endif