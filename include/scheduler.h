#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "task.h"
#include "device.h"

typedef struct {
    double cpu_weight;
    double queue_weight;
    double latency_weight;
} SchedulerWeights;

int device_can_run_task(
    const Task *task,
    const Device *device
);

double calculate_device_score(
    const Task *task,
    const Device *device,
    const SchedulerWeights *weights
);

Device *schedule_task(
    Task *task,
    Device devices[],
    int device_count,
    const SchedulerWeights *weights
);

#endif
