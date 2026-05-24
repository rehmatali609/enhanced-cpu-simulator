// =============================================================================
//  Simple CPU Simulator
//  Author  : [Your Name]
//  Course  : Computer Architecture Lab
//  Project : CPU Simulator demonstrating ALU, Registers, Control Unit & ISA
// =============================================================================
//
//  Architecture Overview:
//  ┌─────────────────────────────────────────────┐
//  │                    CPU                      │
//  │  ┌──────────┐  ┌──────────┐  ┌──────────┐  │
//  │  │  Control │  │Register  │  │   ALU    │  │
//  │  │   Unit   │  │  File    │  │          │  │
//  │  │ (Decode) │  │ R0 – R7  │  │+,-,&,|,~│  │
//  │  └──────────┘  └──────────┘  └──────────┘  │
//  │        ↑             ↑              ↑        │
//  │  ┌─────────────────────────────────────┐    │
//  │  │          Data / Control Bus         │    │
//  │  └─────────────────────────────────────┘    │
//  │        ↑                        ↑           │
//  │  ┌──────────┐            ┌──────────┐       │
//  │  │Instruction│            │  Data   │       │
//  │  │  Memory  │            │  Memory │       │
//  │  └──────────┘            └──────────┘       │
//  └─────────────────────────────────────────────┘
//
//  Instruction Set (11 Instructions):
//  ─────────────────────────────────────────────
//   Opcode  Mnemonic  Operation
//   0x0     ADD       Rd = Rs1 + Rs2
//   0x1     SUB       Rd = Rs1 - Rs2
//   0x2     AND       Rd = Rs1 & Rs2
//   0x3     OR        Rd = Rs1 | Rs2
//   0x4     NOT       Rd = ~Rs1
//   0x5     MOV       Rd = Immediate
//   0x6     LOAD      Rd = MEM[addr]
//   0x7     STORE     MEM[addr] = Rs1
//   0x8     JMP       PC = addr
//   0x9     BEQ       if Rs1==Rs2: PC = addr
//   0xF     HALT      Stop execution
// =============================================================================

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <map>
#include <sstream>

using namespace std;

// =============================================================================
//  SECTION 1: Opcode Definitions (Instruction Set Architecture)
// =============================================================================

enum Opcode {
    ADD   = 0x0,   // Arithmetic: Add
    SUB   = 0x1,   // Arithmetic: Subtract
    AND   = 0x2,   // Logic: Bitwise AND
    OR    = 0x3,   // Logic: Bitwise OR
    NOT   = 0x4,   // Logic: Bitwise NOT (unary)
    MOV   = 0x5,   // Data Transfer: Move Immediate to Register
    LOAD  = 0x6,   // Data Transfer: Load from Memory
    STORE = 0x7,   // Data Transfer: Store to Memory
    JMP   = 0x8,   // Control Flow: Unconditional Jump
    BEQ   = 0x9,   // Control Flow: Branch if Equal
    HALT  = 0xF    // Control Flow: Halt execution
};

// =============================================================================
//  SECTION 2: Instruction Structure
// =============================================================================

struct Instruction {
    Opcode opcode;  // Operation code
    int    rd;      // Destination register index (0-7)
    int    rs1;     // Source register 1 index (0-7)
    int    rs2;     // Source register 2 index (0-7)
    int    imm;     // Immediate value OR memory address
};

// =============================================================================
//  SECTION 3: ALU (Arithmetic Logic Unit)
//  Performs all arithmetic and logical computations.
// =============================================================================

class ALU {
public:
    int  result;    // Result of last operation
    bool zeroFlag;  // Set when result == 0
    bool negFlag;   // Set when result < 0

    // Execute ALU operation and update flags
    int execute(Opcode op, int operandA, int operandB) {
        switch (op) {
            case ADD: result = operandA + operandB; break;
            case SUB: result = operandA - operandB; break;
            case AND: result = operandA & operandB; break;
            case OR:  result = operandA | operandB; break;
            case NOT: result = ~operandA;            break;
            default:  result = 0;                   break;
        }
        // Update condition flags
        zeroFlag = (result == 0);
        negFlag  = (result < 0);
        return result;
    }

    // Return a string name for the operation (for logging)
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
//  8 general-purpose registers: R0 through R7
//  R0 is hardwired to 0 (read-only zero register convention, optional)
// =============================================================================

class RegisterFile {
public:
    static const int NUM_REGS = 8;
    int regs[NUM_REGS];

    // Initialize all registers to 0
    RegisterFile() {
        for (int i = 0; i < NUM_REGS; i++) regs[i] = 0;
    }

    // Read value from register
    int read(int idx) {
        if (idx < 0 || idx >= NUM_REGS) {
            cerr << "[ERROR] Register index out of range: " << idx << "\n";
            return 0;
        }
        return regs[idx];
    }

    // Write value to register
    void write(int idx, int val) {
        if (idx < 0 || idx >= NUM_REGS) {
            cerr << "[ERROR] Register index out of range: " << idx << "\n";
            return;
        }
        regs[idx] = val;
    }

    // Display all register values in a formatted table
    void display() const {
        cout << "\n  ┌──────────────────────────────────────┐\n";
        cout <<   "  │           Register File State         │\n";
        cout <<   "  ├────────┬────────┬────────┬────────────┤\n";
        for (int i = 0; i < NUM_REGS; i += 2) {
            cout << "  │ R" << i << " = " << setw(4) << regs[i]
                 << " │ R" << (i+1) << " = " << setw(4) << regs[i+1] << "          │\n";
        }
        cout <<   "  └────────┴────────┴────────┴────────────┘\n";
    }
};

// =============================================================================
//  SECTION 5: Data Memory
//  256-word addressable data memory (word-addressable, 32-bit words)
// =============================================================================

class Memory {
public:
    static const int MEM_SIZE = 256;
    int data[MEM_SIZE];

    // Initialize all memory locations to 0
    Memory() {
        for (int i = 0; i < MEM_SIZE; i++) data[i] = 0;
    }

    // Read a word from memory
    int read(int addr) {
        if (addr < 0 || addr >= MEM_SIZE) {
            cerr << "[ERROR] Memory read out of range: addr=" << addr << "\n";
            return 0;
        }
        return data[addr];
    }

    // Write a word to memory
    void write(int addr, int val) {
        if (addr < 0 || addr >= MEM_SIZE) {
            cerr << "[ERROR] Memory write out of range: addr=" << addr << "\n";
            return;
        }
        data[addr] = val;
    }

    // Display non-zero memory locations
    void displayNonZero() const {
        cout << "\n  [ Data Memory - Non-Zero Locations ]\n";
        bool anyNonZero = false;
        for (int i = 0; i < MEM_SIZE; i++) {
            if (data[i] != 0) {
                cout << "  MEM[" << setw(3) << i << "] = " << data[i] << "\n";
                anyNonZero = true;
            }
        }
        if (!anyNonZero) cout << "  (all zero)\n";
    }
};

// =============================================================================
//  SECTION 6: Control Unit
//  Decodes the opcode and generates control signals for the datapath.
// =============================================================================

class ControlUnit {
public:
    // Control signals that drive the datapath components
    struct ControlSignals {
        bool regWrite;   // Write result back to register file
        bool memRead;    // Read from data memory
        bool memWrite;   // Write to data memory
        bool aluSrc;     // ALU second operand from immediate (vs register)
        bool branch;     // Conditional branch instruction
        bool jump;       // Unconditional jump instruction
        bool halt;       // Stop the processor
    };

    // Decode opcode → produce control signals
    ControlSignals decode(Opcode op) {
        ControlSignals cs = {false, false, false, false, false, false, false};

        switch (op) {
            case ADD: case SUB: case AND: case OR: case NOT:
                cs.regWrite = true;                              // R-type: write result
                break;
            case MOV:
                cs.regWrite = true;  cs.aluSrc = true;          // Immediate → register
                break;
            case LOAD:
                cs.regWrite = true;  cs.memRead = true;          // Memory → register
                break;
            case STORE:
                cs.memWrite = true;                              // Register → memory
                break;
            case JMP:
                cs.jump = true;                                  // Unconditional branch
                break;
            case BEQ:
                cs.branch = true;                                // Conditional branch
                break;
            case HALT:
                cs.halt = true;                                  // Stop
                break;
            default:
                cerr << "[WARN] Unknown opcode in Control Unit\n";
        }
        return cs;
    }
};

// =============================================================================
//  SECTION 7: CPU (Top-Level)
//  Ties together: PC, Instruction Memory, Control Unit, Register File, ALU,
//  and Data Memory — executing the Fetch → Decode → Execute cycle.
// =============================================================================

class CPU {
    ALU            alu;
    RegisterFile   rf;
    Memory         dmem;       // Data memory
    ControlUnit    cu;

    int  PC;                   // Program Counter
    bool running;
    int  cycleCount;

    vector<Instruction> imem;  // Instruction memory (program)

    // Helper: opcode → string
    string opName(Opcode op) {
        static map<Opcode, string> names = {
            {ADD,"ADD"}, {SUB,"SUB"}, {AND,"AND"}, {OR,"OR"}, {NOT,"NOT"},
            {MOV,"MOV"}, {LOAD,"LOAD"}, {STORE,"STORE"},
            {JMP,"JMP"}, {BEQ,"BEQ"}, {HALT,"HALT"}
        };
        return names.count(op) ? names[op] : "???";
    }

    // Print a horizontal separator
    void sep(char c = '-', int len = 60) {
        cout << "  " << string(len, c) << "\n";
    }

public:
    CPU() : PC(0), running(false), cycleCount(0) {}

    // Load a program into instruction memory
    void loadProgram(const vector<Instruction>& prog) {
        imem = prog;
        PC   = 0;
        cycleCount = 0;
    }

    // Pre-load a value into data memory (for LOAD tests)
    void preloadMemory(int addr, int val) {
        dmem.write(addr, val);
    }

    // ──────────────────────────────────────────
    //  MAIN EXECUTION LOOP: Fetch – Decode – Execute
    // ──────────────────────────────────────────
    void run() {
        running = true;

        cout << "\n";
        cout << "  ╔════════════════════════════════════════════════════════╗\n";
        cout << "  ║           Simple CPU Simulator — Execution Log         ║\n";
        cout << "  ╚════════════════════════════════════════════════════════╝\n";

        while (running && PC >= 0 && PC < (int)imem.size()) {

            // ── STAGE 1: FETCH ──────────────────────────────────────────
            Instruction instr = imem[PC];
            cycleCount++;

            cout << "\n";
            sep();
            cout << "  CYCLE " << cycleCount
                 << "  |  PC = " << PC
                 << "  |  Instruction: " << opName(instr.opcode) << "\n";
            sep();

            // ── STAGE 2: DECODE ─────────────────────────────────────────
            ControlUnit::ControlSignals cs = cu.decode(instr.opcode);
            cout << "  [DECODE]   RegWrite=" << cs.regWrite
                 << "  MemRead="  << cs.memRead
                 << "  MemWrite=" << cs.memWrite
                 << "  Branch="   << cs.branch
                 << "  Jump="     << cs.jump  << "\n";

            // ── STAGE 3: EXECUTE ─────────────────────────────────────────
            if (cs.halt) {
                // ─ HALT ─────────────────────────────────────────────────
                cout << "  [EXECUTE]  HALT — processor stopped.\n";
                running = false;
                break;

            } else if (instr.opcode == MOV) {
                // ─ MOV Rd, imm ──────────────────────────────────────────
                rf.write(instr.rd, instr.imm);
                cout << "  [EXECUTE]  MOV R" << instr.rd
                     << " ← " << instr.imm << "\n";
                PC++;

            } else if (instr.opcode == LOAD) {
                // ─ LOAD Rd, [addr] ──────────────────────────────────────
                int val = dmem.read(instr.imm);
                rf.write(instr.rd, val);
                cout << "  [EXECUTE]  LOAD R" << instr.rd
                     << " ← MEM[" << instr.imm << "] = " << val << "\n";
                PC++;

            } else if (instr.opcode == STORE) {
                // ─ STORE Rs1, [addr] ────────────────────────────────────
                int val = rf.read(instr.rs1);
                dmem.write(instr.imm, val);
                cout << "  [EXECUTE]  STORE MEM[" << instr.imm
                     << "] ← R" << instr.rs1 << " = " << val << "\n";
                PC++;

            } else if (instr.opcode == JMP) {
                // ─ JMP addr ─────────────────────────────────────────────
                cout << "  [EXECUTE]  JMP → PC = " << instr.imm << "\n";
                PC = instr.imm;

            } else if (instr.opcode == BEQ) {
                // ─ BEQ Rs1, Rs2, addr ───────────────────────────────────
                int a = rf.read(instr.rs1);
                int b = rf.read(instr.rs2);
                cout << "  [EXECUTE]  BEQ R" << instr.rs1
                     << "(" << a << ") == R" << instr.rs2 << "(" << b << ")? ";
                if (a == b) {
                    cout << "YES → Branch taken, PC = " << instr.imm << "\n";
                    PC = instr.imm;
                } else {
                    cout << "NO  → Branch NOT taken\n";
                    PC++;
                }

            } else if (instr.opcode == NOT) {
                // ─ NOT Rd, Rs1 (unary) ───────────────────────────────────
                int a = rf.read(instr.rs1);
                int res = alu.execute(instr.opcode, a, 0);
                rf.write(instr.rd, res);
                cout << "  [EXECUTE]  ALU: NOT " << a << " = " << res
                     << "  |  ZeroFlag=" << alu.zeroFlag
                     << "  NegFlag=" << alu.negFlag << "\n";
                cout << "  [WRITEBACK] R" << instr.rd << " ← " << res << "\n";
                PC++;

            } else {
                // ─ R-Type: ADD, SUB, AND, OR ─────────────────────────────
                int a   = rf.read(instr.rs1);
                int b   = rf.read(instr.rs2);
                int res = alu.execute(instr.opcode, a, b);
                rf.write(instr.rd, res);
                cout << "  [EXECUTE]  ALU: " << a << " " << opName(instr.opcode)
                     << " " << b << " = " << res
                     << "  |  ZeroFlag=" << alu.zeroFlag
                     << "  NegFlag=" << alu.negFlag << "\n";
                cout << "  [WRITEBACK] R" << instr.rd << " ← " << res << "\n";
                PC++;
            }
        }

        // ── FINAL STATE REPORT ──────────────────────────────────────────
        cout << "\n";
        cout << "  ╔════════════════════════════════════════════════════════╗\n";
        cout << "  ║                Execution Complete                      ║\n";
        cout << "  ║   Total Cycles : " << setw(4) << cycleCount
             << "                                  ║\n";
        cout << "  ║   Final PC     : " << setw(4) << PC
             << "                                  ║\n";
        cout << "  ╚════════════════════════════════════════════════════════╝\n";

        rf.display();
        dmem.displayNonZero();
        cout << "\n";
    }
};

// =============================================================================
//  SECTION 8: Main — Demo Programs
// =============================================================================

int main() {

    cout << "============================================================\n";
    cout << "          Computer Architecture — CPU Simulator\n";
    cout << "          Simple RISC-like Processor in C++\n";
    cout << "============================================================\n";

    // ------------------------------------------------------------------
    //  Demo Program:
    //  Demonstrates all instruction types.
    //
    //   MOV  R0, 10        → R0 = 10
    //   MOV  R1, 20        → R1 = 20
    //   ADD  R2, R0, R1    → R2 = 30
    //   SUB  R3, R2, R0    → R3 = 20
    //   AND  R4, R0, R1    → R4 = 10 & 20 = 0
    //   OR   R5, R0, R1    → R5 = 10 | 20 = 30
    //   NOT  R6, R0        → R6 = ~10
    //   STORE R2, [10]     → MEM[10] = 30
    //   LOAD  R7, [10]     → R7 = MEM[10] = 30
    //   BEQ  R0, R1, 12   → R0≠R1 → not taken
    //   HALT
    // ------------------------------------------------------------------

    vector<Instruction> demoProgram = {
        //  opcode   rd  rs1  rs2  imm
        {  MOV,      0,   0,   0,  10 },  // R0 = 10
        {  MOV,      1,   0,   0,  20 },  // R1 = 20
        {  ADD,      2,   0,   1,   0 },  // R2 = R0 + R1 = 30
        {  SUB,      3,   2,   0,   0 },  // R3 = R2 - R0 = 20
        {  AND,      4,   0,   1,   0 },  // R4 = R0 & R1 = 0
        {  OR,       5,   0,   1,   0 },  // R5 = R0 | R1 = 30
        {  NOT,      6,   0,   0,   0 },  // R6 = ~R0
        {  STORE,    0,   2,   0,  10 },  // MEM[10] = R2 = 30
        {  LOAD,     7,   0,   0,  10 },  // R7 = MEM[10] = 30
        {  BEQ,      0,   0,   1,  12 },  // if R0==R1 jump 12 (not taken)
        {  HALT,     0,   0,   0,   0 },  // Stop
    };

    CPU cpu;
    cpu.loadProgram(demoProgram);
    cpu.run();

    return 0;
}

// =============================================================================
//  END OF FILE
//  Compile: g++ -std=c++17 -o cpu_sim cpu_simulator.cpp
//  Run:     ./cpu_sim
// =============================================================================
