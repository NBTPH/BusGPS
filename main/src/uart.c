#include <uart.h>

RMC_MSG_t GPS_data = {0};

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

    while(1){
        int length = uart_read_bytes(UART_NUM_2, buffer, (BUFFER_SIZE - 1), 5);
        if(length > 0){
            // printf("%s\n", buffer);
            // memset(buffer, 0, sizeof(buffer));
            char *p_cmd = strstr(buffer, "$GNRMC");
            if(p_cmd != NULL){
                p_cmd++;
                char *dollar = strchr(p_cmd, '$');
                // if(dollar != NULL){
                //     printf("%s\n\n", dollar);
                // }
                size_t length = dollar - p_cmd;
                Parse_RMC_MSG(p_cmd, length, &GPS_data);
            }
        }
    }
}