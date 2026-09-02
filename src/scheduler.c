#include "scheduler.h"
#include <stdio.h>
#include<stddef.h>

int device_can_run_task(
    const Task *task,
    const Device *device
)
{
    if (device->status == DEVICE_OFFLINE)
        return 0;

    if (device->resources.ram_free <
        task->requirements.ram_required)
        return 0;

    if (task->requirements.adc_required &&
        !device->capabilities.adc)
        return 0;

    if (task->requirements.pwm_required &&
        !device->capabilities.pwm)
        return 0;

    if (task->requirements.uart_required &&
        !device->capabilities.uart)
        return 0;

    if (task->requirements.wifi_required &&
        !device->capabilities.wifi)
        return 0;

    if (task->requirements.ble_required &&
        !device->capabilities.ble)
        return 0;

    if (task->requirements.cnn_required &&
        !device->capabilities.cnn)
        return 0;

    if (device->resources.queue_length >=
        device->resources.queue_capacity)
        return 0;

    return 1;
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

        if (!device_can_run_task(task, device))
            continue;

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
