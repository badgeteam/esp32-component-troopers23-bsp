#include <stdio.h>
#include <string.h>
#include <sdkconfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_system.h>
#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <nvs.h>
#include "wifi_connection.h"
#include "wifi_connect.h"

static const char *TAG = "wifi_connect";

bool wifi_connect_to_stored() {
    bool result = false;
    // Open NVS.
    nvs_handle_t handle;
    nvs_open("system", NVS_READWRITE, &handle);
    wifi_auth_mode_t authmode = 0;
    esp_eap_ttls_phase2_types phase2 = 0;
    char *ssid = NULL;
    char *ident = NULL;
    char *anon_ident = NULL;
    char *password = NULL;
    size_t len;
    
    // Read NVS.
    esp_err_t res;
    res = nvs_get_str(handle, "wifi.ssid", NULL, &len);
    if (res) goto errcheck;
    ssid = malloc(len);
    res = nvs_get_str(handle, "wifi.ssid", ssid, &len);
    if (res) goto errcheck;
    
    // Check whether connection is enterprise.
    res = nvs_get_u8(handle, "wifi.authmode", &authmode);
    bool use_ent = authmode == WIFI_AUTH_WPA2_ENTERPRISE;
    if (res) goto errcheck;
    
    if (use_ent) {
        // Read enterprise-specific parameters.
        
        // Read phase2 mode.
        res = nvs_get_u8(handle, "wifi.phase2", &phase2);
        if (res) goto errcheck;
        
        // Read identity.
        res = nvs_get_str(handle, "wifi.username", NULL, &len);
        if (res) goto errcheck;
        ident = malloc(len);
        res = nvs_get_str(handle, "wifi.username", ident, &len);
        
        // Read anonymous identity.
        res = nvs_get_str(handle, "wifi.anon_ident", NULL, &len);
        if (res == ESP_ERR_NVS_NOT_FOUND) {
            // Default is use the same thing.
            anon_ident = strdup(ident);
        } else {
            if (res) goto errcheck;
            anon_ident = malloc(len);
            res = nvs_get_str(handle, "wifi.anon_ident", anon_ident, &len);
            if (res) goto errcheck;
        }
    }
    res = nvs_get_str(handle, "wifi.password", NULL, &len);
    if (res) goto errcheck;
    password = malloc(len);
    res = nvs_get_str(handle, "wifi.password", password, &len);
    if (res) goto errcheck;

    nvs_close(handle);
    
    // Open the appropriate connection.
    if (use_ent) {
        result = wifi_connect_ent(ssid, ident, anon_ident, password, phase2, 2);
    } else {
        result = wifi_connect(ssid, password, authmode, 2);
    }
    
    errcheck:
    if (res == ESP_ERR_NVS_NOT_FOUND || res == ESP_ERR_NVS_NOT_INITIALIZED) {
        ESP_LOGE(TAG, "Failed to read WiFi configuration from NVS");
    } else if (res) {
        // Other errors.
        ESP_LOGE(TAG, "Error connecting to WiFi: %s", esp_err_to_name(res));
    }
    
    // Free memory.
    if (ssid) free(ssid);
    if (ident) free(ident);
    if (anon_ident) free(anon_ident);
    if (password) free(password);
    
    return result;
}

void wifi_disconnect_and_disable() {
    esp_wifi_disconnect();
    esp_wifi_stop();
}
