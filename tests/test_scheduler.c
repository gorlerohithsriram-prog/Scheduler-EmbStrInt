#include<stdio.h>
#include<assert.h>

#include "scheduler.h"

void test_basic_eligibility(void)
{
    Device device = {0};

    device.id = 1;
    device.status = DEVICE_IDLE;

    device.resources.ram_free = 100;
    device.resources.queue_length = 0;
    device.resources.queue_capacity = 4;

    device.capabilities.adc = 1;

    Task task = {0};

    task.id = 1;
    task.requirements.ram_required = 20;
    task.requirements.adc_required = 1;

    EligibilityResult result =
        check_device_eligibility(
            &task,
            &device
        );

    assert(result == DEVICE_ELIGIBLE);

    printf("TEST basic eligibility: PASS\n");
}

void test_insufficient_ram(void)
{
    Device device = {0};

    device.status = DEVICE_IDLE;

    device.resources.ram_free = 10;
    device.resources.queue_length = 0;
    device.resources.queue_capacity = 4;

    Task task = {0};

    task.requirements.ram_required = 20;

    EligibilityResult result =
        check_device_eligibility(
            &task,
            &device
        );

    assert(result == REJECT_RAM);

    printf("TEST insufficient RAM: PASS\n");
}

void test_offline_device(void)
{
    Device device = {0};

    device.status = DEVICE_OFFLINE;

    device.resources.ram_free = 100;

    Task task = {0};

    task.requirements.ram_required = 20;

    EligibilityResult result =
        check_device_eligibility(
            &task,
            &device
        );

    assert(result == REJECT_OFFLINE);

    printf("TEST offline device: PASS\n");
}

void test_queue_full(void)
{
    Device device = {0};

    device.status = DEVICE_IDLE;

    device.resources.ram_free = 100;

    device.resources.queue_length = 4;
    device.resources.queue_capacity = 4;

    Task task = {0};

    task.requirements.ram_required = 20;

    EligibilityResult result =
        check_device_eligibility(
            &task,
            &device
        );

    assert(result == REJECT_QUEUE_FULL);

    printf("TEST queue full: PASS\n");
}

void test_missing_capability(void)
{
    Device device = {0};

    device.status = DEVICE_IDLE;

    device.resources.ram_free = 100;

    device.resources.queue_length = 0;
    device.resources.queue_capacity = 4;

    device.capabilities.cnn = 0;

    Task task = {0};

    task.requirements.ram_required = 20;
    task.requirements.cnn_required = 1;

    EligibilityResult result =
        check_device_eligibility(
            &task,
            &device
        );

    assert(result == REJECT_CAPABILITY_CNN);

    printf("TEST missing capability: PASS\n");
}

void test_best_device_selection(void)
{
    Device devices[2] = {0};

    /*
     * MAX78000
     */
    devices[0].id = 1;
    devices[0].status = DEVICE_IDLE;

    devices[0].resources.ram_free = 100;
    devices[0].resources.queue_length = 1;
    devices[0].resources.queue_capacity = 4;
    devices[0].resources.cpu_usage = 20;
    devices[0].resources.latency_ms = 2;

    devices[0].capabilities.adc = 1;


    /*
     * ESP32
     */
    devices[1].id = 2;
    devices[1].status = DEVICE_IDLE;

    devices[1].resources.ram_free = 400;
    devices[1].resources.queue_length = 3;
    devices[1].resources.queue_capacity = 4;
    devices[1].resources.cpu_usage = 70;
    devices[1].resources.latency_ms = 8;

    devices[1].capabilities.adc = 1;


    Task task = {0};

    task.id = 1;
    task.requirements.ram_required = 20;
    task.requirements.adc_required = 1;


    SchedulerWeights weights = {
        .cpu_weight = 0.4,
        .queue_weight = 0.3,
        .latency_weight = 0.3
    };


    Device *selected =
        schedule_task(
            &task,
            devices,
            2,
            &weights
        );


    assert(selected != NULL);

    assert(selected->id == 1);

    printf("TEST best device selection: PASS\n");
}

void print_device_state(
    const Device *device
)
{
    printf(
        "Device %d: RAM=%d KB, CPU=%d%%, Queue=%d/%d\n",
        device->id,
        device->resources.ram_free,
        device->resources.cpu_usage,
        device->resources.queue_length,
        device->resources.queue_capacity
    );
}

void test_multiple_tasks(void)
{
    Device devices[2] = {0};

    /* MAX78000 */
    devices[0].id = 1;
    devices[0].type = DEVICE_MAX78000;
    devices[0].status = DEVICE_IDLE;

    devices[0].resources.ram_total = 128;
    devices[0].resources.ram_free = 100;
    devices[0].resources.cpu_usage = 20;
    devices[0].resources.queue_length = 0;
    devices[0].resources.queue_capacity = 4;
    devices[0].resources.latency_ms = 2;

    devices[0].capabilities.adc = 1;
    devices[0].capabilities.pwm = 1;
    devices[0].capabilities.uart = 1;
    devices[0].capabilities.wifi = 0;
    devices[0].capabilities.ble = 0;
    devices[0].capabilities.cnn = 1;


    /* ESP32-C6 */
    devices[1].id = 2;
    devices[1].type = DEVICE_ESP32C6;
    devices[1].status = DEVICE_IDLE;

    devices[1].resources.ram_total = 512;
    devices[1].resources.ram_free = 400;
    devices[1].resources.cpu_usage = 30;
    devices[1].resources.queue_length = 0;
    devices[1].resources.queue_capacity = 4;
    devices[1].resources.latency_ms = 8;

    devices[1].capabilities.adc = 1;
    devices[1].capabilities.pwm = 1;
    devices[1].capabilities.uart = 1;
    devices[1].capabilities.wifi = 1;
    devices[1].capabilities.ble = 1;
    devices[1].capabilities.cnn = 0;


    /* Scheduler weights */
    SchedulerWeights weights = {
        .cpu_weight = 0.4,
        .queue_weight = 0.3,
        .latency_weight = 0.3
    };


    /* Tasks */
    Task tasks[5] = {0};


    /* Task 1 - ADC */
    tasks[0].id = 1;
    tasks[0].state = TASK_SUBMITTED;
    tasks[0].requirements.ram_required = 20;
    tasks[0].requirements.cpu_required = 10;
    tasks[0].requirements.adc_required = 1;


    /* Task 2 - CNN */
    tasks[1].id = 2;
    tasks[1].state = TASK_SUBMITTED;
    tasks[1].requirements.ram_required = 30;
    tasks[1].requirements.cpu_required = 30;
    tasks[1].requirements.cnn_required = 1;


    /* Task 3 - WiFi */
    tasks[2].id = 3;
    tasks[2].state = TASK_SUBMITTED;
    tasks[2].requirements.ram_required = 40;
    tasks[2].requirements.cpu_required = 15;
    tasks[2].requirements.wifi_required = 1;


    /* Task 4 - ADC */
    tasks[3].id = 4;
    tasks[3].state = TASK_SUBMITTED;
    tasks[3].requirements.ram_required = 40;
    tasks[3].requirements.cpu_required = 20;
    tasks[3].requirements.adc_required = 1;


    /* Task 5 - CNN */
    tasks[4].id = 5;
    tasks[4].state = TASK_SUBMITTED;
    tasks[4].requirements.ram_required = 50;
    tasks[4].requirements.cpu_required = 25;
    tasks[4].requirements.cnn_required = 1;


    /* Schedule all tasks */
    for (int i = 0; i < 5; i++) {

        Task *task = &tasks[i];

        printf("\n============================\n");
        printf("Scheduling Task %d\n", task->id);
        printf("============================\n");

        for (int j = 0; j < 2; j++) {

            EligibilityResult result =
                check_device_eligibility(
                    task,
                    &devices[j]
                );

            printf(
                "Device %d: %s\n",
                devices[j].id,
                eligibility_result_string(result)
            );
        }

        Device *selected =
            schedule_task(
                task,
                devices,
                2,
                &weights
            );

        if (selected != NULL) {

    printf(
        "Task %d -> Device %d\n",
        task->id,
        selected->id
    );

    reserve_device_resources(
        task,
        selected
    );

    printf("\nDevice state after scheduling:\n");

    print_device_state(&devices[0]);
    print_device_state(&devices[1]);

} else {

    printf(
        "Task %d -> BLOCKED\n",
        task->id
    );
}
}
}

int main(void)
{
    test_basic_eligibility();

    test_insufficient_ram();

    test_offline_device();

    test_queue_full();

    test_missing_capability();

    test_best_device_selection();

    test_multiple_tasks();

    printf("\nAll tests passed!\n");

    return 0;
}
