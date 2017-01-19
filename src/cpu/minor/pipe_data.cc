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
ForwardLineData::injectFault(unsigned int loc)
{
    DPRINTF(FICallTrace, "injectFault() @ForwardLineData\n");

    /** 32-bit ISA */
    const unsigned int BYTE_PER_INST = sizeof(uint32_t);
    const unsigned int BIT_PER_INST = sizeof(uint32_t) * BIT_PER_BYTE;

    /** To log instruction change */
    ArmISA::Decoder *decoder = new ArmISA::Decoder(nullptr);

    /** Same with (loc - (loc % BIT_PER_INST)) / (BIT_PER_BYTE) */
    unsigned int offset_to_inst = BYTE_PER_INST * (loc / BIT_PER_INST);

    /** fault-injected instruction's address */
    Addr inst_addr = lineBaseAddr + offset_to_inst;

    /** Print out fault injection informations */
    DPRINTF(FIReport, "--- Fault Injection ---\n");
    DPRINTF(FIReport, "     * loc:  %u\n", loc);
    DPRINTF(FIReport, "     * addr: %#x\n", inst_addr);

    /**
     *  Bit Flip Procss: Do fault injection if and only if
     *                   the line is neither bubble nor fault
     */
    if(bubbleFlag)
        DPRINTF(FIReport, "     * Injected into BUBBLE\n");
    else if(isFault())
        DPRINTF(FIReport, "     * Injected into FAULT\n");
    else {
        /** original binary instruction */
        const uint32_t golden_bin = *(uint32_t *)&line[offset_to_inst];

        /** Soft error is transient, so we can use cached golden instruction */
        const std::string golden_inst = decoder->decodeInst(golden_bin)->generateDisassembly(inst_addr, debugSymbolTable);

        /** actual bit flip */
        line[loc/BIT_PER_BYTE] = BITFLIP(line[loc/BIT_PER_BYTE], loc%BIT_PER_BYTE);

        /** fault-injected binary instruction */
        const uint32_t faulty_bin = *(uint32_t *)&line[offset_to_inst];

        /** 
         *  We can NOT use cached faulty instruction,
         *  so set address parameter to 0 to prevent use of decode cache
         */
        const std::string faulty_inst = decoder->decodeInst(faulty_bin)->generateDisassembly(0, debugSymbolTable);

        /**
         *  Print out changes of binary instruction and instruction mnemonic
         *  by injected soft error
         */
        DPRINTF(FIReport, "     * bin:  %#x -> %#x\n", golden_bin, faulty_bin);
        DPRINTF(FIReport, "     * inst: %s -> %s\n", golden_inst, faulty_inst);
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

void
ForwardInstData::injectFault(unsigned int loc)
{
    DPRINTF(FICallTrace, "injectFault() @ForwardInstData\n");
    // TODO
}

}
