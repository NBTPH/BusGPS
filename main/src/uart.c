#include <uart.h>

RMC_MSG_t GPS_RMC_data = {0};
GGA_MSG_t GPS_GGA_data = {0};

void TaskGPS(void *pvParameters){
    printf("UART task start\n");

    const int BUFFER_SIZE = 2048;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

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

    int64_t last_millis = millis();

    while(1){
        int length = uart_read_bytes(UART_NUM_2, buffer, (BUFFER_SIZE - 1), 5);
        if(length > 0){
            int64_t current_millis = millis();
            printf("Interval: %lld\n", current_millis - last_millis);
            last_millis = current_millis;
            char *p_cmd = strstr(buffer, "$GNRMC");
            if(p_cmd != NULL){
                p_cmd++; //move up one character to avoid the dollar sign
                char *carriage = strchr(p_cmd, '\n'); //find the linefeed (end of message)
                size_t length = carriage - p_cmd;
                if(length < 100){
                    char cmd[100];
                    memset(cmd, 0, sizeof(cmd));
                    memcpy(cmd, p_cmd, length);
                    Parse_RMC_MSG(p_cmd, length, &GPS_RMC_data);
                }
            }
            p_cmd = strstr(buffer, "$GNGGA");
            if(p_cmd != NULL){
                p_cmd++; //move up one character to avoid the dollar sign
                char *carriage = strchr(p_cmd, '\n'); //find the linefeed (end of message)
                size_t length = carriage - p_cmd;
                if(length < 100){
                    char cmd[100];
                    memset(cmd, 0, sizeof(cmd));
                    memcpy(cmd, p_cmd, length);
                    Parse_GGA_MSG(p_cmd, length, &GPS_GGA_data);
                }
            } 
            memset(buffer, 0, sizeof(buffer));
        }
    }
}