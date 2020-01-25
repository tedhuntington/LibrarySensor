#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "mgos.h"
#include "mgos_debug.h"
#include "mgos_rpc.h"
#include "mgos_shadow.h"
#include "mgos_gpio.h"
#include "mgos_adc.h"
#include "common/cs_dbg.h"

#include "ms_sensors.h"
#include "ms_state.h"
#include "ms_wifi.h"
#include "ms_cloud.h"

RTC_DATA_ATTR bool time_synced = false; // Whether we have synced via SNTP yet
RTC_DATA_ATTR time_t power_on_timestamp = 0;


static void button_cb(int pin, void *arg) {

    LOG(LL_INFO, ("button_cb"));
//  mgos_gpio_toggle(LED_GPIO);

  //if GPIO2=1 perform reading, if GPIO2=0 toggle AP mode enable/disable
  if (mgos_gpio_read(AP_GPIO)) {
    perform_readings(1);
  } else { 
    //if AP_GPIO (GPIO2) is 0, this button toggles AP mode enable/disable
    LOG(LL_INFO, ("toggle apmode"));
    //printf("apmode, %d\n", mgos_sys_config_get_apmode());
    if (mgos_sys_config_get_apmode()) {
        set_ap_mode(false);
    } else {
      set_ap_mode(true);
    }
  } //if mos_gpio_read(AP_GPIO)

  (void) arg;
} //static void button_cb(int pin, void *arg) {


void init_buttons(void) {
  
  //button callback
/* Publish to MQTT on button press */
  int btn_pin = mgos_sys_config_get_board_btn1_pin();
  if (btn_pin >= 0) {
    enum mgos_gpio_pull_type btn_pull;
    enum mgos_gpio_int_mode btn_int_edge;
    if (mgos_sys_config_get_board_btn1_pull_up()) {
      btn_pull = MGOS_GPIO_PULL_UP;
      btn_int_edge = MGOS_GPIO_INT_EDGE_NEG;
    } else {
      btn_pull = MGOS_GPIO_PULL_DOWN;
      btn_int_edge = MGOS_GPIO_INT_EDGE_POS;
    }
    mgos_gpio_set_button_handler(btn_pin, btn_pull, btn_int_edge, 20, button_cb,
                                 NULL);
  }

}


void init_gpio(void) {
  
  //initialize LED pin
  mgos_gpio_set_mode(LED_GPIO,MGOS_GPIO_MODE_OUTPUT); //set LED pin to output

  //initialize AP_GPIO pin
  mgos_gpio_set_mode(AP_GPIO,MGOS_GPIO_MODE_INPUT); //set AP pin to input
  mgos_gpio_set_pull(AP_GPIO,MGOS_GPIO_PULL_UP); //pull up

  //initialize ADC pin
  //ADC currently uses pin32 ADC 1, Channel 4
  mgos_adc_enable(ADC_GPIO);

}

enum mgos_app_init_result mgos_app_init(void) {

  init_gpio();
  init_state();
  init_sensors();
  init_buttons();
  init_wifi();
  init_cloud();

  return MGOS_APP_INIT_SUCCESS;
} //enum mgos_app_init_result mgos_app_init(void) {

