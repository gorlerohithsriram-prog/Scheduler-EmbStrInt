#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "task.h"
#include "device.h"

int device_can_run_task(
    const Task *task,
    const Device *device
);

Device *schedule_task(
    Task *task,
    Device devices[],
    int device_count
);

#endif
