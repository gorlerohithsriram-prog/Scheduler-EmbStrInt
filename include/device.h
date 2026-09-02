#ifndef DEVICE_H
#define DEVICE_H

typedef enum {
    DEVICE_MAX78000,
    DEVICE_ESP32C6
} DeviceType;

typedef enum {
    DEVICE_IDLE,
    DEVICE_BUSY,
    DEVICE_OFFLINE
} DeviceStatus;

typedef struct {
    int adc;
    int pwm;
    int uart;
    int wifi;
    int ble;
    int cnn;
} DeviceCapabilities;

typedef struct {
    int ram_total;
    int ram_free;

    int cpu_usage;

    int queue_length;
    int queue_capacity;
    
    int latency_ms;
} DeviceResources;

typedef struct {
    int id;
    DeviceType type;

    DeviceStatus status;

    DeviceCapabilities capabilities;
    DeviceResources resources;

} Device;

#endif
