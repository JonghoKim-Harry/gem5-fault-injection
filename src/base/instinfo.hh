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
    std::string opclass2string(const OpClass opclass);
    inline std::string disassemble(const Minor::MinorDynInstPtr& inst);
    void print_instinfo(const Minor::MinorDynInstPtr& inst);
} // namespace InstInfo

#endif // __BASE_INSTINFO_HH__
