#include <uart.h>
#define BUFFER_SIZE 2048
RMC_MSG_t GPS_RMC_data = {0};
GGA_MSG_t GPS_GGA_data = {0};
ACK_MSG_t GPS_ACK_data = {0};

QueueHandle_t GPS_Queue = NULL;
bool GPS_Fixed = false;

char buffer[BUFFER_SIZE] = {0};
bool LC76G_Init(){
    char tx_buffer[256] = {0};
    Set_Fix_Rate_MSG(250, tx_buffer); //construct set fix rate message
    uart_write_bytes(UART_NUM_2, tx_buffer, strlen(tx_buffer)); //send the message

    memset(buffer, 0, sizeof(buffer));
    bool ack_msg_found = false;
    while(!ack_msg_found){ //wait until PAIR001 respond message found
        int length = uart_read_bytes(UART_NUM_2, buffer, (BUFFER_SIZE - 1), 5);
        if(length > 0){
            char *p_msg = strstr(buffer, "$PAIR001"); //find PAIR001 message (we only use strstr when buffer is not empty do dont need to account for empty string)
            if(p_msg != NULL){
                ack_msg_found = true;
                p_msg++; //move up one character, skiping '$'
                char *carriage = strchr(p_msg, '\n'); //find the linefeed (end of message)
                size_t length = carriage - p_msg; //calculate message length
                if(length < 100){
                    char cmd[100] = {0};
                    memcpy(cmd, p_msg, length);
                    Parse_ACK_MSG(p_msg, length, &GPS_ACK_data);
                }
            }
            memset(buffer, 0, sizeof(buffer));
        }
    }

    return (GPS_ACK_data.CommandID == 50) && (GPS_ACK_data.Result == 0);
}

void TaskUART(void *pvParameters){
    printf("UART task start\n");
    GPS_Queue = xQueueCreate(GPS_QUEUE_LENGTH, sizeof(GPS_t));

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, BUFFER_SIZE, BUFFER_SIZE, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    if(!LC76G_Init()){
        printf("LC76G Initialize unsuccess\r\n");
        return; //exit task
    }
    else{
        printf("LC76G Initialize successfully\r\n");
    }

    GPS_t GPS_Data = {0};

    while(1){
        int length = uart_read_bytes(UART_NUM_2, buffer, (BUFFER_SIZE - 1), 5);
        if(length > 0){
            char *p_msg = strstr(buffer, "$GNGGA"); //parse GGA first
            if(p_msg != NULL){
                p_msg++; //move up one character to avoid the dollar sign
                char *carriage = strchr(p_msg, '\n'); //find the linefeed (end of message)
                size_t length = carriage - p_msg;
                if(length < 100){
                    char cmd[100];
                    memset(cmd, 0, sizeof(cmd));
                    memcpy(cmd, p_msg, length);
                    Parse_GGA_MSG(p_msg, length, &GPS_GGA_data);
                }
            }
            p_msg = strstr(buffer, "$GNRMC");
            if(p_msg != NULL){
                p_msg++; //move up one character to avoid the dollar sign
                char *carriage = strchr(p_msg, '\n'); //find the linefeed (end of message)
                size_t length = carriage - p_msg;
                if(length < 100){
                    char cmd[100];
                    memset(cmd, 0, sizeof(cmd));
                    memcpy(cmd, p_msg, length);
                    Parse_RMC_MSG(p_msg, length, &GPS_RMC_data);
                }
            }
            // printf("%s\r\n\n", buffer);
            memset(buffer, 0, sizeof(buffer));

            if(GPS_GGA_data.Quality != 0 && GPS_RMC_data.Status != 'V'){ //Ensure that we have a reliable position fix
                GPS_Fixed = true;

                GPS_Data.DATE = GPS_RMC_data.DATE;
                GPS_Data.UTC = GPS_GGA_data.UTC;
                GPS_Data.Lat = GPS_GGA_data.Lat;
                GPS_Data.NS = GPS_GGA_data.NS;
                GPS_Data.Lon = GPS_GGA_data.Lon;
                GPS_Data.EW = GPS_GGA_data.EW;
                GPS_Data.Alt = GPS_GGA_data.Alt;
                GPS_Data.SOG = GPS_RMC_data.SOG;
                GPS_Data.COG = GPS_RMC_data.COG;
                if(xQueueSend(GPS_Queue, (void *)&GPS_Data, 2) == errQUEUE_FULL){
                    printf("GPS QUEUE FULL\r\n");
                }
            }
            else{
                GPS_Fixed = false;
            }
        }
        delay(10);
    }
}