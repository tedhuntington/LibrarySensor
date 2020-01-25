#include <stdint.h> // uint16_t
#include <stdlib.h> // va_*
#include <time.h>
#include "common/cs_dbg.h"
#include "mgos_event.h"
#include "mgos_gpio.h"
#include "mgos_mqtt.h"
#include "mgos_rpc.h"
#include "mgos_shadow.h"
#include "mgos_system.h"
#include "mgos_time.h" //for mgos_time_changed_arg
#include "mgos_timers.h"
#include "ms_cloud.h"
#include "ms_sensors.h"
#include "ms_state.h"

uint32_t ShadowAcceptTimeout; //seconds to wait for a shadow accept before giving up
mgos_timer_id ShadowAcceptTimeoutTimer = MGOS_INVALID_TIMER_ID;

extern RTC_DATA_ATTR bool time_synced;
extern RTC_DATA_ATTR time_t power_on_timestamp;

// Called when device connects or reconnects to AWS
static void shadow_connected_cb(int ev, void *ev_data, void *arg) {


    LOG(LL_INFO, ("MGOS_SHADOW_CONNECTED"));

    // **************************************************************
    // * Perform post-cloud-connection processes here, e.g. connect *
    // * and reconnect events that communicate with the backend)    *
    // **************************************************************

    //perform_readings(0);
}


// Called when device receives the current AWS device shadow
static void shadow_get_accepted_cb(int ev, void *ev_data, void *arg) {
    LOG(LL_INFO, ("MGOS_SHADOW_GET_ACCEPTED"));

    // ********************************************************
    // * The latest device shadow is passed to this callback. *
    // * Call any device shadow handlers here                 *
    // ********************************************************
}

// Called when AWS device shadow is changed (external input, aka through user input)
static void shadow_delta_cb(int ev, void *ev_data, void *arg) {
	struct mg_str *delta = (struct mg_str *) ev_data;
	char *url = NULL; 
    int value;

	//char *environ = NULL; 
    LOG(LL_INFO, ("MGOS_SHADOW_UPDATE_DELTA"));

	 LOG(LL_INFO, ("GOT DELTA: [%.*s]", (int) delta->len, delta->p));
    // ******************************************************************
    // * Logic for if the Device Shadow has been updated in the backend *
    // ******************************************************************

      if (json_scanf(delta->p, delta->len, "{LED: %d}", &value)) {
          LOG(LL_INFO, ("Got LED delta"));          
          mgos_shadow_updatef(0,"{\"LED\": \"%d\"}",value); //update state to clear delta
          mgos_gpio_write(LED_GPIO,value);                // according to the delta
      }  //if (json_scanf(

}

// Called when AWS accepts an update to the device shadow from this device
static void shadow_update_accepted_cb(int ev, void *ev_data, void *arg) {
    LOG(LL_INFO, ("MGOS_SHADOW_UPDATE_ACCEPTED (%d)",DeviceState.wait_for_shadow));

    // ***********************************************************
    // * Logic for after confirmation that the Device Shadow     *
    // * has been successfully updated in the cloud              *
    // ***********************************************************

    DeviceState.wait_for_shadow--;

    // stop the wait for shadow timeout timer if no shadows remain to be recvd
    if (DeviceState.wait_for_shadow<=0) {
        LOG(LL_INFO, ("Stopping Shadow Accept timeout timer"));
        mgos_clear_timer(ShadowAcceptTimeoutTimer);
        ShadowAcceptTimeoutTimer = MGOS_INVALID_TIMER_ID;
		DeviceState.wait_for_shadow=0;
    }

}


static void time_changed_cb(int ev, void *evd, void *arg) {
    const struct mgos_time_changed_arg *ev_data = (const struct mgos_time_changed_arg *)(evd);
    double delta = ev_data->delta;
	if (!time_synced) { //only do this for the initial time or else power_on_time gets higher and wait for IAC baseline never reaches 12 hours)
	    LOG(LL_INFO, ("Time has changed by %d", (int)delta));
	    power_on_timestamp += (int)delta;
		time_synced = true;
	}
    // TODO: update DeviceState.sensors.sgp30_baseline_timestamp  and baseline timestamp ?
    // DeviceState.sensors.sgp30_baseline_timestamp+=(int)delta;

    (void)ev;
    (void)arg;
}


// Called when the wait for shadow accept timer has timed out
void shadow_accept_timeout_cb(void *arg) {
    LOG(LL_INFO, ("ShadowAcceptTimeoutTimer timed out waiting for shadow to be accepted"));

    DeviceState.wait_for_shadow=0;
    ShadowAcceptTimeoutTimer = MGOS_INVALID_TIMER_ID;

    (void)arg;
}


void init_cloud() {
    LOG(LL_INFO, ("INIT CLOUD"));
    mgos_event_add_handler(MGOS_SHADOW_CONNECTED, shadow_connected_cb, NULL);
    mgos_event_add_handler(MGOS_SHADOW_GET_ACCEPTED, shadow_get_accepted_cb, NULL);
    mgos_event_add_handler(MGOS_SHADOW_UPDATE_DELTA, shadow_delta_cb, NULL);
    mgos_event_add_handler(MGOS_SHADOW_UPDATE_ACCEPTED, shadow_update_accepted_cb, NULL);
    mgos_event_add_handler(MGOS_EVENT_TIME_CHANGED, time_changed_cb, NULL);


    ShadowAcceptTimeout = mgos_sys_config_get_ucilib_wait_for_shadow_accept();
    LOG(LL_INFO, ("Config: Wait for shadow accept timeout timer: %d seconds", ShadowAcceptTimeout));

}

