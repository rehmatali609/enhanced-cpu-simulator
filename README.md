# enhanced-cpu-simulator
Enhanced CPU Simulator in C++ featuring 5-stage pipelining, hazard detection, data forwarding, and L1 cache memory simulation.
# Enhanced CPU Simulator

A complete CPU Simulator implemented in C++17 for Computer Architecture coursework.

This project demonstrates how modern processors work internally using:

- 5-Stage Instruction Pipeline
- Hazard Detection
- Data Forwarding / Bypassing
- Cache Memory Simulation
- Custom RISC-like ISA
- Fetch–Decode–Execute Cycle
- Performance Metrics (CPI, Cache Hit Rate)

---

## Features

### CPU Architecture
- ALU
- Register File
- Control Unit
- Data Memory
- Instruction Decoder

### Pipeline Support
- IF → ID → EX → MEM → WB stages
- Pipeline latches
- Stall insertion
- Bubble handling

### Hazard Handling
- RAW hazard detection
- Load-use hazard handling
- EX/MEM forwarding
- MEM/WB forwarding

### Cache Memory
- 2-Way Set Associative L1 Cache
- LRU Replacement Policy
- Cache hit/miss tracking

### Instruction Set
Implemented custom 11-instruction RISC-like ISA:
- ADD
- SUB
- AND
- OR
- NOT
- MOV
- LOAD
- STORE
- JMP
- BEQ
- HALT

---

## Project Structure

```bash
cpu_simulator.cpp
enhanced_cpu_simulator.cpp
cache_memory.h
pipeline.h
demo_programs.h
