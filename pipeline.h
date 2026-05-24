// =============================================================================
//  5-Stage Pipeline Implementation
//  Stages: IF (Fetch) → ID (Decode) → EX (Execute) → MEM (Memory) → WB (Writeback)
//  Includes: Hazard Detection, Data Forwarding (Bypassing)
// =============================================================================

#ifndef PIPELINE_H
#define PIPELINE_H

#include <iostream>
#include <iomanip>
#include <string>
#include <map>

using namespace std;

// Forward declarations (actual definitions provided in main file)
// enum Opcode and struct Instruction must be defined before including this header

// Pipeline Stage Registers (Pipeline Latches)
// ─────────────────────────────────────────

// IF/ID Pipeline Register
struct IF_ID_Latch {
    int opcode;  // Instruction opcode
    int rd, rs1, rs2, imm;  // Instruction fields
    int PC;
    bool valid;
    
    IF_ID_Latch() : opcode(0), rd(0), rs1(0), rs2(0), imm(0), PC(0), valid(false) {}
};

// ID/EX Pipeline Register
struct ID_EX_Latch {
    int opcode;         // Instruction opcode
    int rd, rs1, rs2;   // Instruction register fields
    int rs1_data;       // Source operand 1 value
    int rs2_data;       // Source operand 2 value
    int imm;            // Immediate value
    int PC;
    bool regWrite;      // Control signal
    bool memRead;
    bool memWrite;
    bool branch;
    bool jump;
    bool valid;
    
    ID_EX_Latch() : opcode(0), rd(0), rs1(0), rs2(0), rs1_data(0), rs2_data(0), imm(0), PC(0),
                    regWrite(false), memRead(false), memWrite(false),
                    branch(false), jump(false), valid(false) {}
};

// EX/MEM Pipeline Register
struct EX_MEM_Latch {
    int opcode;         // Instruction opcode
    int aluResult;      // Result from ALU
    int memAddr;        // Memory address for LOAD/STORE
    int writeData;      // Data to write to memory
    int rd;             // Destination register
    int PC;
    bool regWrite;
    bool memRead;
    bool memWrite;
    bool valid;
    
    EX_MEM_Latch() : opcode(0), aluResult(0), memAddr(0), writeData(0), rd(0), PC(0),
                     regWrite(false), memRead(false), memWrite(false), valid(false) {}
};

// MEM/WB Pipeline Register
struct MEM_WB_Latch {
    int opcode;         // Instruction opcode
    int aluResult;
    int memData;        // Data read from memory
    int rd;             // Destination register
    int PC;
    bool regWrite;
    bool memRead;       // Indicates if data came from memory
    bool valid;
    
    MEM_WB_Latch() : opcode(0), aluResult(0), memData(0), rd(0), PC(0),
                     regWrite(false), memRead(false), valid(false) {}
};

// ─────────────────────────────────────────
//  Hazard Detection Unit
// ─────────────────────────────────────────

class HazardDetectionUnit {
public:
    // Detect RAW (Read After Write) hazard
    static bool detectRAW_EX(const ID_EX_Latch& idex, int rs) {
        if (!idex.valid || !idex.regWrite) return false;
        return (idex.rd == rs) && (rs != 0);
    }

    static bool detectRAW_MEM(const EX_MEM_Latch& exmem, int rs) {
        if (!exmem.valid || !exmem.regWrite) return false;
        return (exmem.rd == rs) && (rs != 0);
    }

    // Detect WAW (Write After Write) hazard
    static bool detectWAW(const ID_EX_Latch& idex, const EX_MEM_Latch& exmem, int rd) {
        bool idex_hazard = idex.valid && idex.regWrite && (idex.rd == rd) && (rd != 0);
        bool exmem_hazard = exmem.valid && exmem.regWrite && (exmem.rd == rd) && (rd != 0);
        return idex_hazard || exmem_hazard;
    }

    // Detect WAR (Write After Read) hazard
    static bool detectWAR(const ID_EX_Latch& idex, int rd) {
        if (!idex.valid) return false;
        return ((idex.rs1 == rd || idex.rs2 == rd) && (rd != 0));
    }

    // Check if load-use hazard exists
    static bool detectLoadUseHazard(const ID_EX_Latch& idex, 
                                     int rs1_needed, int rs2_needed) {
        if (!idex.valid || !idex.memRead) return false;
        if (idex.rd == rs1_needed || idex.rd == rs2_needed) {
            return (idex.rd != 0);
        }
        return false;
    }

    // Print hazard info
    static void printHazardInfo(bool raw_ex, bool raw_mem, bool waw, bool war, bool load_use) {
        cout << "  [HAZARD CHECK] ";
        if (raw_ex) cout << "RAW_EX ";
        if (raw_mem) cout << "RAW_MEM ";
        if (waw) cout << "WAW ";
        if (war) cout << "WAR ";
        if (load_use) cout << "LOAD_USE ";
        if (!raw_ex && !raw_mem && !waw && !war && !load_use) cout << "No hazards";
        cout << "\n";
    }
};

// ─────────────────────────────────────────
//  Data Forwarding (Bypassing) Unit
// ─────────────────────────────────────────

class ForwardingUnit {
public:
    // Forwarding sources for operands
    enum ForwardSource {
        FROM_REGISTER = 0,  // Original register file value
        FROM_EX_MEM = 1,    // From EX/MEM latch (EX stage result)
        FROM_MEM_WB = 2     // From MEM/WB latch (WB stage result)
    };

    struct ForwardPath {
        ForwardSource source;
        int forwardedValue;
    };

    // Check forwarding for Rs1 (source register 1)
    static ForwardPath checkForwardRs1(const ID_EX_Latch& idex,
                                       const EX_MEM_Latch& exmem,
                                       const MEM_WB_Latch& memwb,
                                       int rs1_value) {
        // Priority: EX/MEM > MEM/WB > Register File

        // Forward from EX/MEM (most recent)
        if (exmem.valid && exmem.regWrite && exmem.rd == idex.rs1 && 
            idex.rs1 != 0) {
            return {FROM_EX_MEM, exmem.aluResult};
        }

        // Forward from MEM/WB
        if (memwb.valid && memwb.regWrite && memwb.rd == idex.rs1 && 
            idex.rs1 != 0) {
            int value = memwb.memRead ? memwb.memData : memwb.aluResult;
            return {FROM_MEM_WB, value};
        }

        // Use register file value
        return {FROM_REGISTER, rs1_value};
    }

    // Check forwarding for Rs2 (source register 2)
    static ForwardPath checkForwardRs2(const ID_EX_Latch& idex,
                                       const EX_MEM_Latch& exmem,
                                       const MEM_WB_Latch& memwb,
                                       int rs2_value) {
        // Priority: EX/MEM > MEM/WB > Register File

        // Forward from EX/MEM (most recent)
        if (exmem.valid && exmem.regWrite && exmem.rd == idex.rs2 && 
            idex.rs2 != 0) {
            return {FROM_EX_MEM, exmem.aluResult};
        }

        // Forward from MEM/WB
        if (memwb.valid && memwb.regWrite && memwb.rd == idex.rs2 && 
            idex.rs2 != 0) {
            int value = memwb.memRead ? memwb.memData : memwb.aluResult;
            return {FROM_MEM_WB, value};
        }

        // Use register file value
        return {FROM_REGISTER, rs2_value};
    }

    // Print forwarding info
    static void printForwardingInfo(const string& operand, ForwardSource source) {
        cout << "  [FORWARD] " << operand << " from ";
        switch(source) {
            case FROM_REGISTER: cout << "Register File"; break;
            case FROM_EX_MEM: cout << "EX/MEM (ALU result)"; break;
            case FROM_MEM_WB: cout << "MEM/WB (WB result)"; break;
        }
        cout << "\n";
    }
};

// ─────────────────────────────────────────
//  Pipeline Controller
// ─────────────────────────────────────────

class PipelineController {
private:
    int stallCounter;
    bool pipelineStall;

public:
    PipelineController() : stallCounter(0), pipelineStall(false) {}

    // Initiate pipeline stall
    void stallPipeline(int cycles = 1) {
        pipelineStall = true;
        stallCounter = cycles;
    }

    // Decrement stall counter
    void tick() {
        if (pipelineStall) {
            stallCounter--;
            if (stallCounter == 0) {
                pipelineStall = false;
            }
        }
    }

    // Check if pipeline is stalled
    bool isStalled() const {
        return pipelineStall;
    }

    // Get stall count
    int getStallCount() const {
        return stallCounter;
    }

    // Reset stall
    void resetStall() {
        pipelineStall = false;
        stallCounter = 0;
    }
};

#endif // PIPELINE_H
