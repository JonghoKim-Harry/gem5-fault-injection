/*
 * Copyright (c) 2013-2014 ARM Limited
 * All rights reserved
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
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
 * Authors: Andrew Bardsley
 */

#include "cpu/minor/pipe_data.hh"

// JONGHO
#include "arch/arm/decoder.hh"
#include "debug/FICallTrace.hh"
#include "debug/FIReport.hh"

namespace Minor
{

std::ostream &
operator <<(std::ostream &os, BranchData::Reason reason)
{
    switch (reason)
    {
      case BranchData::NoBranch:
        os << "NoBranch";
        break;
      case BranchData::UnpredictedBranch:
        os << "UnpredictedBranch";
        break;
      case BranchData::BranchPrediction:
        os << "BranchPrediction";
        break;
      case BranchData::CorrectlyPredictedBranch:
        os << "CorrectlyPredictedBranch";
        break;
      case BranchData::BadlyPredictedBranch:
        os << "BadlyPredictedBranch";
        break;
      case BranchData::BadlyPredictedBranchTarget:
        os << "BadlyPredictedBranchTarget";
        break;
      case BranchData::Interrupt:
        os << "Interrupt";
        break;
      case BranchData::SuspendThread:
        os << "SuspendThread";
        break;
      case BranchData::HaltFetch:
        os << "HaltFetch";
        break;
    }

    return os;
}

bool
BranchData::isStreamChange(const BranchData::Reason reason)
{
    bool ret = false;

    switch (reason)
    {
        /* No change of stream (see the enum comment in pipe_data.hh) */
      case NoBranch:
      case CorrectlyPredictedBranch:
        ret = false;
        break;

        /* Change of stream (Fetch1 should act on) */
      case UnpredictedBranch:
      case BranchPrediction:
      case BadlyPredictedBranchTarget:
      case BadlyPredictedBranch:
      case SuspendThread:
      case Interrupt:
      case HaltFetch:
        ret = true;
        break;
    }

    return ret;
}

bool
BranchData::isBranch(const BranchData::Reason reason)
{
    bool ret = false;

    switch (reason)
    {
        /* No change of stream (see the enum comment in pipe_data.hh) */
      case NoBranch:
      case CorrectlyPredictedBranch:
      case SuspendThread:
      case Interrupt:
      case HaltFetch:
        ret = false;
        break;

        /* Change of stream (Fetch1 should act on) */
      case UnpredictedBranch:
      case BranchPrediction:
      case BadlyPredictedBranchTarget:
      case BadlyPredictedBranch:
        ret = true;
        break;
    }

    return ret;
}

void
BranchData::reportData(std::ostream &os) const
{
    if (isBubble()) {
        os << '-';
    } else {
        os << reason
            << ';' << newStreamSeqNum << '.' << newPredictionSeqNum
            << ";0x" << std::hex << target.instAddr() << std::dec
            << ';';
        inst->reportData(os);
    }
}

std::ostream &
operator <<(std::ostream &os, const BranchData &branch)
{
    os << branch.reason << " target: 0x"
        << std::hex << branch.target.instAddr() << std::dec
        << ' ' << *branch.inst
        << ' ' << branch.newStreamSeqNum << "(stream)."
        << branch.newPredictionSeqNum << "(pred)";

    return os;
}

// JONGHO
void
BranchData::corrupt(const unsigned int loc)
{
    DPRINTF(FICallTrace, "corrupt() @BranchData\n");

    DPRINTF(FIReport, "--- Fault Injection ---\n");
    DPRINTF(FIReport, "     @BranchData\n");

    if(isBubble()) {
        DPRINTF(FIReport, "     * Injected into BUBBLE\n");
    }
    else {
        if(!isBranch()) {
            DPRINTF(FIReport, "     * Injected into BUBBLE (No Branch)\n");
        }
        else {
        /* Fault Injection HERE */

        std::string why_branch;
        switch (reason) {
            case NoBranch:
            case CorrectlyPredictedBranch:
            case SuspendThread:
            case Interrupt:
            case HaltFetch:
                break;

            /* Change of stream (Fetch1 should act on) */
            case UnpredictedBranch:
                why_branch = "UnpredictedBranch";
                break;
            case BranchPrediction:
                why_branch = "BranchPrediction";
                break;
            case BadlyPredictedBranchTarget:
                why_branch = "BadlyPredictedBranchTarget";
                break;
            case BadlyPredictedBranch:
                why_branch = "BadlyPredictedBranch";
                break;
        }
            
        DPRINTF(FIReport, "     * fetching caused by: %s\n", why_branch);

        /** These values are all unavailable when it is not branch */
        // Address of instruction which cause branch
        if(inst) {
            const Addr cause_inst_addr = inst->pc.pc();
            const ExtMachInst cause_bin_inst = inst->staticInst->machInst;
            const std::string cause_inst = inst->staticInst->generateDisassembly(cause_inst_addr, debugSymbolTable);
            DPRINTF(FIReport, "     * inst which cause fetching: %#x: %#x %s\n", cause_inst_addr, cause_bin_inst, cause_inst);
        }

        // Golden Target Address
        const Addr golden_target_addr = target.pc();

        // Bit Flip
        target.set(BITFLIP(golden_target_addr, loc%32));

        // Faulty Target Address
        const Addr faulty_target_addr = target.pc();

        DPRINTF(FIReport, "     * Flip target address: %#x -> %#x (%s)\n", golden_target_addr, faulty_target_addr, why_branch);
        }
    }
}

void
ForwardLineData::setFault(Fault fault_)
{
    fault = fault_;
    if (isFault())
        bubbleFlag = false;
}

void
ForwardLineData::allocateLine(unsigned int width_)
{
    lineWidth = width_;
    bubbleFlag = false;

    assert(!isFault());
    assert(!line);

    line = new uint8_t[width_];
}

void
ForwardLineData::adoptPacketData(Packet *packet)
{
    this->packet = packet;
    lineWidth = packet->req->getSize();
    bubbleFlag = false;

    assert(!isFault());
    assert(!line);

    line = packet->getPtr<uint8_t>();
}

void
ForwardLineData::freeLine()
{
    /* Only free lines in non-faulting, non-bubble lines */
    if (!isFault() && !isBubble()) {
        assert(line);
        /* If packet is not NULL then the line must belong to the packet so
         *  we don't need to separately deallocate the line */
        if (packet) {
            delete packet;
        } else {
            delete [] line;
        }
        line = NULL;
        bubbleFlag = true;
    }
}

void
ForwardLineData::reportData(std::ostream &os) const
{
    if (isBubble())
        os << '-';
    else if (fault != NoFault)
        os << "F;" << id;
    else
        os << id;
}

// JONGHO
void
ForwardLineData::corrupt(const unsigned int loc)
{
    DPRINTF(FICallTrace, "corrupt() @ForwardLineData\n");

    /** 32-bit ISA */
    const unsigned int BYTE_PER_INST = sizeof(uint32_t);
    const unsigned int BIT_PER_INST = sizeof(uint32_t) * BIT_PER_BYTE;

    /** Print out fault injection informations */
    DPRINTF(FIReport, "--- Fault Injection ---\n");
    DPRINTF(FIReport, "     @ForwardLineData\n");

    /**
     *  Bit Flip Procss: Do fault injection if and only if
     *                   the line is neither bubble nor fault
     */
    if(bubbleFlag)
        DPRINTF(FIReport, "     * Injected into BUBBLE\n");
    else if(isFault())
        DPRINTF(FIReport, "     * Injected into FAULT\n");
    else {
        /** 
         *  Do NOT use loc. Use valid_loc instead.
         *  Note that if the given instance of ForwardLineData is bubble,
         *  valid_loc will be undefined, and many following variables will
         *  remain undefined
         */
        const unsigned int valid_loc = loc % lineWidth;
        DPRINTF(FIReport, "     * loc:  %u\n", valid_loc);

        /** To log instruction change */
        ArmISA::Decoder *decoder = new ArmISA::Decoder(nullptr);

        /** Same with (valid_loc - (valid_loc % BIT_PER_INST)) / (BIT_PER_BYTE) */
        unsigned int offset_to_inst = BYTE_PER_INST * (valid_loc / BIT_PER_INST);

        /** fault-injected instruction's address */
        Addr inst_addr = lineBaseAddr + offset_to_inst;
        DPRINTF(FIReport, "     * addr: %#x\n", inst_addr);

        /** original binary instruction */
        const uint32_t golden_bin = *(uint32_t *)&line[offset_to_inst];

        /** Soft error is transient, so we can use cached golden instruction */
        const std::string golden_inst = decoder->decodeInst(golden_bin)->generateDisassembly(inst_addr, debugSymbolTable);

        /** actual bit flip */
        line[valid_loc/BIT_PER_BYTE] = BITFLIP(line[valid_loc/BIT_PER_BYTE], valid_loc%BIT_PER_BYTE);

        /** fault-injected binary instruction */
        const uint32_t faulty_bin = *(uint32_t *)&line[offset_to_inst];

        /** 
         *  We can NOT use cached faulty instruction,
         *  so set address parameter to 0 to prevent use of decode cache
         */
        const std::string faulty_inst = decoder->decodeInst(faulty_bin)->generateDisassembly(0, debugSymbolTable);

        /* Print out change of instruction by injected soft error */
        DPRINTF(FIReport, "     * Flip inst: %#x %s -> %#x %s\n", golden_bin, golden_inst, faulty_bin, faulty_inst);
    }
}

ForwardInstData::ForwardInstData(unsigned int width, ThreadID tid) :
    numInsts(width), threadId(tid)
{
    bubbleFill();
}

ForwardInstData::ForwardInstData(const ForwardInstData &src)
{
    *this = src;
}

ForwardInstData &
ForwardInstData::operator =(const ForwardInstData &src)
{
    numInsts = src.numInsts;

    for (unsigned int i = 0; i < src.numInsts; i++)
        insts[i] = src.insts[i];

    return *this;
}

bool
ForwardInstData::isBubble() const
{
    return numInsts == 0 || insts[0]->isBubble();
}

void
ForwardInstData::bubbleFill()
{
    for (unsigned int i = 0; i < numInsts; i++)
        insts[i] = MinorDynInst::bubble();
}

void
ForwardInstData::resize(unsigned int width)
{
    assert(width < MAX_FORWARD_INSTS);
    numInsts = width;

    bubbleFill();
}

void
ForwardInstData::reportData(std::ostream &os) const
{
    if (isBubble()) {
        os << '-';
    } else {
        unsigned int i = 0;

        os << '(';
        while (i != numInsts) {
            insts[i]->reportData(os);
            i++;
            if (i != numInsts)
                os << ',';
        }
        os << ')';
    }
}

// JONGHO
void
ForwardInstData::corrupt(const unsigned int loc)
{
    fatal("You can NOT use corrupt() for ForwardInstData - Use corruptInst() or corruptOp()\n");
}

// JONGHO
/* 
 * Target HW component: Pipeline Register [F->D]
 * Target data: ARM Instruction
 */
void
ForwardInstData::corruptInst(const unsigned int loc)
{
    DPRINTF(FICallTrace, "corruptInst() @ForwardInstData\n");

    /** 32-bit ISA */
    const unsigned int BIT_PER_INST = sizeof(uint32_t) * BIT_PER_BYTE;

    /** Print out fault injection information */
    DPRINTF(FIReport, "--- Fault Injection ---\n");
    DPRINTF(FIReport, "     @ForwardInstData\n");
    DPRINTF(FIReport, "     * target HW component: Pipeline Register [F->D]\n");

    if(isBubble()) {
        DPRINTF(FIReport, "     * Injected into BUBBLE@ForwardInstData\n");
        return;
    }

    /*
     *  Do NOT use loc. Use valid_loc instead.
     *  Note that if the given instance of ForwardInstData is bubble,
     *  valid_loc will be undefined, and many following variables will
     *  remain undefined
     */
    const unsigned int valid_loc = loc % (numInsts * BIT_PER_INST);
    DPRINTF(FIReport, "     * loc:  %u\n", valid_loc);

    /** Select instruction to inject fault into */
    unsigned int inst_index = valid_loc / BIT_PER_INST;
    MinorDynInstPtr target_dynamic_wrapper = insts[inst_index];

    if(target_dynamic_wrapper->isBubble()) {
        DPRINTF(FIReport, "     * Injected into BUBBLE@MinorDynInst\n");
        return;
    }

    if(target_dynamic_wrapper->isFault()) {
        DPRINTF(FIReport, "     * Injected into FAULT\n");
        return;
    }

    DPRINTF(FIReport, "     * target data: ARM Instruction\n");

    /*
     *  We can get address only if the instruction is
     *  neither bubble nor fault
     */
    const Addr addr = target_dynamic_wrapper->pc.instAddr();
    DPRINTF(FIReport, "     * addr: %#x\n", addr);

    /*
     *  Wrapper for original instruction
     *  Note that changes in golden_static_wrapper has no effect on
     *  fault-injected instruction
     */
    const StaticInstPtr golden_static_wrapper = target_dynamic_wrapper->staticInst;

    /* original binary instruction & instruction mnemonic */
    const uint32_t golden_bin = golden_static_wrapper->machInst;
    const std::string golden_inst = golden_static_wrapper->generateDisassembly(addr, debugSymbolTable);

    /* fault-injected binary instruction */
    const uint32_t faulty_bin = BITFLIP(golden_bin, valid_loc % BIT_PER_INST);

    /* Print C++ typeid of target instruction */
    DPRINTF(FIReport, "     * typeid: %s\n", typeid(*golden_static_wrapper).name());

    ArmISA::Decoder *decoder = new ArmISA::Decoder(nullptr);
    target_dynamic_wrapper->staticInst = decoder->decodeInst(faulty_bin);

    /*
     *  We can NOT use cached faulty instruction,
     *  so set address parameter to 0 to prevent use of decode cache
     */
    std::string faulty_inst = target_dynamic_wrapper->staticInst->generateDisassembly(0, debugSymbolTable);

    /*  Print out changes of instruction by soft error */
    DPRINTF(FIReport, "     * Flip inst: %#x %s -> %#x %s\n", golden_bin, golden_inst, faulty_bin, faulty_inst);
}

// JONGHO
/* 
 * Target HW component: Pipeline Register [D->E]
 * Target data: ARM Instruction or uop
 */
void
ForwardInstData::corruptOp(const unsigned int loc)
{
    DPRINTF(FICallTrace, "corruptOp() @ForwardInstData\n");

    /* Print out fault injection information */
    DPRINTF(FIReport, "--- Fault Injection ---\n");
    DPRINTF(FIReport, "     @ForwardInstData\n");
    DPRINTF(FIReport, "     * target HW component: Pipeline Register [D->E]\n");

    if(isBubble()) {
        DPRINTF(FIReport, "     * Injected into BUBBLE@ForwardInstData\n");
        return;
    }

    /* With our config, MAX(n(src reg)) = 34, MAX(n(dest reg)) = 8 */
    const unsigned int MAX_BIT_USAGE_PER_OP = sizeof(RegIndex) * BIT_PER_BYTE * (StaticInst::MaxInstSrcRegs + StaticInst::MaxInstDestRegs);
    const unsigned int MAX_BIT_USAGE = MAX_BIT_USAGE_PER_OP * numInsts;

    /* Select instruction to inject fault into */
    const unsigned int inst_idx = (loc % MAX_BIT_USAGE) / MAX_BIT_USAGE_PER_OP;
    MinorDynInstPtr dynamic_inst = insts[inst_idx];

    if(dynamic_inst->isBubble()) {
        DPRINTF(FIReport, "     * Injected into BUBBLE@MinorDynInst\n");
        return;
    }

    if(dynamic_inst->isFault()) {
        DPRINTF(FIReport, "     * Injected into FAULT\n");
        return;
    }

    StaticInstPtr static_inst = dynamic_inst->staticInst;
    if(static_inst->numSrcRegs() + static_inst->numDestRegs() == 0) {
        DPRINTF(FIReport, "     * NO src nor dest register\n");
        return;
    }

    /*
     * If control flow reach HERE, actual bit flip will happen
     */

    /*
     * Parent instruction of the target op.
     *
     * If ARM instruction, parent instruction is itself.
     * If uop, parent instruction is macro operation that
     * the target uop comes from
     */
    DPRINTF(FIReport, "     * parent addr: %#x\n", dynamic_inst->pc.instAddr());
    std::string type = static_inst->isMicroop() ? "ARM instruction" : "ARM uop";
    DPRINTF(FIReport, "     * target op: %s%s\n", type, static_inst->generateDisassembly(0, debugSymbolTable));

    const unsigned int num_reg_used = static_inst->numSrcRegs() + static_inst->numDestRegs();
    const unsigned int range = num_reg_used * sizeof(RegIndex) * BIT_PER_BYTE;
    unsigned int reg_idx = (loc % range) / (sizeof(RegIndex) * BIT_PER_BYTE);
    const unsigned int bit_idx = loc % (sizeof(RegIndex) * BIT_PER_BYTE);
    const bool corrupt_srcreg = (reg_idx < static_inst->numSrcRegs()) ? true : false;
    DPRINTF(FIReport, "     * bit index: %u (= 2^%u)\n", bit_idx, bit_idx);

    /**/
    std::stringstream srcreg_info, destreg_info;
    for(int i=0; i<static_inst->numSrcRegs(); ++i)
        srcreg_info << static_inst->srcRegIdx(i) << " ";
    if(static_inst->numSrcRegs() == 0)
        srcreg_info << "None";
    for(int i=0; i<static_inst->numDestRegs(); ++i)
        destreg_info << static_inst->destRegIdx(i) << " ";
    if(static_inst->numDestRegs() == 0)
        destreg_info << "None";

    if(corrupt_srcreg) {
        static_inst->_srcRegIdx[reg_idx] = BITFLIP(static_inst->srcRegIdx(reg_idx), bit_idx);

        /**/
        DPRINTF(FIReport, "     * Flip %u-th srcReg (reg idx = %u)\n", reg_idx + 1, reg_idx);
        srcreg_info << " ->  ";
        for(int i=0; i<static_inst->numSrcRegs(); ++i)
            srcreg_info << static_inst->srcRegIdx(i) << " ";
    }
    else {
        reg_idx -= static_inst->numSrcRegs();
        static_inst->_destRegIdx[reg_idx] = BITFLIP(static_inst->destRegIdx(reg_idx), bit_idx);

        /**/
        DPRINTF(FIReport, "     * Flip %u-th destReg (reg idx = %u)\n", reg_idx + 1, reg_idx);
        destreg_info << " ->  ";
        for(int i=0; i<static_inst->numDestRegs(); ++i)
            destreg_info << static_inst->destRegIdx(i) << " ";
    }

    DPRINTF(FIReport, "     * src reg: %s\n", srcreg_info.str());
    DPRINTF(FIReport, "     * dest reg: %s\n", destreg_info.str());
}

} // namespace Minor
