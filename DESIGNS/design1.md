# Scheduler Design Document

## Objective

Design a scheduling framework that assigns kernels to heterogeneous devices such as CPU, GPU, FPGA, or NPU. The scheduler operates as a **greedy per-kernel dispatcher** by default: each arriving kernel is independently assigned to the device with the highest affinity score at that instant. A batch/global optimization mode is noted as a future extension.

---

## 1. Hard Parameters (Non-negotiable)

These constraints **must** be satisfied. If no device passes all hard checks, the kernel is **rejected** (queued for retry or reported as an error to the host).

### 1.1 Hardware Capability

A kernel declares a required capability (or set of capabilities). Only devices that possess *all* required capabilities are considered.

| Kernel Requirement          | Suitable Device(s)          |
| --------------------------- | --------------------------- |
| CNN inference               | CNN Accelerator, NPU        |
| UART ISR                    | CPU only                    |
| Floating-point SIMD         | CPU (with FPU), GPU         |
| Cryptographic offload       | FPGA, CPU (AES-NI)          |
| BLE communication           | Board with BLE radio        |

Capabilities are represented as a **bitmask** so a kernel can require multiple capabilities simultaneously (e.g., `CNN_ACCEL | BLE_RADIO`).

### 1.2 Maximum Concurrent Threads / Tasks

Each device has a finite number of hardware and software execution contexts. The scheduler tracks `active_threads` against `max_threads` per device.

| Constraint Type         | Example Limit                  |
| ----------------------- | ------------------------------ |
| RTOS threads            | 8 per Cortex-M4                |
| Hardware interrupt vectors | 32                          |
| CNN accelerator queues  | 2 concurrent inferences        |
| DMA channels            | 4                              |
| Stack memory per thread | 4 KB each, total ≤ available   |

**Rule:** `kernel.threads_needed + device.active_threads ≤ device.max_threads`

### 1.3 Memory Availability

A kernel's minimum memory requirement must fit within the device's free memory. Multiple pools may need individual checking.

| Memory Pool   | Check                                    |
| ------------- | ---------------------------------------- |
| SRAM          | `kernel.min_ram ≤ device.free_sram`      |
| Flash         | `kernel.code_size ≤ device.free_flash`   |
| CNN/Accel RAM | `kernel.accel_ram ≤ device.free_accel_ram` |
| Stack         | `kernel.stack_per_thread * threads_needed ≤ remaining_stack` |

**Rule:** *All* memory pools required by the kernel must pass their respective checks.

---

## 2. Soft Parameters (Optimisation, not mandatory)

These parameters influence affinity scoring. They do not block placement.

### 2.1 Memory Usage Optimisation

Prefer devices where the kernel consumes a smaller *fraction* of remaining free memory. This leaves room for future kernels.

### 2.2 CPU / GPU / Accelerator Utilisation

Avoid pushing any device to 100% utilisation. The scheduler aims to balance load across devices of the same type.

### 2.3 Throughput

Maximise `kernels_completed / second` across the system. Favour faster devices for compute-heavy kernels.

### 2.4 Waiting Time

Minimise queuing delay. A device with a long backlog should receive a lower score so new kernels go to idle or lightly-loaded devices.

### 2.5 Power Efficiency

On battery-powered deployments, prefer low-power scheduling:

- **Batch tasks** so the device wakes once, processes many kernels, then sleeps longer.
- **Avoid waking** a second device if one already-active device can handle the kernel.
- **Prefer** devices with lower active power draw (mW) when all else is equal.

---

## 3. Scoring Functions (Normalised to [0, 1])

Each raw metric is mapped to a normalised score in `[0, 1]` before being multiplied by its weight. This prevents any single large-magnitude metric from dominating the weighted sum.

### 3.1 Capability Score — C

```
C = 1.0  if device has ALL required capabilities
C = 0.0  otherwise
```

If a graded capability model is desired in future, a device that supports the capability *partially* or *at reduced speed* could score in `(0, 1)`.

### 3.2 Memory Availability Score — M

Considers the most constrained memory pool (the one with the smallest headroom ratio):

```
ratio_i = (free_pool_i - kernel_requirement_i) / total_pool_i
M = min(ratio_i for all required pools)
M = max(0.0, M)   # clamp negative ratios to 0
```

### 3.3 Load Score — L

Lower current load = higher score:

```
L = 1.0 - (device.active_threads / device.max_threads)
```

If the device tracks CPU utilisation percentage directly, use that instead:

```
L = 1.0 - (device.cpu_load_pct / 100.0)
```

### 3.4 Power Efficiency Score — P

Lower power draw relative to the device's maximum = higher score:

```
P = 1.0 - (device.power_draw_mw / device.max_power_mw)
```

### 3.5 Deadline Score — D

How comfortably the kernel can meet its deadline on this device:

```
slack = deadline_ms - estimated_runtime_on_device_ms
D = clamp(slack / deadline_ms, 0.0, 1.0)   # if deadline_ms == 0: D = 1.0
```

A value near 0 means the kernel will barely finish in time (risky); near 1 means ample slack.

---

## 4. The Dynamic Affinity Function

```
A(k, d) = w1*C + w2*M + w3*L + w4*P + w5*D

where:
  C = Capability score         [0, 1]
  M = Memory availability      [0, 1]
  L = Load / utilisation       [0, 1]
  P = Power efficiency         [0, 1]
  D = Deadline slack           [0, 1]

Constraints:
  w1 + w2 + w3 + w4 + w5 = 1.0
  wi ≥ 0
```

If `C = 0` (device lacks a required capability), the affinity is immediately **0** and no further computation is needed — this device is invalid.

### 4.1 Weight Presets

Weights can be configured at compile time or runtime to match deployment policy:

| Policy       | w1 (Cap) | w2 (Mem) | w3 (Load) | w4 (Power) | w5 (Deadline) | Use Case                      |
| ------------ | -------- | -------- | --------- | ---------- | ------------- | ----------------------------- |
| Balanced     | 0.25     | 0.20     | 0.25      | 0.10       | 0.20          | General-purpose               |
| Performance  | 0.20     | 0.10     | 0.40      | 0.05       | 0.25          | Maximise throughput           |
| Power-save   | 0.20     | 0.10     | 0.10      | 0.50       | 0.10          | Battery-constrained           |
| Real-time    | 0.30     | 0.10     | 0.10      | 0.05       | 0.45          | Hard/soft real-time deadlines |

---

## 5. Static Affinity (Alternative / Baseline)

Task-to-device mapping is fixed at compile time via a lookup table.

### Advantages
- Zero runtime CPU overhead — no score computation.
- Predictable — same kernel always goes to the same device type.
- Simple to implement, debug, and test.

### Disadvantages
- No load balancing — if the CNN accelerator is busy, the kernel waits even if an NPU is idle.
- No power adaptation — cannot switch to low-power devices at night.
- Requires manual updates when hardware changes.

### When to Use
- Homogeneous or single-device deployments.
- Safety-critical systems where non-determinism is unacceptable.
- As a **fallback** when dynamic scheduling fails (e.g., all dynamic candidates tied).

---

## Scheduling Workflow**

1. Kernel arrives

2. Check hard constraints

3. Remove invalid devices

4. Compute affinity scores

5. Select highest affinity device

6. Dispatch kernel
