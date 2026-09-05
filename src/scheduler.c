#include "scheduler.h"
#include<stdio.h>
#include<stddef.h>

EligibilityResult check_device_eligibility(
    const Task *task,
    const Device *device
)
{
    if (device->status == DEVICE_OFFLINE)
        return REJECT_OFFLINE;

    if (device->resources.ram_free <
        task->requirements.ram_required)
        return REJECT_RAM;

    if (task->requirements.adc_required &&
        !device->capabilities.adc)
        return REJECT_CAPABILITY_ADC;

    if (task->requirements.pwm_required &&
        !device->capabilities.pwm)
        return REJECT_CAPABILITY_PWM;

    if (task->requirements.uart_required &&
        !device->capabilities.uart)
        return REJECT_CAPABILITY_UART;

    if (task->requirements.wifi_required &&
        !device->capabilities.wifi)
        return REJECT_CAPABILITY_WIFI;

    if (task->requirements.ble_required &&
        !device->capabilities.ble)
        return REJECT_CAPABILITY_BLE;

    if (task->requirements.cnn_required &&
        !device->capabilities.cnn)
        return REJECT_CAPABILITY_CNN;

    if (device->resources.queue_length >=
        device->resources.queue_capacity)
        return REJECT_QUEUE_FULL;

    return DEVICE_ELIGIBLE;
}

const char *eligibility_result_string(
    EligibilityResult result
)
{
    switch (result) {

        case DEVICE_ELIGIBLE:
            return "ELIGIBLE";

        case REJECT_OFFLINE:
            return "REJECT_OFFLINE";

        case REJECT_RAM:
            return "REJECT_RAM";

        case REJECT_QUEUE_FULL:
            return "REJECT_QUEUE_FULL";

        case REJECT_CAPABILITY_ADC:
            return "REJECT_CAPABILITY_ADC";

        case REJECT_CAPABILITY_PWM:
            return "REJECT_CAPABILITY_PWM";

        case REJECT_CAPABILITY_UART:
            return "REJECT_CAPABILITY_UART";

        case REJECT_CAPABILITY_WIFI:
            return "REJECT_CAPABILITY_WIFI";

        case REJECT_CAPABILITY_BLE:
            return "REJECT_CAPABILITY_BLE";

        case REJECT_CAPABILITY_CNN:
            return "REJECT_CAPABILITY_CNN";

        default:
            return "UNKNOWN";
    }
}


Device *schedule_task(
    Task *task,
    Device devices[],
    int device_count,
    const SchedulerWeights *weights
)
{
    Device *best_device = NULL;
    double best_score = 1e9;

    for (int i = 0; i < device_count; i++) {

        Device *device = &devices[i];
        
        EligibilityResult result = 
              check_device_eligibility(task,device);
        
        if(result != DEVICE_ELIGIBLE){
              printf("Device %d -> %s\n",
                device->id,
                eligibility_result_string(result));
              
              continue;
        }
              
        double score =
            calculate_device_score(
                task,
                device,
                weights
            );

        printf(
            "Device %d score = %.3f\n",
            device->id,
            score
        );

        if (score < best_score) {
            best_score = score;
            best_device = device;
        }
    }

    if (best_device != NULL) {
        task->state = TASK_PENDING;
    }
    else {
        task->state = TASK_BLOCKED;
    }

    return best_device;
}


double calculate_device_score(
    const Task *task,
    const Device *device,
    const SchedulerWeights *weights
)
{
    (void)task;

    double cpu_score =
        (double)device->resources.cpu_usage / 100.0;

    double queue_score =
        (double)device->resources.queue_length /
        device->resources.queue_capacity;

    double latency_score =
        (double)device->resources.latency_ms / 100.0;

    double score =
        weights->cpu_weight * cpu_score +
        weights->queue_weight * queue_score +
        weights->latency_weight * latency_score;

    return score;
}

void reserve_device_resources(
    const Task *task,
    Device *device
)
{
    device->resources.ram_free -=
        task->requirements.ram_required;

    device->resources.queue_length++;

    device->resources.cpu_usage +=
        task->requirements.cpu_required;

    if (device->resources.cpu_usage > 100)
        device->resources.cpu_usage = 100;
}

void release_device_resources(
    const Task *task,
    Device *device
)
{
    device->resources.ram_free +=
        task->requirements.ram_required;

    device->resources.queue_length--;

    device->resources.cpu_usage -=
        task->requirements.cpu_required;

    if (device->resources.cpu_usage < 0)
        device->resources.cpu_usage = 0;
}
