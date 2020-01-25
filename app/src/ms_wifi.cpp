/**
 * Handles WiFi-related connection and callback logic
 */
#include "mgos_wifi.h"
#include "common/cs_dbg.h"
#include "ms_wifi.h"

/**
 * Enable or disable AP mode
 */
void set_ap_mode(bool state) {
    struct mgos_config_wifi_ap ap_cfg;
    memcpy(&ap_cfg, mgos_sys_config_get_wifi_ap(), sizeof(ap_cfg));
    ap_cfg.enable = state;
	mgos_sys_config_set_wifi_ap_enable(state);
    if (!mgos_wifi_setup_ap(&ap_cfg)) {
        LOG(LL_ERROR, ("Wifi AP setup failed"));
    }
}

static void wifi_cb(int ev, void *evd, void *arg) {
	char *msg = NULL;

    switch (ev) {
    case MGOS_WIFI_EV_STA_DISCONNECTED:
        LOG(LL_INFO, ("WiFi STA | Disconnected"));
//        set_status(STATUS_WIFI_LISTENING);
        break;
    case MGOS_WIFI_EV_STA_CONNECTING:
        LOG(LL_INFO, ("WiFi STA | Connecting"));
//        set_status(STATUS_WIFI_CONNECTING);
        break;
    case MGOS_WIFI_EV_STA_CONNECTED:
        LOG(LL_INFO, ("WiFi STA | Connected"));
        break;
    case MGOS_WIFI_EV_STA_IP_ACQUIRED:
        LOG(LL_INFO, ("WiFi STA | IP acquired"));

        LOG(LL_INFO, ("WiFi STA | Disabling AP"));
        set_ap_mode(false);

		if (!mgos_sys_config_save(&mgos_sys_config, 0, &msg)) {  //save STA SSID, pass, user to configuration
			LOG(LL_ERROR, ("Disable AP Mode, Save Config Error: %s", msg));
		} else {
			LOG(LL_INFO, ("Saved WiFi Config"));
		}
	    if (msg!=0) {
			free(msg);
		}
		
//        set_status(STATUS_CLOUD_CONNECTING);
        break;
    case MGOS_WIFI_EV_AP_STA_CONNECTED: {
        struct mgos_wifi_ap_sta_connected_arg *aa = (struct mgos_wifi_ap_sta_connected_arg *)evd;
        LOG(LL_INFO, ("WiFi AP | Client connected. MAC: %02x:%02x:%02x:%02x:%02x:%02x",
                      aa->mac[0], aa->mac[1], aa->mac[2], aa->mac[3], aa->mac[4], aa->mac[5]));

        // Stop the reconnect timer
        LOG(LL_INFO, ("WiFi AP | Disabling reconnect timer"));
        mgos_wifi_disconnect();

//        set_status(STATUS_WIFI_HOSTING);
        break;
    }
    case MGOS_WIFI_EV_AP_STA_DISCONNECTED: {
        struct mgos_wifi_ap_sta_disconnected_arg *aa = (struct mgos_wifi_ap_sta_disconnected_arg *)evd;
        LOG(LL_INFO, ("WiFi AP | Client disconnected. MAC %02x:%02x:%02x:%02x:%02x:%02x",
                      aa->mac[0], aa->mac[1], aa->mac[2], aa->mac[3], aa->mac[4], aa->mac[5]));
//        set_status(STATUS_WIFI_LISTENING);
				
        // Restart the reconnect timer
        LOG(LL_INFO, ("WiFi AP | Enabling reconnect to last known wifi"));
        mgos_wifi_connect();

        break;
    }
    }
}

void init_wifi() {
    LOG(LL_INFO, ("INIT WIFI"));

    //output MAC address for STA
    uint8_t mac[20];
    device_get_mac_address(mac);
    LOG(LL_INFO, ("MAC: %02x%02x%02x%02x%02x%02x\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5] ));

    mgos_event_add_group_handler(MGOS_WIFI_EV_BASE, wifi_cb, NULL);
}
