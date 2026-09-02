#include <stdio.h>

#include "scheduler.h"

int main(void)
{
    Device devices[2];

    devices[0].id = 1;
    devices[0].type = DEVICE_MAX78000;
    devices[0].status = DEVICE_IDLE;

    devices[0].resources.ram_total = 128;
    devices[0].resources.ram_free = 100;
    devices[0].resources.cpu_usage = 20;
    devices[0].resources.queue_length = 1;
    devices[0].resources.queue_capacity = 4;
    devices[0].resources.latency_ms = 2;

    devices[0].capabilities.adc = 1;
    devices[0].capabilities.cnn = 1;
    devices[0].capabilities.pwm = 1;
    devices[0].capabilities.uart = 1;
    devices[0].capabilities.wifi = 0;
    devices[0].capabilities.ble = 0;


    devices[1].id = 2;
    devices[1].type = DEVICE_ESP32C6;
    devices[1].status = DEVICE_IDLE;

    devices[1].resources.ram_total = 512;
    devices[1].resources.ram_free = 400;
    devices[1].resources.cpu_usage = 70;
    devices[1].resources.queue_length = 3;
    devices[1].resources.queue_capacity = 4;
    devices[1].resources.latency_ms = 8;

    devices[1].capabilities.adc = 1;
    devices[1].capabilities.cnn = 0;
    devices[1].capabilities.pwm = 1;
    devices[1].capabilities.uart = 1;
    devices[1].capabilities.wifi = 1;
    devices[1].capabilities.ble = 1;


    Task task;

    task.id = 1;
    task.parent_id = 0;
    task.state = TASK_SUBMITTED;

    task.requirements.ram_required = 20;
    task.requirements.cpu_required = 10;

    task.requirements.adc_required = 1;
    task.requirements.pwm_required = 0;
    task.requirements.uart_required = 0;
    task.requirements.wifi_required = 0;
    task.requirements.ble_required = 0;
    task.requirements.cnn_required = 0;


    printf("Task %d submitted\n", task.id);
    
    SchedulerWeights weights = {
        .cpu_weight = 0.4,
        .queue_weight = 0.3,
        .latency_weight = 0.3
      };
      
    Device *selected =
        schedule_task(&task, devices, 2,&weights);

    if (selected != NULL) {
        printf("Task %d scheduled on device %d\n",
               task.id,
               selected->id);
    }
    else {
        printf("Task %d could not be scheduled\n",
               task.id);
    }

    return 0;
}
