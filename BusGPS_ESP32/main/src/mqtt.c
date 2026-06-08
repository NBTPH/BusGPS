#include <mqtt.h>

bool DataFrame_2_JSON(char *buffer, size_t max_length, DataFrame_t data){
    char ignition_str[64] = {0};
    switch(data.Ignition){
        case 0:
            snprintf(ignition_str, sizeof(ignition_str), "null");
            break;
        case 1:
            snprintf(ignition_str, sizeof(ignition_str), "false");
            break;
        case 2:
            snprintf(ignition_str, sizeof(ignition_str), "true");
            break;
    }

    char door_str[64] = {0};
    switch(data.Door_Open){
        case 0:
            snprintf(door_str, sizeof(ignition_str), "null");
            break;
        case 1:
            snprintf(door_str, sizeof(ignition_str), "false");
            break;
        case 2:
            snprintf(door_str, sizeof(ignition_str), "true");
            break;
    }

    int ret = snprintf(buffer, max_length, "{"
                                                "\"id\": %ld, "
                                                "\"date\": {"
                                                    "\"day\": %d, "
                                                    "\"month\": %d, "
                                                    "\"year\": %d"
                                                    "}, "
                                                "\"utc\": {"
                                                    "\"hours\": %d, "
                                                    "\"minutes\": %d, "
                                                    "\"seconds\": %.6f"
                                                    "}, "
                                                "\"lat\": %.7f, "
                                                "\"lon\": %.7f, "
                                                "\"heading\": %.7f, "
                                                "\"ignition\": %s, "
                                                "\"doorOpen\": %s, "
                                                "\"ac\": %s"
                                            "}",
                                            data.ID, 
                                                data.Date.Day, 
                                                data.Date.Month, 
                                                data.Date.Year, 
                                                data.Time.Hours, 
                                                data.Time.Minutes, 
                                                data.Time.Seconds, 
                                            data.Lat, 
                                            data.Lon, 
                                            data.Heading, 
                                            ignition_str, 
                                            door_str, 
                                            data.AC ? "true" : "false");
    if(ret < 0){
        return false;
    }
    return true;
}

esp_mqtt_client_handle_t client;
DataFrame_t Global_Data = {0};
static void log_error_if_nonzero(const char *message, int error_code){
    if (error_code != 0) {
        printf("[MQTT_Status] Last error %s: 0x%x\r\n", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    printf("[MQTT_Status] Event dispatched from event loop base=%s, event_id=%ld \r\n" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    client = event->client;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        printf("[MQTT_Status] MQTT_EVENT_CONNECTED\r\n");
        break;
    case MQTT_EVENT_DISCONNECTED:
        printf("[MQTT_Status] MQTT_EVENT_DISCONNECTED\r\n");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        debug_printf("[MQTT_Status] MQTT_EVENT_SUBSCRIBED, msg_id=%d, return code=0x%02x\r\n", event->msg_id, (uint8_t)*event->data);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        debug_printf("[MQTT_Status] MQTT_EVENT_UNSUBSCRIBED, msg_id=%d\r\n", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        debug_printf("[MQTT_Status] MQTT_EVENT_PUBLISHED, msg_id=%d\r\n", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        debug_printf("[MQTT_Status] MQTT_EVENT_DATA\r\n");
        debug_printf("[MQTT_Status] TOPIC=%.*s\r\n\r\n", event->topic_len, event->topic);
        debug_printf("[MQTT_Status] DATA=%.*s\r\n\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        printf("[MQTT_Status] MQTT_EVENT_ERROR\r\n");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls\r\n", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack\r\n", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno\r\n",  event->error_handle->esp_transport_sock_errno);
            printf("[MQTT_Status] Last errno string (%s)\r\n", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        printf("[MQTT_Status] Other event id:%d\r\n", event->event_id);
        break;
    }
}

void MQTT_Send_MSG(char *buff, size_t length){
    int msg_id = esp_mqtt_client_publish(client, "", buff, length, 0, 0);
    if(msg_id == 0){
        debug_printf("[MQTT_Status] publish message success: %s\r\n", buff);
    }
    else{
        printf("[MQTT_Status] publish message failed: %s\r\n", buff);
    }
}

extern const uint8_t isrgrootx1_pem_start[]  asm("_binary_isrgrootx1_pem_start");
void TaskMQTT(void *pvParameters){
    esp_mqtt_client_config_t mqtt_cfg = {0};
    mqtt_cfg.broker.address.uri = "";
    mqtt_cfg.broker.verification.certificate = (const char *)isrgrootx1_pem_start;
    mqtt_cfg.broker.address.port = 0;
    mqtt_cfg.credentials.username = "";
    mqtt_cfg.credentials.authentication.password = "";
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    
    while(!WiFi_Connected) delay(1000); //wait for wifi to connect
    esp_mqtt_client_start(client); //we start the connection

    int64_t last_MSG_millis = millis();
    uint32_t msg_count = 1;
    while(1){
        int64_t current_millis = millis();
        char msg_out[512] = {0};
        snprintf(msg_out, sizeof(msg_out), "[BusGPS %ld] [Message %ld] [%lld]: ", Global_Data.ID, msg_count, current_millis); //message headers are this messages index and time since startup

        if(ulTaskNotifyTake(pdTRUE, 1) > 0){ //if notification from main task is received, we are going to send data
            char json_msg[256] = {0};
            if(DataFrame_2_JSON(json_msg, sizeof(json_msg), Global_Data)){
                strcat(msg_out, json_msg);
                MQTT_Send_MSG(msg_out, strlen(msg_out));
                msg_count++;
                last_MSG_millis = current_millis;
            }
        }

        if(current_millis - last_MSG_millis >= 2000){ //data messages has not been sent out for a while, we send a heart beat message of the current state
            char msg[64] = {0};
            if(!GPS_Fixed){
                snprintf(msg, sizeof(msg), "NO GPS FIX, DATA IS NOT RELIABLE");
            }
            else if(MPU6050_is_Calibrating || HMC5883_is_Calibrating){
                snprintf(msg, sizeof(msg), "SENSOR IS CALIBRATING");
            }
            strcat(msg_out, msg);
            MQTT_Send_MSG(msg_out, strlen(msg_out));
            msg_count++;
            last_MSG_millis = current_millis;
        }

        delay(2);//watch dog feed
    }
}