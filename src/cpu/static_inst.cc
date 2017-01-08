/*
 * Copyright (c) 2003-2005 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Steve Reinhardt
 *          Nathan Binkert
 */

#include <iostream>

#include "cpu/static_inst.hh"
#include "sim/core.hh"

// JONGHO
#include "debug/FI.hh"

StaticInstPtr StaticInst::nullStaticInstPtr;

using namespace std;

StaticInst::~StaticInst()
{
    if (cachedDisassembly)
        delete cachedDisassembly;
}

bool
StaticInst::hasBranchTarget(const TheISA::PCState &pc, ThreadContext *tc,
                            TheISA::PCState &tgt) const
{
    if (isDirectCtrl()) {
        tgt = branchTarget(pc);
        return true;
    }

    if (isIndirectCtrl()) {
        tgt = branchTarget(tc);
        return true;
    }

    return false;
}

StaticInstPtr
StaticInst::fetchMicroop(MicroPC upc) const
{
    panic("StaticInst::fetchMicroop() called on instruction "
          "that is not microcoded.");
}

TheISA::PCState
StaticInst::branchTarget(const TheISA::PCState &pc) const
{
    panic("StaticInst::branchTarget() called on instruction "
          "that is not a PC-relative branch.");
    M5_DUMMY_RETURN;
}

TheISA::PCState
StaticInst::branchTarget(ThreadContext *tc) const
{
    panic("StaticInst::branchTarget() called on instruction "
          "that is not an indirect branch.");
    M5_DUMMY_RETURN;
}

const string &
StaticInst::disassemble(Addr pc, const SymbolTable *symtab) const
{
    if (!cachedDisassembly)
        cachedDisassembly = new string(generateDisassembly(pc, symtab));

    return *cachedDisassembly;
}

void
StaticInst::printFlags(std::ostream &outs,
    const std::string &separator) const
{
    bool printed_a_flag = false;

    for (unsigned int flag = IsNop; flag < Num_Flags; flag++) {
        if (flags[flag]) {
            if (printed_a_flag)
                outs << separator;

            outs << FlagsStrings[flag];
            printed_a_flag = true;
        }
    }
}

// JONGHO
bool StaticInst::injectFault(unsigned int loc) {

    if(isMicroop()) {
        uint8_t *target_byte = (uint8_t *)uop_data_byte(loc);
        *target_byte = BITFLIP(*target_byte, loc%8);
        DPRINTF(FI, "Flip bit[%u] of %s:%s\n", loc%8, uop_type(), uop_data_name(loc));
    }
    else {
    }

    return true;
    /*
    unsigned int num_destreg = 0, num_srcreg = 0;
    for(int j=0; j<_numDestRegs; j++)
        if(_destRegIdx[j] < 16)
            ++num_destreg;
    for(int j=0; j<_numSrcRegs; j++)
        if(_srcRegIdx[j] < 16)
            ++num_srcreg;

    // We assume that there are at most one dest reg and two src regs
    assert(num_destreg <= 1 && num_srcreg <= 2);

    unsigned int idx = loc%12;
    if(idx <= 3) {
        // Flip destination register index
        if(num_destreg == 0)
            return false;
        for(int j=0; j<_numDestRegs; j++) {
            if(_destRegIdx[j] < 16) {
                const RegIndex golden = _destRegIdx[j];
                _destRegIdx[j] = BITFLIP(_destRegIdx[j], idx%4);
                DPRINTF(FI, "Flip DestRegIdx:\treg%u -> reg%u\n", golden, _destRegIdx[j]);
                break;
            }
        }
    }
    else if(4 <= idx && idx <= 7) {
        // Flip 1st source register index
        if(num_srcreg == 0)
            return false;
        for(int j=0; j<_numSrcRegs; j++) {
            if(_srcRegIdx[j] < 16) {
                RegIndex golden = _srcRegIdx[j];
                _srcRegIdx[j] = BITFLIP(_srcRegIdx[j], idx%4);
                DPRINTF(FI, "Flip 1st SrcRegIdx:\treg%u -> reg%u\n", golden, _srcRegIdx[j]);
                break;
            }
        }
    }
    else if(8 <= idx && idx <= 11) {
        // Flip 2nd source register index
        if(num_srcreg <= 1)
            return false;
        for(int j=0, k=0; j<_numSrcRegs; j++) {
            if(_srcRegIdx[j] < 16) {
                ++k;
                if(k==2) {
                    RegIndex golden = _srcRegIdx[j];
                    _srcRegIdx[j] = BITFLIP(_srcRegIdx[j], idx%4);
                    DPRINTF(FI, "Flip 2nd SrcRegIdx:\treg%u -> reg%u\n", golden, _srcRegIdx[j]);
                    break;
                }
            }
        }
    }
    else
        return false;

    return true;
    */
}
