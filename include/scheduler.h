#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "task.h"
#include "device.h"

typedef enum {
    DEVICE_ELIGIBLE,
    
    REJECT_OFFLINE,
    REJECT_RAM,
    REJECT_QUEUE_FULL,
    
    REJECT_CAPABILITY_ADC,
    REJECT_CAPABILITY_PWM,
    REJECT_CAPABILITY_UART,
    REJECT_CAPABILITY_WIFI,
    REJECT_CAPABILITY_BLE,
    REJECT_CAPABILITY_CNN,
} EligibilityResult;
  
typedef struct {
    double cpu_weight;
    double queue_weight;
    double latency_weight;
} SchedulerWeights;

EligibilityResult check_device_eligibility(
    const Task *task,
    const Device *device
);

const char *eligibility_result_string(
    EligibilityResult result
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

void reserve_device_resources(
    const Task *task,
    Device *device
);

void release_device_resources(
    const Task *task,
    Device *device
);

#endif
