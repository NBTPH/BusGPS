#include <button.h>

QueueHandle_t Button_Queue = NULL;

bool debounceLogic(const bool raw_input, bool *prev_state, uint8_t *count){
    // printf("debounce pooling\n");
    bool pressed = false;
    if(raw_input != (*prev_state)){
        if(*count < DEBOUNCE_COUNT_LIMIT){
            *count = *count + 1;
        }
        else{ 
            if(!(*prev_state) && raw_input){ //register on rising edge
                pressed = true;
            }
            *prev_state = raw_input;
            *count = 0;
        }
    }
    else{
        *count = 0;
    }
    return pressed;
}

void TaskButton(void *pvParameters){
    Button_Queue = xQueueCreate(5, sizeof(uint8_t));

    gpio_config_t config;
    config.intr_type = GPIO_INTR_DISABLE;
    config.mode = GPIO_MODE_INPUT;
    config.pull_down_en = 0;
    config.pull_up_en = 1; //enable pull up
    config.pin_bit_mask = (1ULL << MPU6050_CALIB_BUTTON) | (1ULL << HMC5883_CALIB_BUTTON);
    gpio_config(&config);

    bool prevState[2] = {true, true};
    uint8_t debounceCount[2] = {0};
    int64_t print_delay = millis();
    printf("Button debounce function started \r\n");
    while(1){
        int64_t current_millis = millis();
        uint8_t msg_button = 5;
        if(debounceLogic(gpio_get_level(MPU6050_CALIB_BUTTON), &prevState[0], &debounceCount[0])){
            msg_button = 0;
            if(xQueueSend(Button_Queue, (void *)&msg_button, 1) != pdTRUE){
                printf("Button queue FULL!\n");
            }
        }
        if(debounceLogic(gpio_get_level(HMC5883_CALIB_BUTTON), &prevState[1], &debounceCount[1])){
            msg_button = 1;
            if(xQueueSend(Button_Queue, (void *)&msg_button, 1) != pdTRUE){
                printf("Button queue FULL!\n");
            }
        }
        delay(DEBOUNCE_INTERVAL_MS);
    }
}