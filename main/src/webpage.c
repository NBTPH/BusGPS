#include <webpage.h>

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

EventGroupHandle_t network_event_group;

int retry_num = 0;
static void connect_to_wifi_event_handler (void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){ //wtf is all this shit?
    EventGroupHandle_t wifi_event_group = *(EventGroupHandle_t *)arg;
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
}

bool WiFi_init(){
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

    //Inititalize the TCP/IP WiFi network interface for Station mode 
    esp_netif_create_default_wifi_sta();

    //Initialize WiFi peripheral, with disable NVS because we arent using that
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    cfg.nvs_enable = false;
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM)); //only using RAM for WiFi handling

    //Configure WiFi
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (password len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    //Registering event
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &connect_to_wifi_event_handler,
                                                        &network_event_group,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &connect_to_wifi_event_handler,
                                                        &network_event_group,
                                                        &instance_got_ip));
    return true;
}

bool WiFi_connect(){
    printf("[WiFi_status] Starting connecting to WiFi");
    ESP_ERROR_CHECK(esp_wifi_start());

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
static esp_err_t js_get_handler(httpd_req_t *req) {
    const size_t size = (coordinates_js_end - coordinates_js_start);
    httpd_resp_set_type(req, "text/plain");
    return httpd_resp_send(req, (const char *)coordinates_js_start, size);
}

static const httpd_uri_t js_uri = {
    .uri       = "/coordinates.js", //what the html will ask for
    .method    = HTTP_GET,
    .handler   = js_get_handler    
};

//Struct and event handler for updating the page
float lat = 0, lon = 0;
static esp_err_t send_msg_handler(httpd_req_t *req){
    printf("GET VALUE CALLED\r\n");
    char message[128] = {0};
    snprintf(message, sizeof(message), "{\"lat\": %10.6f, \"lon\": %10.6f}", lat, lon);
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, (const char *)message, strlen(message));
}

static const httpd_uri_t value_get_uri = {
    .uri       = "/get_value", //what the html will ask for
    .method    = HTTP_GET,
    .handler   = send_msg_handler    
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
        httpd_register_uri_handler(server, &value_get_uri);
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
    
    WiFi_init();
    WiFi_connect();

    httpd_handle_t http_server = HTTP_start();

    uint32_t RNG = 0;


    while(http_server){
        //while there are a server, we only need to change the value of lat and lon, the client will request an update on lat and lon
        RNG = esp_random();
        lat = 3.14159265358979323846;
        RNG = esp_random();
        lon = 3.14159265358979323846;
        delay(10);
    }
}

