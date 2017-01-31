/**
 *  This file is created by Jongho Kim
 */

#ifndef __BASE_INSTINFO_HH__
#define __BASE_INSTINFO_HH__

#include <vector>

#include "cpu/minor/dyn_inst.hh"
#include "base/trace.hh"
#include "enums/OpClass.hh"
#include "cpu/minor/dyn_inst.hh"
#include "base/loader/symtab.hh"

namespace InstInfo
{
    std::string opclass2string(const OpClass opclass);
    inline std::string disassemble(const Minor::MinorDynInstPtr& inst);
    void print_instinfo(const Minor::MinorDynInstPtr& inst);

    /* Global variables to easily track instructions */
    void push_fetch1_addr(Addr addr);
    std::vector<Addr> fetch1_addr();
    void clear_fetch1_addr();

    void push_fetch2_addr(Addr addr);
    std::vector<Addr> fetch2_addr();
    void clear_fetch2_addr();

    void push_decode_op(Minor::MinorDynInstPtr inst);
    std::vector<Minor::MinorDynInstPtr> decode_op();
    void clear_decode_op();

    /*
    void push_microop_addr(Addr addr);
    std::vector<MicroPC> microop_addr();
    void clear_microop_addr();
    */

    void push_execute_addr(Addr addr);
    std::vector<Addr> execute_addr();
    void clear_execute_addr();
} // namespace InstInfo

#endif // __BASE_INSTINFO_HH__
