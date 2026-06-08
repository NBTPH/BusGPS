#include <wifi.h>

#define CONNECTION_MAX_RETRY 10
#define CONNECTION_TIMEOUT_SEC 10
#define SOCKET_TIMEOUT_SEC 5

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

const char *AP_SSID = "CONFIG_AP_SSID";
const char *AP_PASSWORD = "CONFIG_AP_PASSWORD";
const char *STA_SSID = "CONFIG_STA_SSID";
const char *STA_PASSWORD = "CONFIG_STA_PASSWORD";

EventGroupHandle_t network_event_group;
bool WiFi_Connected = false;

int retry_num = 0;
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){ //wtf is all this shit?
    EventGroupHandle_t wifi_event_group = *(EventGroupHandle_t *)arg;
    
    //Station mode events
    //if event is WiFi STATION start we attempt to connect to wifi
    if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START){
        esp_wifi_connect();
    } 
    else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED){ //if can't connected or disconnected
        WiFi_Connected = false;
        if(retry_num < CONNECTION_MAX_RETRY){ //if number of retries hasn't reached max, then try to connect to WiFi again
            esp_wifi_connect();
            retry_num++;
            printf("[WiFi_status] Retry to connect to the AP\r\n");
        } 
        else{ //if it has, set WIFI_FAIL_BIT
            xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
        }
        printf("[WiFi_status] Connect to the AP fail\r\n");
    }
    else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP){ //if connected to WiFi, set WIFI_CONNECTED_BIT and reset retry
        WiFi_Connected = true;
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        printf("[WiFi_status] Connection success, got ip: %d.%d.%d.%d\r\n", IP2STR(&event->ip_info.ip));
        retry_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }

    //Access point mode events
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        printf("[WiFi_status] Client %02x:%02x:%02x:%02x:%02x:%02x connected to our AP, AID=%d\n", MAC2STR(event->mac), event->aid);
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        printf("[WiFi_status] Client %02x:%02x:%02x:%02x:%02x:%02x disconnected from our AP, AID=%d\n", MAC2STR(event->mac), event->aid);
    }
}

bool WiFi_init(bool ap_mode){
    debug_printf("[WiFi_status] Starting initializing WiFi");

    //Event group for wifi connection
    network_event_group = xEventGroupCreate();

    // //Initialize NVS peripheral because WiFi driver uses NVS
    // esp_err_t ret = nvs_flash_init();
    // if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){ //if unsuccesfull erase NVS and try again
    //     ESP_ERROR_CHECK(nvs_flash_erase());
    // }
    // ESP_ERROR_CHECK(nvs_flash_init());

    //Initialize TCP/IP network interface
    ESP_ERROR_CHECK(esp_netif_init());

    //Create event loop for the network event group, wifi needs events to function
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_event_handler_instance_t instance_any_id;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                    ESP_EVENT_ANY_ID,
                                                    &wifi_event_handler,
                                                    &network_event_group,
                                                    &instance_any_id)); //catch all WIFI events

    //Inititalize the TCP/IP WiFi network interface for Station mode 
    if(ap_mode){
        esp_netif_create_default_wifi_ap();
    }
    else{
        esp_netif_create_default_wifi_sta();
    }

    //Initialize WiFi peripheral, with disable NVS because we arent using that
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    cfg.nvs_enable = false;
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM)); //only using RAM for WiFi handling

    //Configure WiFi
    wifi_config_t wifi_config = {0};
    if(ap_mode){
        memcpy(&wifi_config.ap.ssid, AP_SSID, strlen(AP_SSID));
        wifi_config.ap.ssid_len = strlen(AP_SSID);
        memcpy(&wifi_config.ap.password, AP_PASSWORD, strlen(AP_PASSWORD));
        wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
        wifi_config.ap.max_connection = 4;

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    }
    else{
        memcpy(&wifi_config.sta.ssid, STA_SSID, strlen(STA_SSID));
        memcpy(&wifi_config.sta.password, STA_PASSWORD, strlen(STA_PASSWORD));
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

        //Registering event
        esp_event_handler_instance_t instance_got_ip;
        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                            IP_EVENT_STA_GOT_IP,
                                                            &wifi_event_handler,
                                                            &network_event_group,
                                                            &instance_got_ip));
    }

    return true;
}

bool WiFi_start(bool ap_mode){
    printf(ap_mode ? "[WiFi_status] Starting broadcasting WiFi" : "[WiFi_status] Starting connecting to WiFi");
    ESP_ERROR_CHECK(esp_wifi_start());

    if(ap_mode){
        printf("[WiFi_status] Access Point started successfully. SSID: %s\n", AP_SSID);
        return true;
    }
    
    //Block and wait until wifi is connected(WIFI_CONNECTED_BIT) or failed to connect(WIFI_FAIL_BIT)
    EventBits_t bits = xEventGroupWaitBits(network_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    //xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually happened
    if(bits & WIFI_CONNECTED_BIT){ //because bits is a bit mask so we use bit operator
        printf("[WiFi_status] Connected to AP SSID:%s password:%s",
                AP_SSID, AP_PASSWORD);
        return true;
    } 
    else if(bits & WIFI_FAIL_BIT){
        printf("[WiFi_status] Failed to connect to SSID:%s, password:%s",
                AP_SSID, AP_PASSWORD);
    } 
    else{
        printf("[WiFi_status] UNEXPECTED EVENT");
    }
    return false;
}