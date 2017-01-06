#include "base/instinfo.hh"
#include "debug/InstInfo.hh"

namespace InstInfo
{
    std::string opclass2string(const OpClass opclass) {
        switch(opclass) {
            case Enums::IntAlu:
                return "IntAlu";
            case Enums::IntMult:
                return "IntMult";
            case Enums::IntDiv:
                return "IntDiv";
            case Enums::FloatAdd:
                return "FloatAdd";
            case Enums::FloatCmp:
                return "FloatCmp";
            case Enums::FloatCvt:
                return "FloatCvt";
            case Enums::FloatMult:
                return "FloatMult";
            case Enums::FloatDiv:
                return "FloatDiv";
            case Enums::FloatSqrt:
                return "FloatSqrt";
            case Enums::SimdAdd:
                return "SimdAdd";
            case Enums::SimdAddAcc:
                return "SimdAddAcc";
            case Enums::SimdAlu:
                return "SimdAlu";
            case Enums::SimdCmp:
                return "SimdCmp";
            case Enums::SimdCvt:
                return "SimdCvt";
            case Enums::SimdMisc:
                return "SimdMisc";
            case Enums::SimdMult:
                return "SimdMult";
            case Enums::SimdMultAcc:
                return "SimdMultAcc";
            case Enums::SimdShift:
                return "SimdShift";
            case Enums::SimdShiftAcc:
                return "SimdShiftAcc";
            case Enums::SimdSqrt:
                return "SimdSqrt";
            case Enums::SimdFloatAdd:
                return "SimdFloatAdd";
            case Enums::SimdFloatAlu:
                return "SimdFloatAlu";
            case Enums::SimdFloatCmp:
                return "SimdFloatCmp";
            case Enums::SimdFloatCvt:
                return "SimdFloatCvt";
            case Enums::SimdFloatDiv:
                return "SimdFloatDiv";
            case Enums::SimdFloatMisc:
                return "SimdFloatMisc";
            case Enums::SimdFloatMult:
                return "SimdFloatMult";
            case Enums::SimdFloatMultAcc:
                return "SimdFloatMultAcc";
            case Enums::SimdFloatSqrt:
                return "SimdFloatSqrt";
            case Enums::MemRead:
                return "MemRead";
            case Enums::MemWrite:
                return "MemWrite";
            case Enums::IprAccess:
                return "IprAccess";
            case Enums::InstPrefetch:
                return "InstPrefetch";
            case Enums::Num_OpClass:
                return "Num_OpClass";
            default:
                return NULL;
        }
    }
    
    inline std::string disassemble(const Minor::MinorDynInstPtr& inst) {
        return inst->staticInst->generateDisassembly(inst->pc.instAddr(), debugSymbolTable);
    }
              
    void print_instinfo(const Minor::MinorDynInstPtr& inst) {

        if(DTRACE(InstInfo)) {
            StaticInstPtr static_inst = inst->staticInst;
            std::ostream& debug_file = Trace::output();

            debug_file  << "Instruction Information from Debug Flag \"InstInfo\"" << std::endl
                        << disassemble(inst) << std::endl
                        << "\t" << *inst << std::endl
                        << "\t0x" << std::hex << static_inst->machInst << std::dec << std::endl
                        << "\tOpClass: " << opclass2string(static_inst->opClass()) << std::endl
                        << "\tOP Type: " << (static_inst->isMicroop() ? "Micro OP" : "Single Inst") << std::endl;

            if (static_inst->isMicroop()) {
            
                debug_file << "#(uop) = " << static_inst->numMicroops << std::endl;
            }

            /** Print source registers among R0 ~ R15 */
            int count = 0;
            for (int i = 0 ; i < static_inst->numSrcRegs(); i++)
                if(static_inst->srcRegIdx(i) < 16)
                    count++;

            debug_file << "\tsrc regs:\t";

            if (count > 0) {
                for (int i = 0 ; i < static_inst->numSrcRegs(); i++) {
                    if(static_inst->srcRegIdx(i) < 16)
                        debug_file << "reg" << static_inst->srcRegIdx(i) << '\t';
                }
            }

            debug_file << std::endl;
        
            /** Print destination registers among R0 ~ R15 */
            count = 0;
            for (int i = 0 ; i < static_inst->numDestRegs(); i++)
                if(static_inst->destRegIdx(i) < 16)
                    count++;

            debug_file << "\tdest regs:\t";

            if (count > 0) {
                for (int i = 0 ; i < static_inst->numDestRegs(); i++) {
                    if(static_inst->destRegIdx(i) < 16)
                        debug_file << "reg" << static_inst->destRegIdx(i) << '\t';
                }
            }

            debug_file << std::endl;
        }
    }
} // namespace InstInfo

