# shm-bench

**shm-bench** is a microbenchmark tool designed to -bench-test shared memory access patterns across multiple NUMA (Non-Uniform Memory Access) nodes.  
It launches multiple threads, distributes them across all available NUMA nodes and CPU cores, and simultaneously performs uncontrolled memory operations on a shared memory region — all without synchronization or locking.

The goal is to trigger NUMA-related behavior (e.g., local vs. remote memory access, page migrations, hinting faults) and analyze memory access performance under high contention.

---

## Features

- Multithreaded memory workload generator
- NUMA-aware thread placement
- Shared memory -bench without synchronization
- Useful for analyzing page locality, NUMA faults, and cache/memory traffic

---

## Build Instructions

Ensure you have the NUMA development libraries installed:

```bash
sudo apt update
sudo apt install libnuma-dev
