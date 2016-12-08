/**
 *  This file is created by Jongho Kim.
 */

#ifndef __BASE_OPCLASS_HH__
#define __BASE_OPCLASS_HH__

#include "enums/OpClass.hh"

std::string opclass2string(const OpClass opclass)
{
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

#endif // __BASE_OPCLASS_HH__
