/*
 * Copyright (c) 2011-2012 Google
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
 * Authors: Gabe Black
 */

#include "arch/generic/decode_cache.hh"
#include "arch/decoder.hh"
#include "arch/types.hh"
#include "config/the_isa.hh"
#include "cpu/static_inst.hh"

// JONGHO
#include "base/loader/symtab.hh"
#include "base/softerror.hh"
#include "debug/FI.hh"

namespace GenericISA
{

StaticInstPtr
BasicDecodeCache::decode(TheISA::Decoder *decoder,
        TheISA::ExtMachInst mach_inst, Addr addr)
{
    // JONGHO
    if(     SoftError::timeToInject() &&
            (SoftError::injComp == SoftError::F2TOD ||
                (SoftError::injComp == SoftError::DTOE &&
                (!decoder->decodeInst(mach_inst)->isMacroop())))) {
        if(SoftError::injReady()) {
            SoftError::injDone = true;
            const TheISA::ExtMachInst golden_bin = mach_inst;
            mach_inst = BITFLIP(mach_inst, SoftError::injLoc);
            const TheISA::ExtMachInst faulty_bin = mach_inst;

            /** */
            std::string golden_inst = decoder->decodeInst(golden_bin)->generateDisassembly(addr, debugSymbolTable); 
            std::string faulty_inst = decoder->decodeInst(faulty_bin)->generateDisassembly(addr, debugSymbolTable); 
            const std::string mnemonic = decoder->decodeInst(golden_bin)->mnemonic;
            if(SoftError::injComp == SoftError::F2TOD)
                DPRINTF(FI, "Fault Injection into f2ToD @decoder - mnemonic: %s - Bit[%u] Flipped, %#x:\t\t%#x  %s -> %#x  %s\n", mnemonic, SoftError::injLoc, addr, golden_bin, golden_inst, faulty_bin, faulty_inst);
            else
                DPRINTF(FI, "Fault Injection into dToE @decoder - mnemonic: %s - Bit[%u] Flipped, %#x:\t\t%#x  %s -> %#x  %s\n", mnemonic, SoftError::injLoc, addr, golden_bin, golden_inst, faulty_bin, faulty_inst);
        }
        else
            SoftError::injWait -= 1;
    }

    StaticInstPtr &si = decodePages.lookup(addr);
    if (si && (si->machInst == mach_inst))
        return si;

    DecodeCache::InstMap::iterator iter = instMap.find(mach_inst);
    if (iter != instMap.end()) {
        si = iter->second;
        return si;
    }

    si = decoder->decodeInst(mach_inst);
    instMap[mach_inst] = si;
    return si;
}

} // namespace GenericISA
