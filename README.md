# scheduling-algorithms
_Implementing popular scheduling algorithms to solve synchronisation problems. Concepts: critical sections, semaphores, mutexes, parallel programming, buffers._

Process scheduling in a static environment where all jobs are known at the start: `static-a.c` implements First Come First Served (FCFS). `static-b.c` implements Round Robin (RR).

The Single Producer and Single Consumer Problem. Unbounded buffer and binary semaphore: `pc-unbounded.c`. Bounded buffer and "counting semaphore" as a "sleep mechanism": `pc-bounded.c`.

Simplified Windows Scheduler: `windows-simple.c`, using 32 bounded buffers (implemented as linked lists) to represent the queues (half FFCFS and half RR) at different process priority levels (single producer, multiple consumers). `windows-pc.c` allows the producer thread to pre-empt a FCFS job from running if a higher priority job was generated and to boost the priority of RR jobs dynamically if they have not run for a pre-determined amount of time.
