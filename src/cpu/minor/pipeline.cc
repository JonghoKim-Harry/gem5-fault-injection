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

#include <algorithm>

#include "cpu/minor/decode.hh"
#include "cpu/minor/execute.hh"
#include "cpu/minor/fetch1.hh"
#include "cpu/minor/fetch2.hh"
#include "cpu/minor/pipeline.hh"
#include "debug/Drain.hh"
#include "debug/MinorCPU.hh"
#include "debug/MinorTrace.hh"
#include "debug/Quiesce.hh"
#include "debug/ShsTemp.hh"

// JONGHO
#include "base/instinfo.hh"
#include "base/softerror.hh"
#include "base/vulnerable.hh"
#include "debug/Bubble.hh"
#include "debug/PrintAllFU.hh"
#include "debug/ForwardInstData.hh"

namespace Minor
{

Pipeline::Pipeline(MinorCPU &cpu_, MinorCPUParams &params) :
    Ticked(cpu_, &(cpu_.BaseCPU::numCycles)),
    cpu(cpu_),
    allow_idling(params.enableIdling),
    f1ToF2(cpu.name() + ".f1ToF2", "lines",
        params.fetch1ToFetch2ForwardDelay),
    f2ToF1(cpu.name() + ".f2ToF1", "prediction",
        params.fetch1ToFetch2BackwardDelay, true),
    f2ToD(cpu.name() + ".f2ToD", "insts",
        params.fetch2ToDecodeForwardDelay),
    dToE(cpu.name() + ".dToE", "insts",
        params.decodeToExecuteForwardDelay),
    eToF1(cpu.name() + ".eToF1", "branch",
        params.executeBranchDelay),
    execute(cpu.name() + ".execute", cpu, params,
        dToE.output(), eToF1.input()),
    decode(cpu.name() + ".decode", cpu, params,
        f2ToD.output(), dToE.input(), execute.inputBuffer),
    fetch2(cpu.name() + ".fetch2", cpu, params,
        f1ToF2.output(), eToF1.output(), f2ToF1.input(), f2ToD.input(),
        decode.inputBuffer),
    fetch1(cpu.name() + ".fetch1", cpu, params,
        eToF1.output(), f1ToF2.input(), f2ToF1.output(), fetch2.inputBuffer),
    activityRecorder(cpu.name() + ".activity", Num_StageId,
        /* The max depth of inter-stage FIFOs */
        std::max(params.fetch1ToFetch2ForwardDelay,
        std::max(params.fetch2ToDecodeForwardDelay,
        std::max(params.decodeToExecuteForwardDelay,
        params.executeBranchDelay)))),
    needToSignalDrained(false)
{
    // JONGHO: Print all FUs if the debug flag "PrintAllFU" is set
    if(DTRACE(PrintAllFU)) {
        std::ostream& debug_file = Trace::output();
        execute.printAllFU(debug_file);
    }

    if (params.fetch1ToFetch2ForwardDelay < 1) {
        fatal("%s: fetch1ToFetch2ForwardDelay must be >= 1 (%d)\n",
            cpu.name(), params.fetch1ToFetch2ForwardDelay);
    }

    if (params.fetch2ToDecodeForwardDelay < 1) {
        fatal("%s: fetch2ToDecodeForwardDelay must be >= 1 (%d)\n",
            cpu.name(), params.fetch2ToDecodeForwardDelay);
    }

    if (params.decodeToExecuteForwardDelay < 1) {
        fatal("%s: decodeToExecuteForwardDelay must be >= 1 (%d)\n",
            cpu.name(), params.decodeToExecuteForwardDelay);
    }

    if (params.executeBranchDelay < 1) {
        fatal("%s: executeBranchDelay must be >= 1\n",
            cpu.name(), params.executeBranchDelay);
    }

    // JONGHO
    DPRINTF(ForwardInstData, "Instruction Width: %u\n", params.decodeInputWidth);

    // JONGHO: Register fault injection
    if(params.injectComp == "f1ToF2") {
        f1ToF2.registerFi(params.injectTime, params.injectLoc);
    }
    else if(params.injectComp == "f2ToD") {
        f2ToD.registerFi(params.injectTime, params.injectLoc);
    }
    else if(params.injectComp == "dToE") {
        dToE.registerFi(params.injectTime, params.injectLoc);
    }
    else if(params.injectComp == "eToF1") {
        eToF1.registerFi(params.injectTime, params.injectLoc);
    }
    else if(params.injectComp == "f2ToF1") {
        f2ToF1.registerFi(params.injectTime, params.injectLoc);
    }
}

// JONGHO
/* Draw pipeline datapath */
void
Pipeline::drawDatapath(std::ostream& os)
{
/*
         11111111112222222222333333333344444444445555555555666666666677777777778
12345678901234567890123456789012345678901234567890123456789012345678901234567890
    <--- [F->$] <-----+                       <--- [E->$] <-----+
                      |                                         |
($) ---> [$->F] ---> (F) ---> [F->D] ---> (D) ---> [D->E] ---> (E)
*/
 
    /* 1st Line: Pipeline registers in which address is stored */
    os.width(4);
    os  << " ";
    os  << "<--- [F->$] <-----+";
    os.width(23);
    os  << " ";
    os  << "<--- [E->$] <-----+" << std::endl;

    /* 2nd Line */
    os.width(22);
    os  << " ";
    os  << "|";
    os.width(41);
    os  << " ";
    os  << "|" << std::endl;

    /* 3rd Line: Pipeline registers in which line/instruction/uop is stored */
    os  << "($) ---> [$->F] ---> (F) ---> [F->D] ---> (D) ---> [D->E] ---> (E)" << std::endl;
}

// JONGHO
/* Draw ascii-arted picture of pipeline status */
void
Pipeline::drawDataflow(std::ostream& os, DataFlow flow) const
{
    const ForwardLineData& f1ToF2_data = f1ToF2.buffer[-1];
    const ForwardInstData& f2ToD_data = f2ToD.buffer[-1];
    const ForwardInstData& dToE_data = dToE.buffer[-1];
    const BranchData& eToF1_data = eToF1.buffer[-1];
    const BranchData& f2ToF1_data = f2ToF1.buffer[-1];

    if(flow == DataFlow::INPUT) {
        os << "______________________________________________________________________" << std::endl;
        os << "[SNAPSHOT]" << std::endl;
    }

/*
         11111111112222222222333333333344444444445555555555666666666677777777778
12345678901234567890123456789012345678901234567890123456789012345678901234567890
    data        _BB_                          _BB_        data
    <--- [F->$] <-----+                       <--- [E->$] <-----+
                      |                                         |
($) ---> [$->F] ---> (F) ---> [F->D] ---> (D) ---> [D->E] ---> (E)
    _BB_        _BB_     data        data     data        data
*/

    /*
     * Draw input/output of pipeline registers
     * in which address is stored
     */
    if(flow == DataFlow::INPUT)
        os.width(16);
    else if(flow == DataFlow::OUTPUT)
        os.width(4);
    os << " ";
    os.width(4);
    os << ((f2ToF1_data.isBubble() || !f2ToF1_data.isBranch())?" BB ":"data");
    os.width(38);
    os << " ";
    os.width(4);
    os << ((eToF1_data.isBubble() || !eToF1_data.isBranch())?" BB ":"data");
    os << std::endl;

    /* Draw pipeline datapath */
    drawDatapath(os);

    /*
     * Draw input/output of pipeline registers
     * in which line/instruction/micro-operation is stored
     */
    if(flow == DataFlow::INPUT)
        os.width(4);
    else if(flow == DataFlow::OUTPUT)
        os.width(16);
    os << " ";
    os.width(4);
    os << (f1ToF2_data.isBubble()?" BB ":"data");
    os.width(17);
    os << " ";
    os.width(4);
    os << (f2ToD_data.isBubble()?" BB ":"data");
    os.width(17);
    os << " ";
    os.width(4);
    os << (dToE_data.isBubble()?" BB ":"data");
    os << std::endl << std::endl;

    if(flow == DataFlow::OUTPUT) {
        os << "[Tick] about " << curTick() << " (because of delay)" << std::endl;

        /* [$->F] */
        os  << "[$->F] ";
        f1ToF2_data.reportData(os);
        os  << std::endl;

        /* [F->D] */
        os  << "[F->D] ";
        f2ToD_data.reportData(os);
        os  << std::endl;

        /* [D->E] */
        os  << "[D->E] ";
        dToE_data.reportData(os);
        os  << std::endl;

        /* [E->$] */
        os  << "[E->$] " << eToF1_data << std::endl;
        os.width(7);
        os << " ";
        os << "Predicted: " << (eToF1_data.inst->predictedTaken?"T":"NT") << std::endl;

        /* [F->$] */
        os  << "[F->$] " << f2ToF1_data << std::endl;
    }
}

// JONGHO
/*
 *  Note that this method will be called at initialization stage,
 *  so you have to use only predefined-stats
 */
void
Pipeline::regStats()
{
    Ticked::regStats();

    /*
     *  Rule for stat naming
     *
     *   1) Stat's name consists of one or more tokens,
     *      which is seperated by '.' (dot)
     *       (ex) Hello.Jongho.Kim
     *
     *   2) Empty token is not allowed
     *       (ex) Hello.Jongho.
     *
     *   3) Each character in token can be
     *      alphabet, number, or '_' (underscore)
     *
     *   4) First characters of each token can't be number
     */

    /*
     * Register custom statistics for Minor CPU pipeline.
     */
    snapshot_count.name("num_snapshot")
                    .desc("JONGHO: Number of snapshots")
                    ;

    /* [$->F] */
    f1ToF2_bubble_ticks.name("Pipereg.Cache2Fetch.bubble_ticks")
                        .desc("JONGHO: [$->F] How long is it bubble?")
                        ;
    f1ToF2_bubble_ticks_percentage.name("Pipereg.Cache2Fetch.bubble_ticks_percentage")
                                .desc("JONGHO: [$->F] BB\% among total time")
                                ;
    f1ToF2_bubble_ticks_percentage = 100 * f1ToF2_bubble_ticks / simTicks;

    /* [F->D] */
    f2ToD_bubble_ticks.name("Pipereg.Fetch2Decode.bubble_ticks")
                        .desc("JONGHO: [F->D] How long is it bubble?")
                        ;
    f2ToD_bubble_ticks_percentage.name("Pipereg.Fetch2Decode.bubble_ticks_percentage")
                                .desc("JONGHO: [F->D] BB\% among total time")
                                ;
    f2ToD_bubble_ticks_percentage = 100 * f2ToD_bubble_ticks / simTicks;

    /* [D->E] */
    dToE_bubble_ticks.name("Pipereg.Decode2Execute.bubble_ticks")
                        .desc("JONGHO: [D->E] How long is it bubble?")
                        ;
    dToE_bubble_ticks_percentage.name("Pipereg.Decode2Execute.bubble_ticks_percentage")
                                .desc("JONGHO: [D->E] BB\% among total time")
                                ;
    dToE_bubble_ticks_percentage = 100 * dToE_bubble_ticks / simTicks;

    /* [E->$] */
    eToF1_bubble_ticks.name("Pipereg.Execute2Cache.bubble_ticks")
                        .desc("JONGHO: [E->$] How long is it bubble?")
                        ;
    eToF1_bubble_ticks_percentage.name("Pipereg.Execute2Cache.bubble_ticks_percentage")
                                .desc("JONGHO: [E->$] BB\% among total time")
                                ;
    eToF1_bubble_ticks_percentage = 100 * eToF1_bubble_ticks / simTicks;

    /* */
    eToF1_predT_T_ticks.name("Pipereg.Execute2Cache.predT_T_ticks")
                            .desc("JONGHO: [E->$] How long it is predicted to be TAKEN and then TAKEN?")
                            ;

    /* These stats RARELY DEPEND ON HARDWARE */
    predT_T_count.name("Inst.predT_T_count")
                            .desc("JONGHO: How many (dynamic) instruction is predicted to be TAKEN and then TAKEN?")
                            ;
    eToF1_predT_WT_ticks.name("Pipereg.Execute2Cache.predT_WT_ticks")
                            .desc("JONGHO: [E->$] How long it is predicted to be TAKEN and then TAKEN, but wrong branch target?")
                            ;
    predT_WT_count.name("Inst.predT_WT_count")
                            .desc("JONGHO: How many (dynamic) instruction is predicted to be TAKEN and then TAKEN, but wrong branch target?")
                            ;
    eToF1_predT_NT_ticks.name("Pipereg.Execute2Cache.predT_NT_ticks")
                            .desc("JONGHO: [E->$] How long it is predicted to be TAKEN but NOT TAKEN?")
                            ;
    predT_NT_count.name("Inst.predT_NT_count")
                            .desc("JONGHO: How many (dynamic) instruction is predicted to be TAKEN but NOT TAKEN?")
                            ;
    eToF1_predNT_T_ticks.name("Pipereg.Execute2Cache.predNT_T_ticks")
                            .desc("JONGHO: [E->$] How long it is predicted to be NOT TAKEN but TAKEN?")
                            ;
    predNT_T_count.name("Pipereg.predNT_T_count")
                            .desc("JONGHO: How many (dynamic) instruction is predicted to be NOT TAKEN but TAKEN?")
                            ;

    /* Probabilities */
    /* P(T|pred-T) */
    prob_T_given_predT_percentage.name("Pipereg.probability_T_given_predT")
                                .desc("JONGHO: P(T|pred-T)")
                                ;
    prob_T_given_predT_percentage = 100 * predT_T_count / (predT_WT_count + predT_T_count + predT_NT_count);

    /* P(NT|pred-T) */
    prob_NT_given_predT_percentage.name("Pipereg.probability_NT_given_predT")
                                .desc("JONGHO: P(NT|pred-T)")
                                ;
    prob_NT_given_predT_percentage = 100 * predT_NT_count / (predT_WT_count + predT_T_count + predT_NT_count);

    /* P(WT|pred-T) */
    prob_WT_given_predT_percentage.name("Pipereg.probability_WT_given_predT")
                                .desc("JONGHO: P(WT|pred-T)")
                                ;
    prob_WT_given_predT_percentage = 100 * predT_WT_count / (predT_WT_count + predT_T_count + predT_NT_count);

    /* Time in which data in [E->$] is vulnerable */
    eToF1_vul_ticks.name("Pipereg.Execute2Fetch.vulnerable_time")
                    .desc("JONGHO: Time in which data in [E->$] is vulnerable")
                    ;
    eToF1_vul_ticks = eToF1_predT_WT_ticks + eToF1_predT_NT_ticks + eToF1_predNT_T_ticks;
    eToF1_vul_ticks_percentage.name("Pipereg.Execute2Fetch.vulnerable_time_among_reuntime_percentage")
                                .desc("JONGHO: Proportion of time in which data in [E->$] is vulnerable among runtime (%)")
                                ;
    eToF1_vul_ticks_percentage = 100 * eToF1_vul_ticks / simTicks;

    /* [F->$] */
    f2ToF1_bubble_ticks.name("Pipereg.Fetch2Cache.bubble_ticks")
                        .desc("JONGHO: [F->$] How long is it bubble?")
                        ;
    f2ToF1_bubble_ticks_percentage.name("Pipereg.Fetch2Cache.bubble_ticks_percentage")
                                .desc("JONGHO: [F->$] BB\% among total time")
                                ;
    f2ToF1_bubble_ticks_percentage = 100 * f2ToF1_bubble_ticks / simTicks;
    f2ToF1_predT_ticks.name("Pipereg.Fetch2Cache.predT_ticks")
                            .desc("JONGHO: [F->$] How long it is predicted to be TAKEN?")
                            ;

    /* Time in which data in [E->$] or [F->$] is vulnerable */
    addr_vul_ticks.name("Pipereg.AddrPipereg.vul_ticks")
                    .desc("JONGHO: TIME in which data in [E->$] or [F->$] is vulnerable")
                    ;
    addr_vul_ticks = eToF1_predT_T_ticks + eToF1_predT_WT_ticks + eToF1_predT_NT_ticks + eToF1_predNT_T_ticks;

    addr_vul_ticks_percentage.name("Pipereg.AddrPipereg.vul_ticks_percentage")
                    .desc("JONGHO: TIME in which data in [E->$] or [F->$] is vulnerable among runtime (%)")
                    ;
    addr_vul_ticks_percentage = 100 * addr_vul_ticks / simTicks;

    /* P([E->$] Vul | Addr Vul) */
    eToF1_vul_given_addr_vul_ticks_percentage.name("Pipereg.Execute2Cache.prob_given_addr_vul")
                                            .desc("JONGHO: P([E->$] Vul | Addr Vul) (%)")
                                            ;
    eToF1_vul_given_addr_vul_ticks_percentage = 100 * (eToF1_predT_WT_ticks + eToF1_predT_NT_ticks + eToF1_predNT_T_ticks) / addr_vul_ticks;

    /* P([F->$] Vul | Addr Vul) */
    f2ToF1_vul_given_addr_vul_ticks_percentage.name("Pipereg.Fetch2Cache.prob_given_addr_vul")
                                            .desc("JONGHO: P([F->$] Vul | Addr Vul) (%)")
                                            ;
    f2ToF1_vul_given_addr_vul_ticks_percentage = 100 * eToF1_predT_T_ticks / addr_vul_ticks;
}

void
Pipeline::minorTrace() const
{
    fetch1.minorTrace();
    f1ToF2.minorTrace();
    f2ToF1.minorTrace();
    fetch2.minorTrace();
    f2ToD.minorTrace();
    decode.minorTrace();
    dToE.minorTrace();
    execute.minorTrace();
    eToF1.minorTrace();
    activityRecorder.minorTrace();
}

void
Pipeline::evaluate()
{
    std::ostream& debug_file = Trace::output();

    // JONGHO
    /* Output of pipeline registers */
    ForwardLineData f1ToF2_output(*f1ToF2.output().outputWire);
    ForwardInstData f2ToD_output(*f2ToD.output().outputWire);
    ForwardInstData dToE_output(*dToE.output().outputWire);
    BranchData eToF1_output(*eToF1.output().outputWire);
    BranchData f2ToF1_output(*f2ToF1.output().outputWire);

    // JONGHO
    /*
     *                       WHAT THIS METHOD DO?
     *
     *
     *              prev stage           pipeline register          next stage
     *   n-th        +------+             +------+------+            +------+
     * SNAPSHOT      |      |             |//////| data1|            |      |
     *               +------+             +------+------+            +------+
     *
     *
     * Vulnerable::evaluate():
     *
     *  - If there is fault injection, corrupt data1.
     *    Else, do nothing.
     *
     * pipeline stage evalute():
     *
     *  - Read input, which is output of previous stage,
     *    from pipeline register.
     *
     *              prev stage          pipeline register          next stage
     *               +------+            +------+------+   data1    +------+
     *               |      |            |//////| data1|  ------>   |      |
     *               +------+            +------+------+            +------+
     *
     *  - Process input to generate output.
     *
     *              prev stage          pipeline register          next stage
     *               +------+            +------+------+            +------+
     *               |@@@@@@|            |//////| data1|            |@@@@@@|
     *               +------+            +------+------+            +------+
     *             (running...)                                   (running...)
     *
     *  - Write output  into pipeline register.
     *
     *              prev stage           pipeline register          next stage
     *               +------+    data2   +------+------+            +------+
     *               |      |   ------>  |//////| data1|            |      |
     *               +------+            +------+------+            +------+
     *
     *              prev stage          pipeline register          next stage
     *               +------+            +------+------+            +------+
     *               |      |            |data2 | data1|            |      |
     *               +------+            +------+------+            +------+
     *                                  (A pipeline register
     *                                   can store only one
     *                                   data at once:
     *                                   This is not a valid
     *                                   state of hardware,
     *                                   so it needs to delete
     *                                   'data1')
     *
     *
     * pipeline register evaluate():
     *
     *              prev stage          pipeline register          next stage
     * (n+1)-th      +------+            +------+------+            +------+
     * SNAPSHOT      |      |            |//////| data2|            |      |
     *               +------+            +------+------+            +------+
     */

    // JONGHO
    /*
     * Fault Injection into Pipeline Registers
     *
     *   Inject fault into data in pipeline registers before each pipeline
     *   stages do evaluate(). This enables fault injection timing be correct
     */
    Vulnerable::evaluate();

    cpu.injectFaultRegFunc();

    //HwiSoo
    execute.getLSQ().injectFaultLSQFunc();
    execute.getLSQ().FIProfiling();

    //ybkim: Fault injection on FU
    //TODO: move these flags to the pipeline
    if (!cpu.isFaultInjectedToFu && cpu.injectFaultToFu)
        cpu.isFaultInjectedToFu = execute.injectFaultToFu();

    // JONGHO: Output at the start of evaluate()
    if(DTRACE(Bubble))
        drawDataflow(debug_file, DataFlow::OUTPUT);

    /* Note that it's important to evaluate the stages in order to allow
     *  'immediate', 0-time-offset TimeBuffer activity to be visible from
     *  later stages to earlier ones in the same cycle */
    execute.evaluate();
    decode.evaluate();
    fetch2.evaluate();
    fetch1.evaluate();

    if (DTRACE(MinorTrace))
        minorTrace();

    /* Update the time buffers after the stages */
    f1ToF2.evaluate();
    f2ToF1.evaluate();
    f2ToD.evaluate();
    dToE.evaluate();
    eToF1.evaluate();

    // JONGHO: Input at the start of evaluate()
    if(DTRACE(Bubble))
        drawDataflow(debug_file, DataFlow::INPUT);

/*
    if(DTRACE(Bubble)) {
        std::vector<Addr> fetch1_addr_list = InstInfo::fetch1_addr();
        std::vector<Addr> fetch2_addr_list = InstInfo::fetch2_addr();
        std::vector<Minor::MinorDynInstPtr> decode_op_list = InstInfo::decode_op();
        std::vector<Addr> execute_addr_list = InstInfo::execute_addr();
        unsigned int max_size = std::max(std::max(std::max(fetch1_addr_list.size(), fetch2_addr_list.size()), decode_op_list.size()), execute_addr_list.size());
        debug_file << std::showbase << std::hex << std::left;

        for(int j=0; j<max_size; ++j) {
            debug_file.width(20);
            if(j < fetch1_addr_list.size())
                debug_file << fetch1_addr_list[j];
            else
                debug_file << " ";

            debug_file.width(21);
            if(j < fetch2_addr_list.size())
                debug_file << fetch2_addr_list[j];
            else
                debug_file << " ";

            debug_file.width(18);
            if(j < decode_op_list.size()) {
                if(decode_op_list[j]->staticInst->isMicroop()) {
                    std::stringstream ss1, ss2;
                    ss1 << std::hex << decode_op_list[j]->pc.pc();
                    ss2 << decode_op_list[j]->pc.microPC();
                    debug_file << "0x" + ss1.str() + "." + ss2.str();
                }
                else
                    debug_file << decode_op_list[j]->pc.pc();
            }
            else
                debug_file << " ";

            if(j < execute_addr_list.size())
                debug_file << execute_addr_list[j];

            debug_file << std::endl;
        }

        debug_file << std::noshowbase << std::dec << std::right;
        debug_file << std::endl;

        InstInfo::clear_fetch1_addr();
        InstInfo::clear_fetch2_addr();
        InstInfo::clear_decode_op();
        InstInfo::clear_execute_addr();
    }

*/

    /* The activity recorder must be be called after all the stages and
     *  before the idler (which acts on the advice of the activity recorder */
    activityRecorder.evaluate();

    if (allow_idling) {
        /* Become idle if we can but are not draining */
        if (!activityRecorder.active() && !needToSignalDrained) {
            DPRINTF(Quiesce, "Suspending as the processor is idle\n");
            stop();
        }

        /* Deactivate all stages.  Note that the stages *could*
         *  activate and deactivate themselves but that's fraught
         *  with additional difficulty.
         *  As organised herre */
        activityRecorder.deactivateStage(Pipeline::CPUStageId);
        activityRecorder.deactivateStage(Pipeline::Fetch1StageId);
        activityRecorder.deactivateStage(Pipeline::Fetch2StageId);
        activityRecorder.deactivateStage(Pipeline::DecodeStageId);
        activityRecorder.deactivateStage(Pipeline::ExecuteStageId);
    }

    if (needToSignalDrained) /* Must be draining */
    {
        DPRINTF(Drain, "Still draining\n");
        if (isDrained()) {
            DPRINTF(Drain, "Signalling end of draining\n");
            cpu.signalDrainDone();
            needToSignalDrained = false;
            stop();
        }
    }

    /*
     * JONGHO: Profile data in pipeline registers.
     *         Just use pipeline register's output in SNAPSHOT
     */
    if(f1ToF2_output.isBubble())
        f1ToF2_bubble_ticks += (curTick() - last_snapshot_time);
    if(f2ToD_output.isBubble())
        f2ToD_bubble_ticks += (curTick() - last_snapshot_time);
    if(dToE_output.isBubble())
        dToE_bubble_ticks += (curTick() - last_snapshot_time);
    if(eToF1_output.isBubble() || (!eToF1_output.isBranch()))
        eToF1_bubble_ticks += (curTick() - last_snapshot_time);
    if(f2ToF1_output.isBubble() || (!f2ToF1_output.isBranch()))
        f2ToF1_bubble_ticks += (curTick() - last_snapshot_time);

    /*
     * To profile branch target stored in pipeline register [E->$]
     */
    switch(eToF1_output.reason) {
        /*
         * Predicted:  T
         * Actual:     T
         */
        case BranchData::CorrectlyPredictedBranch:
            eToF1_predT_T_ticks += (curTick() - last_snapshot_time);
            ++ predT_T_count;
            break;

        /*
         * Predicted: WT (wrong branch target)
         * Actual:     T
         */
        case BranchData::BadlyPredictedBranchTarget:
            eToF1_predT_WT_ticks += (curTick() - last_snapshot_time);
            ++ predT_WT_count;
            break;

        /*
         * Predicted:  T
         * Actual:    NT
         */
        case BranchData::BadlyPredictedBranch:
            eToF1_predT_NT_ticks += (curTick() - last_snapshot_time);
            ++ predT_NT_count;
            break;

        /*
         * Predicted: NT
         * Actual:     T
         */
        case BranchData::UnpredictedBranch:
            eToF1_predNT_T_ticks += (curTick() - last_snapshot_time);
            ++ predNT_T_count;
            break;

        default:
            break;
    }

    /*
     * To profile branch target prediction stored in pipeline register [F->$]
     */
    if((!f2ToF1_output.isBubble()) && f2ToF1_output.isBranch()) {
        if(f2ToF1_output.inst->predictedTaken)
            f2ToF1_predT_ticks += (curTick() - last_snapshot_time);
    }

    /* End of profiling pipeline register data */
    ++snapshot_count;
    last_snapshot_time = curTick();
}

MinorCPU::MinorCPUPort &
Pipeline::getInstPort()
{
    return fetch1.getIcachePort();
}

MinorCPU::MinorCPUPort &
Pipeline::getDataPort()
{
    return execute.getDcachePort();
}

void
Pipeline::wakeupFetch(ThreadID tid)
{
    fetch1.wakeupFetch(tid);
}

bool
Pipeline::drain()
{
    DPRINTF(MinorCPU, "Draining pipeline by halting inst fetches. "
        " Execution should drain naturally\n");

    execute.drain();

    /* Make sure that needToSignalDrained isn't accidentally set if we
     *  are 'pre-drained' */
    bool drained = isDrained();
    needToSignalDrained = !drained;

    return drained;
}

void
Pipeline::drainResume()
{
    DPRINTF(Drain, "Drain resume\n");

    for (ThreadID tid = 0; tid < cpu.numThreads; tid++) {
        fetch1.wakeupFetch(tid);
    }

    execute.drainResume();
}

bool
Pipeline::isDrained()
{
    bool fetch1_drained = fetch1.isDrained();
    bool fetch2_drained = fetch2.isDrained();
    bool decode_drained = decode.isDrained();
    bool execute_drained = execute.isDrained();

    bool f1_to_f2_drained = f1ToF2.empty();
    bool f2_to_f1_drained = f2ToF1.empty();
    bool f2_to_d_drained = f2ToD.empty();
    bool d_to_e_drained = dToE.empty();

    bool ret = fetch1_drained && fetch2_drained &&
        decode_drained && execute_drained &&
        f1_to_f2_drained && f2_to_f1_drained &&
        f2_to_d_drained && d_to_e_drained;

    DPRINTF(MinorCPU, "Pipeline undrained stages state:%s%s%s%s%s%s%s%s\n",
        (fetch1_drained ? "" : " Fetch1"),
        (fetch2_drained ? "" : " Fetch2"),
        (decode_drained ? "" : " Decode"),
        (execute_drained ? "" : " Execute"),
        (f1_to_f2_drained ? "" : " F1->F2"),
        (f2_to_f1_drained ? "" : " F2->F1"),
        (f2_to_d_drained ? "" : " F2->D"),
        (d_to_e_drained ? "" : " D->E")
        );

    return ret;
}

}
