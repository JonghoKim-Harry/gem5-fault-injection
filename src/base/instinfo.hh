/**
 *  This file is created by Jongho Kim
 */

#ifndef __BASE_INSTINFO_HH__
#define __BASE_INSTINFO_HH__

#include "base/trace.hh"
#include "enums/OpClass.hh"
#include "cpu/minor/dyn_inst.hh"
#include "base/loader/symtab.hh"

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
            StaticInstPtr myinst = inst->staticInst;
            std::ostream& debug_file = Trace::output();

            debug_file  << "Instruction Information from Debug Flag \"InstInfo\"" << std::endl
                        << disassemble(inst) << std::endl
                        << "\t" << *inst << std::endl
                        << "\tOpClass: " << opclass2string(myinst->opClass()) << std::endl
                        << "\tOP Type: " << (myinst->isMicroop() ? "Micro OP" : "Single Inst") << std::endl;

            /** Print source registers among R0 ~ R15 */
            int count = 0;
            for (int i = 0 ; i < myinst->numSrcRegs(); i++)
                if(myinst->srcRegIdx(i) < 16)
                    count++;

            debug_file << "\tsrc regs:\t";

            if (count > 0) {
                for (int i = 0 ; i < myinst->numSrcRegs(); i++) {
                    if(myinst->srcRegIdx(i) < 16)
                        debug_file << "reg" << myinst->srcRegIdx(i) << '\t';
                }
            }

            debug_file << std::endl;
        
            /** Print destination registers among R0 ~ R15 */
            count = 0;
            for (int i = 0 ; i < myinst->numDestRegs(); i++)
                if(myinst->destRegIdx(i) < 16)
                    count++;

            debug_file << "\tdest regs:\t";

            if (count > 0) {
                for (int i = 0 ; i < myinst->numDestRegs(); i++) {
                    if(myinst->destRegIdx(i) < 16)
                        debug_file << "reg" << myinst->destRegIdx(i) << '\t';
                }
            }

            debug_file << std::endl;
        }
    }
} // namespace InstInfo

#endif // __BASE_INSTINFO_HH__
