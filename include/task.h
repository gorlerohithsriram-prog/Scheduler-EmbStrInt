#ifndef TASK_H
#define TASK_H

typedef enum {
    TASK_SUBMITTED,
    TASK_PENDING,
    TASK_RUNNING,
    TASK_COMPLETED,
    TASK_ERROR,
    TASK_BLOCKED
} TaskState;

typedef struct {

    int ram_required;
    int cpu_required;

    int adc_required;
    int pwm_required;
    int uart_required;
    int wifi_required;
    int ble_required;
    int cnn_required;

} TaskRequirements;

typedef struct {

    int id;
    int parent_id;

    TaskState state;

    TaskRequirements requirements;

} Task;

#endif
