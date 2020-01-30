/**
 * Handles persistent local data storage access. 
 * see https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/storage/nvs_flash.html
 */

#include "ms_storage.h"

#include "common/cs_dbg.h"
#include "esp_err.h"
#include "nvs_flash.h"


bool save_u32(const char *name, uint32_t value) {	
	esp_err_t err;
	nvs_handle_t my_handle;

	//presumes was already nvs_flash_init()
    err = nvs_open(NVS_BASE_NAME, NVS_READWRITE,  &my_handle);
    if (err != ESP_OK) {
        LOG(LL_INFO, ("Error (%s) opening NVS handle!", esp_err_to_name(err)));
		return(false);
    } else {
        err = nvs_set_u32(my_handle, name, value);
		if (err != ESP_OK) {
	        LOG(LL_INFO, ("Failed to save %s with value %u",name,value));
		} else {
	        LOG(LL_INFO, ("Saved %s with value %u",name,value));
	        err = nvs_commit(my_handle);
			if (err != ESP_OK) {
			    LOG(LL_INFO, ("NVS commit failed"));
			} else {
			    //LOG(LL_INFO, ("NVS commit succeeded"));
        	} 
		}
	}
    nvs_close(my_handle);
	return(true);
}

bool load_u32(const char *name, uint32_t *value) {	
	esp_err_t err;
	nvs_handle_t my_handle;

    err = nvs_open(NVS_BASE_NAME, NVS_READWRITE,  &my_handle);
    if (err != ESP_OK) {
        LOG(LL_INFO, ("Error (%s) opening NVS handle!", esp_err_to_name(err)));
		return(false);
    } else {
		err = nvs_get_u32(my_handle, name, value);
        switch (err) {
            case ESP_OK:
			    nvs_close(my_handle);
				return(true);		
            break;
            case ESP_ERR_NVS_NOT_FOUND:
	 	 	    LOG(LL_WARN, ("In NVS the value %s has not been initialized yet!", name));				
            break;
            default:
            	LOG(LL_ERROR, ("Error reading value %s from NVS", name));
        }	}
    nvs_close(my_handle);
	return(false);
}

void init_storage(void) {
    LOG(LL_INFO, ("Initializing Data Storage: NVS"));

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}
