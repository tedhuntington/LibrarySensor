/**
 * BME280 | Temperature, pressure, humidity sensor
 */
#ifndef _MS_SENSOR_BME280_H_
#define _MS_SENSOR_BME280_H_

#define BME280_ADDRESS 0x76
#define SENSOR_FLAGS_FOUND_BME280  0x00000001

bool init_bme280(void);
void read_bme280(bool enabled);

#endif
