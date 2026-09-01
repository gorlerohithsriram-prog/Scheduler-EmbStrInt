#include "scheduler.h"
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
    int device_count
)
{
    for (int i = 0; i < device_count; i++) {

        if (device_can_run_task(task, &devices[i])) {
            task->state = TASK_PENDING;
            return &devices[i];
        }
    }

    task->state = TASK_BLOCKED;

    return NULL;
}
