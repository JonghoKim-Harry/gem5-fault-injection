/*
 * Copyright (c) 2010-2014 ARM Limited
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
 * Copyright (c) 2007-2008 The Florida State University
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
 * Authors: Stephen Hines
 */
#ifndef __ARCH_ARM_MACROMEM_HH__
#define __ARCH_ARM_MACROMEM_HH__

#include "arch/arm/insts/pred_inst.hh"
#include "arch/arm/tlb.hh"

namespace ArmISA
{

static inline unsigned int
number_of_ones(int32_t val)
{
    uint32_t ones = 0;
    for (int i = 0; i < 32; i++ )
    {
        if ( val & (1<<i) )
            ones++;
    }
    return ones;
}

/**
 * Base class for Memory microops
 */
class MicroOp : public PredOp
{ 
    // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "MicroOp";
    }

  protected:
    MicroOp(const char *mnem, ExtMachInst machInst, OpClass __opClass)
            : PredOp(mnem, machInst, __opClass)
    {
    }

  public:
    void
    advancePC(PCState &pcState) const
    {
        if (flags[IsLastMicroop]) {
            pcState.uEnd();
        } else if (flags[IsMicroop]) {
            pcState.uAdvance();
        } else {
            pcState.advance();
        }
    }
};

class MicroOpX : public ArmStaticInst
{
   // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "MicroOpX";
    }

 protected:
    MicroOpX(const char *mnem, ExtMachInst machInst, OpClass __opClass)
            : ArmStaticInst(mnem, machInst, __opClass)
    {}

  public:

    void
    advancePC(PCState &pcState) const
    {
        if (flags[IsLastMicroop]) {
            pcState.uEnd();
        } else if (flags[IsMicroop]) {
            pcState.uAdvance();
        } else {
            pcState.advance();
        }
    }
};

/**
 * Microops for Neon loads/stores
 */
class MicroNeonMemOp : public MicroOp
{ 
   // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "MicroNeonMemOp";
    }

 protected:
    RegIndex dest, ura;
    uint32_t imm;
    unsigned memAccessFlags;

    MicroNeonMemOp(const char *mnem, ExtMachInst machInst, OpClass __opClass,
                   RegIndex _dest, RegIndex _ura, uint32_t _imm)
            : MicroOp(mnem, machInst, __opClass),
              dest(_dest), ura(_ura), imm(_imm),
              memAccessFlags(TLB::MustBeOne)
    {
    }
};

/**
 * Microops for Neon load/store (de)interleaving
 */
class MicroNeonMixOp : public MicroOp
{
   // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "MicroNeonMixOp";
    }

 protected:
    RegIndex dest, op1;
    uint32_t step;

    MicroNeonMixOp(const char *mnem, ExtMachInst machInst, OpClass __opClass,
                   RegIndex _dest, RegIndex _op1, uint32_t _step)
            : MicroOp(mnem, machInst, __opClass),
              dest(_dest), op1(_op1), step(_step)
    {
    }
};

class MicroNeonMixLaneOp : public MicroNeonMixOp
{
   // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "MicroNeonMixLaneOp";
    }

 protected:
    unsigned lane;

    MicroNeonMixLaneOp(const char *mnem, ExtMachInst machInst,
                       OpClass __opClass, RegIndex _dest, RegIndex _op1,
                       uint32_t _step, unsigned _lane)
            : MicroNeonMixOp(mnem, machInst, __opClass, _dest, _op1, _step),
              lane(_lane)
    {
    }
};

/**
 * Microops for AArch64 NEON load/store (de)interleaving
 */
class MicroNeonMixOp64 : public MicroOp
{
   // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "MicroNeonMixOp64";
    }

 protected:
    RegIndex dest, op1;
    uint8_t eSize, dataSize, numStructElems, numRegs, step;

    MicroNeonMixOp64(const char *mnem, ExtMachInst machInst, OpClass __opClass,
                     RegIndex _dest, RegIndex _op1, uint8_t _eSize,
                     uint8_t _dataSize, uint8_t _numStructElems,
                     uint8_t _numRegs, uint8_t _step)
        : MicroOp(mnem, machInst, __opClass), dest(_dest), op1(_op1),
          eSize(_eSize), dataSize(_dataSize), numStructElems(_numStructElems),
          numRegs(_numRegs), step(_step)
    {
    }
};

class MicroNeonMixLaneOp64 : public MicroOp
{
   // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "MicroNeonMixLaneOp64";
    }

 protected:
    RegIndex dest, op1;
    uint8_t eSize, dataSize, numStructElems, lane, step;
    bool replicate;

    MicroNeonMixLaneOp64(const char *mnem, ExtMachInst machInst,
                         OpClass __opClass, RegIndex _dest, RegIndex _op1,
                         uint8_t _eSize, uint8_t _dataSize,
                         uint8_t _numStructElems, uint8_t _lane, uint8_t _step,
                         bool _replicate = false)
        : MicroOp(mnem, machInst, __opClass), dest(_dest), op1(_op1),
          eSize(_eSize), dataSize(_dataSize), numStructElems(_numStructElems),
          lane(_lane), step(_step), replicate(_replicate)
    {
    }
};

/**
 * Base classes for microcoded AArch64 NEON memory instructions.
 */
class VldMultOp64 : public PredMacroOp
{
    // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "VldMultOp64";
    }

 protected:
    uint8_t eSize, dataSize, numStructElems, numRegs;
    bool wb;

    VldMultOp64(const char *mnem, ExtMachInst machInst, OpClass __opClass,
                RegIndex rn, RegIndex vd, RegIndex rm, uint8_t eSize,
                uint8_t dataSize, uint8_t numStructElems, uint8_t numRegs,
                bool wb);
};

class VstMultOp64 : public PredMacroOp
{
    // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "VstMultOp64";
    }

 protected:
    uint8_t eSize, dataSize, numStructElems, numRegs;
    bool wb;

    VstMultOp64(const char *mnem, ExtMachInst machInst, OpClass __opClass,
                RegIndex rn, RegIndex vd, RegIndex rm, uint8_t eSize,
                uint8_t dataSize, uint8_t numStructElems, uint8_t numRegs,
                bool wb);
};

class VldSingleOp64 : public PredMacroOp
{
    // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "VldSingleOp64";
    }

 protected:
    uint8_t eSize, dataSize, numStructElems, index;
    bool wb, replicate;

    VldSingleOp64(const char *mnem, ExtMachInst machInst, OpClass __opClass,
                  RegIndex rn, RegIndex vd, RegIndex rm, uint8_t eSize,
                  uint8_t dataSize, uint8_t numStructElems, uint8_t index,
                  bool wb, bool replicate = false);
};

class VstSingleOp64 : public PredMacroOp
{
    // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "VstSingleOp64";
    }

 protected:
    uint8_t eSize, dataSize, numStructElems, index;
    bool wb, replicate;

    VstSingleOp64(const char *mnem, ExtMachInst machInst, OpClass __opClass,
                  RegIndex rn, RegIndex vd, RegIndex rm, uint8_t eSize,
                  uint8_t dataSize, uint8_t numStructElems, uint8_t index,
                  bool wb, bool replicate = false);
};

/**
 * Microops of the form
 * PC   = IntRegA
 * CPSR = IntRegB
 */
class MicroSetPCCPSR : public MicroOp
{
  // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "MicroSetPCCPSR";
    }
  
    std::string uop_data_list() const override
    {
        return "RegIndex ura, RegIndex urb, RegIndex urc";
    }
  
    uint8_t uop_data_bitlen() const override
    {
        return 8 * (sizeof(ura) + sizeof(urb) + sizeof(urc));
    }

    void *uop_data_byte(unsigned int loc) override
    {
        const unsigned int pos = loc % uop_data_bitlen();
        if(pos < sizeof(ura))
            return &ura + (pos/8);
        else if (sizeof(ura) <= pos && pos < sizeof(ura) + sizeof(urb))
            return &urb + ((pos - sizeof(ura))/8);
        else if (sizeof(ura) + sizeof(urb) <= pos && pos < sizeof(ura) + sizeof(urb) + sizeof(urc))
            return &urc + ((pos - sizeof(ura) - sizeof(urb))/8);
        else
            return NULL;
    }

    std::string uop_data_name(unsigned int loc) override
    {
        const unsigned int pos = loc % uop_data_bitlen();
        if(pos < sizeof(ura))
            return "ura";
        else if (sizeof(ura) <= pos && pos < sizeof(ura) + sizeof(urb))
            return "urb";
        else if (sizeof(ura) + sizeof(urb) <= pos && pos < sizeof(ura) + sizeof(urb) + sizeof(urc))
            return "urc";
        else
            return "";
    }

  protected:
    IntRegIndex ura, urb, urc;

    MicroSetPCCPSR(const char *mnem, ExtMachInst machInst, OpClass __opClass,
                   IntRegIndex _ura, IntRegIndex _urb, IntRegIndex _urc)
        : MicroOp(mnem, machInst, __opClass),
          ura(_ura), urb(_urb), urc(_urc)
    {
    }

    std::string generateDisassembly(Addr pc, const SymbolTable *symtab) const;
};

/**
 * Microops of the form IntRegA = IntRegB
 */
class MicroIntMov : public MicroOp
{ 
  // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "MicroIntMov";
    }
  
    std::string uop_data_list() const override
    {
        return "RegIndex ura, RegIndex urb";
    }
  
    uint8_t uop_data_bitlen() const override
    {
        return 8 * (sizeof(ura) + sizeof(urb));
    }

    void *uop_data_byte(unsigned int loc) override
    {
        const unsigned int pos = loc % uop_data_bitlen();
        if(pos < sizeof(ura))
            return &ura + (pos/8);
        else if (sizeof(ura) <= pos && pos < sizeof(ura) + sizeof(urb))
            return &urb + ((pos - sizeof(ura))/8);
        else
            return NULL;

    }

    std::string uop_data_name(unsigned int loc) override
    {
        const unsigned int pos = loc % uop_data_bitlen();
        if(pos < sizeof(ura))
            return "ura";
        else if (sizeof(ura) <= pos && pos < sizeof(ura) + sizeof(urb))
            return "urb";
        else
            return "";
    }

  protected:
    RegIndex ura, urb;

    MicroIntMov(const char *mnem, ExtMachInst machInst, OpClass __opClass,
               RegIndex _ura, RegIndex _urb)
            : MicroOp(mnem, machInst, __opClass),
              ura(_ura), urb(_urb)
    {
    }

    std::string generateDisassembly(Addr pc, const SymbolTable *symtab) const;
};

/**
 * Microops of the form IntRegA = IntRegB op Imm
 */
class MicroIntImmOp : public MicroOp
{
    // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "MicroIntImmOp";
    }
  
    std::string uop_data_list() const override
    {
        return "RegIndex ura, RegIndex urb, int32 imm";
    }
  
    uint8_t uop_data_bitlen() const override
    {
        return 8 * (sizeof(ura) + sizeof(urb) + sizeof(imm));
    }

    void *uop_data_byte(unsigned int loc) override
    {
        const unsigned int pos = loc % uop_data_bitlen();
        if(pos < sizeof(ura))
            return &ura + (pos/8);
        else if (sizeof(ura) <= pos && pos < sizeof(ura) + sizeof(urb))
            return &urb + ((pos - sizeof(ura))/8);
        else if (sizeof(ura) + sizeof(urb) <= pos && pos < sizeof(ura) + sizeof(urb) + sizeof(imm))
            return &imm + ((pos - sizeof(ura) - sizeof(urb))/8);
        else
            return NULL;
    }

    std::string uop_data_name(unsigned int loc) override
    {
        const unsigned int pos = loc % uop_data_bitlen();
        if(pos < sizeof(ura))
            return "ura";
        else if (sizeof(ura) <= pos && pos < sizeof(ura) + sizeof(urb))
            return "urb";
        else if (sizeof(ura) + sizeof(urb) <= pos && pos < sizeof(ura) + sizeof(urb) + sizeof(imm))
            return "imm";
        else
            return "";
    }

  protected:
    RegIndex ura, urb;
    int32_t imm;

    MicroIntImmOp(const char *mnem, ExtMachInst machInst, OpClass __opClass,
                  RegIndex _ura, RegIndex _urb, int32_t _imm)
            : MicroOp(mnem, machInst, __opClass),
              ura(_ura), urb(_urb), imm(_imm)
    {
    }

    std::string generateDisassembly(Addr pc, const SymbolTable *symtab) const;
};

class MicroIntImmXOp : public MicroOpX
{
   // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "MicroOpX";
    }

 protected:
    RegIndex ura, urb;
    int64_t imm;

    MicroIntImmXOp(const char *mnem, ExtMachInst machInst, OpClass __opClass,
                   RegIndex _ura, RegIndex _urb, int64_t _imm)
            : MicroOpX(mnem, machInst, __opClass),
              ura(_ura), urb(_urb), imm(_imm)
    {
    }

    std::string generateDisassembly(Addr pc, const SymbolTable *symtab) const;
};

/**
 * Microops of the form IntRegA = IntRegB op IntRegC
 */
class MicroIntOp : public MicroOp
{
  // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "MicroIntOp";
    }
  
    std::string uop_data_list() const override
    {
        return "RegIndex ura, RegIndex urb, RegIndex urc";
    }
  
    uint8_t uop_data_bitlen() const override
    {
        return 8 * (sizeof(ura) + sizeof(urb) + sizeof(urc));
    }

    void *uop_data_byte(unsigned int loc) override
    {
        const unsigned int pos = loc % uop_data_bitlen();
        if(pos < sizeof(ura))
            return &ura + (pos/8);
        else if (sizeof(ura) <= pos && pos < sizeof(ura) + sizeof(urb))
            return &urb + ((pos - sizeof(ura))/8);
        else if (sizeof(ura) + sizeof(urb) <= pos && pos < sizeof(ura) + sizeof(urb) + sizeof(urc))
            return &urc + ((pos - sizeof(ura) - sizeof(urb))/8);
        else
            return NULL;
    }

    std::string uop_data_name(unsigned int loc) override
    {
        const unsigned int pos = loc % uop_data_bitlen();
        if(pos < sizeof(ura))
            return "ura";
        else if (sizeof(ura) <= pos && pos < sizeof(ura) + sizeof(urb))
            return "urb";
        else if (sizeof(ura) + sizeof(urb) <= pos && pos < sizeof(ura) + sizeof(urb) + sizeof(urc))
            return "urc";
        else
            return "";
    }

  protected:
    RegIndex ura, urb, urc;

    MicroIntOp(const char *mnem, ExtMachInst machInst, OpClass __opClass,
               RegIndex _ura, RegIndex _urb, RegIndex _urc)
            : MicroOp(mnem, machInst, __opClass),
              ura(_ura), urb(_urb), urc(_urc)
    {
    }

    std::string generateDisassembly(Addr pc, const SymbolTable *symtab) const;
};

class MicroIntRegXOp : public MicroOp
{
  // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "MicroIntRegXOp";
    }
  
    std::string uop_data_list() const override
    {
        return "RegIndex ura, RegIndex urb, RegIndex urc, ArmExtendType type, uint32 shiftAmt";
    }
  
    uint8_t uop_data_bitlen() const override
    {
        return 8 * (sizeof(ura) + sizeof(urb) + sizeof(urc) + sizeof(type) + sizeof(shiftAmt));
    }

    void *uop_data_byte(unsigned int loc) override
    {
        const unsigned int pos = loc % uop_data_bitlen();
        if(pos < sizeof(ura))
            return &ura + (pos/8);
        else if (sizeof(ura) <= pos && pos < sizeof(ura) + sizeof(urb))
            return &urb + ((pos - sizeof(ura))/8);
        else if (sizeof(ura) + sizeof(urb) <= pos && pos < sizeof(ura) + sizeof(urb) + sizeof(urc))
            return &urc + ((pos - sizeof(ura) - sizeof(urb))/8);
        else if (sizeof(ura) + sizeof(urb) + sizeof(urc) <= pos && pos < sizeof(ura) + sizeof(urb) + sizeof(urc) + sizeof(type))
            return &type + ((pos - sizeof(ura) - sizeof(urb) - sizeof(urc))/8);
        else if (sizeof(ura) + sizeof(urb) + sizeof(urc) + sizeof(type) <= pos && pos < sizeof(ura) + sizeof(urb) + sizeof(urc) + sizeof(type) + sizeof(shiftAmt))
            return &shiftAmt + ((pos - sizeof(ura) - sizeof(urb) - sizeof(urc) - sizeof(type))/8);
        else
            return NULL;
    }

    std::string uop_data_name(unsigned int loc) override
    {
        const unsigned int pos = loc % uop_data_bitlen();
        if(pos < sizeof(ura))
            return "ura";
        else if (sizeof(ura) <= pos && pos < sizeof(ura) + sizeof(urb))
            return "urb";
        else if (sizeof(ura) + sizeof(urb) <= pos && pos < sizeof(ura) + sizeof(urb) + sizeof(urc))
            return "urc";
        else if (sizeof(ura) + sizeof(urb) + sizeof(urc) <= pos && pos < sizeof(ura) + sizeof(urb) + sizeof(urc) + sizeof(type))
            return "type";
        else if (sizeof(ura) + sizeof(urb) + sizeof(urc) + sizeof(type) <= pos && pos < sizeof(ura) + sizeof(urb) + sizeof(urc) + sizeof(type) + sizeof(shiftAmt))
            return "shiftAmt";
        else
            return "";
    }

  protected:
    RegIndex ura, urb, urc;
    ArmExtendType type;
    uint32_t shiftAmt;

    MicroIntRegXOp(const char *mnem, ExtMachInst machInst, OpClass __opClass,
                   RegIndex _ura, RegIndex _urb, RegIndex _urc,
                   ArmExtendType _type, uint32_t _shiftAmt)
            : MicroOp(mnem, machInst, __opClass),
              ura(_ura), urb(_urb), urc(_urc),
              type(_type), shiftAmt(_shiftAmt)
    {
    }

    std::string generateDisassembly(Addr pc, const SymbolTable *symtab) const;
};

/**
 * Microops of the form IntRegA = IntRegB op shifted IntRegC
 */
class MicroIntRegOp : public MicroOp
{
  // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "MicroIntRegOp";
    }

    std::string uop_data_list() const override
    {
        return "RegIndex ura, RegIndex urb, RegIndex urc, int32 shiftAmt, ArmShiftType shiftType";
    }
  
    uint8_t uop_data_bitlen() const override
    {
        return 8 * (sizeof(ura) + sizeof(urb) + sizeof(urc) + sizeof(shiftAmt) + sizeof(shiftType));
    }

    void *uop_data_byte(unsigned int loc) override
    {
        const unsigned int pos = loc % uop_data_bitlen();
        if(pos < sizeof(ura))
            return &ura + (pos/8);
        else if (sizeof(ura) <= pos && pos < sizeof(ura) + sizeof(urb))
            return &urb + ((pos - sizeof(ura))/8);
        else if (sizeof(ura) + sizeof(urb) <= pos && pos < sizeof(ura) + sizeof(urb) + sizeof(urc))
            return &urc + ((pos - sizeof(ura) - sizeof(urb))/8);
        else if (sizeof(ura) + sizeof(urb) + sizeof(urc) <= pos && pos < sizeof(ura) + sizeof(urb) + sizeof(urc) + sizeof(shiftAmt))
            return &shiftAmt + ((pos - sizeof(ura) - sizeof(urb) - sizeof(urc))/8);
        else if (sizeof(ura) + sizeof(urb) + sizeof(urc) + sizeof(shiftAmt) <= pos && pos < sizeof(ura) + sizeof(urb) + sizeof(urc) + sizeof(shiftAmt) + sizeof(shiftType))
            return &shiftType + ((pos - sizeof(ura) - sizeof(urb) - sizeof(urc) - sizeof(shiftAmt))/8);
        else
            return NULL;
    }

    std::string uop_data_name(unsigned int loc) override
    {
        const unsigned int pos = loc % uop_data_bitlen();
        if(pos < sizeof(ura))
            return "ura";
        else if (sizeof(ura) <= pos && pos < sizeof(ura) + sizeof(urb))
            return "urb";
        else if (sizeof(ura) + sizeof(urb) <= pos && pos < sizeof(ura) + sizeof(urb) + sizeof(urc))
            return "urc";
        else if (sizeof(ura) + sizeof(urb) + sizeof(urc) <= pos && pos < sizeof(ura) + sizeof(urb) + sizeof(urc) + sizeof(shiftAmt))
            return "shiftAmt";
        else if (sizeof(ura) + sizeof(urb) + sizeof(urc) + sizeof(shiftAmt) <= pos && pos < sizeof(ura) + sizeof(urb) + sizeof(urc) + sizeof(shiftAmt) + sizeof(shiftType))
            return "shiftType";
        else
            return "";
    }

  protected:
    RegIndex ura, urb, urc;
    int32_t shiftAmt;
    ArmShiftType shiftType;

    MicroIntRegOp(const char *mnem, ExtMachInst machInst, OpClass __opClass,
               RegIndex _ura, RegIndex _urb, RegIndex _urc,
               int32_t _shiftAmt, ArmShiftType _shiftType)
            : MicroOp(mnem, machInst, __opClass),
              ura(_ura), urb(_urb), urc(_urc),
              shiftAmt(_shiftAmt), shiftType(_shiftType)
    {
    }
};

/**
 * Memory microops which use IntReg + Imm addressing
 */
class MicroMemOp : public MicroIntImmOp
{
  // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "MicroMemOp";
    }
  
    std::string uop_data_list() const override
    {
        return "flag up, flags memAccessFlags";
    }
  
    uint8_t uop_data_bitlen() const override
    {
        return 8 * (sizeof(up) + sizeof(memAccessFlags));
    }

    void *uop_data_byte(unsigned int loc) override
    {
        const unsigned int pos = loc % uop_data_bitlen();
        if(pos < sizeof(up))
            return &up + (pos/8);
        else if (sizeof(up) <= pos && pos < sizeof(up) + sizeof(memAccessFlags))
            return &memAccessFlags + ((pos - sizeof(up))/8);
        else
            return NULL;

    }

    std::string uop_data_name(unsigned int loc) override
    {
        const unsigned int pos = loc % uop_data_bitlen();
        if(pos < sizeof(ura))
            return "ura";
        else if (sizeof(ura) <= pos && pos < sizeof(ura) + sizeof(urb))
            return "urb";
        else
            return "";
    }
    
  protected:
    bool up;
    unsigned memAccessFlags;

    MicroMemOp(const char *mnem, ExtMachInst machInst, OpClass __opClass,
               RegIndex _ura, RegIndex _urb, bool _up, uint8_t _imm)
            : MicroIntImmOp(mnem, machInst, __opClass, _ura, _urb, _imm),
              up(_up), memAccessFlags(TLB::MustBeOne | TLB::AlignWord)
    {
    }

    std::string generateDisassembly(Addr pc, const SymbolTable *symtab) const;
};

class MicroMemPairOp : public MicroOp
{
  // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "MicroMemPairOp";
    }
  
    std::string uop_data_list() const override
    {
        return "RegIndex dest, RegIndex dest2, RegIndex urb, flag up, int32 imm, flags memAccessFlags";
    }
  
    uint8_t uop_data_bitlen() const override
    {
        return 8 * (sizeof(dest) + sizeof(dest2) + sizeof(urb) + sizeof(up) + sizeof(imm) + sizeof(memAccessFlags));
    }

    void *uop_data_byte(unsigned int loc) override
    {
        const unsigned int pos = loc % uop_data_bitlen();
        if(pos < sizeof(dest))
            return &dest + (pos/8);
        else if (sizeof(dest) <= pos && pos < sizeof(dest) + sizeof(dest2))
            return &dest2 + ((pos - sizeof(dest))/8);
        else if (sizeof(dest) + sizeof(dest2) <= pos && pos < sizeof(dest) + sizeof(dest2) + sizeof(urb))
            return &urb + ((pos - sizeof(dest) - sizeof(dest2))/8);
        else if (sizeof(dest) + sizeof(dest2) + sizeof(urb) <= pos && pos < sizeof(dest) + sizeof(dest2) + sizeof(urb) + sizeof(up))
            return &up + ((pos - sizeof(dest) - sizeof(dest2) - sizeof(urb))/8);
        else if (sizeof(dest) + sizeof(dest2) + sizeof(urb) + sizeof(up) <= pos && pos < sizeof(dest) + sizeof(dest2) + sizeof(urb) + sizeof(up) + sizeof(imm))
            return &imm + ((pos - sizeof(dest) - sizeof(dest2) - sizeof(urb) - sizeof(up))/8);
        else if (sizeof(dest) + sizeof(dest2) + sizeof(urb) + sizeof(up) + sizeof(imm) <= pos && pos < sizeof(dest) + sizeof(dest2) + sizeof(urb) + sizeof(up) + sizeof(imm) + sizeof(memAccessFlags))
            return &memAccessFlags + ((pos - sizeof(dest) - sizeof(dest2) - sizeof(urb) - sizeof(up) - sizeof(imm))/8);
        else
            return NULL;

    }

    std::string uop_data_name(unsigned int loc) override
    {
        const unsigned int pos = loc % uop_data_bitlen();
        if(pos < sizeof(dest))
            return "dest";
        else if (sizeof(dest) <= pos && pos < sizeof(dest) + sizeof(dest2))
            return "dest2";
         else if (sizeof(dest) + sizeof(dest2) <= pos && pos < sizeof(dest) + sizeof(dest2) + sizeof(urb))
            return "urb";
        else if (sizeof(dest) + sizeof(dest2) + sizeof(urb) <= pos && pos < sizeof(dest) + sizeof(dest2) + sizeof(urb) + sizeof(up))
            return "up";
        else if (sizeof(dest) + sizeof(dest2) + sizeof(urb) + sizeof(up) <= pos && pos < sizeof(dest) + sizeof(dest2) + sizeof(urb) + sizeof(up) + sizeof(imm))
            return "imm";
        else if (sizeof(dest) + sizeof(dest2) + sizeof(urb) + sizeof(up) + sizeof(imm) <= pos && pos < sizeof(dest) + sizeof(dest2) + sizeof(urb) + sizeof(up) + sizeof(imm) + sizeof(memAccessFlags))
            return "memAccessFlags";
       else
            return "";
    }

  protected:
    RegIndex dest, dest2, urb;
    bool up;
    int32_t imm;
    unsigned memAccessFlags;

    MicroMemPairOp(const char *mnem, ExtMachInst machInst, OpClass __opClass,
            RegIndex _dreg1, RegIndex _dreg2, RegIndex _base,
            bool _up, uint8_t _imm)
        : MicroOp(mnem, machInst, __opClass),
        dest(_dreg1), dest2(_dreg2), urb(_base), up(_up), imm(_imm),
        memAccessFlags(TLB::MustBeOne | TLB::AlignWord)
    {
    }

    std::string generateDisassembly(Addr pc, const SymbolTable *symtab) const;
};

/**
 * Base class for microcoded integer memory instructions.
 */
class MacroMemOp : public PredMacroOp
{
    // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "MacroMemOp";
    }

 protected:
    MacroMemOp(const char *mnem, ExtMachInst machInst, OpClass __opClass,
               IntRegIndex rn, bool index, bool up, bool user,
               bool writeback, bool load, uint32_t reglist);
};

/**
 * Base class for pair load/store instructions.
 */
class PairMemOp : public PredMacroOp
{
    // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "PairMemOp";
    }

 public:
    enum AddrMode {
        AddrMd_Offset,
        AddrMd_PreIndex,
        AddrMd_PostIndex
    };

  protected:
    PairMemOp(const char *mnem, ExtMachInst machInst, OpClass __opClass,
              uint32_t size, bool fp, bool load, bool noAlloc, bool signExt,
              bool exclusive, bool acrel, int64_t imm, AddrMode mode,
              IntRegIndex rn, IntRegIndex rt, IntRegIndex rt2);
};

class BigFpMemImmOp : public PredMacroOp
{
    // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "BigFpMemImmOp";
    }

 protected:
    BigFpMemImmOp(const char *mnem, ExtMachInst machInst, OpClass __opClass,
                  bool load, IntRegIndex dest, IntRegIndex base, int64_t imm);
};

class BigFpMemPostOp : public PredMacroOp
{
    // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "BigFpMemPostOp";
    }

 protected:
    BigFpMemPostOp(const char *mnem, ExtMachInst machInst, OpClass __opClass,
                   bool load, IntRegIndex dest, IntRegIndex base, int64_t imm);
};

class BigFpMemPreOp : public PredMacroOp
{
    // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "BigFpMemPreOp";
    }

 protected:
    BigFpMemPreOp(const char *mnem, ExtMachInst machInst, OpClass __opClass,
                  bool load, IntRegIndex dest, IntRegIndex base, int64_t imm);
};

class BigFpMemRegOp : public PredMacroOp
{
    // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "BigFpMemRegOp";
    }

 protected:
    BigFpMemRegOp(const char *mnem, ExtMachInst machInst, OpClass __opClass,
                  bool load, IntRegIndex dest, IntRegIndex base,
                  IntRegIndex offset, ArmExtendType type, int64_t imm);
};

class BigFpMemLitOp : public PredMacroOp
{
    // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "BigFpMemLitOp";
    }

 protected:
    BigFpMemLitOp(const char *mnem, ExtMachInst machInst, OpClass __opClass,
                  IntRegIndex dest, int64_t imm);
};

/**
 * Base classes for microcoded integer memory instructions.
 */
class VldMultOp : public PredMacroOp
{
    // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "VldMultOp";
    }

 protected:
    VldMultOp(const char *mnem, ExtMachInst machInst, OpClass __opClass,
              unsigned elems, RegIndex rn, RegIndex vd, unsigned regs,
              unsigned inc, uint32_t size, uint32_t align, RegIndex rm);
};

class VldSingleOp : public PredMacroOp
{
    // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "VldSingleOp";
    }

 protected:
    VldSingleOp(const char *mnem, ExtMachInst machInst, OpClass __opClass,
                bool all, unsigned elems, RegIndex rn, RegIndex vd,
                unsigned regs, unsigned inc, uint32_t size,
                uint32_t align, RegIndex rm, unsigned lane);
};

/**
 * Base class for microcoded integer memory instructions.
 */
class VstMultOp : public PredMacroOp
{
     // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "VstMultOp";
    }

 protected:
    VstMultOp(const char *mnem, ExtMachInst machInst, OpClass __opClass,
              unsigned width, RegIndex rn, RegIndex vd, unsigned regs,
              unsigned inc, uint32_t size, uint32_t align, RegIndex rm);
};

class VstSingleOp : public PredMacroOp
{
    // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "VstSingleOp";
    }

 protected:
    VstSingleOp(const char *mnem, ExtMachInst machInst, OpClass __opClass,
                bool all, unsigned elems, RegIndex rn, RegIndex vd,
                unsigned regs, unsigned inc, uint32_t size,
                uint32_t align, RegIndex rm, unsigned lane);
};

/**
 * Base class for microcoded floating point memory instructions.
 */
class MacroVFPMemOp : public PredMacroOp
{
    // JONGHO
  public:
  
    std::string uop_type() const override
    {
        return "MacroVFPMemOp";
    }

 protected:
    MacroVFPMemOp(const char *mnem, ExtMachInst machInst, OpClass __opClass,
                  IntRegIndex rn, RegIndex vd, bool single, bool up,
                  bool writeback, bool load, uint32_t offset);
};

}

#endif //__ARCH_ARM_INSTS_MACROMEM_HH__
