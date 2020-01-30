/**
 * Handles everything to do with sensors and taking readings
 */

#include "mgos_mqtt.h"
#include "mgos_i2c.h"
#include "mgos_bme280.h"  //bme280 sensor

#include "common/cs_dbg.h"
#include "mgos_gpio.h"
#include "mgos_mqtt.h"
#include "mgos_shadow.h"
#include "mgos_timers.h"
#include "ms_cloud.h"
#include "ms_sensors.h"
#include "ms_state.h"
#include "ms_sensor_bme280.h"
#include "ms_sensor_water_level.h"


uint32_t SensorFlags;
const char topicReadings[]= "readings";
const char topicAlerts[]= "alerts";

uint32_t ReadingInterval;     // Seconds in between performing readings
uint32_t SendingInterval;     // Seconds in between sending readings (in shadow update)
RTC_DATA_ATTR time_t LastSendTimeStamp=0;  //timestamp when data was last sent in a shadow update

mgos_timer_id ReadingTimer = MGOS_INVALID_TIMER_ID;

extern RTC_DATA_ATTR bool time_synced;
extern uint32_t ShadowAcceptTimeout; //seconds to wait for a shadow accept before giving up
extern mgos_timer_id ShadowAcceptTimeoutTimer;

void perform_readings(int button) {
	time_t TimeToNextSend;

  LOG(LL_INFO, ("\n\n****************\nREADINGS | begin"));

  read_bme280(SensorFlags & SENSOR_FLAGS_FOUND_BME280);


	LOG(LL_INFO, ("button=%d",button));

	TimeToNextSend=LastSendTimeStamp+SendingInterval-time(NULL);
	if (button || (time_synced && (TimeToNextSend<0))) {
        //Send data
		if (button) {
		    LOG(LL_INFO, ("Button read and send"));
		} 
		
		LastSendTimeStamp=time(NULL);

        //Send MQTT

    bool res = mgos_mqtt_pubf(topicReadings, 0, false /* retain */,
        "{DeviceID: %Q,Temperature: %f,Humidity: %f,Pressure: %f,WaterLevel: %f,Location: %Q,AlertToggle: %d,HighTemp: %f,LowTemp: %f,HighHumidity: %f,LowHumidity: %f,HighWater: %f}",
        mgos_sys_config_get_device_id(),DeviceState.readings.temperature,DeviceState.readings.humidity,DeviceState.readings.pressure,DeviceState.readings.WaterLevel,
        DeviceState.location,DeviceState.alerts.AlertToggle,DeviceState.alerts.HighTemp,
        DeviceState.alerts.LowTemp,DeviceState.alerts.HighHumidity,DeviceState.alerts.LowHumidity,
        DeviceState.alerts.HighWater);


        //Send shadow update
    mgos_shadow_updatef(0,"{Temperature: %f,Humidity: %f,Pressure: %f,Location: %Q,AlertToggle: %d,HighTemp: %f,LowTemp: %f,HighHumidity: %f,LowHumidity: %f,HighWater: %f}",
        DeviceState.readings.temperature,DeviceState.readings.humidity,DeviceState.readings.pressure,
        DeviceState.location,DeviceState.alerts.AlertToggle,DeviceState.alerts.HighTemp,
        DeviceState.alerts.LowTemp,DeviceState.alerts.HighHumidity,DeviceState.alerts.LowHumidity,
        DeviceState.alerts.HighWater);

		//LOG(LL_INFO, ("\nshadow_updatef result=%d\n*******************\n", result));

		DeviceState.wait_for_shadow++;
		LOG(LL_INFO, ("\nSent shadow (%d)\n",DeviceState.wait_for_shadow));
		//start/restart ShadowAcceptTimeoutTimer
		if (ShadowAcceptTimeoutTimer != MGOS_INVALID_TIMER_ID) {
		    LOG(LL_INFO, ("Stopping Shadow Accept timeout timer to restart it"));
		    mgos_clear_timer(ShadowAcceptTimeoutTimer);
		    ShadowAcceptTimeoutTimer = MGOS_INVALID_TIMER_ID;
		}

		LOG(LL_INFO, ("Starting shadow accept timeout timer"));
		ShadowAcceptTimeoutTimer=mgos_set_timer(ShadowAcceptTimeout * 1000, 0, shadow_accept_timeout_cb, NULL);

	} //	if (button || (TimeToNextSend<0)) {
	if (!time_synced) {
		LOG(LL_INFO, ("Internet time is not synced"));		
	}
	LOG(LL_INFO, ("Time to next send: %lim %lis",(TimeToNextSend/60),TimeToNextSend-(TimeToNextSend/60)*60));

    LOG(LL_INFO, ("\nREADINGS | complete\n*******************\n"));
}

// Main process for sensor readings
void process(void *arg) {
    if (!time_synced) {
        LOG(LL_INFO, ("Waiting for Internet time (SNTP). Skipping sensor readings"));
        return;
    }
    //perform_readings(DeviceState.user_button);
    perform_readings(0);
}

void init_sensor_timers(void) {


    ReadingInterval = mgos_sys_config_get_ucilib_reading_interval();
    LOG(LL_INFO, ("Config: Reading interval: %d seconds", ReadingInterval));

    SendingInterval = mgos_sys_config_get_ucilib_sending_interval();
    LOG(LL_INFO, ("Config: Sending interval: %d seconds", SendingInterval));

    // start the all sensors read and send to cloud timer (default 10 minutes)
    if (ReadingTimer == MGOS_INVALID_TIMER_ID) {
        LOG(LL_INFO, ("Starting readings timer (every %d seconds)", ReadingInterval));
        ReadingTimer = mgos_set_timer(ReadingInterval * 1000, MGOS_TIMER_REPEAT, process, NULL);
    }

}

void init_sensors(void) {
    LOG(LL_INFO, ("INIT SENSORS"));

    // initialize sensors
    SensorFlags = 0;

    if (init_bme280()) {
        SensorFlags |= SENSOR_FLAGS_FOUND_BME280;
    }

    init_sensor_timers();

}
