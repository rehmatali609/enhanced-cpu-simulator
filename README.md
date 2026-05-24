# R609 # Rehmat Ali # 2023609
# Computer Architecture CPU Simulator Project - CS 361
# Instructor: Ms. Laraib Noor
## Project Overview

This is a comprehensive CPU simulator demonstrating key computer architecture concepts including:

- **CPU Organization**: ALU, Control Unit, Registers, and Memory
- **Instruction Set Architecture (ISA)**: 11-instruction RISC-like ISA
- **5-Stage Pipelining**: IF → ID → EX → MEM → WB stages
- **Hazard Detection**: RAW, WAW, WAR, and Load-Use hazards
- **Data Forwarding**: Bypassing technique to resolve hazards
- **Cache Memory**: 2-way associative cache with LRU replacement policy

## Project Structure

```
├── cpu_simulator.cpp              # Basic CPU (3-stage: Fetch-Decode-Execute)
├── enhanced_cpu_simulator.cpp     # Enhanced CPU with 5-stage pipeline & cache
├── cache_memory.h                 # Cache memory module (LRU, set-associative)
├── pipeline.h                     # Pipeline stages, hazard detection, forwarding
├── demo_programs.h                # 7 different demonstration programs
└── README.md                      # This file
```

## Instruction Set Architecture (ISA)

### Supported Instructions (11 Total)

| Opcode | Mnemonic | Format | Operation | Example |
|--------|----------|--------|-----------|---------|
| 0x0 | ADD | R-Type | Rd = Rs1 + Rs2 | ADD R0, R1, R2 |
| 0x1 | SUB | R-Type | Rd = Rs1 - Rs2 | SUB R0, R1, R2 |
| 0x2 | AND | R-Type | Rd = Rs1 & Rs2 | AND R0, R1, R2 |
| 0x3 | OR | R-Type | Rd = Rs1 \| Rs2 | OR R0, R1, R2 |
| 0x4 | NOT | R-Type | Rd = ~Rs1 | NOT R0, R1 |
| 0x5 | MOV | I-Type | Rd = Immediate | MOV R0, 100 |
| 0x6 | LOAD | I-Type | Rd = MEM[addr] | LOAD R0, [20] |
| 0x7 | STORE | I-Type | MEM[addr] = Rs1 | STORE R0, [20] |
| 0x8 | JMP | J-Type | PC = addr | JMP 10 |
| 0x9 | BEQ | J-Type | if Rs1==Rs2: PC=addr | BEQ R0, R1, 10 |
| 0xF | HALT | Special | Stop execution | HALT |

### Register File
- 8 General-Purpose Registers: R0 - R7
- Each register holds a 32-bit signed integer

### Memory
- **Basic CPU**: 256 words addressable
- **Enhanced CPU**: 512 words + 128-byte L1 Cache (2-way associative, 4-byte blocks)

## CPU Architecture Details

### Basic CPU (cpu_simulator.cpp)

#### Components:
1. **Arithmetic Logic Unit (ALU)**
   - Supports: ADD, SUB, AND, OR, NOT
   - Maintains flags: Zero, Negative

2. **Register File**
   - 8 registers (R0-R7)
   - Read/Write ports

3. **Control Unit**
   - Decodes opcodes into control signals
   - Generates: RegWrite, MemRead, MemWrite, Branch, Jump, Halt signals

4. **Execution Loop**
   - Fetch: Get instruction from memory
   - Decode: Generate control signals
   - Execute: Perform operation based on instruction type

#### Stages:
```
    ┌──────────┐
    │   FETCH  │  (PC → Instruction)
    └────┬─────┘
         │
    ┌────▼──────┐
    │   DECODE  │  (Instruction → Control Signals)
    └────┬──────┘
         │
    ┌────▼──────────────────────────────────┐
    │   EXECUTE (Operation-specific)        │
    │   ├─ ALU ops: R-type arithmetic       │
    │   ├─ MOV: Load immediate              │
    │   ├─ LOAD: Memory read                │
    │   ├─ STORE: Memory write              │
    │   ├─ JMP: Set PC                      │
    │   ├─ BEQ: Conditional jump            │
    │   └─ HALT: Stop                       │
    └────┬───────────────────────────────────┘
         │
    ┌────▼─────┐
    │  PC++    │  (Increment for next cycle)
    └──────────┘
```

### Enhanced CPU (enhanced_cpu_simulator.cpp)

#### 5-Stage Pipeline:

```
Cycle:    1    2    3    4    5    6
          │    │    │    │    │    │
Instr1:   IF──ID──EX──MEM──WB
              │    │    │    │    │
Instr2:      IF──ID──EX──MEM──WB
                  │    │    │    │
Instr3:         IF──ID──EX──MEM──WB
```

#### Pipeline Stages:

1. **IF (Instruction Fetch)**
   - Fetch instruction from memory at PC
   - Increment PC

2. **ID (Instruction Decode)**
   - Decode instruction opcode
   - Read operands from register file
   - Generate control signals

3. **EX (Execute)**
   - ALU performs arithmetic/logic operations
   - Calculate memory addresses
   - Evaluate branch conditions
   - **Data Forwarding**: Bypass results from later stages

4. **MEM (Memory Access)**
   - Read/write data memory
   - Cache operations (hit/miss detection)

5. **WB (Write Back)**
   - Write results back to register file

#### Hazard Detection:

The system detects and reports:
- **RAW (Read After Write)**: Instruction reads register before previous write
- **WAW (Write After Write)**: Two instructions write to same register
- **WAR (Write After Read)**: Instruction writes to register being read
- **Load-Use**: LOAD followed immediately by instruction using loaded value

#### Data Forwarding (Bypassing):

Resolves RAW hazards by forwarding values from:
- EX/MEM latch (ALU result in progress)
- MEM/WB latch (Result being written back)

#### Cache Memory:

- **Type**: L1 Data Cache (Write-Through)
- **Size**: 128 bytes
- **Block Size**: 4 bytes (word)
- **Associativity**: 2-way set-associative
- **Replacement**: LRU (Least Recently Used)
- **Tracks**: Hit rate, Miss rate, Total accesses

## Demonstration Programs

### 1. Basic ALU Operations
Tests: ADD, SUB, AND, OR, NOT operations
```
MOV R0, 15
MOV R1, 10
ADD R2, R0, R1    → R2 = 25
SUB R3, R0, R1    → R3 = 5
```

### 2. Data Transfer Operations
Tests: MOV, LOAD, STORE
```
MOV R0, 42
STORE R0, [20]    → MEM[20] = 42
LOAD R2, [20]     → R2 = 42
```

### 3. Control Flow Operations
Tests: JMP, BEQ
```
MOV R0, 50
MOV R1, 50
BEQ R0, R1, 7     → Branch if equal
```

### 4. Data Hazard Scenario
Demonstrates RAW hazard and forwarding
```
MOV R0, 10
ADD R2, R0, R1
SUB R3, R2, R0    → R2 not yet available → Hazard!
```

### 5. Load-Use Hazard
LOAD followed by immediate use (cannot be fully resolved)
```
LOAD R1, [15]
ADD R2, R1, R1    → R1 from memory not ready → Stall needed
```

### 6. Complex Program
Combines all instruction types

### 7. Pipeline Demonstration
Shows instruction progress through all 5 stages

## Compilation and Execution

### Basic CPU Simulator

```bash
# Compile
g++ -std=c++17 -o cpu_sim cpu_simulator.cpp

# Run
./cpu_sim
```

Output:
- Cycle-by-cycle execution log
- Register file state after completion
- Memory contents (non-zero locations)

### Enhanced CPU Simulator (with Pipeline & Cache)

```bash
# Compile
g++ -std=c++17 -o enhanced_cpu enhanced_cpu_simulator.cpp

# Run
./enhanced_cpu
```

Output:
- Pipeline stage progression for each cycle
- Hazard detection information
- Forwarding path details
- Cache statistics (hit rate, miss rate)
- Final register file and memory state

### Windows Compilation:
```cmd
g++ -std=c++17 -o enhanced_cpu.exe enhanced_cpu_simulator.cpp
enhanced_cpu.exe
```

## Performance Metrics

### Basic CPU
- **CPI (Cycles Per Instruction)**: 1.0 (single-cycle per instruction for non-memory ops)
- **No hazard handling**: Sequential execution only

### Enhanced CPU with Pipeline
- **CPI (best case)**: 1.0 (5 instructions in flight simultaneously after startup)
- **CPI (with hazards)**: 1+ (depends on hazards and forwarding effectiveness)
- **Cache Performance**: Tracked hit rate and miss rate

### Example Metrics from Complex Program:
```
Total Accesses : 20
Hits           : 15
Misses         : 5
Hit Rate       : 75.00%
Miss Rate      : 25.00%
```

## Key Concepts Demonstrated

### 1. Instruction Set Architecture (ISA)
- Fixed instruction format
- Opcode-based decoding
- Operand types (register, immediate, memory address)

### 2. CPU Organization
- ALU for computation
- Register file for storage
- Control unit for signal generation
- Data path coordination

### 3. Pipelining
- Instruction overlap
- Multiple stages processing simultaneously
- Pipeline depth = 5 stages

### 4. Hazards and Solutions
- **RAW Hazard**: Resolved by data forwarding/bypassing
- **Load-Use Hazard**: Requires pipeline stall
- **Control Hazard**: Branch prediction (simplified - no speculation)

### 5. Memory Hierarchy
- Cache reduces memory access time
- Hit/miss detection
- LRU replacement policy

## Extending the Project

### Possible Enhancements:
1. **Superscalar Execution**: Decode/execute multiple instructions per cycle
2. **Branch Prediction**: Speculative execution with prediction
3. **Multiple Cache Levels**: L1, L2, L3 cache hierarchy
4. **Out-of-Order Execution**: ReOrder Buffer (ROB)
5. **VLIW (Very Long Instruction Word)**: Multiple operations per instruction
6. **Floating-Point Unit (FPU)**: Support for FP operations
7. **Interrupts and Exceptions**: Trap handling
8. **Virtual Memory**: TLB, page tables

## Testing and Validation

### Run All Demos:
```cpp
// Modify main() in enhanced_cpu_simulator.cpp:
vector<Instruction> programs[] = {
    DemoPrograms::basicALUOperations(),
    DemoPrograms::dataTransferOps(),
    DemoPrograms::controlFlowOps(),
    DemoPrograms::dataHazardScenario(),
    DemoPrograms::loadUseHazard(),
    DemoPrograms::complexProgram(),
    DemoPrograms::pipelineDemo()
};

for (int i = 0; i < 7; i++) {
    cout << "\n\n========== DEMO " << (i+1) << " ==========\n";
    EnhancedCPU cpu;
    cpu.loadProgram(programs[i]);
    cpu.run();
}
```

## References

Key computer architecture topics:
- **Pipelining**: Hennessy & Patterson - Computer Architecture 6th Edition
- **Hazards**: Stallings - Computer Organization and Architecture
- **Cache**: Tanenbaum - Structured Computer Organization
- **ISA Design**: RISC-V Specification

## Project Completion Checklist

- ✅ CPU Organization (ALU, CU, Registers)
- ✅ Instruction Set Architecture (11 instructions)
- ✅ 5-Stage Pipelining
- ✅ Hazard Detection (RAW, WAW, WAR, Load-Use)
- ✅ Data Forwarding/Bypassing
- ✅ Cache Memory (2-way associative, LRU)
- ✅ Comprehensive Documentation
- ✅ Multiple Demonstration Programs
- ✅ Performance Metrics (CPI, Cache Stats)

## Author Notes

This implementation focuses on **educational clarity** over production efficiency:
- Clear separation of concerns (pipeline stages, hazard detection, etc.)
- Detailed console logging for visualization
- Modular design for easy extension
- Emphasis on understanding over optimization

---

**Compilation Command (All Features):**
```bash
g++ -std=c++17 -Wall -O2 -o enhanced_cpu enhanced_cpu_simulator.cpp
./enhanced_cpu
```

**Submission Includes:**
- cpu_simulator.cpp (basic version)
- enhanced_cpu_simulator.cpp (with pipeline & cache)
- cache_memory.h (cache module)
- pipeline.h (pipeline structures & units)
- demo_programs.h (7 demonstration programs)
- README.md (this file)

## Author
**Rehmat Ali**  
Computer Architecture - CS 361
Submitted: Monday, 13th May
