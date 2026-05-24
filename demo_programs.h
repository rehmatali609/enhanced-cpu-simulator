// =============================================================================
//  CPU Simulator Demonstration Programs
//  Shows various CPU features: basic instructions, pipelining effects,
//  hazard detection, forwarding, and cache behavior
// =============================================================================

#ifndef DEMO_PROGRAMS_H
#define DEMO_PROGRAMS_H

#include <iostream>
#include <vector>
#include <string>

using namespace std;

// Note: enum Opcode and struct Instruction must be defined before including this header

// ─────────────────────────────────────────
//  Demo Programs
// ─────────────────────────────────────────

class DemoPrograms {
public:

    // Demo 1: Basic ALU Operations
    // Tests: ADD, SUB, AND, OR, NOT
    static vector<Instruction> basicALUOperations() {
        return {
            // opcode   rd  rs1  rs2  imm
            {0x5,  0,   0,   0,   15},    // MOV R0, 15
            {0x5,  1,   0,   0,   10},    // MOV R1, 10
            {0x0,  2,   0,   1,    0},    // ADD R2, R0, R1    → R2 = 25
            {0x1,  3,   0,   1,    0},    // SUB R3, R0, R1    → R3 = 5
            {0x2,  4,   0,   1,    0},    // AND R4, R0, R1    → R4 = 10
            {0x3,  5,   0,   1,    0},    // OR  R5, R0, R1    → R5 = 15
            {0x4,  6,   0,   0,    0},    // NOT R6, R0        → R6 = ~15
            {0xF,  0,   0,   0,    0}     // HALT
        };
    }

    // Demo 2: Data Transfer (LOAD, STORE)
    // Tests: MOV, STORE, LOAD
    static vector<Instruction> dataTransferOps() {
        return {
            // opcode   rd  rs1  rs2  imm
            {0x5,  0,   0,   0,   42},    // MOV R0, 42
            {0x5,  1,   0,   0,   99},    // MOV R1, 99
            {0x7,  0,   0,   0,   20},    // STORE R0, [20]    → MEM[20] = 42
            {0x7,  0,   1,   0,   30},    // STORE R1, [30]    → MEM[30] = 99
            {0x6,  2,   0,   0,   20},    // LOAD  R2, [20]    → R2 = 42
            {0x6,  3,   0,   0,   30},    // LOAD  R3, [30]    → R3 = 99
            {0x0,  4,   2,   3,    0},    // ADD  R4, R2, R3   → R4 = 141
            {0xF,  0,   0,   0,    0}     // HALT
        };
    }

    // Demo 3: Control Flow (JMP, BEQ)
    // Tests: Unconditional and conditional jumps
    static vector<Instruction> controlFlowOps() {
        return {
            // opcode   rd  rs1  rs2  imm
            {0x5,  0,   0,   0,   50},    // MOV R0, 50
            {0x5,  1,   0,   0,   50},    // MOV R1, 50
            {0x9,  0,   0,   1,    7},    // BEQ R0, R1, 7     → Equal, jump to 7
            {0x5,  2,   0,   0,    1},    // MOV R2, 1         (skipped if branch taken)
            {0xF,  0,   0,   0,    0},    // HALT (not reached)
            {0x0,  0,   0,   0,    0},    // Padding
            {0x0,  0,   0,   0,    0},    // Padding
            {0x5,  2,   0,   0,    2},    // MOV R2, 2         (jumped here)
            {0xF,  0,   0,   0,    0}     // HALT
        };
    }

    // Demo 4: Data Hazard Scenario (demonstrates forwarding need)
    // Tests: RAW hazard and forwarding
    static vector<Instruction> dataHazardScenario() {
        return {
            // opcode   rd  rs1  rs2  imm
            {0x5,  0,   0,   0,   10},    // MOV R0, 10        (Cycle 1)
            {0x5,  1,   0,   0,   20},    // MOV R1, 20        (Cycle 2)
            {0x0,  2,   0,   1,    0},    // ADD R2, R0, R1    (Cycle 3)
            {0x1,  3,   2,   0,    0},    // SUB R3, R2, R0    (Cycle 4) - R2 not ready! RAW hazard
            {0x0,  4,   3,   1,    0},    // ADD R4, R3, R1    (Cycle 5) - Another dependent instruction
            {0xF,  0,   0,   0,    0}     // HALT
        };
    }

    // Demo 5: Load-Use Hazard (cannot be fully resolved by forwarding)
    // Tests: LOAD followed by immediate use
    static vector<Instruction> loadUseHazard() {
        return {
            // opcode   rd  rs1  rs2  imm
            {0x5,  0,   0,   0,   55},    // MOV R0, 55        → R0 = 55
            {0x7,  0,   0,   0,   15},    // STORE R0, [15]    → MEM[15] = 55
            {0x6,  1,   0,   0,   15},    // LOAD R1, [15]     → R1 = 55 (from mem)
            {0x0,  2,   1,   1,    0},    // ADD R2, R1, R1    → R2 = 110 (depends on R1 from LOAD)
            {0x1,  3,   2,   0,    0},    // SUB R3, R2, R0    → R3 = 55
            {0xF,  0,   0,   0,    0}     // HALT
        };
    }

    // Demo 6: Complex Program (combination of all features)
    // Tests: All instruction types together
    static vector<Instruction> complexProgram() {
        return {
            // opcode   rd  rs1  rs2  imm
            {0x5,  0,   0,   0,   100},   // MOV R0, 100
            {0x5,  1,   0,   0,   50},    // MOV R1, 50
            {0x0,  2,   0,   1,    0},    // ADD R2, R0, R1    → R2 = 150
            {0x1,  3,   0,   1,    0},    // SUB R3, R0, R1    → R3 = 50
            {0x2,  4,   2,   3,    0},    // AND R4, R2, R3    → R4 = 32
            {0x3,  5,   2,   3,    0},    // OR  R5, R2, R3    → R5 = 166
            {0x7,  0,   2,   0,   40},    // STORE R2, [40]    → MEM[40] = 150
            {0x7,  0,   5,   0,   45},    // STORE R5, [45]    → MEM[45] = 166
            {0x6,  6,   0,   0,   40},    // LOAD  R6, [40]    → R6 = 150
            {0x6,  7,   0,   0,   45},    // LOAD  R7, [45]    → R7 = 166
            {0x9,  0,   6,   0,   15},    // BEQ R6, R0, 15    → Not equal, don't branch
            {0x0,  0,   0,   0,    0},    // ADD (padding)
            {0x0,  0,   0,   0,    0},    // ADD (padding)
            {0x0,  0,   0,   0,    0},    // ADD (padding)
            {0xF,  0,   0,   0,    0},    // HALT
            {0xF,  0,   0,   0,    0}     // HALT (jump target)
        };
    }

    // Demo 7: Pipelining Demonstration
    // Shows how instructions progress through pipeline stages
    static vector<Instruction> pipelineDemo() {
        return {
            // opcode   rd  rs1  rs2  imm
            {0x5,  0,   0,   0,   11},    // MOV R0, 11        Cycle 1-5
            {0x5,  1,   0,   0,   22},    // MOV R1, 22        Cycle 2-6
            {0x0,  2,   0,   1,    0},    // ADD R2, R0, R1    Cycle 3-7
            {0x1,  3,   2,   0,    0},    // SUB R3, R2, R0    Cycle 4-8 (depends on R2)
            {0x3,  4,   3,   1,    0},    // OR  R4, R3, R1    Cycle 5-9 (depends on R3)
            {0xF,  0,   0,   0,    0}     // HALT             Cycle 6-10
        };
    }

    // Get demo info
    static void printDemoInfo() {
        cout << "\n  Available Demo Programs:\n";
        cout << "  ───────────────────────────────────────────────────\n";
        cout << "  1. Basic ALU Operations      - ADD, SUB, AND, OR, NOT\n";
        cout << "  2. Data Transfer Operations  - LOAD, STORE, MOV\n";
        cout << "  3. Control Flow Operations   - JMP, BEQ\n";
        cout << "  4. Data Hazard Scenario      - RAW hazard + Forwarding\n";
        cout << "  5. Load-Use Hazard Scenario  - Cannot be fully bypassed\n";
        cout << "  6. Complex Program           - All features combined\n";
        cout << "  7. Pipeline Demonstration    - Multi-cycle instruction flow\n";
        cout << "  ───────────────────────────────────────────────────\n\n";
    }
};

#endif // DEMO_PROGRAMS_H
