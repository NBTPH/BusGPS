#include <distance.h>
#include "esp_timer.h"
#include "math.h" //for NaN

QueueHandle_t Distance_Queue = NULL;
int64_t pulse_start = 0;
int64_t pulse_end = 0;
static void IRAM_ATTR edge_ISR(void *arg){
    if(gpio_get_level(ECHO_PIN)){//if after interrupt triggered, pin is high, it means that a rising pulse happened
        pulse_start = esp_timer_get_time(); //record current micros
    }
    else{//falling edge happened
        pulse_end = esp_timer_get_time(); //record current micros
        TaskHandle_t task_notify = (TaskHandle_t)arg; //derefernce it and cast it to a taskhandle
        vTaskNotifyGiveFromISR(task_notify, NULL);
    }
}

void TaskDoor(void *pvParameters){
    Distance_Queue = xQueueCreate(DISTANCE_QUEUE_LENGTH, sizeof(float));

    gpio_config_t config;
    config.intr_type = GPIO_INTR_DISABLE; //10ms trigger pin
    config.mode = GPIO_MODE_OUTPUT;
    config.pull_down_en = 1;
    config.pull_up_en = 0;
    config.pin_bit_mask = (1ULL << TRIG_PIN);
    gpio_config(&config);
    gpio_set_level(TRIG_PIN, 0);

    config.intr_type = GPIO_INTR_ANYEDGE; //mesuring pulse length
    config.mode = GPIO_MODE_INPUT;
    config.pull_down_en = 1;
    config.pull_up_en = 0;
    config.pin_bit_mask = (1ULL << ECHO_PIN);
    gpio_config(&config);

    gpio_intr_disable(ECHO_PIN);
    gpio_install_isr_service(0);
    // TaskHandle_t current_task_handle = NULL;
    // while(current_task_handle == NULL) current_task_handle = xTaskGetCurrentTaskHandle();
    gpio_isr_handler_add(ECHO_PIN, edge_ISR, (void*)xTaskGetCurrentTaskHandle());

    int64_t last_print_millis = millis();
    while(1){
        //sending out 10us pulse to trig pin
        int64_t current_millis = millis();
        float distance = 0;

        gpio_set_level(TRIG_PIN, 1);
        delay_micros(10);
        gpio_set_level(TRIG_PIN, 0);
        
        gpio_intr_enable(ECHO_PIN); //enable intr on TRIG pin
        if(ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(50)) > 0){ //wait for pulse finished signal on ISR, set timeout to 50ms
            int64_t pulse_length = pulse_end - pulse_start; //this is measured in microseconds
            distance = ((pulse_length / 1000000.0f) * 340.0f) / 2.0f; //convert to seconds, then multiplyed with the speed of sound (about 343m/s in room temperature), divided by 2 to get to get distance
            pulse_start = pulse_end = 0; //reset records
            if(xQueueSend(Distance_Queue, (void *)&distance, 2) == errQUEUE_FULL){
                printf("DISTANCE QUEUE FULL\r\n");
            }
        }
        else{
            //Because the sensor still send a pulse about 36ms even if no objects detected, so if after 50ms no pulse detected, potentially faulty sensor or not connected
            distance = NAN;
            printf("[TaskDoor] Sensor not connected\r\n");
            delay(2000);
        }
        gpio_intr_disable(ECHO_PIN); //disable intr on TRIG pin to avoid false triggers

        if(xQueueSend(Distance_Queue, (void *)&distance, 2) == errQUEUE_FULL){
            printf("DISTANCE QUEUE FULL\r\n");
        }

        delay(20); //sensor cool down
    }
}