#include <webpage.h>
DataFrame_t Global_Data = {0};
///////////////////////////////////////////////////////////////////////
/////////////////////////// WiFi Stuff ////////////////////////////////
///////////////////////////////////////////////////////////////////////

#define CONNECTION_MAX_RETRY 10
#define CONNECTION_TIMEOUT_SEC 10
#define SOCKET_TIMEOUT_SEC 5

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

const char *AP_SSID = "";
const char *AP_PASSWORD = "";

const char *STA_SSID = "";
const char *STA_PASSWORD = "";

EventGroupHandle_t network_event_group;

int retry_num = 0;
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){ //wtf is all this shit?
    EventGroupHandle_t wifi_event_group = *(EventGroupHandle_t *)arg;
    
    //Station mode events
    //if event is WiFi STATION start we attempt to connect to wifi
    if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START){
        esp_wifi_connect();
    } 
    else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED){ //if can't connected or disconnected
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
    
    //Wait until wifi is connected(WIFI_CONNECTED_BIT) or connection failed(WIFI_FAIL_BIT)
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

///////////////////////////////////////////////////////////////////////
////////////////////////// Webpage Stuff //////////////////////////////
///////////////////////////////////////////////////////////////////////

//Embedded file labels are generated by the build system based on filenames*/
extern const uint8_t coordinates_html_start[] asm("_binary_coordinates_html_start");
extern const uint8_t coordinates_html_end[]   asm("_binary_coordinates_html_end");

extern const uint8_t coordinates_js_start[]  asm("_binary_coordinates_js_start");
extern const uint8_t coordinates_js_end[]    asm("_binary_coordinates_js_end");

//Struct and event handler for html
static esp_err_t html_get_handler(httpd_req_t *req){
    const size_t size = (coordinates_html_end - coordinates_html_start);
    httpd_resp_set_type(req, "text/html");
    
    //Send the response. HTTPD_RESP_USE_STRLEN tells the server to calculate the length automatically.
    esp_err_t error = httpd_resp_send(req, (const char *)coordinates_html_start, size);
    
    if(error != ESP_OK){
        printf("[HTTP_Status] Failed to send HTML response!");
    }
    return error;
}
static const httpd_uri_t html_uri = {
    .uri       = "/",                   
    .method    = HTTP_GET,            
    .handler   = html_get_handler                 
};

//Struct and event handler for javascript
static esp_err_t js_get_handler(httpd_req_t *req){
    const size_t size = (coordinates_js_end - coordinates_js_start);
    httpd_resp_set_type(req, "text/javascript");
    return httpd_resp_send(req, (const char *)coordinates_js_start, size);
}

static const httpd_uri_t js_uri = {
    .uri       = "/coordinates.js", //what the html will ask for
    .method    = HTTP_GET,
    .handler   = js_get_handler    
};

static esp_err_t send_status_handler(httpd_req_t *req){
    char status = GPS_Fixed ? '1' : '0';
    httpd_resp_set_type(req, "text/plain");
    return httpd_resp_send(req, (const char *)&status, 1);
}

static const httpd_uri_t status_get_uri = {
    .uri       = "/get_status", //what the script will ask for
    .method    = HTTP_GET,
    .handler   = send_status_handler    
};

//Struct and event handler for updating the page
static esp_err_t send_data_handler(httpd_req_t *req){
    char message[256] = {0};
    snprintf(message, sizeof(message), "{"
                                        "\"ID\": %ld, "
                                        "\"Date\": {"
                                            "\"Day\": %d, "
                                            "\"Month\": %d, "
                                            "\"Year\": %d"
                                            "}, "
                                        "\"UTC\": {"
                                            "\"Hours\": %d, "
                                            "\"Minutes\": %d, "
                                            "\"Seconds\": %.6f"
                                            "}, "
                                        "\"Lat\": %.6f, "
                                        "\"Lon\": %.6f, "
                                        "\"Heading\": %.6f, "
                                        "\"Ignition\": %s, "
                                        "\"Door_Open\": %s, "
                                        "\"AC\": %s"
                                        "}", 
                                        Global_Data.ID, 
                                            Global_Data.Date.Day, 
                                            Global_Data.Date.Month, 
                                            Global_Data.Date.Year, 
                                            Global_Data.Time.Hours, 
                                            Global_Data.Time.Minutes, 
                                            Global_Data.Time.Seconds, 
                                        Global_Data.Lat, 
                                        Global_Data.Lon, 
                                        Global_Data.Heading, 
                                        Global_Data.Ignition ? "true" : "false", 
                                        Global_Data.Door_Open ? "true" : "false", 
                                        Global_Data.AC ? "true" : "false");
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, (const char *)message, strlen(message));
}

static const httpd_uri_t data_get_uri = {
    .uri       = "/get_data", //what the script will ask for
    .method    = HTTP_GET,
    .handler   = send_data_handler    
};

static httpd_handle_t HTTP_start(){
    httpd_handle_t server = NULL;
    httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();
    server_config.lru_purge_enable = true; //allow the http server to close the socket if needed (least recently used_purge_enable)

    debug_printf("[HTTP_Status] Starting server on port: '%d'", server_config.server_port);
    if(httpd_start(&server, &server_config) == ESP_OK){
        // Set URI handlers
        debug_printf("[HTTP_Status] Registering URI handlers");
        httpd_register_uri_handler(server, &html_uri);
        httpd_register_uri_handler(server, &js_uri);
        httpd_register_uri_handler(server, &data_get_uri);
        httpd_register_uri_handler(server, &status_get_uri);
        return server;
    }

    printf("[HTTP_Status] Error starting server!");
    return NULL;
}

///////////////////////////////////////////////////////////////////////
///////////////////////// Main Functions //////////////////////////////
///////////////////////////////////////////////////////////////////////

void TaskWebpage(void *pvParameters){
    printf("======= Webserver function started =======\r\n");
    
    WiFi_init(true);
    WiFi_start(true);

    httpd_handle_t http_server = HTTP_start();
    while(http_server){
        //while there are a server, we only need to change the value of lat and lon, the client will request an update on lat and lon
        delay(1000);
    }
}

