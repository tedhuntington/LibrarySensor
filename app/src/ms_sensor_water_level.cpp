#include "common/cs_dbg.h"
#include "mgos_adc.h"
#include "mgos_gpio.h"
#include "ms_sensors.h"
#include "ms_state.h"
#include "ms_sensor_water_level.h"

void read_water_level(void) 
{
  int WaterLevelReading=0;  //digital value  (12-bit 0-4095)
  WaterLevelReading=mgos_adc_read_voltage(ADC_GPIO);
  DeviceState.readings.WaterLevel= (float)WaterLevelReading/4095.0;  // divide by 1240.9 for voltage relative to 3.3v
  LOG(LL_INFO, ("WaterLevel: %f",DeviceState.readings.WaterLevel));

}