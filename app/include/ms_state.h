
/**
 * Holds the current state of the device.
 * TODO: store state in device configuration and/or filesystem
 */
#ifndef _MS_STATE_H_
#define _MS_STATE_H_

#include "time.h"
#include <stdint.h> // uintXX_t
#include "mgos_system.h"
#include "mgos_time.h" //for mgos_time_changed_arg
#include "mgos_timers.h"


// Latest sensor readings
typedef struct {
    float temperature, humidity, pressure; // BME280
    float WaterLevel; 
} MS_STATE_READINGS;

// sensor alert info
typedef struct {
    int AlertToggle; //alert occurred alert threshold
    float HighTemp; //high temperature alert threshold
    float LowTemp;  //low temperature alert threshold
    float HighHumidity; //high humidity alert threshold
    float LowHumidity; //low humidity alert threshold
    float HighWater; //high water alert threshold
} MS_STATE_ALERTS;

// Current state of the sensor unit
typedef struct {
    MS_STATE_READINGS readings;
    MS_STATE_ALERTS alerts;
    char *location; //location of device
    int wait_for_shadow; //waiting for shadow_update from cloud
} MS_STATE;

// Holds the current local state of the device (to be compared against the AWS IOT device shadow)
extern MS_STATE DeviceState;

void init_state(void);

#endif