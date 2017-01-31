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

    namespace
    {
        std::vector<Addr> __fetch1_addr;
        std::vector<Addr> __fetch2_addr;
        std::vector<Minor::MinorDynInstPtr> __decode_op;
        std::vector<Addr> __microop_addr;
        std::vector<Addr> __execute_addr;
    }

    std::vector<Addr> fetch1_addr() {
        return *(new std::vector<Addr>(__fetch1_addr.begin(), __fetch1_addr.end()));
    }

    void push_fetch1_addr(Addr addr) {
        __fetch1_addr.push_back(addr);
    }

    void clear_fetch1_addr() {
        __fetch1_addr.clear();
    }

    std::vector<Addr> fetch2_addr() {
        return *(new std::vector<Addr>(__fetch2_addr.begin(), __fetch2_addr.end()));
    }

    void push_fetch2_addr(Addr addr) {
        __fetch2_addr.push_back(addr);
    }

    void clear_fetch2_addr() {
        __fetch2_addr.clear();
    }

    std::vector<Minor::MinorDynInstPtr> decode_op() {
        return *(new std::vector<Minor::MinorDynInstPtr>(__decode_op.begin(), __decode_op.end()));
    }

    void push_decode_op(Minor::MinorDynInstPtr op) {
        __decode_op.push_back(op);
    }

    void clear_decode_op() {
        __decode_op.clear();
    }

    /*
    std::vector<Addr> microop_addr() {
        return *(new std::vector<Addr>(__microop_addr.begin(), __microop_addr.end()));
    }

    void push_microop_addr(Addr addr) {
        __microop_addr.push_back(addr);
    }

    void clear_microop_addr() {
        __microop_addr.clear();
    }
    */

    std::vector<Addr> execute_addr() {
        return *(new std::vector<Addr>(__execute_addr.begin(), __execute_addr.end()));
    }

    void push_execute_addr(Addr addr) {
        __execute_addr.push_back(addr);
    }

    void clear_execute_addr() {
        __execute_addr.clear();
    }
} // namespace InstInfo
