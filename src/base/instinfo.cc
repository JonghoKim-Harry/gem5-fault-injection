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
        /**
         *  CAUTION
         *
         *    Note that isMacroop() is used in decode stage, then the
         *    'macroop' flag will set to 0 whether the instruction is
         *    macro op or not. So, after decode stage, the isMacroop() method
         *    is invalid
         */

        if(DTRACE(InstInfo)) {
            StaticInstPtr static_inst = inst->staticInst;
            std::ostream& debug_file = Trace::output();

            debug_file  << "Instruction Information from Debug Flag \"InstInfo\"" << std::endl
                        << disassemble(inst) << std::endl
                        << "\t" << *inst << std::endl
                        << "\tOpClass:\t" << opclass2string(static_inst->opClass()) << std::endl
                        << "\tOP Type:\t" << (static_inst->isMicroop() ? "uop" : "single instruction");

            if(static_inst->isMicroop()) {
                // TODO: Do Something
                debug_file  << " (" << static_inst->uop_type() <<")" << std::endl
                            << "\t\t* uop data #(bit):\t" << (static_inst->uop_data_bitlen() == 255 ? "N/A" : std::to_string(unsigned(static_inst->uop_data_bitlen()))) << std::endl
                            << "\t\t* uop data list:\t" << static_inst->uop_data_list();
            }
            debug_file << std::endl;

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

