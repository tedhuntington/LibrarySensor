#include "ms_sensor_bme280.h"

#include "common/cs_dbg.h"
#include "mgos_bme280.h"
#include "ms_state.h"

struct mgos_bme280 *bme;

extern uint32_t SensorFlags; // Which sensors have been enabled


bool init_bme280(void) {
    LOG(LL_INFO, ("BME | initializing"));

    bme = mgos_bme280_i2c_create(BME280_ADDRESS);
    if (bme) {
        SensorFlags |= SENSOR_FLAGS_FOUND_BME280; //set flag

        read_bme280(true);

        LOG(LL_INFO, ("BME | initialized"));
        return true;
    } else {
        SensorFlags &= ~SENSOR_FLAGS_FOUND_BME280; //clear flag
    }

    LOG(LL_INFO, ("BME | initialization failed"));
    return false;
}

void read_bme280(bool enabled) {
    float temperature = -1.0, humidity = -1.0, pressure = -1.0;

    if (enabled) {
        setForcedMode(bme); // needs to be set again
        temperature = mgos_bme280_read_temperature(bme);
        humidity = mgos_bme280_read_humidity(bme);
        pressure = mgos_bme280_read_pressure(bme);
    }    
    DeviceState.readings.temperature = temperature;
    DeviceState.readings.humidity = humidity;
    DeviceState.readings.pressure = pressure;

    LOG(LL_INFO, ("--- Environment ---"));
    LOG(LL_INFO, ("Temperature: %f C", DeviceState.readings.temperature));
    LOG(LL_INFO, ("Humidity: %f %%RH", DeviceState.readings.humidity));
    LOG(LL_INFO, ("Pressure: %f Pa", DeviceState.readings.pressure));

    

}
