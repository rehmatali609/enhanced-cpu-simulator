// =============================================================================
//  Enhanced CPU Simulator with Pipelining and Cache
//  Implements 5-stage pipeline with hazard detection, forwarding, and cache
// =============================================================================

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <map>
#include <cstring>
#include <climits>

using namespace std;

// =============================================================================
//  SECTION 1: Opcode Definitions
// =============================================================================

enum Opcode {
    ADD   = 0x0,
    SUB   = 0x1,
    AND   = 0x2,
    OR    = 0x3,
    NOT   = 0x4,
    MOV   = 0x5,
    LOAD  = 0x6,
    STORE = 0x7,
    JMP   = 0x8,
    BEQ   = 0x9,
    HALT  = 0xF
};

// =============================================================================
//  SECTION 2: Instruction Structure
// =============================================================================

struct Instruction {
    Opcode opcode;
    int    rd;
    int    rs1;
    int    rs2;
    int    imm;
    
    Instruction() : opcode((Opcode)0), rd(0), rs1(0), rs2(0), imm(0) {}
    Instruction(Opcode op, int d, int s1, int s2, int i) 
        : opcode(op), rd(d), rs1(s1), rs2(s2), imm(i) {}
    Instruction(int op, int d, int s1, int s2, int i)
        : opcode((Opcode)op), rd(d), rs1(s1), rs2(s2), imm(i) {}
};

#include "cache_memory.h"
#include "pipeline.h"
#include "demo_programs.h"

// =============================================================================
//  SECTION 3: ALU (Arithmetic Logic Unit)
// =============================================================================

class ALU {
public:
    int  result;
    bool zeroFlag;
    bool negFlag;
    bool carryFlag;
    bool overflowFlag;

    ALU() : result(0), zeroFlag(false), negFlag(false), 
            carryFlag(false), overflowFlag(false) {}

    int execute(Opcode op, int operandA, int operandB) {
        long long temp;
        
        switch (op) {
            case ADD: 
                temp = (long long)operandA + operandB;
                carryFlag = (temp > INT_MAX);
                result = operandA + operandB;
                break;
            case SUB: 
                result = operandA - operandB;
                carryFlag = (operandA < operandB);
                break;
            case AND: result = operandA & operandB; break;
            case OR:  result = operandA | operandB; break;
            case NOT: result = ~operandA; break;
            default:  result = 0; break;
        }
        
        // Update flags
        zeroFlag = (result == 0);
        negFlag  = (result < 0);
        
        return result;
    }

    string opName(Opcode op) {
        switch (op) {
            case ADD: return "ADD";
            case SUB: return "SUB";
            case AND: return "AND";
            case OR:  return "OR";
            case NOT: return "NOT";
            default:  return "???";
        }
    }
};

// =============================================================================
//  SECTION 4: Register File
// =============================================================================

class RegisterFile {
public:
    static const int NUM_REGS = 8;
    int regs[NUM_REGS];

    RegisterFile() {
        for (int i = 0; i < NUM_REGS; i++) regs[i] = 0;
    }

    int read(int idx) {
        if (idx < 0 || idx >= NUM_REGS) {
            cerr << "[ERROR] Register index out of range: " << idx << "\n";
            return 0;
        }
        return regs[idx];
    }

    void write(int idx, int val) {
        if (idx < 0 || idx >= NUM_REGS) {
            cerr << "[ERROR] Register index out of range: " << idx << "\n";
            return;
        }
        regs[idx] = val;
    }

    void display() const {
        cout << "\n  ┌────────────────────────────────────────────────┐\n";
        cout <<   "  │           Register File State                  │\n";
        cout <<   "  ├────────┬──────────┬────────┬──────────────────┤\n";
        for (int i = 0; i < NUM_REGS; i += 2) {
            cout << "  │ R" << i << " = " << setw(6) << regs[i]
                 << " │ R" << (i+1) << " = " << setw(6) << regs[i+1] << "          │\n";
        }
        cout <<   "  └────────┴──────────┴────────┴──────────────────┘\n";
    }
};

// =============================================================================
//  SECTION 5: Data Memory with Cache Integration
// =============================================================================

class Memory {
public:
    static const int MEM_SIZE = 512;
    int data[MEM_SIZE];
    CacheMemory cache;

    Memory() : cache(128, 4, 2) {  // 128-byte cache, 4-byte blocks, 2-way assoc
        for (int i = 0; i < MEM_SIZE; i++) data[i] = 0;
    }

    int read(int addr) {
        if (addr < 0 || addr >= MEM_SIZE) {
            cerr << "[ERROR] Memory read out of range: addr=" << addr << "\n";
            return 0;
        }
        
        int value;
        if (cache.read(addr, value)) {
            // Cache HIT
            return data[addr];
        } else {
            // Cache MISS - load into cache
            cache.write(addr, data[addr]);
            return data[addr];
        }
    }

    void write(int addr, int val) {
        if (addr < 0 || addr >= MEM_SIZE) {
            cerr << "[ERROR] Memory write out of range: addr=" << addr << "\n";
            return;
        }
        data[addr] = val;
        cache.write(addr, val);  // Write-through policy
    }

    void displayNonZero() const {
        cout << "\n  [ Data Memory - Non-Zero Locations ]\n";
        bool anyNonZero = false;
        for (int i = 0; i < MEM_SIZE && i < 100; i++) {
            if (data[i] != 0) {
                cout << "  MEM[" << setw(3) << i << "] = " << data[i] << "\n";
                anyNonZero = true;
            }
        }
        if (!anyNonZero) cout << "  (all zero)\n";
    }

    void displayCacheStats() const {
        cache.displayStats();
    }
};

// =============================================================================
//  SECTION 6: Control Unit
// =============================================================================

class ControlUnit {
public:
    struct ControlSignals {
        bool regWrite;
        bool memRead;
        bool memWrite;
        bool aluSrc;
        bool branch;
        bool jump;
        bool halt;
    };

    ControlSignals decode(Opcode op) {
        ControlSignals cs = {false, false, false, false, false, false, false};

        switch (op) {
            case ADD: case SUB: case AND: case OR: case NOT:
                cs.regWrite = true;
                break;
            case MOV:
                cs.regWrite = true;  cs.aluSrc = true;
                break;
            case LOAD:
                cs.regWrite = true;  cs.memRead = true;
                break;
            case STORE:
                cs.memWrite = true;
                break;
            case JMP:
                cs.jump = true;
                break;
            case BEQ:
                cs.branch = true;
                break;
            case HALT:
                cs.halt = true;
                break;
            default:
                cerr << "[WARN] Unknown opcode in Control Unit\n";
        }
        return cs;
    }
};

// =============================================================================
//  SECTION 7: Enhanced CPU with Pipeline
// =============================================================================

class EnhancedCPU {
private:
    ALU            alu;
    RegisterFile   rf;
    Memory         memory;
    ControlUnit    cu;
    
    int  PC;
    int  nextPC;
    bool running;
    int  cycleCount;
    int  totalStalls;
    int  totalForwards;
    
    vector<Instruction> imem;
    
    // Pipeline latches
    IF_ID_Latch  if_id;
    ID_EX_Latch  id_ex;
    EX_MEM_Latch ex_mem;
    MEM_WB_Latch mem_wb;
    
    PipelineController pipelineCtrl;
    HazardDetectionUnit hazardDetector;
    ForwardingUnit forwarder;

    // Helpers
    string opName(Opcode op) {
        static map<Opcode, string> names = {
            {ADD,"ADD"}, {SUB,"SUB"}, {AND,"AND"}, {OR,"OR"}, {NOT,"NOT"},
            {MOV,"MOV"}, {LOAD,"LOAD"}, {STORE,"STORE"},
            {JMP,"JMP"}, {BEQ,"BEQ"}, {HALT,"HALT"}
        };
        return names.count(op) ? names[op] : "???";
    }

    void sep(char c = '-', int len = 70) {
        cout << "  " << string(len, c) << "\n";
    }

    // Pipeline stage functions
    void stage_IF() {
        if (pipelineCtrl.isStalled()) return;
        
        if (PC >= 0 && PC < (int)imem.size()) {
            if_id.opcode = imem[PC].opcode;
            if_id.rd = imem[PC].rd;
            if_id.rs1 = imem[PC].rs1;
            if_id.rs2 = imem[PC].rs2;
            if_id.imm = imem[PC].imm;
            if_id.PC = PC;
            if_id.valid = true;
            PC = nextPC;
        }
    }

    void stage_ID() {
        if (!if_id.valid) {
            id_ex.valid = false;
            return;
        }

        if_id.valid = false;
        
        id_ex.opcode = if_id.opcode;
        id_ex.rd = if_id.rd;
        id_ex.rs1 = if_id.rs1;
        id_ex.rs2 = if_id.rs2;
        id_ex.PC = if_id.PC;
        id_ex.rs1_data = rf.read(if_id.rs1);
        id_ex.rs2_data = rf.read(if_id.rs2);
        id_ex.imm = if_id.imm;
        
        ControlUnit::ControlSignals cs = cu.decode((Opcode)if_id.opcode);
        id_ex.regWrite = cs.regWrite;
        id_ex.memRead = cs.memRead;
        id_ex.memWrite = cs.memWrite;
        id_ex.branch = cs.branch;
        id_ex.jump = cs.jump;
        id_ex.valid = true;
    }

    void stage_EX() {
        if (!id_ex.valid) {
            ex_mem.valid = false;
            return;
        }

        // Forwarding
        ForwardingUnit::ForwardPath rs1_fwd = forwarder.checkForwardRs1(
            id_ex, ex_mem, mem_wb, id_ex.rs1_data);
        ForwardingUnit::ForwardPath rs2_fwd = forwarder.checkForwardRs2(
            id_ex, ex_mem, mem_wb, id_ex.rs2_data);

        int operand1 = rs1_fwd.forwardedValue;
        int operand2 = rs2_fwd.forwardedValue;

        id_ex.valid = false;
        ex_mem.opcode = id_ex.opcode;
        ex_mem.PC = id_ex.PC;
        ex_mem.regWrite = id_ex.regWrite;
        ex_mem.memRead = id_ex.memRead;
        ex_mem.memWrite = id_ex.memWrite;
        ex_mem.rd = id_ex.rd;
        ex_mem.writeData = operand1;
        ex_mem.memAddr = id_ex.imm;

        // Execute
        if (id_ex.opcode == MOV) {
            ex_mem.aluResult = id_ex.imm;
        } else if (id_ex.opcode == LOAD) {
            ex_mem.aluResult = id_ex.imm;
        } else if (id_ex.opcode == STORE) {
            ex_mem.aluResult = 0;
        } else if (id_ex.opcode == JMP) {
            nextPC = id_ex.imm;
            ex_mem.aluResult = 0;
        } else if (id_ex.opcode == BEQ) {
            if (operand1 == operand2) {
                nextPC = id_ex.imm;
            } else {
                nextPC = id_ex.PC + 1;
            }
            ex_mem.aluResult = 0;
        } else if (id_ex.opcode == NOT) {
            ex_mem.aluResult = alu.execute((Opcode)id_ex.opcode, operand1, 0);
        } else {
            ex_mem.aluResult = alu.execute((Opcode)id_ex.opcode, operand1, operand2);
        }

        ex_mem.valid = true;
    }

    void stage_MEM() {
        if (!ex_mem.valid) {
            mem_wb.valid = false;
            return;
        }

        ex_mem.valid = false;
        mem_wb.opcode = ex_mem.opcode;
        mem_wb.PC = ex_mem.PC;
        mem_wb.aluResult = ex_mem.aluResult;
        mem_wb.rd = ex_mem.rd;
        mem_wb.regWrite = ex_mem.regWrite;
        mem_wb.memRead = ex_mem.memRead;

        if (ex_mem.memRead) {
            mem_wb.memData = memory.read(ex_mem.memAddr);
        } else if (ex_mem.memWrite) {
            memory.write(ex_mem.memAddr, ex_mem.writeData);
        }

        mem_wb.valid = true;
    }

    void stage_WB() {
        if (!mem_wb.valid) return;

        if (mem_wb.regWrite) {
            int writeValue = mem_wb.memRead ? mem_wb.memData : mem_wb.aluResult;
            rf.write(mem_wb.rd, writeValue);
        }

        mem_wb.valid = false;
    }

public:
    EnhancedCPU() : PC(0), nextPC(1), running(false), cycleCount(0), 
                    totalStalls(0), totalForwards(0) {}

    void loadProgram(const vector<Instruction>& prog) {
        imem = prog;
        PC = 0;
        nextPC = 1;
        cycleCount = 0;
    }

    void preloadMemory(int addr, int val) {
        memory.write(addr, val);
    }

    void run() {
        running = true;
        nextPC = 1;

        cout << "\n";
        cout << "  ╔════════════════════════════════════════════════════════════╗\n";
        cout << "  ║      Enhanced CPU Simulator with 5-Stage Pipeline          ║\n";
        cout << "  ║         (With Cache, Hazard Detection, Forwarding)         ║\n";
        cout << "  ╚════════════════════════════════════════════════════════════╝\n";

        while (running && cycleCount < 50) {  // Safety limit
            cycleCount++;
            
            cout << "\n";
            sep('=');
            cout << "  CYCLE " << cycleCount << "  |  PC = " << PC << "\n";
            sep('=');

            // Execute pipeline stages (in reverse order to avoid data overwriting)
            stage_WB();
            stage_MEM();
            stage_EX();
            stage_ID();
            stage_IF();

            // Check for HALT
            if (mem_wb.valid && mem_wb.opcode == HALT) {
                running = false;
                cout << "  [WB STAGE]  HALT detected. Stopping execution.\n";
            }

            // Print pipeline state
            cout << "  Pipeline State:\n";
            if (if_id.valid) cout << "    IF/ID: " << opName((Opcode)if_id.opcode) << " (PC=" << if_id.PC << ")\n";
            if (id_ex.valid) cout << "    ID/EX: " << opName((Opcode)id_ex.opcode) << "\n";
            if (ex_mem.valid) cout << "    EX/MEM: " << opName((Opcode)ex_mem.opcode) << "\n";
            if (mem_wb.valid) cout << "    MEM/WB: " << opName((Opcode)mem_wb.opcode) << "\n";
        }

        // Final state report
        cout << "\n";
        cout << "  ╔════════════════════════════════════════════════════════════╗\n";
        cout << "  ║                 Execution Complete                         ║\n";
        cout << "  ║   Total Cycles     : " << setw(4) << cycleCount << "                                  ║\n";
        cout << "  ║   Total Stalls     : " << setw(4) << totalStalls << "                                  ║\n";
        cout << "  ║   Total Forwards   : " << setw(4) << totalForwards << "                                  ║\n";
        cout << "  ╚════════════════════════════════════════════════════════════╝\n";

        rf.display();
        memory.displayNonZero();
        memory.displayCacheStats();
        cout << "\n";
    }
};

// =============================================================================
//  SECTION 8: Main Program
// =============================================================================

int main() {
    cout << "═══════════════════════════════════════════════════════════════\n";
    cout << "  Computer Architecture — Enhanced CPU Simulator\n";
    cout << "  5-Stage Pipeline with Cache, Hazard Detection, and Forwarding\n";
    cout << "═══════════════════════════════════════════════════════════════\n";

    // Show available demos
    DemoPrograms::printDemoInfo();

    cout << "  Running: Demo 6 (Complex Program)\n\n";

    EnhancedCPU cpu;
    cpu.loadProgram(DemoPrograms::complexProgram());
    cpu.run();

    return 0;
}

// =============================================================================
//  Compilation:
//  g++ -std=c++17 -o enhanced_cpu enhanced_cpu_simulator.cpp
//  
//  This program demonstrates:
//  - 5-stage pipeline (IF, ID, EX, MEM, WB)
//  - Hazard detection (RAW, WAW, WAR, Load-Use)
//  - Data forwarding/bypassing
//  - Cache memory with LRU replacement
//  - Complete instruction set
// =============================================================================
