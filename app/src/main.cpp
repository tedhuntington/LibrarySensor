#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mgos.h"
#include "mgos_mqtt.h"
#ifdef MGOS_HAVE_WIFI
#include "mgos_wifi.h"
#endif
#include "mgos_debug.h"
#include "mgos_i2c.h"
#include "mgos_rpc.h"
#include "mgos_shadow.h"
#include "mgos_bme280.h"  //bme280 sensor
#include "mgos_gpio.h"
#include "mgos_adc.h"


#define LED_GPIO 12  //GPIO number for LED
#define AP_GPIO 2 //GPIO number for reading in user toggle AP mode flag
#define BME208_ADDRESS 0x76//0x77
#define ADC_GPIO 32 //GPIO number for reading in water sensor voltage (ADC currently uses pin32 ADC 1, Channel 4)
//struct mgos_i2c* i2c = NULL;
struct mgos_bme280 *bme; //bme280 structure
const char *state_json;  //shadow state data
uint32_t SensorFlags;
#define SENSOR_FLAGS_FOUND_BME280  0x00000001
uint32_t EnableAP;
const char topicReadings[]= "readings";
const char topicAlerts[]= "alerts";

static void mqtt_timer_cb(void *arg) {
  //char topic[100];//,json[256];
  float temperature,humidity, pressure;

 
  
  //snprintf(topic, sizeof(topic), "/devices/%s/events",mgos_sys_config_get_device_id());
  //snprintf(topic, sizeof(topic), "readings");

 // char buf[8];
//  int x = mgos_gpio_toggle(mgos_sys_config_get_board_led3_pin());
  mgos_gpio_toggle(LED_GPIO);
  //LOG(LL_INFO, ("Pin: %s, published: %s x %d", mgos_gpio_str(pin, buf),
  //              res ? "yes" : "no", x));


  
//  bool val = mgos_gpio_toggle(mgos_sys_config_get_board_led1_pin());
//  LOG(LL_INFO, ("%s uptime: %.2lf, RAM: %lu, %lu free", val ? "Tick" : "Tock",
//                mgos_uptime(), (unsigned long) mgos_get_heap_size(),
//                (unsigned long) mgos_get_free_heap_size()));
//  (void) arg;

//  LOG(LL_INFO, ("Temperature: %f C",mgos_bme280_read_temperature(bme)));
//  LOG(LL_INFO, ("Humidity: %f %%RH",mgos_bme280_read_humidity(bme)));
//  LOG(LL_INFO, ("Pressure: %f Pa",mgos_bme280_read_pressure(bme)));

//  LOG(LL_INFO,("{Temperature: %f}", bme->getTemperature()));
//  LOG(LL_INFO,("{Hum: %f}", bme->getHumidity()));

  //get sensor data
  
  //BME280 sensor
  if (SensorFlags&SENSOR_FLAGS_FOUND_BME280) {
    temperature=mgos_bme280_read_temperature(bme);
    humidity=mgos_bme280_read_humidity(bme);
    pressure=mgos_bme280_read_pressure(bme);
  } else {
    temperature=0;
    humidity=0;
    pressure=0;
  }
    
  LOG(LL_INFO, ("Temperature: %f C",temperature));
  LOG(LL_INFO, ("Humidity: %f %%RH",humidity));
  LOG(LL_INFO, ("Pressure: %f Pa",pressure));

//  bool res = mgos_mqtt_pubf(topic, 0, false /* retain */,
//                            "{Temperature: %f, Humidity: %f, Pressure: %f}",
//                            temperature,humidity,pressure);

//  bool res = mgos_mqtt_pubf(topic, 0, false /* retain */,
//                            "{total_ram: %lu, free_ram: %lu}",
//                            (unsigned long) mgos_get_heap_size(),
//                            (unsigned long) mgos_get_free_heap_size());

  int WaterLevelReading=0;  //digital value  (12-bit 0-4095)
  float WaterLevel=0.0;  //fractional value
  WaterLevelReading=mgos_adc_read_voltage(ADC_GPIO);
  WaterLevel= (float)WaterLevelReading/4095.0;  // divide by 1240.9 for voltage relative to 3.3v
  LOG(LL_INFO, ("WaterLevel: %f",WaterLevel));

  //snprintf(json, sizeof (json),"{Temperature: %f, Humidity: %f, Pressure: %f,Luminosity: %d,IR: %d,Full %d,Visible %d,Lux %d,TVOC: %d,CO2: %d}",
  //                          temperature,humidity,pressure,x,ir,full,full-ir,lux,tvoc,co2);

  //bool res = mgos_mqtt_pubf(topic,MG_MQTT_QOS(1),false,json);
//  bool res = mgos_mqtt_pubf(topicReadings, 0, false /* retain */,
//            "{DeviceID: \"%s\",Temperature: %f,Humidity: %f,Pressure: %f,WaterLevel: %f}",
//            mgos_sys_config_get_device_id(),temperature,humidity,pressure,WaterLevel);
     bool res = mgos_mqtt_pubf(topicReadings, 0, false /* retain */,
    "{DeviceID: \"%s\",Temperature: %f,Humidity: %f,Pressure: %f,WaterLevel: %f,Location: %s,AlertToggle: %d,HighTemp: %f,LowTemp: %f,HighHumidity: %f,LowHumidity: %f,HighWater: %f}",
    mgos_sys_config_get_device_id(),temperature,humidity,pressure,WaterLevel,
    mgos_sys_config_get_Location(),mgos_sys_config_get_AlertToggle(),mgos_sys_config_get_HighTemp(),
      mgos_sys_config_get_LowTemp(),mgos_sys_config_get_HighHumidity(),mgos_sys_config_get_LowHumidity(),
      mgos_sys_config_get_HighWater()
    );


} //static void led_timer_cb(void *arg) {

static void net_cb(int ev, void *evd, void *arg) {
  switch (ev) {
    case MGOS_NET_EV_DISCONNECTED:
      LOG(LL_INFO, ("%s", "Net disconnected"));
      break;
    case MGOS_NET_EV_CONNECTING:
      LOG(LL_INFO, ("%s", "Net connecting..."));
      break;
    case MGOS_NET_EV_CONNECTED:
      LOG(LL_INFO, ("%s", "Net connected"));
      break;
    case MGOS_NET_EV_IP_ACQUIRED:
      LOG(LL_INFO, ("%s", "Net got IP address"));
      break;
  }

  (void) evd;
  (void) arg;
}  //static void net_cb(int ev, void *evd, void *arg) {

#ifdef MGOS_HAVE_WIFI
static void wifi_cb(int ev, void *evd, void *arg) {
  switch (ev) {
    case MGOS_WIFI_EV_STA_DISCONNECTED:
      LOG(LL_INFO, ("WiFi STA disconnected %p", arg));
      break;
    case MGOS_WIFI_EV_STA_CONNECTING:
      LOG(LL_INFO, ("WiFi STA connecting %p", arg));
      break;
    case MGOS_WIFI_EV_STA_CONNECTED:
      LOG(LL_INFO, ("WiFi STA connected %p", arg));

      break;
    case MGOS_WIFI_EV_STA_IP_ACQUIRED:
      LOG(LL_INFO, ("WiFi STA IP acquired %p", arg));
      //if got IP then disable AP mode unless user has enabled
      if (!mgos_sys_config_get_apmode()) {  //user did not set apmode on
        if (mgos_sys_config_get_wifi_ap_enable()) { //ap mode is enabled 
          //disable access point mode
          LOG(LL_INFO, ("Disabling AP Mode"));
          struct mgos_config_wifi_ap ap_cfg;
          memcpy(&ap_cfg, mgos_sys_config_get_wifi_ap(), sizeof(ap_cfg));   //read in existing wifi config
          ap_cfg.enable = false;
          mgos_sys_config_set_wifi_ap_enable(false);
          save_cfg(&mgos_sys_config, NULL);
          if (mgos_wifi_setup_ap(&ap_cfg)) {
            LOG(LL_ERROR, ("AP off"));
          } else {
            LOG(LL_ERROR, ("Disable AP failed"));
          }
  	    //mgos_system_restart(0);
        } //if (mgos_sys_config_get_wifi_ap_enable()) { //ap mode is enabled 
       } //if (!mgos_sys_config_get_apmode()) {  //user did not set apmode on

      break;
    case MGOS_WIFI_EV_AP_STA_CONNECTED: {
      struct mgos_wifi_ap_sta_connected_arg *aa =
          (struct mgos_wifi_ap_sta_connected_arg *) evd;
      LOG(LL_INFO, ("WiFi AP STA connected MAC %02x:%02x:%02x:%02x:%02x:%02x",
                    aa->mac[0], aa->mac[1], aa->mac[2], aa->mac[3], aa->mac[4],
                    aa->mac[5]));
      break;
    }
    case MGOS_WIFI_EV_AP_STA_DISCONNECTED: {
      struct mgos_wifi_ap_sta_disconnected_arg *aa =
          (struct mgos_wifi_ap_sta_disconnected_arg *) evd;
      LOG(LL_INFO,
          ("WiFi AP STA disconnected MAC %02x:%02x:%02x:%02x:%02x:%02x",
           aa->mac[0], aa->mac[1], aa->mac[2], aa->mac[3], aa->mac[4],
           aa->mac[5]));

      //re-enable AP mode?     
      break;
    }
  }
  (void) arg;
}
#endif /* MGOS_HAVE_WIFI */

static void button_cb(int pin, void *arg) {
  //char topic[100];
  float temperature,humidity, pressure;

    LOG(LL_INFO, ("button_cb"));

  //Toggle LED
  char buf[8];
//  int x = mgos_gpio_toggle(mgos_sys_config_get_board_led3_pin());
  mgos_gpio_toggle(LED_GPIO);
  //LOG(LL_INFO, ("Pin: %s, published: %s x %d", mgos_gpio_str(pin, buf),
  //              res ? "yes" : "no", x));




  //if GPIO2 is floating (or 1), this button sends mqtt to readings topic
  //if GPIO2 is 0, this button toggles AP mode enable/disable
  if (mgos_gpio_read(AP_GPIO)) {

  //#if 0   
  //  snprintf(topic, sizeof(topic), "/devices/%s/events",
  //           mgos_sys_config_get_device_id());

    //get sensor data
    if (SensorFlags&SENSOR_FLAGS_FOUND_BME280) {
      temperature=mgos_bme280_read_temperature(bme);
      humidity=mgos_bme280_read_humidity(bme);
      pressure=mgos_bme280_read_pressure(bme);
    } else {
      temperature=0;
      humidity=0;
      pressure=0;
    }
      
    LOG(LL_INFO, ("Temperature: %f C",temperature));
    LOG(LL_INFO, ("Humidity: %f %%RH",humidity));
    LOG(LL_INFO, ("Pressure: %f Pa",pressure));

  //  bool res = mgos_mqtt_pubf(topic, 0, false /* retain */,
  //                            "{Temperature: %f, Humidity: %f, Pressure: %f}",
  //                            temperature,humidity,pressure);

  //  bool res = mgos_mqtt_pubf(topic, 0, false /* retain */,
  //                            "{total_ram: %lu, free_ram: %lu}",
  //                            (unsigned long) mgos_get_heap_size(),
  //                            (unsigned long) mgos_get_free_heap_size());


    int WaterLevelReading=0;  //digital value  (12-bit 0-4095)
    float WaterLevel=0.0;  //fractional value
    WaterLevelReading=mgos_adc_read_voltage(ADC_GPIO);
    WaterLevel= (float)WaterLevelReading/4095.0;  // divide by 1240.9 for voltage relative to 3.3v
    LOG(LL_INFO, ("WaterLevel: %f",WaterLevel));


    LOG(LL_INFO, ("Location: %s",mgos_sys_config_get_Location()));
    LOG(LL_INFO, ("AlertToggle: %d",mgos_sys_config_get_AlertToggle()));

    //publish to "readings" topic  
    //bool res = mgos_mqtt_pubf("readings", 0, false /* retain */,
    //                          "{DeviceID: \"%s\",Temperature: %f, Humidity: %f, Pressure: %f}",
    //                        mgos_sys_config_get_device_id(),temperature,humidity,pressure);
    //bool res = mgos_mqtt_pubf(topicReadings, 0, false /* retain */,
    //        "{DeviceID: \"%s\",Temperature: %f,Humidity: %f,Pressure: %f,WaterLevel: %f}",
    //        mgos_sys_config_get_device_id(),temperature,humidity,pressure,WaterLevel);
    bool res = mgos_mqtt_pubf(topicReadings, 0, false /* retain */,
    "{DeviceID: \"%s\",Temperature: %f,Humidity: %f,Pressure: %f,WaterLevel: %f,Location: %s,AlertToggle: %d,HighTemp: %f,LowTemp: %f,HighHumidity: %f,LowHumidity: %f,HighWater: %f}",
    mgos_sys_config_get_device_id(),temperature,humidity,pressure,WaterLevel,
    mgos_sys_config_get_Location(),mgos_sys_config_get_AlertToggle(),mgos_sys_config_get_HighTemp(),
      mgos_sys_config_get_LowTemp(),mgos_sys_config_get_HighHumidity(),mgos_sys_config_get_LowHumidity(),
      mgos_sys_config_get_HighWater()
    );



  //#endif 
    
    
  #if 0 
    bool res = mgos_mqtt_pubf(topic, 0, false /* retain */,
                              "{total_ram: %lu, free_ram: %lu, Temperature: %f, Humidity: %f, Pressure: %f}",
                              (unsigned long) mgos_get_heap_size(),
                              (unsigned long) mgos_get_free_heap_size(),
                              temperature,humidity,pressure);
  #endif

  //  bool res = mgos_mqtt_pubf(topic, 0, false /* retain */,
  //                            "{total_ram: %lu, free_ram: %lu}",
  //                            (unsigned long) mgos_get_heap_size(),
  //                            (unsigned long) mgos_get_free_heap_size());


  } else { 

    //if AP_GPIO (GPIO2) is 0, this button toggles AP mode enable/disable

    LOG(LL_INFO, ("toggle apmode"));
    //printf("apmode, %d\n", mgos_sys_config_get_apmode());
    if (mgos_sys_config_get_apmode()) {
        mgos_sys_config_set_apmode(0);
    } else {
      mgos_sys_config_set_apmode(1);
    }
    LOG(LL_INFO, ("apmode=%d\n", mgos_sys_config_get_apmode()));

    //save the apmode setting
    save_cfg(&mgos_sys_config, NULL); //writes conf9.json


    struct mgos_config_wifi_ap ap_cfg;
    if (mgos_sys_config_get_apmode()) {
      memcpy(&ap_cfg, mgos_sys_config_get_wifi_ap(), sizeof(ap_cfg));
      ap_cfg.enable = true;
      LOG(LL_INFO, ("AP SSID: %s",ap_cfg.ssid));
      //mgos_sys_config_set_wifi_ap_enable(true);
      save_cfg(&mgos_sys_config, NULL);
      if (mgos_wifi_setup_ap(&ap_cfg)) {
        LOG(LL_INFO, ("AP on"));
      } else {
        LOG(LL_ERROR, ("Enable AP failed"));
      }
    } //if (mgos_sys_config_get_apmode()) {
  } //if mos_gpio_read(AP_GPIO)

  (void) arg;
} //static void button_cb(int pin, void *arg) {


static void shadow_cb(int ev, void *evd, void *arg) {

  struct mg_str *event_data;
  float temperature,humidity,pressure;
  char json[512];
  struct json_token t;
  int i,value;

  switch(ev) {
    case MGOS_SHADOW_CONNECTED:
      LOG(LL_INFO, ("Got MGOS_SHADOW_CONNECTED"));

//  LOG(LL_INFO, ("Temperature: %f C",mgos_bme280_read_temperature(bme)));
//  LOG(LL_INFO, ("Humidity: %f %%RH",mgos_bme280_read_humidity(bme)));
//  LOG(LL_INFO, ("Pressure: %f Pa",mgos_bme280_read_pressure(bme)));

      //mgos_shadow_get();
      if (SensorFlags&SENSOR_FLAGS_FOUND_BME280) {
        temperature=mgos_bme280_read_temperature(bme);
        humidity=mgos_bme280_read_humidity(bme);
        pressure=mgos_bme280_read_pressure(bme);
      } else {
        temperature=0;
        humidity=0;
        pressure=0;
      }
//      sprintf(json,"{ \"LED\": \"0\",\"Temperature\": \"%f\",\"Humidity\": \"%f\",\"Pressure\": \"%f\"}",temperature,humidity,pressure);
      sprintf(json,"{ \"LED\": \"0\",\"Temperature\": \"%f\",\"Humidity\": \"%f\",\"Pressure\": \"%f\",\"Location\": \"%s\",\"AlertToggle\": \"%d\",\"HighTemp\": \"%f\",\"LowTemp\": \"%f\",\"HighHumidity\": \"%f\",\"LowHumidity\": \"%f\",\"HighWater\": \"%f\"}",
      temperature,humidity,pressure,
      mgos_sys_config_get_Location(),mgos_sys_config_get_AlertToggle(),mgos_sys_config_get_HighTemp(),
      mgos_sys_config_get_LowTemp(),mgos_sys_config_get_HighHumidity(),mgos_sys_config_get_LowHumidity(),
      mgos_sys_config_get_HighWater());
    

      mgos_shadow_update(0,json);

    break;
    case MGOS_SHADOW_UPDATE_DELTA:
      LOG(LL_INFO, ("Got MGOS_SHADOW_UPDATE_DELTA"));
      event_data = (struct mg_str *)evd;  //get key
      memcpy(json,event_data->p,event_data->len);
      json[event_data->len]=0; //terminate string
      LOG(LL_INFO, ("data: '%s'",json));
      printf("Parsing array: %.*s\n", event_data->len, event_data->p);
      if (json_scanf(event_data->p, event_data->len, "{LED: %d}", &value)) {
          LOG(LL_INFO, ("Got LED delta"));          
          sprintf(json,"{\"LED\": \"%d\"}",value);  
          printf("Sending: '%s'",json);      
          mgos_shadow_update(0,json); //update state to clear delta
          mgos_gpio_write(LED_GPIO,value);                // according to the delta
      }  //if (json_scanf(
/*
      for (i = 0; json_scanf_array_elem(event_data->p, event_data->len, "", i, &t) > 0; i++) {
        printf("Index %d, token [%.*s]\n", i, t.len, t.ptr);
        if (json_scanf(t.ptr, t.len, "{LED: %d}", &value)) {
          LOG(LL_INFO, ("Got LED delta"));          
          sprintf(json,"{LED:\"%d\"}",value);        
          mgos_shadow_update(0,json); //update state to clear delta
        }  //if (json_scanf(
      } //for i
      */

//      sprintf(json,"{\"%d\"}",event_data->p);

    break;
    default:
      //LOG(LL_INFO, ("Got Shadow Message %d",ev));
      LOG(LL_INFO, ("Got Shadow Message %s",mgos_shadow_event_name(ev))); 
    break;
  };  //switch
//  mgos_shadow_update(0, const char *state_json);

} //static void shadow_cb(int ev, void *evd, void *arg) {

enum mgos_app_init_result mgos_app_init(void) {
 
  int res;
  float temperature,humidity,pressure;
  char buf[8];

  //output MAC address for STA
  uint8_t mac[20];
  device_get_mac_address(mac);
  LOG(LL_INFO, ("MAC: %02x%02x%02x%02x%02x%02x\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5] ));

/*
  // Blink built-in LED every second 
  int led_pin = mgos_sys_config_get_board_led1_pin();
  if (led_pin >= 0) {
    LOG(LL_INFO, ("LED pin %s", mgos_gpio_str(led_pin, buf)));
    mgos_gpio_set_mode(led_pin, MGOS_GPIO_MODE_OUTPUT);
    mgos_set_timer(1000, MGOS_TIMER_REPEAT, led_timer_cb, NULL);
  }
  
  mgos_gpio_set_mode(mgos_sys_config_get_board_led3_pin(),
                     MGOS_GPIO_MODE_OUTPUT);
  mgos_gpio_write(mgos_sys_config_get_board_led3_pin(), 0);
*/
    //call timer cb every 10minutes
    mgos_set_timer(600000, MGOS_TIMER_REPEAT, mqtt_timer_cb, NULL);  //10min
    //mgos_set_timer(60000, MGOS_TIMER_REPEAT, mqtt_timer_cb, NULL); //1 min

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
    LOG(LL_INFO, ("Button pin %s, active %s", mgos_gpio_str(btn_pin, buf),
                  (mgos_sys_config_get_board_btn1_pull_up() ? "low" : "high")));
    mgos_gpio_set_button_handler(btn_pin, btn_pull, btn_int_edge, 20, button_cb,
                                 NULL);
  }

  /* Network connectivity events */
  mgos_event_add_group_handler(MGOS_EVENT_GRP_NET, net_cb, NULL);

#ifdef MGOS_HAVE_WIFI
  mgos_event_add_group_handler(MGOS_WIFI_EV_BASE, wifi_cb, NULL);
#endif

  mgos_event_add_group_handler(MGOS_SHADOW_BASE, shadow_cb, NULL);


  SensorFlags=0; //clear sensor found flags
  
  LOG(LL_INFO, ("create bme"));
  
  //check bme device id
  bme=mgos_bme280_i2c_create(BME208_ADDRESS);
  temperature=0;
  humidity=0;
  if (bme) {
      LOG(LL_INFO, ("bme created"));
      SensorFlags|=SENSOR_FLAGS_FOUND_BME280;
      temperature=mgos_bme280_read_temperature(bme);
      humidity=mgos_bme280_read_humidity(bme);
      pressure=mgos_bme280_read_pressure(bme);

      LOG(LL_INFO, ("Temperature: %f C",temperature));
      LOG(LL_INFO, ("Humidity: %f %%RH",humidity));
      LOG(LL_INFO, ("Pressure: %f Pa",pressure));

  } else {
      LOG(LL_INFO, ("bme create failed"));
  }



  //initialize LED pin
  mgos_gpio_set_mode(LED_GPIO,MGOS_GPIO_MODE_OUTPUT); //set LED pin to output

  //initialize AP_GPIO pin
  mgos_gpio_set_mode(AP_GPIO,MGOS_GPIO_MODE_INPUT); //set AP pin to input
  mgos_gpio_set_pull(AP_GPIO,MGOS_GPIO_PULL_UP); //pull up

  //initialize ADC pin
  //ADC currently uses pin32 ADC 1, Channel 4
  mgos_adc_enable(ADC_GPIO);

  int WaterLevelReading=0;  //digital value  (12-bit 0-4095)
  float WaterLevel=0.0;  //fractional value
  WaterLevelReading=mgos_adc_read_voltage(ADC_GPIO);
  WaterLevel= (float)WaterLevelReading/4095.0;  // divide by 1240.9 for voltage relative to 3.3v
  LOG(LL_INFO, ("WaterLevel: %f",WaterLevel));


	EnableAP=0; //used to know if user wants AP mode enabled

  return MGOS_APP_INIT_SUCCESS;
} //enum mgos_app_init_result mgos_app_init(void) {

