# Designing Mini-Schedulers for Concurrent Kernel Execution

## 1. Objective

The primary goal of this task is to design an intelligent scheduling system capable of deciding where computational kernels should be executed within a heterogeneous multi-device environment.

Modern embedded and distributed computing systems may consist of multiple processing devices such as:

* CPUs
* GPUs
* Microcontrollers
* DSPs
* AI Accelerators
* FPGA-based platforms

When multiple kernels are issued at the same time, determining the best execution location becomes a critical challenge.

The scheduler evaluates the characteristics of each kernel and the current state of available devices before making placement decisions.

---

# 2. Scheduling Parameters

The scheduler uses two categories of parameters.

## 2.1 Hard Parameters

* Total available memory on the device
* Maximum number of concurrent threads
* Number of processing cores
* Maximum queue capacity

## 2.2 Soft Parameters

* Current CPU utilization
* Current memory usage
* Device workload
* Queue length

---

# 3. Scheduling Workflow

## Step 1: Kernel Analysis

The scheduler analyzes the incoming kernel and gathers metadata such as:

* Computational complexity
* Memory requirements
* Input/output data size
* Priority level
* Estimated execution duration

## Step 2: Device Monitoring

The scheduler continuously monitors all available devices and collects:

* Resource utilization
* Available memory
* Processing capacity
* Current workload

## Step 3: Suitability Scoring

Each device receives a suitability score based on how effectively it can execute the kernel.

Example scoring factors:

* Available resources
* Expected completion time
* Data transfer cost
* Current load
* Power efficiency

A weighted scoring model can be used:

```text
Score =
(Performance Weight × Processing Capability)
+ (Memory Weight × Available Memory)
- (Load Weight × Current Utilization)
- (Transfer Weight × Communication Cost)
```

## Step 4: Device Selection

The device with the highest score and sufficient resources is selected as the execution target.

## Step 5: Dynamic Re-evaluation

If system conditions change significantly during runtime, the scheduler can re-evaluate future kernel placements to maintain optimal performance.

---

# 4. Mini-Scheduler Design for Concurrent Kernel Execution

The mini-scheduler runs on each device and manages multiple kernels assigned to that device.

Each task is classified as either CPU-intensive or I/O-intensive.

---

## 4.1 Priority-Based Scheduling

Each task is assigned a priority level:

* **High Priority** – system-critical tasks
* **Medium Priority** – normal workloads
* **Low Priority** – background operations

Higher-priority tasks receive CPU access before lower-priority tasks.

However, an aging mechanism gradually increases the priority of waiting tasks to avoid starvation.

---

# 5. Watchdog Timer Mechanism

Every task is associated with a watchdog timer.

The watchdog is used to:

* Detect tasks exceeding their expected execution time
* Recover from deadlocks or infinite execution loops
* Improve system reliability

### Watchdog Timeout

```text
Watchdog Time =
Estimated Kernel Load × Scaling Factor
```

Example:

* Light kernel → 100 ms timeout
* Medium kernel → 500 ms timeout
* Heavy kernel → 2000 ms timeout

If a task exceeds its timeout:

1. Warning is generated
2. Task is paused or terminated
3. Resources are released
4. Scheduler proceeds to the next task

---

# 6. CPU and I/O Time Tracking

The scheduler maintains two execution metrics:

## CPU Time

Amount of time actively spent executing instructions.

## I/O Time

Amount of time spent waiting for external operations such as:

* Disk access
* Network communication
* Sensor input
* File operations

Task statistics are continuously updated during execution.

### Example

```text
Task A:
CPU Time = 90 ms
I/O Time = 10 ms

Task B:
CPU Time = 20 ms
I/O Time = 80 ms
```

Therefore:

* Task A is CPU-heavy
* Task B is I/O-heavy

---

# 7. Dynamic CPU Allocation

CPU allocation is determined using the ratio:

```text
CPU Utilization Ratio =
CPU Time / (CPU Time + I/O Time)
```

## Task Classification

```text
Ratio > 0.7
    CPU-Heavy Task

Ratio < 0.3
    I/O-Heavy Task

Otherwise
    Balanced Task
```

---

# 8. Scheduling Decisions

## CPU-Heavy Tasks

* Receive larger CPU bursts
* Scheduled less frequently
* Minimize context-switching overhead

## I/O-Heavy Tasks

* Receive shorter CPU bursts
* Scheduled more frequently
* Quickly return to the waiting state

## Balanced Tasks

* Receive standard time slices

### Example Allocation

```text
CPU-Heavy Task → 40 ms quantum
Balanced Task  → 25 ms quantum
I/O-Heavy Task → 10 ms quantum
```

---

# 9. Overall Scheduling Flow

```text
Multiple Kernels
       |
       v
Kernel Analysis
       |
       v
Device Monitoring
       |
       v
Hard Parameter Check
       |
       v
Suitability Scoring
       |
       v
Device Selection
       |
       v
Device Mini-Scheduler
       |
       v
Priority + CPU/I/O Classification
       |
       v
Dynamic CPU Allocation
       |
       v
Kernel Execution
       |
       v
Watchdog + Resource Monitoring
       |
       v
Next Kernel
```
